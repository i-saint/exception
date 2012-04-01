#ifndef enemy_boss2_h
#define enemy_boss2_h

namespace exception {
namespace stage2 {


  class VBoundBlock : public Inherit2(HaveVelocity, ChildEnemy)
  {
  typedef Inherit2(HaveVelocity, ChildEnemy) Super;
  public:
    VBoundBlock(Deserializer& s) : Super(s) {}
    VBoundBlock()
    {
      setBound(box(vector4(2000,1000,500), vector4(-500,-1000,-500)));
    }

    void setParent(gobj_ptr v)
    {
      Super::setParent(v);
      setGroup(0);
    }

    int getFractionCount() { return Super::getFractionCount()/4; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      vector4 pos = getRelativePosition();
      pos.y+=getVel().y;
      pos.z*=0.95f;
      setPosition(pos);
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);

      solid_ptr from = ToSolid(m.getFrom());
      if(!from) {
        return;
      }
      vector4 n = m.getNormal();
      vector4 vel = getVel();
      vector4 accel = getAccel();
      vector4 pos = getRelativePosition();
      if(IsGround(from)) {
        if(n.dot(vel.normal()) < 0.0f) { // 正面衝突 
          pos.y-=(getAccel().normal()*vel.norm()).y;
          setPosition(pos);
          setVel(getVel()*-0.1f);
        }
      }
      else if(IsEnemy(from) && !IsFraction(from)) {
        SendDamageMessage(0, from, 0.2f);
        if(from->getVolume()>getVolume()) {
          SendDestroyMessage(0, this);
        }
      }
    }
  };



  class Burst : public Inherit2(HaveVelocity, Optional)
  {
  typedef Inherit2(HaveVelocity, Optional) Super;
  private:
    int m_frame;
    int m_state;
    float m_power;
    vector4 m_aim;
    enum {
      FALL,
      BURST,
    };

  public:
    Burst(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_state >> m_power >> m_aim;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_state << m_power << m_aim;
    }

  public:
    Burst(const vector4& aim, float power) : m_frame(0), m_state(FALL), m_power(power), m_aim(aim)
    {
      setAccel(vector4(-0.025f, aim.y*0.09f, 0.0f));
    }

    float getDrawPriority() { return 1.1f; }

    void draw()
    {
      if(m_state==FALL) {
        DrawSprite("flare.png",
          getPosition(),
          vector4(38.0f+::sinf(m_frame*34.0f*ist::radian)*6.0f));
      }
    }

    virtual void hit()
    {
      if(m_state==BURST && m_frame%1==0) {
        CubeExplode *e = CubeExplode::Factory::create();
        e->setPosition(getPosition());
        e->accel((m_aim*(-m_power/4.0f))*GenRand());
      }

      cdetector cd;
      point_collision pc;
      pc.setPosition(getPosition());
      pc.setRadius(10.0f);
      gobj_iter& it = GetObjects(pc.getBoundingBox());
      while(it.has_next()) {
        solid_ptr p = ToSolid(it.iterate());
        if(p && cd.detect(pc, p->getCollision())) {
          if(m_state==FALL) {
            if(IsGround(p)) {
              m_state = BURST;
              setVel(vector4());
              setAccel(vector4(0.015f, 0, 0));
            }
          }
          else {
            if(dynamic_cast<VBoundBlock*>(p)) {
              SendCallMessage(0, p, "setVel", m_aim*-m_power);
            }
          }
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      setPosition(getPosition()+getVel());
      hit();

      KillIfOutOfScreen(this, rect(vector2(600, 100)));
    }
  };

  class Burst2 : public Inherit2(HaveVelocity, Optional)
  {
  typedef Inherit2(HaveVelocity, Optional) Super;
  private:
    int m_frame;
    float m_power;
    vector4 m_aim;

  public:
    Burst2(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_power >> m_aim;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_power << m_aim;
    }

  public:
    Burst2(const vector4& aim, float power, const vector4& vel) : m_frame(0), m_power(power), m_aim(aim)
    {
      setVel(vel);
    }

    float getDrawPriority() { return 1.1f; }

    virtual void hit()
    {
      if(m_frame%1==0) {
        CubeExplode *e = CubeExplode::Factory::create();
        e->setPosition(getPosition());
        e->accel((m_aim*(-m_power/4.0f))*GenRand());
      }

      cdetector cd;
      point_collision pc;
      pc.setPosition(getPosition());
      pc.setRadius(10.0f);
      gobj_iter& it = GetObjects(pc.getBoundingBox());
      while(it.has_next()) {
        solid_ptr p = ToSolid(it.iterate());
        if(p && cd.detect(pc, p->getCollision())) {
          if(dynamic_cast<VBoundBlock*>(p)) {
            SendCallMessage(0, p, "setVel", m_aim*-m_power);
          }
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      setPosition(getPosition()+getVel());
      hit();

      vector4 vel = getVel();
      vel*=0.975f;
      setVel(vel);
      if(vel.norm()<0.5f) {
        SendKillMessage(0, this);
      }

      KillIfOutOfScreen(this, rect(vector2(600, 100)));
    }
  };


  class BoundBurstMine : public Mine
  {
  typedef Mine Super;
  private:
    int m_bound;
    bool m_bounded;
    vector4 m_cpos;

  public:
    BoundBurstMine(Deserializer& s) : Super(s)
    {
      s >> m_bound >> m_bounded >> m_cpos;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_bound << m_bounded << m_cpos;
    }

  public:
    BoundBurstMine() : m_bound(0), m_bounded(false)
    {
      setMaxSpeed(10.0f);
      setLife(20.0f);
    }

    void draw()
    {
      float emission = (sinf(float(getFrame()*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission,0,0).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);
    }

    void move()
    {
      m_bounded = false;

      vector4 pos = getRelativePosition();
      pos+=getVel();
      pos.z*=0.95f;
      setPosition(pos);
    }

    void effect()
    {
      if(m_bound!=2) {
        return;
      }

      float ay = getVel().y<0.0f ?-1.0f:1.0f;
      vector4 v[] = {vector4(12.0f,0,0), vector4(-12.0f,0,0)};
      vector4 a[] = {vector4(0,ay,0), vector4(0,ay,0)};
      for(int i=0; i<2; ++i) {
        Burst2 *b = new Burst2(a[i], 7.0f, v[i]);
        b->setPosition(m_cpos);
      }
    }

    void onDamage(DamageMessage& m)
    {
      if(!dynamic_cast<GLaser*>(m.getSource()) &&
         !dynamic_cast<Ray*>(m.getSource()) ) { // 自機の武装無効 
         Super::onDamage(m);
      }
    }

    void onCollide(CollideMessage& m)
    {
      gobj_ptr from = m.getFrom();
      if(IsGround(from) && !m_bounded) {
        ++m_bound;
        m_bounded = true;
        if(m_bound==2) {
          m_cpos = m.getPosition();
          SendDestroyMessage(0, this);
        }
        else {
          vector4 v = getVel();
          setPosition(getRelativePosition()+m.getNormal()*(v.norm()+2.0f));
          v.y = -v.y;
          setVel(v*0.6f);
        }
      }
      else if(dynamic_cast<VBoundBlock*>(from)) {
        SendDestroyMessage(0, from);
      }
    }
  };



  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class ICore : public Inherit3(HaveInvincibleMode, HaveDirection, Enemy)
    {
    typedef Inherit3(HaveInvincibleMode, HaveDirection, Enemy) Super;
    public:
      ICore(Deserializer& s) : Super(s) {}
      ICore() {}
    };

    class CoreParts : public Inherit2(HaveDirection, ChildEnemy)
    {
    typedef Inherit2(HaveDirection, ChildEnemy) Super;
    private:
      ICore *m_core;

    public:
      CoreParts(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      CoreParts(ICore *core) : m_core(core)
      {
        setParent(core);
        setLife(9999);
      }

      float getDeltaDamage()
      {
        if(m_core) {
          return m_core->getDeltaDamage();
        }
        return 0.0f;
      }

      void drawLifeGauge() {}

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_EMISSION, m_core->getEmission().v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f,0.8f).v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f,1.0f).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
      }

      void onCollide(CollideMessage& m)
      {
        if(IsFraction(m.getFrom()) || dynamic_cast<VBoundBlock*>(m.getFrom())) {
          SendDestroyMessage(0, m.getFrom());
        }
      }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(IsFraction(src) || dynamic_cast<MediumBlock*>(src)) {
          SendDamageMessage(m.getFrom(), m_core, m.getDamage()*2.0f);
        }
        else {
          SendDamageMessage(m.getFrom(), m_core, m.getDamage());
        }
      }

      int getFractionCount() { return 0; }
    };

    class Core : public ICore
    {
    typedef ICore Super;
    public:
      Core(Deserializer& s) : Super(s) {}
      Core()
      {
        setLife(getMaxLife());
        setInvincible(true);
      }

      float getMaxLife()
      {
        float l[] = {1500, 1600, 1700, 1800, 1900};
        return l[GetLevel()];
      }

      float getDrawPriority() { return 1.1f; }

      void drawLifeGauge() {}

      void draw()
      {
        if(!isDamaged()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f,0.8f).v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f,1.0f).v);
        }
        else {
          Super::draw();
        }
      }

      void updateEmission()
      {
        vector4 emission = getEmission();
        if(isInvincible()) {
          emission+=(vector4(1.0f, 1.0f, 1.0f)-emission)*0.03f;
        }
        else {
          emission+=(vector4(0.4f, 0.4f, 1.0f)-emission)*0.03f;
        }
        setEmission(emission);
      }

      void updateEmission(bool v)
      {}

      void onCollide(CollideMessage& m)
      {
        if(IsFraction(m.getFrom()) || dynamic_cast<VBoundBlock*>(m.getFrom())) {
          SendDestroyMessage(0, m.getFrom());
        }
      }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(IsFraction(src) || dynamic_cast<MediumBlock*>(src)) {
          SendDamageMessage(m.getFrom(), this, m.getDamage()*2.0f);
        }
        else {
          Super::onDamage(m);
        }
      }

      int getFractionCount() { return 0; }
      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBossDestroyImpact(getPosition());
      }

      void onLifeZero(gobj_ptr from)
      {
        PutSmallExplode(getBox(), getMatrix(), getExplodeCount()/2);
        PutBigImpact(getCenter());
      }
    };

    void putMediumBlock(const vector4& v)
    {
      MediumBlock *e = new MediumBlock();
      e->setPosition(v);
      e->setVel(vector4(GetRand2(), GetRand2(), 0)+vector4(-0.5f, 0, 0));
      e->setAccel(vector4(-0.02f, 0, 0));
      e->setMaxSpeed(3.0f);
    }

    void putLargeBlock(const vector4& v)
    {
      LargeBlock *e = new LargeBlock();
      e->setPosition(v);
      e->setVel((vector4(GetRand2(), GetRand2(), 0.0f)+vector4(-0.5f, 0.0f, 0.0f))*0.8f);
      e->setAccel(vector4(-0.012f, 0.0f, 0.0f));
    }

  private:
    enum {
      APPEAR,
      ACTION1,
      TO_ACTION2,
      ACTION2,
      DESTROY,
    };

    Layer *m_root_layer;
    RotLayer *m_parts_layer1;
    RotLayer *m_parts_layer2;
    Core *m_core;
    CoreParts *m_cparts[12];
    int m_time;
    int m_frame;
    float m_block_y;
    vector4 m_ground_scroll;
    int m_action;

    int m_pattern;
    int m_pframe;

    vector4 m_move;

  public:
    Boss(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_root_layer);
      DeserializeLinkage(s, m_parts_layer1);
      DeserializeLinkage(s, m_parts_layer2);
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_cparts);
      s >> m_time >> m_frame >> m_block_y >> m_ground_scroll >> m_action;
      s >> m_pattern >> m_pframe;
      s >> m_move;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_root_layer);
      SerializeLinkage(s, m_parts_layer1);
      SerializeLinkage(s, m_parts_layer2);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_cparts);
      s << m_time << m_frame << m_block_y << m_ground_scroll << m_action;
      s << m_pattern << m_pframe;
      s << m_move;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_root_layer);
      ReconstructLinkage(m_parts_layer1);
      ReconstructLinkage(m_parts_layer2);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_cparts);
    }

  public:
    Boss() : m_time(60*150), m_frame(0), m_block_y(200.0f), m_action(APPEAR),
      m_pattern(1), m_pframe(0)
    {
      m_ground_scroll.x = -500.0f;

      m_root_layer = new Layer();
      m_root_layer->setPosition(vector4(-600,0,0));

      m_parts_layer1 = new RotLayer();
      m_parts_layer1->setAxis(vector4(0,1,0));
      m_parts_layer1->setRotateSpeed(0.7f);
      m_parts_layer1->setParent(m_root_layer);
      m_parts_layer2 = new RotLayer();
      m_parts_layer2->setAxis(vector4(1,0,0));
      m_parts_layer2->setParent(m_parts_layer1);

      m_core = new Core();
      m_core->setParent(m_root_layer);
      m_core->setBox(box(vector4(40)));

      vector4 dir(1,0,0);
      for(int i=0; i<12; ++i) {
        CoreParts *p = new CoreParts(m_core);
        p->setParent(i%2==0 ? m_parts_layer1 : m_parts_layer2);
        p->setBox(box(vector4(80,25,15), vector4(90,-25,-15)));
        p->setDirection(dir);
        dir = matrix44().rotateY(360.0f/12)*dir;
        m_cparts[i] = p;
      }

      SetCameraMovableArea(vector2(0,100), vector2(0,-100));
      SetGlobalAccel(vector4(0.01f, 0, 0));
    }

    float getDrawPriority() { return 2.0f; }

    void draw()
    {
      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        char buf[16];
        sprintf(buf, "%d", m_time/60+(m_time%60 ? 1 : 0));
        DrawText(sgui::_L(buf), sgui::Point(5,20));
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
      }

      vector2 center(25, 22);
      glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
      DrawRect(center, center+vector2(550, 5));

      static float4 s_col = float4(1,1,1,0.5f);
      s_col+=(float4(1,1,1,0.5f)-s_col)*0.05f;
      if(m_core->isDamaged())    { s_col = float4(1,0,0,0.9f); }
      if(m_core->isInvincible()) { s_col = float4(0.1f,0.1f,1.0f,0.9f); }
      glColor4fv(s_col.v);
      DrawRect(center+vector2(1,1), center+vector2(m_core->getLife()/m_core->getMaxLife()*550.0f, 5)-vector2(1,1));

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }


    void updateBlocks()
    {
      m_ground_scroll+=GetGlobalScroll();
      if(m_ground_scroll.x < -180.0f) {
        m_ground_scroll.x+=180.0f;

        vector4 pos[2] = {
          vector4(1000, m_block_y, 0)+m_ground_scroll,
          vector4(1000,-m_block_y, 0)+m_ground_scroll,
        };
        box b[2] = {
          box(vector4(0, 0, 40), vector4(179, 200, -40)),
          box(vector4(0, 0, 40), vector4(179,-200, -40)),
        };
        for(int i=0; i<2; ++i) {
          LGround *g = new LGround();
          g->setBox(b[i]);
          g->setPosition(pos[i]);
          g->setLightDirection(1);
          for(int j=0; j<3; ++j) {
            VBoundBlock *e = new VBoundBlock();
            e->setParent(g);
            e->setBox(box(vector4(29,30,20), vector4(-29,-30,-20)));
            e->setPosition(vector4(30+60*j, 30*(i==0?-1:1), 0));
            e->setAccel(vector4(0, 0.2f*(i==0?1:-1), 0));
            e->setLife(20.0f);
          }
        }
      }
    }


    void appear()
    {
      if(m_frame>180) {
        int f = m_frame-180;
        float pq = Sin90((1.0f/300)*(f-1));
        float q = Sin90((1.0f/300)*f);
        m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(250, 0, 0)*(q-pq));
      }

      if(m_frame>=480) {
        m_frame = 0;
        m_action = ACTION1;
        m_core->setInvincible(false);
      }
    }

    void action1()
    {
      int f = ++m_pframe;
      if(m_pattern==0) { // 上下ブロックのウェーブ攻撃 
        if(f%90==0 && f<1100) {
          putMediumBlock(vector4(450.0f+GetRand2()*30.0f, GetRand2()*70.0f, 0));
        }

        const int kf[] = {60, 180, 330, 480, 630, 780};
        for(int i=0; i<6; ++i) {
          if(f!=kf[i]) { continue; }
          const float str[] = {4.0f, 4.6f, 5.2f, 5.8f, 6.4f, 7.0f};

          Burst *b = new Burst(vector4(0,i%2==0?1:-1,0), str[i]);
          b->setPosition(m_core->getPosition());
        }

        if(f==1250) {
          ++m_pattern;
          m_pframe = 0;
        }
      }
      else if(m_pattern==1) { // HeavyFighter*4 
        const int kf[] = {60, 360, 660, 960};
        for(int i=0; i<4; ++i) {
          if(f!=kf[i]) { continue; }
          const vector4 pos[] = {
            vector4(-780,-110, 0),
            vector4(-740, 110, 0),
            vector4(-700,-110, 0),
            vector4(-660, 110, 0),
          };

          HeavyFighter *e = new HeavyFighter(new HeavyFighter_Run(pos[i].y<0.0f));
          e->setPosition(pos[i]);
        }

        int kb[] = {90, 390, 690, 990};
        for(int i=0; i<4; ++i) {
          if(f!=kb[i]) { continue; }

          Burst *b = new Burst(vector4(0,i%2==0?-1:1,0), 5.0f);
          b->setPosition(m_core->getPosition());

          for(int j=0; j<5; ++j) {
            Fighter *e = new Fighter(new Fighter_Turn(i%2==0 ? false : true));
            e->setPosition(vector4(-420-(100*j), i%2==0 ? 80 : -80, 0));
            e->setDirection(vector4(1,0,0));
          }
        }

        if(f==1060) {
          m_pattern = 0;
          m_pframe = 0;
        }
      }

      if(m_core->getLife() < m_core->getMaxLife()*0.45f) {
        m_action = TO_ACTION2;
        m_frame = 0;
        m_core->setInvincible(true);
      }
    }

    void to_action2()
    {
      int f = m_frame;

      vector4 ax = m_parts_layer1->getAxis();
      ax+=vector4(0.002f, 0, 0.002f);
      m_parts_layer1->setAxis(ax);
      m_parts_layer2->setRotateSpeed(0.7f/300*f);

      float pq = Sin90((1.0f/300)*(f-1));
      float q = Sin90((1.0f/300)*f);
      m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(50, 0, 0)*(q-pq));

      if(f>=300) {
        m_action = ACTION2;
        m_frame = 0;
        m_core->setInvincible(false);
      }
    }


    void action2()
    {
      int f = m_frame;

      int gf = f%180;
      if(gf==1) {
        m_move.y = std::min<float>(std::max<float>(GetNearestPlayerPosition(getPosition()).y, -140.0f), 140.0f);
        m_move.y = m_move.y-m_root_layer->getRelativePosition().y;
      }
      if(gf<=120) {
        float pq = Cos180I(1.0f/120*(gf-1));
        float q = Cos180I(1.0f/120*gf);
        m_root_layer->setPosition(m_root_layer->getRelativePosition()+m_move*(q-pq));
      }
      if(gf==120) {
        vector4 pos = m_core->getPosition();
        BoundBurstMine *m = new BoundBurstMine();
        m->setPosition(pos);
        m->setVel(vector4(1.5f, 5.0f*(pos.y<0?1:-1), 0,0));
        m->setAccel(vector4(0.01f, 0.12f * (pos.y<0?-1:1), 0));
        m->setGroup(m_core->getGroup());
      }

      if(f%360==330) {
        GravityMine *m = new GravityMine();
        m->setPosition(m_core->getPosition());
        m->setVel(vector4(10.0f, 0, 0));
        m->setGroup(m_core->getGroup());
      }
      if(f%90==0) {
        putMediumBlock(vector4(450.0f+GetRand2()*30.0f, GetRand2()*70.0f, 0));
      }
    }


    void clearEnemy()
    {
      gobj_iter& i = GetAllObjects();
      while(i.has_next()) {
        gobj_ptr p = i.iterate();
        if(enemy_ptr e = ToEnemy(p)) {
          if(e!=m_core && !dynamic_cast<CoreParts*>(e)) {
            SendDestroyMessage(0, e, 1);
          }
        }
      }
    }

    void destroy()
    {
      if(m_frame==1) {
        InvincibleAllPlayers(600);
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60 ? 1 : 0));
        IMusic::FadeOut(6000);
      }

      m_core->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
      if(m_frame%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(m_parts_layer1) {
        m_parts_layer1->setRotateSpeed(m_parts_layer1->getRotateSpeed()*0.995f);
      }
      if(m_parts_layer2) {
        m_parts_layer2->setRotateSpeed(m_parts_layer2->getRotateSpeed()*0.99f);
      }

      if(m_frame==400) {
        SendDestroyMessage(0, m_core);
        SendKillMessage(0, this);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_cparts);
      SweepDeadObject(m_parts_layer1);
      SweepDeadObject(m_parts_layer2);

      ++m_frame;

      vector4 scroll = GetGlobalScroll();
      scroll.x = std::max<float>(-3.0f, scroll.x-0.01f);
      SetGlobalScroll(scroll);

      if(m_core->getLife()<=0.0f && m_action!=DESTROY) {
        m_frame = 0;
        m_action = DESTROY;
      }
      if(m_core->getLife()>0 && !m_core->isInvincible()) {
        if(--m_time==0) {
          SendDestroyMessage(0, this);
        }
      }

      if(m_action!=DESTROY) {
        updateBlocks();
      }

      switch(m_action) {
      case APPEAR:     appear();     break;
      case ACTION1:    action1();    break;
      case TO_ACTION2: to_action2(); break;
      case ACTION2:    action2();    break;
      case DESTROY:    destroy();    break;
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      // デフォルトの挙動握り潰し 
      if(m_core) {
        m_core->setLife(0);
      }
    }
  };


}
}
#endif

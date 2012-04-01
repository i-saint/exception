#ifndef enemy_boss5_h
#define enemy_boss5_h

namespace exception {
namespace stage5 {




  class Pendulum : public Inherit4(HaveSpinAttrib2, HaveVelocity, PolyLine, Ground)
  {
  typedef Inherit4(HaveSpinAttrib2, HaveVelocity, PolyLine, Ground) Super;
  private:
    float m_opa;
    int m_frame;
    vector4 m_emission;
    vector4 m_move;
    vector4 m_ppos;
    int m_action;

  public:
    Pendulum() : m_opa(0), m_frame(0), m_action(0)
    {
      setTexture("rray.png");

      setBox(box(vector4(25)));
      setRotateSpeed(1.2f);
      setAccelResist(1.5f);
      setSize(20);
      setWidth(150.0f);
      enableCollision(false);
    }

    Pendulum(Deserializer& s) : Super(s)
    {
      setTexture("rray.png");
      s >> m_opa >> m_frame >> m_emission >> m_move >> m_ppos >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_opa << m_frame << m_emission << m_move << m_ppos << m_action;
    }

    void setTarget(const vector4& v)
    {
      m_frame = 0;
      m_move = v-getPosition();
    }

    float getDrawPriority() { return 9.0f; }

    void draw()
    {
      {
        ist::MatrixSaver msaver(GL_MODELVIEW_MATRIX);
        glMultMatrixf(this->getMatrix().v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 0.8f, m_opa).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        drawModel();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 0.8f).v);
      }

      drawPolyline();
    }

    void start()
    {
      int f = ++m_frame;
      vector4 move;
      int movetime = int(m_move.norm()/3.3f);
      if(f<=movetime) {
        float pq = Sin90(1.0f/movetime*(f-1));
        float q = Sin90(1.0f/movetime*f);
        move = m_move*(q-pq);
      }
      vector4 pos = getRelativePosition()+getVel()+move;
      setPosition(pos);
      m_emission.x = std::min<float>(0.1f+(pos-m_ppos).norm()*0.2f, 1.0f);
      m_ppos = pos;
      if(f>=movetime+40) {
        ++m_action;
      }
    }

    void move()
    {
      int f = ++m_frame;
      vector4 move;
      int movetime = int(m_move.norm()/5.0f);
      if(f<=movetime) {
        float pq = Cos180I(1.0f/movetime*(f-1));
        float q = Cos180I(1.0f/movetime*f);
        move = m_move*(q-pq);
      }
      setPosition(getRelativePosition()+getVel()+move);
      vector4 pos = getPosition();
      m_emission.x = std::min<float>(0.1f+(pos-m_ppos).norm()*0.15f, 1.0f);
      m_ppos = pos;
      if(f>=movetime+40) {
        vector4 ppos = GetNearestPlayerPosition(getPosition());
        vector4 dir = ppos-getPosition();
        setTarget(ppos+(dir.normal()*(100.0+dir.norm()*0.5f)));
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      switch(m_action) {
      case 0: start(); break;
      case 1: move();  break;
      }

      setVel(getVel()*0.98f);
      m_opa+=(1.0f-m_opa)*0.015f;
      if(m_opa>0.8f) {
        enableCollision(true);
      }

      KillIfOutOfScreen(this, rect(vector2(300, 300)));
    }

    void onCollide(CollideMessage& m)
    {
      float speed = getVel().norm();
      gobj_ptr from = m.getFrom();
      if(IsFraction(from)) {
        SendDestroyMessage(this, from);
      }
      else if(m_emission.x>0.7f && IsEnemy(from)) {
        SendDamageMessage(this, from, 5.0f, this);
      }
      else if(dynamic_cast<MoveGround*>(from)) {
        SendDestroyMessage(0, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }
  };



  class LaserBar : public Inherit3(HaveSpinAttrib, HaveVelocity, Ground)
  {
  typedef Inherit3(HaveSpinAttrib, HaveVelocity, Ground) Super;
  private:
    static const int s_num_parts = 2;
    LaserAmp *m_amp[s_num_parts];
    player_ptr m_blower;
    vector4 m_move;
    int m_frame;
    float m_rot;
    int m_action;

  public:
    LaserBar() : m_blower(0), m_frame(0), m_rot(0.0f), m_action(0)
    {
      setAccelResist(0.9f);
      setAxis(vector4(0,0,1));
      setBox(box(vector4(12.5)));
      setBound(box(vector4(800)));
      setMaxSpeed(1.5f);
      m_rot = GetRand2()*360.0f;

      vector4 dir(1,0,0,0);
      for(int i=0; i<s_num_parts; ++i) {
        LaserAmp *l = new LaserAmp();
        l->setParent(this);
        l->setWait(0);
        l->setBox(box(vector4(10,10,10), vector4(20,-10,-10)));
        l->setDirection(dir);
        m_amp[i] = l;
        dir = matrix44().rotateZ(360.0f/s_num_parts)*dir;
      }
    }

    LaserBar(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_amp);
      DeserializeLinkage(s, m_blower);
      s >> m_move >> m_frame >> m_rot >> m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_amp);
      ReconstructLinkage(m_blower);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_amp);
      SerializeLinkage(s, m_blower);
      s << m_move << m_frame << m_rot << m_action;
    }

    player_ptr getBlower() { return m_blower; }

    void setTartget(const vector4& v)
    {
      m_move = v-getPosition();
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_amp, v);
    }

    void draw()
    {
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.8f, 0.0f, 0.0f).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
    }

    void start()
    {
      int f = ++m_frame;
      float pq = Sin90(1.0f/120*(f-1));
      float q = Sin90(1.0f/120*f);
      setPosition(getRelativePosition()+m_move*(q-pq));
      setRotate(getRotate()+m_rot*(q-pq));

      if(f==120) {
        ++m_action;
        m_frame = 0;
        vector4 accel = (GetNearestPlayerPosition(getPosition())-getPosition());
        accel = accel.normal()*0.006f;

        setAccel(accel);
      }
    }

    void fall()
    {
      float rs = getRotateSpeed();
      rs = std::max<float>(std::min<float>(rs+0.004f*(m_rot<0.0f?-1:1), 1.0f), -1.0f);
      setRotateSpeed(rs);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_amp);
      SweepDeadObject(m_blower);

      switch(m_action) {
      case 0: start(); break;
      case 1: fall();  break;
      }
      setPosition(getRelativePosition()+getVel());
      KillIfOutOfScreen(this, rect(vector2(80)));
    }

    void onCollide(CollideMessage& m)
    {
      if(IsGround(m.getFrom())) {
        SendDestroyMessage(0, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      if(m.getStat()==0) {
        PutSmallImpact(getPosition());
      }
    }

    void onAccel(AccelMessage& m)
    {
      Super::onAccel(m);
      if(player_ptr pl=ToPlayer(m.getFrom())) {
        m_blower = pl;
        setGroup(Solid::createGroupID());
      }
    }
  };




  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class ICore : public Inherit3(HaveSpinAttrib2, HaveInvincibleMode, Enemy)
    {
    typedef Inherit3(HaveSpinAttrib2, HaveInvincibleMode, Enemy) Super;
    public:
      ICore() {}
      ICore(Deserializer& s) : Super(s) {}
      virtual float getOpacity()=0;
    };

    class CoreParts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      ICore *m_core;

    public:
      CoreParts(ICore *core) : m_core(core)
      {
        setParent(core);
        setLife(9999);
        enableCollision(false);
      }

      CoreParts(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
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
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,1,m_core->getOpacity()).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_core->getEmission().v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,1.0f).v);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getOpacity()>0.7f) {
          enableCollision(true);
        }
      }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(IsFraction(p)) {
          SendDestroyMessage(0, p);
        }
        else if(dynamic_cast<RedRay*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 20);
        }
        else if(dynamic_cast<Pendulum*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 250);
        }
        else if(dynamic_cast<LaserBar*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 200);
        }
      }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(!dynamic_cast<GLaser*>(src) && // 自機の攻撃と敵レーザー無効 
           !dynamic_cast<Ray*>(src) &&
           !dynamic_cast<Laser*>(src) ) {
          SendDamageMessage(m.getFrom(), m_core, m.getDamage()*0.9f, this);
        }
      }

      int getFractionCount() { return 0; }
    };

    class Core : public ICore
    {
    typedef ICore Super;
    private:
      CoreParts *m_parts[12];
      float m_opa;

    public:
      Core() : m_opa(0.0f)
      {
        setLife(getMaxLife());
        setBox(box(vector4(40)));
        enableCollision(false);

        box b[12] = {
          box(vector4( 45, 70, 70), vector4(-45, 45, 45)),
          box(vector4( 45, 70,-70), vector4(-45, 45,-45)),
          box(vector4( 45,-70,-70), vector4(-45,-45,-45)),
          box(vector4( 45,-70, 70), vector4(-45,-45, 45)),

          box(vector4( 70, 45, 70), vector4( 45,-45, 45)),
          box(vector4( 70, 45,-70), vector4( 45,-45,-45)),
          box(vector4(-70, 45,-70), vector4(-45,-45,-45)),
          box(vector4(-70, 45, 70), vector4(-45,-45, 45)),

          box(vector4( 70, 70, 45), vector4( 45, 45,-45)),
          box(vector4( 70,-70, 45), vector4( 45,-45,-45)),
          box(vector4(-70,-70, 45), vector4(-45,-45,-45)),
          box(vector4(-70, 70, 45), vector4(-45, 45,-45)),
        };
        for(int i=0; i<12; ++i) {
          CoreParts *e = new CoreParts(this);
          e->setBox(b[i]);
          m_parts[i] = e;
        }
      }

      Core(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_parts);
        s >> m_opa;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_parts);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_parts);
        s << m_opa;
      }

      float getOpacity() { return m_opa; }

      static float getMaxLife()
      {
        float l[] = {4200, 4400, 5000, 5200, 5400};
        return l[GetLevel()];
      }

      float getDrawPriority() { return 1.0f; }
      void drawLifeGauge() {}

      void draw()
      {
        if(!isDamaged()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f,m_opa).v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f).v);
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

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_parts);

        m_opa+=(0.8f-m_opa)*0.005f;
        if(m_opa>0.7f) {
          enableCollision(true);
        }

        if(getLife()<=0) {
          float r = getRotateSpeed();
          r+=(0.0f-r)*0.005f;
          setRotateSpeed(r);
        }
        else {
          float r = getRotateSpeed();
          r+=(0.5f-r)*0.003f;
          setRotateSpeed(r);
        }
      }

      int getFractionCount() { return 0; }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(!dynamic_cast<Ray*>(src) &&
           !dynamic_cast<GLaser*>(src) &&
           !dynamic_cast<Laser*>(src) ) { // 自機の武装と敵レーザー無効 
          Super::onDamage(m);
        }
      }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(IsFraction(p)) {
          SendDestroyMessage(0, p);
        }
        else if(dynamic_cast<RedRay*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 20);
        }
        else if(dynamic_cast<Pendulum*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 250);
        }
        else if(dynamic_cast<LaserBar*>(p)) {
          SendDestroyMessage(0, p);
          SendDamageMessage(p, this, 200);
        }
      }

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


    Fighter* putFighter(const vector4& pos, const vector4& dir)
    {
      Fighter *e = new Fighter(0);
      e->setPosition(pos);
      return e;
    }

    Shell* putShell(const vector4& pos, const vector4& dir)
    {
      Shell *e = new Shell(0);
      e->setPosition(pos);
      return e;
    }

    LaserTurret* putLaserTurret(gobj_ptr p, const vector4& pos, const vector4& dir, int wait=0)
    {
      LaserTurret *e = new LaserTurret(new LaserTurret_Normal(wait));
      e->setParent(p);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    MediumBlock* putMediumBlock(const vector4& pos)
    {
      MediumBlock *e = new MediumBlock();
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setVel(vector4(GetRand2(), GetRand2(), 0)-vector4(0, 1.0f, 0));
      e->setAccel(vector4(0, -0.02f, 0));
      e->setAxis(vector4(GetRand2(), GetRand2(), GetRand2()).normalize());
      e->setMaxSpeed(2.0f);
      e->setLife(40);
      return e;
    }

    LargeBlock* putLargeBlock(const vector4& pos)
    {
      LargeBlock *e = new LargeBlock();
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setVel(vector4(GetRand2(), GetRand2(), 0)-vector4(0, 1.0f, 0));
      e->setAccel(vector4(0, -0.02f, 0));
      e->setAxis(vector4(GetRand2(), GetRand2(), GetRand2()).normalize());
      e->setMaxSpeed(1.5f);
      e->setLife(80);
      return e;
    }


  private:
    enum {
      APPEAR,
      ACTION1,
      TO_ACTION2,
      ACTION2,
      TO_ACTION3,
      ACTION3,
      DESTROY,
    };
    Layer *m_root_layer;
    Core *m_core;
    float4 m_color;
    vector4 m_scroll;
    int m_frame;
    int m_time;
    int m_action;

    static const int s_num_pend = 2;
    Pendulum *m_pend[s_num_pend];
    int m_pend_deadcount[s_num_pend];

    gid m_ground_group;

    static const int s_num_iter = 2;
    WeakIterator *m_iter[s_num_iter];
    int m_iter_deadcount[s_num_iter];
    float m_charge;
    int m_raycount;

  public:
    Boss() : m_frame(0), m_time(60*240), m_action(APPEAR)
    {
      m_root_layer = new Layer();
      m_root_layer->setPosition(vector4(0,-140, 0));

      m_core = new Core();
      m_core->setParent(m_root_layer);
      m_core->setInvincible(false);

      m_color = float4(1,1,1,0.5f);

      for(int i=0; i<s_num_pend; ++i) {
        m_pend[i] = 0;
        m_pend_deadcount[i] = 300+20*i;
      }

      m_ground_group = Solid::createGroupID();

      for(int i=0; i<s_num_iter; ++i) {
        m_iter[i] = 0;
        m_iter_deadcount[i] = 120+240*i;
      }
      m_charge = 0.0f;
      m_raycount = 0;
    }

    Boss(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_root_layer);
      DeserializeLinkage(s, m_core);
      s >> m_scroll >> m_frame >> m_time >> m_action;

      DeserializeLinkage(s, m_pend);
      s >> m_pend_deadcount;

      s >> m_ground_group;

      DeserializeLinkage(s, m_iter);
      s >> m_iter_deadcount >> m_charge >> m_raycount;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_root_layer);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_pend);
      ReconstructLinkage(m_iter);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_root_layer);
      SerializeLinkage(s, m_core);
      s << m_scroll << m_frame << m_time << m_action;

      SerializeLinkage(s, m_pend);
      s << m_pend_deadcount;

      s << m_ground_group;

      SerializeLinkage(s, m_iter);
      s << m_iter_deadcount << m_charge << m_raycount;
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

      m_color+=(float4(1,1,1,0.5f)-m_color)*0.05f;
      if(m_core->isDamaged())    { m_color = float4(1,0,0,0.9f); }
      if(m_core->isInvincible()) { m_color = float4(0.1f,0.1f,1.0f,0.9f); }
      glColor4fv(m_color.v);
      DrawRect(center+vector2(1,1), center+vector2(m_core->getLife()/m_core->getMaxLife()*550.0f, 5)-vector2(1,1));

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }


    void appear()
    {
      int f = m_frame;

      if(f>180 && f<=360) {
        int af = f-180;
        float pq = Cos180I(1.0f/180*(af-1));
        float q = Cos180I(1.0f/180*af);
        m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0, 340, 0)*(q-pq));
      }
      if(f==360) {
        m_frame = 0;
        m_action = ACTION1;
        SetCameraMovableArea(vector2(50, 50), vector2(-50, -50));
      }
    }

    void action1()
    {
      int f = m_frame;

      {
        static const vector4 pos[s_num_pend] = {
          vector4( 80, 150, 0),
          vector4(-80, 150, 0),
        };
        static const vector4 dir[s_num_pend] = {
          vector4( 1.0f,-1.0f,0),
          vector4(-1.0f,-1.0f,0),
        };
        for(int i=0; i<s_num_pend; ++i) {
          if(m_pend[i] && m_pend[i]->isDead()) {
            m_pend[i] = 0;
            m_pend_deadcount[i] = 20*(i+1);
          }
          else if(!m_pend[i]) {
            if(--m_pend_deadcount[i]<=0) {
              Pendulum *e = new Pendulum();
              e->setPosition(pos[i]);
              e->setTarget(pos[i]+dir[i].normal()*300);
              m_pend[i] = e;
            }
          }
        }
      }

      int ff = std::max<int>(f-300, 0)%1700;
      {
        static const int kf[] = {
          300, 330, 360, 390,
          600, 630, 660, 690,
          1000,1030,
          1180,1210,
          1360,1390,
        };
        for(int i=0; i<14; ++i) {
          if(ff!=kf[i]) { continue; }
          static const vector4 pos[] = {
            vector4(  70,-400,0), vector4( 150,-400,0), vector4( 230,-350,0), vector4( 310,-350,0),
            vector4( -70,-400,0), vector4(-150,-400,0), vector4(-230,-350,0), vector4(-310,-350,0),
            vector4(  50,-430,0), vector4( -50,-430,0),
            vector4( 150,-415,0), vector4(-150,-415,0),
            vector4( 250,-430,0), vector4(-250,-430,0),
          };

          if(i<8) {
            Fighter_Rush *c = new Fighter_Rush(false);
            c->setLength(200.0f);

            Fighter *e = new Fighter(c);
            e->setPosition(pos[i]);
            e->setDirection(vector4(0,1,0));
          }
          else if(i<10) {
            Shell_BurstMissile *c = new Shell_BurstMissile(false);
            c->setLength(200.0f);

            Shell *e = new Shell(c);
            e->setPosition(pos[i]);
            e->setDirection(vector4(0,1,0));
          }
          else if(i<12) {
            Shell_GravityMissile *c = new Shell_GravityMissile(false);
            c->setLength(200.0f);

            Shell *e = new Shell(c);
            e->setPosition(pos[i]);
            e->setDirection(vector4(0,1,0));
          }
          else {
            Egg_Laser *c = new Egg_Laser(false);
            c->setLength(200.0f);

            Egg *e = new Egg(c);
            e->setPosition(pos[i]);
            e->setDirection(vector4(0,1,0));
          }
        }
      }

      if(f%120==0) {
        putLargeBlock(vector4(0, 450, 0)+vector4(GetRand2()*250.0f, GetRand2()*50.0f, 0));
      }
      if(f%80==0) {
        putMediumBlock(vector4(0, 450, 0)+vector4(GetRand2()*300.0f, GetRand2()*50.0f, 0));
      }

      if(m_core->getLife() < m_core->getMaxLife()*0.66f) {
        m_frame = 0;
        m_core->setInvincible(true);
        m_action = TO_ACTION2;
      }
    }

    void to_action2()
    {
      int f = m_frame;
      float pq = Cos180I(1.0f/360*(f-1));
      float q = Cos180I(1.0f/360*f);
      m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0,-50,0)*(q-pq));
      if(m_frame==360) {
        m_frame = 0;
        m_core->setInvincible(false);
        m_action = ACTION2;
        SetGlobalAccel(vector4(0, 0.02f, 0));
      }
    }

    void action2()
    {
      int f = m_frame;

      if(f%40==29) {
        static const vector4 pos[] = {
          vector4( 330, 500, 0),
          vector4(-330, 500, 0),
          vector4( 380, 500, 0),
          vector4(-380, 500, 0),
          vector4( 420, 500, 0),
          vector4(-420, 500, 0),
        };
        for(int i=0; i<6; ++i) {
          MoveGround *g = new MoveGround();
          g->setBox(box(vector4(30, 100, 30), vector4(-30,-100,-30)));
          g->setBound(box(vector4(1000, 600, 1000)));
          g->setGroup(m_ground_group);
          g->setAccelResist(0.0f);

          vector4 p = pos[i]+vector4(GetRand2()*40.0f, GetRand2()*50.0f, GetRand2()*20.0f);
          float x = fabsf(p.x);
          g->setVel(vector4(0,-0.5f-x/150.0f, 0));
          g->setPosition(p);
        }
      }

      if(f>360 && f%360==0) {
        MoveGround *g = new MoveGround();
        g->setBox(box(vector4(30, 100, 30), vector4(-30,-100,-30)));
        g->setBound(box(vector4(1000, 600, 1000)));
        g->setVel(vector4(0,-1.5f, 0));
        g->setAccelResist(0.0f);
        if(f/360%2==0) {
          g->setPosition(vector4( 260, 450, 0)+vector4(GetRand2()*10.0f, GetRand2()*50.0f, GetRand2()*20.0f));
          putLaserTurret(g, vector4(-30,20,0), vector4(-1,0,0), 0);
        }
        else {
          g->setPosition(vector4(-260, 450, 0)+vector4(GetRand2()*10.0f, GetRand2()*50.0f, GetRand2()*20.0f));
          putLaserTurret(g, vector4(30,-10,0), vector4(1,0,0), 0);
        }
      }

      if(f%160==0) {
        LaserBar *e = new LaserBar();
        e->setPosition(m_core->getPosition());
        e->setTartget(vector4(0, 0, 0)+vector4(GetRand2()*150.0f, GetRand2()*10.0f, 0));
        e->setGroup(m_core->getGroup());
      }

      if(f%70==0) {
        if(f/70%2==0) {
          vector4 base(-150, -350, 0);
          for(int i=0; i<3; ++i) {
            PutStraightZab(base, vector4(0,1,0), false);
            base+=vector4(150,0,0);
          }
        }
        else {
          vector4 base(-180, -350, 0);
          for(int i=0; i<4; ++i) {
            PutStraightZab(base, vector4(0,1,0), false);
            base+=vector4(120,0,0);
          }
        }
      }

      if(m_core->getLife() < m_core->getMaxLife()*0.30f) {
        m_frame = 0;
        m_core->setInvincible(true);
        m_action = TO_ACTION3;

        gobj_iter& i = GetAllObjects();
        while(i.has_next()) {
          gobj_ptr p = i.iterate();
          if(dynamic_cast<LaserBar*>(p) || dynamic_cast<LaserTurret*>(p)) {
            SendDestroyMessage(0,p,1);
          }
        }
      }
    }

    void to_action3()
    {
      int f = m_frame;
      float pq = Cos180I(1.0f/360*(f-1));
      float q = Cos180I(1.0f/360*f);
      m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0,50,0)*(q-pq));
      if(f==360) {
        m_frame = 0;
        m_core->setInvincible(false);
        m_action = ACTION3;
        SetGlobalAccel(vector4(0,0,0));
      }
    }


    void action3()
    {
      int f = m_frame;
      {
        vector4 pos[s_num_iter] = {
          vector4( 120,30, 0),
          vector4(-120,30, 0),
        };
        vector4 dir[s_num_iter] = {
          vector4(-1,-0.0f,0),
          vector4( 1,-0.0f,0),
        };
        for(int i=0; i<s_num_iter; ++i) {
          if(m_iter[i] && m_iter[i]->isDead()) {
            m_iter[i] = 0;
            m_iter_deadcount[i] = 90;
          }
          else if(!m_iter[i]) {
            if(--m_iter_deadcount[i]<=0) {
              WeakIterator *e = new WeakIterator(new WeakIterator_Defense());
              e->setPosition(pos[i]-dir[i].normal()*500);
              e->setDirection(dir[i]);
              e->setGroup(m_core->getGroup());
              if(i==1) {
                e->invertRotation();
              }
              m_iter[i] = e;
            }
          }
        }
      }

      m_charge+=1.0f+std::min<float>(1.0f, float(f)/1500);
      if(m_charge>=150.0f) {
        m_charge-=150.0f;
        ++m_raycount;
        static const vector4 pos[2] = { vector4(250, 150, 0), vector4(-250, 150, 0)};
        RedRayBit::create(m_core, m_core->getPosition(), pos[m_raycount%2], 150);
      }

      if(f%300==0) {
        for(int i=0; i<3; ++i) {
          vector4 pos = GetNearestPlayerPosition(getPosition()) + (matrix44().rotateZ(GetRand()*180)*vector4(150+GetRand()*50, 0, 0));
          Zab *e = PutZab(pos, false);
          e->setGroup(m_core->getGroup());
        }
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
        else if(ground_ptr g = ToGround(p)) {
          SendDestroyMessage(0, g, 1);
        }
      }
    }

    void destroy()
    {
      int f = m_frame;
      if(f==1) {
        InvincibleAllPlayers(480);
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60 ? 1 : 0));
        IMusic::FadeOut(6000);
        SetCameraMovableArea(vector2(0, 0), vector2(0,-0));
      }

      float pq = Cos180I(1.0f/480*(f-1));
      float q = Cos180I(1.0f/480*f);
      m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0,-340, 0)*(q-pq));
      m_core->setEmission(vector4(1.2f/480*f, 0.5f/480*f, 0));
      if(f%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(f==480) {
        if(Background *bg = stage5::Scene::getBackground()) {
          bg->destroy();
        }
        SendDestroyMessage(0, m_core);
        SendKillMessage(0, this);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;

      if(m_core->getLife()<=0.0f && m_action!=DESTROY) {
        m_frame = 0;
        m_action = DESTROY;
      }
      if(m_core->getLife()>0 && !m_core->isInvincible()) {
        if(--m_time==0) {
          SendDestroyMessage(0, this);
        }
      }

      if(Background *bg = stage5::Scene::getBackground()) {
        float ml = m_core->getMaxLife();
        float l = m_core->getLife();
        bg->setBaseColor(vector4(1,1,1)+vector4(fabsf(l-ml)/ml, 0, l/ml*1.2f));
      }

      switch(m_action) {
      case APPEAR:     appear();     break;
      case ACTION1:    action1();    break;
      case TO_ACTION2: to_action2(); break;
      case ACTION2:    action2();    break;
      case TO_ACTION3: to_action3(); break;
      case ACTION3:    action3();    break;
      case DESTROY:    destroy();    break;
      }
      SweepDeadObject(m_pend);
      SweepDeadObject(m_iter);
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

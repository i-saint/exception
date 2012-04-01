#ifndef enemy_boss1_h
#define enemy_boss1_h

namespace exception {
namespace stage1 {



  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class GeneratorCollision : public BreakablePointCollision
    {
    typedef BreakablePointCollision Super;
    private:
      bool m_invincible;

    public:
      GeneratorCollision(Deserializer& s) : Super(s)
      {
        s >> m_invincible;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_invincible;
      }

    public:
      GeneratorCollision(gobj_ptr p) : m_invincible(false)
      {
        Register(this);

        setParent(p);
        setLife(getMaxLife());
        setRadius(15);
      }

      float getMaxLife() { return 100.0f; }
      void setInvincible(bool v) { m_invincible=v; }
      bool isInvincible() { return m_invincible; }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);

        PutSmallExplode(getPosition(), 10);
        PutSmallImpact(getPosition());
      }

      void onDamage(DamageMessage& m)
      {
        if(!m_invincible) {
          Super::onDamage(m);
        }
      }

      void onCollide(CollideMessage& m)
      {
        Super::onCollide(m);

        if(solid_ptr p = ToSolid(m.getFrom())) {
          if(IsFraction(p)) {
            SendDestroyMessage(0, p);
          }
          else if(typeid(*p)==typeid(MediumBlock&)) {
            SendDestroyMessage(0, p);
            SendDamageMessage(0, this, 8.0f);
          }
          else if(typeid(*p)==typeid(LargeBlock&)) {
            SendDestroyMessage(0, p);
            SendDamageMessage(0, this, 15.0f);
          }
        }
      }
    };

    class Generator : public Inherit4(SpinningCube, HaveParent, HavePosition, IEffect)
    {
    typedef Inherit4(SpinningCube, HaveParent, HavePosition, IEffect) Super;
    private:
      GeneratorCollision *m_col;
      vector4 m_emission;
      int m_inv_time;

    public:
      Generator(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_col);
        s >> m_emission >> m_inv_time;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_col);
        s << m_emission << m_inv_time;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_col);
      }

    public:
      Generator() : m_col(0), m_emission(-1.0f, -1.0f, -1.0f), m_inv_time(0)
      {
        Register(this);
      }

      GeneratorCollision* getCollision() { return m_col; }
      bool isInvincible() { return m_col && m_col->isInvincible(); }
      bool isActive()     { return m_col && !m_col->isDead(); }

      void activate()
      {
        if(!m_col || m_col->isDead()) {
          m_col = new GeneratorCollision(this);
        }
      }

      void invincible(int time=240)
      {
        activate();
        m_col->setInvincible(true);
        m_inv_time = time;
      }

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_col && m_col->isDead()) {
          m_col = 0;
          m_emission = vector4(0.0f, -1.0f, -1.0f);
        }

        if(isActive()) {
          if(m_col->isInvincible()) {
            m_emission+=(vector4(0.4f, 0.4f, 0.5f)-m_emission)*0.02f;
            if(--m_inv_time<=0) {
              m_col->setInvincible(false);
            }
          }
          else {
            m_emission+=(vector4(0.7f, 0.7f, 0.0f)-m_emission)*0.02f;
            if(m_col->isDamaged()) {
              m_emission = vector4(1.0f, 0.0f, 0.0f);
            }
          }
        }
        else {
          m_emission+=(vector4(-0.2f, -1.0f, -1.0f)-m_emission)*0.02f;
        }
      }
    };


    class WallGuard : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      float m_height;

    public:
      WallGuard(Deserializer& s) : Super(s)
      {
        s >> m_height;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_height;
      }

    public:
      WallGuard() : m_height(1.0f)
      {
        setLife(60.0f);
        setHeight(1.0f);
      }

      float getHeight() { return m_height; }
      void setHeight(float v)
      {
        m_height = v;
        setBox(box(vector4(10.0f, 15.0f+m_height, 10.0f), vector4(-10.0f, 15.0f, -10.0f)));
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        float h = getHeight();
        if(h < 130.0f) {
          setHeight(h+0.5f);
        }
      }

      int getFractionCount() { return 0; }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(p && (typeid(*p)==typeid(MediumBlock&) || typeid(*p)==typeid(LargeBlock&))) {
          SendDestroyMessage(0, this);
          SendDamageMessage(0, p, 3.0f);
        }
      }
    };

    class Wall : public Inherit2(HaveDirection, ChildGround)
    {
    typedef Inherit2(HaveDirection, ChildGround) Super;
    private:
      WallGuard *m_child;
      int m_interval;
      gid m_ride;
      bool m_on;
      vector4 m_vel;

    public:
      Wall(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_child);
        s >> m_interval >> m_ride >> m_on >> m_vel;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_child);
        s << m_interval << m_ride << m_on << m_vel;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_child);
      }

    public:
      Wall() : m_child(0), m_interval(0), m_ride(0), m_on(false)
      {
        setBox(box(vector4(15.0f, 15.0f, 12.5f), vector4(-15.0f, 0.0f, -12.5f)));
        setGroup(createGroupID());

        m_child = new WallGuard();
        m_child->setParent(this);
        m_child->setGroup(getGroup());
      }

      void updateMatrix(matrix44& mat)
      {
        mat*=getParentMatrix();
        mat.translate(getRelativePosition().v);
        mat.aimVector2(matrix44().rotateZ(-90)*getDirection());
      }

      void setRideTarget(gid v) { m_ride=v; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_child && m_child->isDead()) {
          m_child = 0;
        }
        else if(!m_child && ++m_interval==100) {
          m_interval = 0;
          m_child = new WallGuard();
          m_child->setParent(this);
          m_child->setGroup(getGroup());
        }

        vector4 pos = getRelativePosition();
        pos+=vector4(-0.4f, 0, 0);

        vector4 dir = getDirection();
        if(!m_on) {
          m_vel-=dir*0.1f;
        }
        else {
          pos+=getDirection();
          m_vel = vector4();
        }
        pos+=m_vel;
        setPosition(pos);
        if(pos.x < -600.0f) {
          SendDestroyMessage(0, this);
        }

        m_on = false;
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutSmallExplode(getPosition(), 2);
      }

      void onCollide(CollideMessage& m)
      {
        Super::onCollide(m);
        if(solid_ptr s=ToSolid(m.getFrom())) {
          if(s->getGroup()==m_ride) {
            m_on = true;
          }
        }
      }
    };


    class Blade : public ChildEnemy
    {
    typedef ChildEnemy Super;
    public:
      Blade(Deserializer& s) : Super(s) {}
      Blade()
      {
        setLife(100.0f);
        setBox(box(vector4(50,50,30)));
      }

      void updateMatrix(matrix44& mat)
      {
        Super::updateMatrix(mat);
        mat.rotateZ(45.0f);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        vector4 pos = getRelativePosition();
        if(pos.y<70.0f) {
          pos.y+=0.25f;
          setPosition(pos);
        }
      }

      int getFractionCount() { return 0; }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(p) {
          if((typeid(*p)==typeid(MediumBlock&))) {
            SendDamageMessage(0, this, 15.0f);
            SendDamageMessage(0, p, 3.0f);
          }
          else if((typeid(*p)==typeid(LargeBlock&))) {
            SendDestroyMessage(0, this);
            SendDamageMessage(0, p, 5.0f);
          }
        }
      }
    };

    class BladeGenerator : public Inherit3(HaveDirection, HaveParent, Optional)
    {
    typedef Inherit3(HaveDirection, HaveParent, Optional) Super;
    private:
      Blade *m_child;
      int m_interval;
      gid m_group;

    public:
      BladeGenerator(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_child);
        s >> m_interval >> m_group;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_child);
        s << m_interval << m_group;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_child);
      }

    public:
      BladeGenerator() : m_child(0), m_interval(0), m_group(0)
      {}

      void updateMatrix(matrix44& mat)
      {
        mat*=getParentMatrix();
        mat.translate(getRelativePosition().v);
        mat.aimVector2(matrix44().rotateZ(-90)*getDirection());
      }

      void setGroup(gid v) { m_group=v; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_child && m_child->isDead()) {
          m_child = 0;
          m_interval = 240;
        }
        else if(!m_child && --m_interval<=0) {
          m_child = new Blade();
          m_child->setParent(this);
          m_child->setGroup(m_group);
        }
      }
    };


    class ArmLayer : public ChildLayer
    {
    typedef ChildLayer Super;
    private:
      ChildLayer *m_layer[2];
      float m_y;

    public:
      ArmLayer(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_layer);
        s >> m_y;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_layer);
        s << m_y;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_layer);
      }

    public:
      ArmLayer() : m_y(0.0f)
      {
        for(int i=0; i<2; ++i) {
          m_layer[i] = new ChildLayer();
          m_layer[i]->setParent(this);
        }
      }

      float getY() { return m_y; }
      void setY(float v)
      {
        m_y = v;
        m_layer[0]->setPosition(vector4(0, m_y, 0));
        m_layer[1]->setPosition(vector4(0,-m_y, 0));
      }

      ChildLayer* getUpper() { return m_layer[0]; }
      ChildLayer* getLower() { return m_layer[1]; }
    };

    class Arm : public ChildGround
    {
    typedef ChildGround Super;
    private:
      vector4 m_emission;

    public:
      Arm(Deserializer& s) : Super(s)
      {
        s >> m_emission;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_emission;
      }

    public:
      Arm()
      {
        setDirection(vector4(1, 0 ,0));
      }

      void setEmission(const vector4& v) { m_emission=v; }

      void draw()
      {
        if(m_emission.x > 0.0f) {
          glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        }
        else {
          Super::draw();
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        float attr = 0.02f;
        m_emission.x = std::max<float>(0, m_emission.x-attr);
        m_emission.y = std::max<float>(0, m_emission.y-attr);
        m_emission.z = std::max<float>(0, m_emission.z-attr);
      }

      int getExplodeCount()
      {
        return int(getVolume()/50000.0f);
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBigImpact(getCenter());
      }
    };


    class Core : public Inherit2(HaveInvincibleMode, ChildEnemy)
    {
    typedef Inherit2(HaveInvincibleMode, ChildEnemy) Super;
    public:
      Core()
      {
        setLife(getMaxLife());
        setBox(box(vector4(80)));
      }

      Core(Deserializer& s) : Super(s) {}

      float getMaxLife()
      {
        float l[] = {1800, 1900, 2000, 2100, 2200};
        return l[GetLevel()];
      }

      void drawLifeGauge() {}
      int getFractionCount() { return 0; }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(IsFraction(p) || dynamic_cast<MediumBlock*>(p) || dynamic_cast<LargeBlock*>(p)) {
          SendDestroyMessage(0, p);
        }
      }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(IsFraction(src) || dynamic_cast<MediumBlock*>(src)) {
          SendDamageMessage(m.getFrom(), this, m.getDamage()*1.5f);
        }
        else {
          Super::onDamage(m);
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


    class BoundLaser : public Inherit2(HavePosition, IBullet)
    {
    typedef Inherit2(HavePosition, IBullet) Super;
    private:
      std::deque<vector4> m_line;
      gobj_ptr m_owner;
      vector4 m_vel;
      int m_col;

    public:
      BoundLaser(gobj_ptr owner) : m_owner(owner), m_col(0)
      {
        Register(this);
      }

      BoundLaser(Deserializer& s) : Super(s)
      {
        ist::deserialize_container(s, m_line);
        gid owner;
        s >> owner >> m_vel >> m_col;
        PushLinkage(owner);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        m_owner = PopLinkage();
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        ist::serialize_container(s, m_line);
        s << ToID(m_owner) << m_vel << m_col;
      }

      gobj_ptr getOwner() { return m_owner; }
      void setVel(const vector4& v) { m_vel=v; }

      void setPosition(const vector4& v)
      {
        Super::setPosition(v);
        if(m_line.empty()) {
          m_line.resize(20, v);
        }
        else {
          m_line.push_front(v);
          m_line.pop_back();
        }
      }

      void drawSprite(const vector4& pos, float scale)
      {
        static IVertexBufferObject *model = GetVBO("plane").get();
        ist::MatrixSaver ms(GL_MODELVIEW_MATRIX);
        matrix44 mat;
        mat.translate(pos.v);
        mat*=GetAimCameraMatrix();
        mat.scale(vector4(scale).v);
        glMultMatrixf(mat.v);
        model->assign();
        model->draw();
        model->disassign();
      }

      void draw()
      {
        texture_ptr tex = GetTexture("flare.png");

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        tex->assign();
        for(size_t i=0; i<m_line.size(); i+=2) {
          float s = 1.0f-(1.0f/20)*i;
          glColor4f(1, 1, 1, s);
          drawSprite(m_line[i], s*10.0f);
        }
        glColor4f(1, 1, 1, 1);
        tex->disassign();

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
      }

      bool hit()
      {
        bool hits = false;
        cdetector cd;
        point_collision c;
        c.setPosition(getPosition());
        c.setRadius(5.0f);

        gobj_iter& it = GetObjects(c.getBoundingBox());
        while(it.has_next()) {
          solid_ptr p = ToSolid(it.iterate());
          if(!p || p==m_owner) {
            continue;
          }
          if(cd.detect(c, p->getCollision())) {
            PutFlash(getPosition(), 40.0f);
            hits = true;

            if(IsFraction(p)) {
              SendDestroyMessage(m_owner, p);
            }
            else if(IsPlayer(p)) {
              SendDamageMessage(m_owner, p, 2.0f, this);
            }
            else if(IsEnemy(p) || IsGround(p)) {
              vector4 n = cd.getNormal();
              n.z = 0;
              n.normalize();
              m_vel = (matrix44().rotateA(n, 180)*m_vel)*-1.0f;
              vector4 pos = getPosition();
              setPosition(pos-cd.getNormal()*3.0f);
            }
            break;
          }
        }
        return hits;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_owner && m_owner->isDead()) {
          m_owner = 0;
        }

        vector4 pos = getPosition();
        setPosition(pos+m_vel);
        if(hit()) {
          m_col++;
          if(m_col > 20) {
            SendDestroyMessage(0, this);
          }
        }
        if(!IsInnerScreen(getPosition(), vector2(20)) || fabs(getPosition().z)>50.0f) {
          SendKillMessage(0, this);
        }
      }
    };


  private:
    Arm *m_arm[6];
    Generator *m_gen[2];
    BladeGenerator *m_blade[5];
    Core *m_core;

    RotLayer *m_root_layer;
    ChildLayer *m_core_layer;
    ArmLayer *m_arm_layer;

    int m_time;
    int m_inv_count;
    int m_frame;
    gid m_group;

    enum {
      APPEAR,
      ACTION1,
      TO_ACTION2,
      ACTION2,
      DESTROY,
    };
    int m_action;

    int m_lasercount;
    vector4 m_laserdir;

    enum {
      NORMAL,
      CLOSE,
      CLOSE_WAIT,
      OPEN,
      OPEN_WAIT,
    };
    int m_arm_action;
    int m_arm_frame;
    float m_arm_speed;
    spline m_arm_motion;

    int m_block_frame;
    int m_block_count;


  public:
    Boss() :
      m_time(60*150), m_inv_count(0), m_frame(0),
      m_lasercount(0), m_group(0), m_action(APPEAR),
      m_arm_action(0), m_arm_frame(0), m_arm_speed(0.0f),
      m_block_frame(0), m_block_count(0)
    {
      m_arm_motion.resize(2);

      SetGlobalScroll(vector4());

      ZeroClear(m_blade);

      m_root_layer = new RotLayer();
      m_root_layer->chain();

      m_core_layer = new ChildLayer();
      m_core_layer->setPosition(vector4(500.0f, 0, 0));
      m_core_layer->setParent(m_root_layer);

      m_arm_layer = new ArmLayer();
      m_arm_layer->setParent(m_core_layer);
      m_arm_layer->setY(500.0f);

      {
        Core *e = new Core();
        e->setParent(m_core_layer);
        e->setPosition(vector4(350,0,0));
        m_core = e;
      }
      {
        vector4 pos[2] = {
          vector4(230, 70, 0),
          vector4(230,-70, 0),
        };
        for(int i=0; i<2; ++i) {
          Generator *e = new Generator();
          e->setParent(m_core_layer);
          e->setPosition(pos[i]);
          m_gen[i] = e;
        }
      }

      m_group = Solid::createGroupID();
      {
        box b[6] = {
          box(vector4(500.0f, 75.0f, 60.0f), vector4(-350.0f, 0.0f, -60.0f)),
          box(vector4(500.0f, 0.0f, 60.0f), vector4(-350.0f, -75.0f, -60.0f)),
          box(vector4(450.0f, 130.0f, 50.0f), vector4(200.0f, -80.0f, -50.0f)),
          box(vector4(450.0f, 80.0f, 50.0f), vector4(200.0f, -130.0f, -50.0f)),
          box(vector4(300.0f, 20.0f, 40.0f), vector4(-700.0f, -60.0f, -40.0f)),
          box(vector4(300.0f, 60.0f, 40.0f), vector4(-700.0f, -20.0f, -40.0f)),
        };
        for(int i=0; i<6; ++i) {
          Arm *a = new Arm();
          a->setParent((i%2) ? m_arm_layer->getLower() : m_arm_layer->getUpper());
          a->setGroup(m_group);
          a->setPosition(vector4());
          a->setBox(b[i]);
          m_arm[i] = a;
        }
      }

      invincible(740);
    }

    Boss(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_arm);
      DeserializeLinkage(s, m_gen);
      DeserializeLinkage(s, m_blade);
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_root_layer);
      DeserializeLinkage(s, m_core_layer);
      DeserializeLinkage(s, m_arm_layer);
      s >> m_time >> m_inv_count >> m_frame >> m_group >> m_action
        >> m_lasercount >> m_laserdir
        >> m_arm_action >> m_arm_frame >> m_arm_speed >> m_arm_motion
        >> m_block_frame >> m_block_count;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_arm);
      ReconstructLinkage(m_gen);
      ReconstructLinkage(m_blade);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_root_layer);
      ReconstructLinkage(m_core_layer);
      ReconstructLinkage(m_arm_layer);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_arm);
      SerializeLinkage(s, m_gen);
      SerializeLinkage(s, m_blade);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_root_layer);
      SerializeLinkage(s, m_core_layer);
      SerializeLinkage(s, m_arm_layer);
      s << m_time << m_inv_count << m_frame << m_group << m_action
        << m_lasercount << m_laserdir
        << m_arm_action << m_arm_frame << m_arm_speed << m_arm_motion
        << m_block_frame << m_block_count;
    }

    float getArmY() { return m_arm_layer->getY(); }
    void setArmY(float v) { m_arm_layer->setY(v); }

    void invincible(int frame)
    {
      m_core->setInvincible(true);

      activateGenerator();
      m_inv_count = 0;
      for(int i=0; i<2; ++i) {
        m_gen[i]->invincible(frame);
      }
    }

    void activateGenerator()
    {
      for(int i=0; i<2; ++i) {
        m_gen[i]->activate();
      }
      m_core->setInvincible(true);
    }

    void activateBlade()
    {
      vector4 pos[5] = {
        vector4(130, 15, 0),
        vector4(50,  -15, 0),
        vector4(-30, 15, 0),
        vector4(-110, -15, 0),
        vector4(-190, 15, 0),
      };
      for(int i=0; i<5; ++i) {
        BladeGenerator *e = new BladeGenerator();
        e->setParent((i%2) ? m_arm_layer->getLower() : m_arm_layer->getUpper());
        e->setPosition(pos[i]);
        e->setGroup(m_group);
        e->setDirection((i%2) ? vector4(0,1,0) : vector4(0,-1,0));
        m_blade[i] = e;
      }
    }


    void checkGeneratorState()
    {
      if(m_inv_count<=0) {
        bool active = false;
        for(int i=0; i<2; ++i) {
          if(m_gen[i]->isActive()) {
            active = true;
          }
        }
        if(!active) {
          m_inv_count = 1400;
          m_core->setInvincible(false);
        }
      }
      else if(--m_inv_count==0) {
        activateGenerator();
        m_core->setInvincible(true);
      }
    }

    void shootBoundLaser()
    {
      vector4 dir = getMatrix()*(matrix44().rotateZ(8.0f*float(m_lasercount))*vector4(-1.0f, 0.0f, 0.0f, 0.0f));
      if(m_lasercount%2==0 && m_gen[0]->isActive()) {
        BoundLaser *l = new BoundLaser(m_gen[0]->getCollision());
        l->setPosition(m_gen[0]->getPosition());
        l->setVel(dir*3.0f);
      }
      else if(m_gen[1]->isActive()) {
        BoundLaser *l = new BoundLaser(m_gen[1]->getCollision());
        l->setPosition(m_gen[1]->getPosition());
        l->setVel((matrix44().rotateA(getMatrix()*vector4(1,0,0,0), 180))*dir*3.0f);
      }
      ++m_lasercount;
    }

    void setArmEmission(const vector4& v)
    {
      for(int i=0; i<6; ++i) {
        m_arm[i]->setEmission(v);
      }
    }



    float getDrawPriority() { return 9.0f; }


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
      DrawRect(center, center+vector2(550, 10));

      static float4 s_col = float4(1,1,1,0.5f);
      static float4 s_colg[2] = {float4(1,1,1,0.5f), float4(1,1,1,0.5f)};
      s_col+=(float4(1,1,1,0.5f)-s_col)*0.05f;
      if(m_core->isDamaged())    { s_col = float4(1,0,0,0.9f); }
      if(m_core->isInvincible()) { s_col = float4(0.1f,0.1f,1.0f,0.9f); }

      glColor4fv(s_col.v);
      DrawRect(center+vector2(1,1), center+vector2(m_core->getLife()/m_core->getMaxLife()*550.0f, 5)-vector2(1,1));

      if(m_gen[0]) {
        bool inv = false;
        center = vector2(25, 27);
        for(int i=0; i<2; ++i) {
          s_colg[i]+=(float4(1,1,1,0.5f)-s_colg[i])*0.05f;
          if(m_gen[i]->isActive()) {
            GeneratorCollision *col = m_gen[i]->getCollision();
            if(col->isDamaged()) { s_colg[i] = float4(1,0,0,0.9f); }
            if(col->isInvincible()) { s_colg[i] = float4(0.1f,0.1f,1.0f,0.9f); }
            glColor4fv(s_colg[i].v);
            DrawRect(center+vector2(1,1), center+vector2(col->getLife()/col->getMaxLife()*100.0f, 5)-vector2(1,1));
            inv = true;
          }
          center+=vector2(100, 0);
        }
        if(!inv) {
          center = vector2(25, 27);
          glColor4f(1,0.6f,0,0.6f);
          DrawRect(center+vector2(1,1), center+vector2(float(m_inv_count)/1400*550, 5)-vector2(1,1));
        }
      }
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }



    void appear()
    {
      if(m_frame==1) {
        m_arm_motion[0] = spline::point_t(vector2(0, 500));
        m_arm_motion[1] = spline::point_t(vector2(0, 250), vector2(500, 250), vector2());
      }

      {
        vector4 pos = m_core_layer->getRelativePosition();
        pos+=(vector4(0,0,0)-pos)*0.01f;
        m_core_layer->setPosition(pos);
      }

      float ay = m_arm_motion.bezier(float(m_frame)).y;
      setArmY(ay);

      if(m_frame==500) {
        m_arm_motion[0] = spline::point_t(vector2(0, 250.0f));
        m_arm_motion[1] = spline::point_t(vector2(120, 250.0f));

        ++m_action;
        m_frame = 0;
        invincible(180);
      }
    }


    void putMediumBlock(const vector4& v)
    {
      MediumBlock *e = new MediumBlock();
      e->setParent(m_root_layer);
      e->setPosition(v);
      e->setVel(vector4(GetRand2(), GetRand2(), 0)+vector4(0.5f, 0, 0));
      e->setAccel(vector4(0.02f, 0, 0));
    }

    void putLargeBlock(const vector4& v)
    {
      LargeBlock *e = new LargeBlock();
      e->setParent(m_root_layer);
      e->setPosition(v);
      e->setVel((vector4(GetRand2(), GetRand2(), 0.0f)+vector4(0.5f, 0.0f, 0.0f))*0.8f);
      e->setAccel(vector4(0.012f, 0.0f, 0.0f));
    }


    void closingArm()
    {
      ++m_arm_frame;
      float ay = getArmY();
      if(m_arm_action==NORMAL) {
        if(player_ptr pl=GetNearestPlayer(getPosition())) {
          cdetector cd;
          box_collision bc;
          point_collision pc;
          bc.setBox(box(vector4(80, 250, 100), vector4(-80, -250, -100)));
          bc.setMatrix(m_core->getMatrix());
          pc.setPosition(pl->getPosition());
          if(cd.detect(bc, pc)) {
            setArmEmission(vector4(1,0,0));
            m_arm_action = CLOSE;
            m_arm_frame = 0;
            m_arm_motion[0] = spline::point_t(vector2(), vector2(0, ay), vector2(90, ay));
            m_arm_motion[1] = spline::point_t(vector2(90, 160));
          }
        }
      }
      else if(m_arm_action==CLOSE) {
        ay = m_arm_motion.bezier(float(m_arm_frame)).y;
        setArmY(ay);
        if(ay==160.0f) {
          m_arm_speed = 0.0f;
          m_arm_action = CLOSE_WAIT;
          m_arm_frame = 0;
        }
      }
      else if(m_arm_action==CLOSE_WAIT) {
        if(m_arm_frame==100) {
          m_arm_frame = 0;
          m_arm_action = NORMAL;
          m_arm_motion[0] = spline::point_t(vector2(), vector2(0, ay), vector2(150, ay));
          m_arm_motion[1] = spline::point_t(vector2(150, 250), vector2(300, 250), vector2());
        }
      }
    }

    void action1()
    {
      if(m_block_frame%180==0) {
        putMediumBlock(vector4(-600.0f, 0, 0)+vector4(GetRand()*50.0f, GetRand()*50.0f, 0));
        ++m_block_count;
      }

      checkGeneratorState();

      {
        float rs = m_root_layer->getRotateSpeed();
        m_root_layer->setRotateSpeed(rs+(0.04f-rs)*0.001f);
      }

      closingArm();
      if(m_arm_action==NORMAL) {
        float ay = m_arm_motion.bezier(float(m_arm_frame)).y;
        setArmY(ay);
      }

      if(m_frame%50==0) {
        shootBoundLaser();
      }

      if(m_frame%600<400 && m_frame%50==0) {
        bool ih=false, iv=false;
        vector4 pos(450, 120, 0);
        if(m_frame/600%4==1) {
          ih = true;
          pos.x = -pos.x;
        }
        else if(m_frame/600%4==2) {
          iv = true;
          pos.y = -pos.y;
        }
        else if(m_frame/600%4==3) {
          ih = true;
          iv = true;
          pos.x = -pos.x;
          pos.y = -pos.y;
        }

        IControler *c;
        if(m_frame/2400%2==0) {
          c = new Fighter_Turn(ih, iv);
        }
        else {
          c = new Fighter_Cross(ih, iv);
        }
        Fighter *e = new Fighter(c);
        e->setParent(m_root_layer);
        e->setPosition(pos);
      }

      if(m_core->getLife()<=(m_core->getMaxLife()*0.55f)) {
        setArmEmission(vector4(0.5f, 0.5f, 1.0f));
        ++m_action;
        invincible(240);
        m_frame = 0;
        m_block_count = 0;
      }
    }

    void to_action2()
    {
      if(m_frame==1) {
        activateBlade();
      }

      if(m_block_frame%160==0) {
        if(m_block_count%5==0) {
          putLargeBlock(vector4(-600.0f, 0, 0)+vector4(GetRand()*40.0f, GetRand()*50.0f, 0));
        }
        else {
          putMediumBlock(vector4(-600.0f, 0, 0)+vector4(GetRand()*40.0f, GetRand()*50.0f, 0));
        }
        ++m_block_count;
      }

      {
        float ay = getArmY();
        ay+=(190.0f-ay)*0.02f;
        setArmY(ay);
      }

      if(m_frame>180) {
        ++m_action;
        invincible(120);
        m_frame = 0;
      }
    }

    void action2()
    {
      if(m_block_frame%180==0) {
        if(m_block_count%5==0) {
          putLargeBlock(vector4(-600.0f, 0, 0)+vector4(GetRand()*40.0f, GetRand()*50.0f, 0));
        }
        else {
          putMediumBlock(vector4(-600.0f, 0, 0)+vector4(GetRand()*40.0f, GetRand()*50.0f, 0));
        }
        ++m_block_count;
      }

      checkGeneratorState();

      {
        float rs = m_root_layer->getRotateSpeed();
        m_root_layer->setRotateSpeed(rs+(-0.06f-rs)*0.004f);
      }

      closingArm();
      if(m_arm_action==NORMAL) {
        float ay = getArmY();
        ay+=(190.0f-ay)*0.02f;
        setArmY(ay);
      }

      if(m_frame%240==0) {
        Wall *e = new Wall();
        e->setParent(m_root_layer);
        e->setPosition(vector4(300, 0, 0));
        e->setDirection(((m_frame/240)%2) ? vector4(0, 1, 0) : vector4(0, -1, 0));
        e->setRideTarget(m_arm[0]->getGroup());
      }
    }


    void clearEnemy()
    {
      gobj_iter& i = GetAllObjects();
      while(i.has_next()) {
        gobj_ptr p = i.iterate();
        if(enemy_ptr e = ToEnemy(p)) {
          if(e!=m_core) {
            SendDestroyMessage(0, e, 1);
          }
        }
        else if(ground_ptr g = ToGround(p)) {
          if(typeid(*g)!=typeid(Arm&)) {
            SendDestroyMessage(0, g, 1);
          }
        }
      }
    }

    void destroy()
    {
      if(m_frame==1) {
        InvincibleAllPlayers(600);
        for(int i=0; i<5; ++i) {
          SendDestroyMessage(0, m_blade[i]);
        }
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60 ? 1 : 0));
        IMusic::FadeOut(6000);
      }

      m_root_layer->setRotateSpeed(m_root_layer->getRotateSpeed()*0.99f);
      m_core_layer->setPosition(m_core_layer->getRelativePosition()+m_core_layer->getParentIMatrix()*vector4(0.2f, -0.15f, -0.2f));
      for(int i=0; i<6; ++i) {
        if(m_arm[i]) {
          m_arm[i]->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
          PutSmallExplode(m_arm[i]->getBox(), m_arm[i]->getMatrix(), 1);
        }
      }
      m_core->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
      if(m_frame%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(m_frame==250) { SendDestroyMessage(0, m_arm[2], 1); }
      if(m_frame==300) { SendDestroyMessage(0, m_arm[5], 1); }
      if(m_frame==340) { SendDestroyMessage(0, m_arm[1], 1); }
      if(m_frame==370) { SendDestroyMessage(0, m_arm[4], 1); }
      if(m_frame==390) { SendDestroyMessage(0, m_arm[3], 1); }
      if(m_frame==400) { SendDestroyMessage(0, m_arm[0], 1); }

      if(m_frame==430) {
        SendDestroyMessage(0, m_root_layer, 1);
        SendKillMessage(0, this);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      SweepDeadObject(m_arm);
      SweepDeadObject(m_gen);
      SweepDeadObject(m_blade);

      ++m_frame;
      ++m_block_frame;
      if(m_core->getLife()<=0.0f && m_action!=4) {
        m_frame = 0;
        m_action = DESTROY;
      }
      if(m_action!=DESTROY && m_gen[0] && !m_gen[0]->isInvincible()) {
        if(--m_time==0) {
          SendDestroyMessage(0, this);
        }
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

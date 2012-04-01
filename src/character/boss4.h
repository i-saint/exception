#ifndef enemy_boss4_h
#define enemy_boss4_h

namespace exception {
namespace stage4 {


  class ArmBase : public Inherit2(HaveBoxBound, ChildRotLayer)
  {
  typedef Inherit2(HaveBoxBound, ChildRotLayer) Super;
  public:

    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts() {}
      int getFractionCount() { return 0; }
    };

    class Core : public Inherit2(HaveInvincibleMode, ChildEnemy)
    {
    typedef Inherit2(HaveInvincibleMode, ChildEnemy) Super;
    public:
      Core(Deserializer& s) : Super(s) {}

      Core()
      {
        setBox(box(vector4(30)));
        setLife(50.0f);
        setEnergy(200.0f);
      }

      void updateEmission()
      {
        vector4 emission = getEmission();
        if(isInvincible()) {
          emission+=(vector4(1.0f, 1.0f, 1.0f)-emission)*0.05f;
        }
        else {
          emission+=(vector4(0.5f, 0.5f, 0.0f)-emission)*0.05f;
        }
        setEmission(emission);
      }

      void updateEmission(bool v)
      {}

      int getFractionCount() { return Super::getFractionCount()*2; }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        if(m.getStat()==0) {
          PutMediumImpact(getCenter());
        }
      }
    };

    class ChildCore : public Core
    {
    typedef Core Super;
    public:
      ChildCore(Deserializer& s) : Super(s) {}

      ChildCore()
      {
        setLife(30.0f);
      }
      int getFractionCount() { return 0; }
    };

    class Shutter : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      float m_width;

    public:
      Shutter(Deserializer& s) : Super(s)
      {
        s >> m_width;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_width;
      }

    public:
      Shutter() : m_width(0.0f)
      {
        setLife(10.0f);
        setEnergy(20.0f);
      }

      int getFractionCount() { return 0; } // 破片飛ばさない 

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        if(m_width<40.0f) {
          m_width+=0.5f;
          setBox(box(vector4(m_width, 5, 10), vector4(0, -5, -10)));
        }
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        GetSound("explosion2.wav")->play(1);
      }
    };

    class ShutterGenerator : public Inherit2(HaveParent, Optional)
    {
    typedef Inherit2(HaveParent, Optional) Super;
    private:
      Shutter *m_shutter;
      int m_wait;
      gid m_group;

    public:
      ShutterGenerator(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_shutter);
        s >> m_wait >> m_group;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_shutter);
        s << m_wait << m_group;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_shutter);
      }

    public:
      ShutterGenerator(int wait=0) : m_shutter(0), m_wait(wait), m_group(0)
      {}

      void setGroup(gid v) { m_group=v; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_shutter) {
          if(m_shutter->isDead()){
            m_shutter = 0;
            m_wait = 60;
          }
        }
        else {
          --m_wait;
          if(m_wait<=0) {
            m_shutter = new Shutter();
            m_shutter->setParent(this);
            m_shutter->setGroup(m_group);
          }
        }
      }
    };

  protected:
    bool m_ih;
    gid m_group;

  public:
    ArmBase(Deserializer& s) : Super(s)
    {
      s >> m_ih >> m_group;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_ih << m_group;
    }

  public:
    ArmBase() : m_ih(false), m_group(0)
    {
      m_group = Solid::createGroupID();
      setBound(box(vector4(1000, 1200, 1000)));
    }

    virtual Core* getCore()=0;

    void setGroup(gid v) { m_group=v; }
    gid getGroup() { return m_group; }
    void setInvert(bool v) { modMatrix(); m_ih=v; }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      if(m_ih) {
        mat.rotateY(180.0f);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      // デフォルトの挙動握り潰し 
      SendDestroyMessage(0, getCore(), m.getStat());
    }
  };


  class Arm1 : public ArmBase
  {
  typedef ArmBase Super;
  private:
    Core *m_core;
    SmallGear *m_gear;
    ShutterGenerator *m_shutterg[12];
    Parts *m_wall[3];
    Parts *m_presser;
    vector4 m_vel;
    int m_frame;
    int m_state;

  public:
    Arm1(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_gear);
      DeserializeLinkage(s, m_shutterg);
      DeserializeLinkage(s, m_wall);
      DeserializeLinkage(s, m_presser);
      s >> m_vel >> m_frame >> m_state;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_gear);
      SerializeLinkage(s, m_shutterg);
      SerializeLinkage(s, m_wall);
      SerializeLinkage(s, m_presser);
      s << m_vel << m_frame << m_state;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_gear);
      ReconstructLinkage(m_shutterg);
      ReconstructLinkage(m_wall);
      ReconstructLinkage(m_presser);
    }

  public:
    Arm1(bool invert) : m_frame(0), m_state(0)
    {
      setInvert(invert);
      setPosition(vector4(150*(invert?-1:1), 500, 0));

      gid g = getGroup();
      m_core = new Core();
      m_core->setParent(this);
      m_core->setGroup(g);
      m_core->setInvincible(false);

      {
        box b[3] = {
          box(vector4(-80, 50, 15), vector4(-50, -50, -15)), // 左壁 
          box(vector4(-80,  50, 15), vector4(150, 100, -15)), // 上壁 
          box(vector4(-80, -50, 15), vector4( 60, -80, -15)), // 下壁 
        };
        for(int i=0; i<3; ++i) {
          Parts *p = new Parts();
          p->setParent(this);
          p->setBox(b[i]);
          p->setGroup(g);
          m_wall[i] = p;
        }
      }
      {
        box b[4] = {
          box(vector4( 20, -50, 15), vector4( 60,-300, -15)), // 左壁 
          box(vector4(100,  50, 15), vector4(160,-250, -15)), // 右壁内側1 
          box(vector4(100,-255, 15), vector4(160,-320, -15)), // 右壁内側2 
          box(vector4(150, 150, 25), vector4(190,-340, -25)), // 右壁外側
        };

        for(int i=0; i<4; ++i) {
          Parts *p = new Parts();
          p->setParent(this);
          p->setBox(b[i]);
          p->setGroup(g);
        }
        m_presser = new Parts();
        m_presser->setParent(this);
        m_presser->setBox(box(vector4( 60, -50, 12), vector4(110, -300, -12)));
        m_presser->setGroup(g);
      }

      for(int i=0; i<12; ++i) {
        ShutterGenerator *sg = new ShutterGenerator();
        sg->setParent(this);
        sg->setPosition(vector4(60, -60, 0)+vector4(0, -20, 0)*i);
        sg->setGroup(g);
        m_shutterg[i] = sg;
      }

      m_gear = new SmallGear();
      m_gear->setParent(this);
      m_gear->setPosition(vector4(130, -375, 0));
      m_gear->setMinRotate(0.0f);
      m_gear->setMaxRotate(360.0f*3.0f);
      m_gear->setIncrementOnly(true);
      m_gear->setReverseSpeed(0.06f);
      m_gear->setRotate(360.0f*3.0f);
      m_gear->setGroup(g);
    }

    Core *getCore() { return m_core; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_core);
      SweepDeadObject(m_gear);
      SweepDeadObject(m_shutterg);
      SweepDeadObject(m_wall);
      SweepDeadObject(m_presser);

      int f = ++m_frame;
      if(!m_core && m_state!=2) { // コアが死んだら下に落とす 
        m_state = 2;
        Destroy(getTSM(), m_wall);
      }

      if(m_gear) {
        if(f<120) {
          m_gear->setRotateSpeed(0.0f);
        }
        if(f==120) {
          m_gear->setRotateSpeed(-15.0f);
        }
      }

      if(m_state==0) { // 入場 
        vector4 pos = getRelativePosition();
        pos.y-=500*(Sin90(1.0f/180*f)-Sin90(1.0f/180*(f-1)));
        setPosition(pos);
        if(f==180) {
          m_state = 1;
        }
      }
      else if(m_state==2) { // 落下 
        m_vel+=vector4(0, -0.02f, 0);
        setPosition(getRelativePosition()+m_vel);
      }

      if(m_presser && m_gear) {
        m_presser->setPosition(vector4(50.0f/(360.0f*3.0f)*m_gear->getRotate(), 0, 0));
      };
    }
  };


  class Arm2 : public ArmBase
  {
  typedef ArmBase Super;
  public:

    class ChildArm : public ArmBase
    {
    typedef ArmBase Super;
    private:
      Core *m_core;
      SmallGear *m_gear;
      ShutterGenerator *m_shutterg[11];
      Parts *m_ground[3];
      Parts *m_presser;
      vector4 m_vel;
      int m_frame;
      int m_state;

    public:
      ChildArm(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        DeserializeLinkage(s, m_gear);
        DeserializeLinkage(s, m_shutterg);
        DeserializeLinkage(s, m_ground);
        DeserializeLinkage(s, m_presser);
        s >> m_vel >> m_frame >> m_state;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        SerializeLinkage(s, m_gear);
        SerializeLinkage(s, m_shutterg);
        SerializeLinkage(s, m_ground);
        SerializeLinkage(s, m_presser);
        s << m_vel << m_frame << m_state;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
        ReconstructLinkage(m_gear);
        ReconstructLinkage(m_shutterg);
        ReconstructLinkage(m_ground);
        ReconstructLinkage(m_presser);
      }

    public:
      ChildArm(gid g) : m_frame(0), m_state(0)
      {
        setGroup(g);

        m_core = new ChildCore();
        m_core->setParent(this);
        m_core->setGroup(g);
        m_core->setInvincible(false);
        m_core->setBox(box(vector4(5,-165, 20), vector4(55,-215, -20)));

        {
          box b[] = {
            box(vector4(-30,-220, 15), vector4( 20,-325, -15)), // 左壁内側1 
            box(vector4(-30,-330, 15), vector4( 20,-450, -15)), // 左壁内側2 
            box(vector4(-60, -80, 25), vector4(-20,-450, -25)), // 左壁外側 
          };
          for(int i=0; i<3; ++i) {
            Parts *p = new Parts();
            p->setParent(this);
            p->setBox(b[i]);
            p->setGroup(g);
            m_ground[i] = p;
          }
          m_presser = new Parts();
          m_presser->setParent(this);
          m_presser->setBox(box(vector4( 60, -220, 12), vector4(10, -450, -12)));
          m_presser->setGroup(getGroup());
        }

        for(int i=0; i<11; ++i) {
          ShutterGenerator *sg = new ShutterGenerator();
          sg->setParent(this);
          sg->setPosition(vector4(20,-230, 0)+vector4(0, -20, 0)*i);
          sg->setGroup(getGroup());
          m_shutterg[i] = sg;
        }

        m_gear = new SmallGear();
        m_gear->setParent(this);
        m_gear->setPosition(vector4(130, -375, 0));
        m_gear->setMinRotate(0.0f);
        m_gear->setMaxRotate(360.0f*3.0f);
        m_gear->setIncrementOnly(true);
        m_gear->setReverseSpeed(0.05f);
        m_gear->setRotate(360.0f*3.0f);
        m_gear->setGroup(getGroup());
      }

      Core *getCore() { return m_core; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_core);
        SweepDeadObject(m_gear);
        SweepDeadObject(m_shutterg);
        SweepDeadObject(m_ground);
        SweepDeadObject(m_presser);

        int f = ++m_frame;
        if(!m_core && m_state!=2) { // コアが死んだら下に落とす 
          m_state = 2;
        }

        if(m_gear) {
          if(f<120) {
            m_gear->setRotateSpeed(0.0f);
          }
          if(f==120) {
            m_gear->setRotateSpeed(-15.0f);
          }
        }

        if(m_state==0) {
          if(f==180) {
            m_state = 1;
          }
        }
        else if(m_state==2) { // 落下 
          m_vel+=vector4(0, -0.02f, 0);
          setPosition(getRelativePosition()+m_vel);
        }

        if(m_presser && m_gear) {
          m_presser->setPosition(vector4(-50.0f/(360.0f*3.0f)*m_gear->getRotate(), 0, 0));
        };
      }
    };

  private:
    Core *m_core;
    SmallGear *m_gear;
    ShutterGenerator *m_shutterg[10];
    Parts *m_presser;
    Parts *m_cap;
    Parts *m_wall[3];
    ChildArm *m_child;
    vector4 m_vel;
    int m_frame;
    int m_state;

  public:
    Arm2(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_gear);
      DeserializeLinkage(s, m_shutterg);
      DeserializeLinkage(s, m_presser);
      DeserializeLinkage(s, m_cap);
      DeserializeLinkage(s, m_wall);
      DeserializeLinkage(s, m_child);
      s >> m_vel >> m_frame >> m_state;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_gear);
      SerializeLinkage(s, m_shutterg);
      SerializeLinkage(s, m_presser);
      SerializeLinkage(s, m_cap);
      SerializeLinkage(s, m_wall);
      SerializeLinkage(s, m_child);
      s << m_vel << m_frame << m_state;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_gear);
      ReconstructLinkage(m_shutterg);
      ReconstructLinkage(m_presser);
      ReconstructLinkage(m_cap);
      ReconstructLinkage(m_wall);
      ReconstructLinkage(m_child);
    }

  public:
    Arm2(bool invert) : m_frame(0), m_state(0)
    {
      setInvert(invert);
      setPosition(vector4(550*(invert?-1:1), 0, 0));

      gid g = getGroup();
      m_core = new Core();
      m_core->setParent(this);
      m_core->setGroup(g);

      {
        box b[3] = {
          box(vector4(-80,  50, 15), vector4(-50, -50, -15)), // 左壁 
          box(vector4(-80,  50, 15), vector4(200, 100, -15)), // 上壁 
          box(vector4(-80, -50, 15), vector4( 60, -80, -15)), // 下壁 
        };
        for(int i=0; i<3; ++i) {
          Parts *p = new Parts();
          p->setParent(this);
          p->setBox(b[i]);
          p->setGroup(g);
          m_wall[i] = p;
        }
      }
      {
        box b[7] = {
          box(vector4( 60, -50, 15), vector4(100,-260, -15)), // 左壁1 
          box(vector4( 60,-300, 15), vector4(100,-450, -15)), // 左壁2 
          box(vector4(140,  50, 15), vector4(200,-250, -15)), // 右壁内側1 
          box(vector4(140,-255, 15), vector4(200,-320, -15)), // 右壁内側2 
          box(vector4(190, 150, 25), vector4(230,-300, -25)), // 右壁外側 
          box(vector4( 80,-310, 15), vector4(160,-340, -15)), // 下蓋 
        };
        for(int i=0; i<7; ++i) {
          Parts *p = new Parts();
          p->setParent(this);
          p->setBox(b[i]);
          p->setGroup(g);
        }
      }

      m_cap = new Parts();
      m_cap->setParent(this);
      m_cap->setBox(box(vector4( 60,-260, 12), vector4(90,-300, -12)));
      m_cap->setGroup(g);

      m_presser = new Parts();
      m_presser->setParent(this);
      m_presser->setBox(box(vector4(100, -50, 12), vector4(150, -260, -12)));
      m_presser->setGroup(g);

      for(int i=0; i<10; ++i) {
        ShutterGenerator *sg = new ShutterGenerator();
        sg->setParent(this);
        sg->setPosition(vector4(100, -60, 0)+vector4(0, -20, 0)*i);
        sg->setGroup(g);
        m_shutterg[i] = sg;
      }

      m_gear = new SmallGear();
      m_gear->setParent(this);
      m_gear->setPosition(vector4(25,-130, 0));
      m_gear->setMinRotate(0.0f);
      m_gear->setMaxRotate(360.0f*3.0f);
      m_gear->setIncrementOnly(true);
      m_gear->setReverseSpeed(0.05f);
      m_gear->setRotate(360.0f*3.0f);
      m_gear->setGroup(g);

      m_child = new ChildArm(g);
      m_child->setParent(this);
    }

    Core *getCore() { return m_core; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_core);
      SweepDeadObject(m_gear);
      SweepDeadObject(m_shutterg);
      SweepDeadObject(m_presser);
      SweepDeadObject(m_cap);
      SweepDeadObject(m_wall);
      SweepDeadObject(m_child);

      int f = ++m_frame;
      if(m_core && m_core->isInvincible() && m_child && !m_child->getCore()) {
        m_core->setInvincible(false); // 子アームが死んだら無敵解除 
      }
      if(!m_core && m_state!=2) { // コアが死んだら下に落とす 
        m_state = 2;
        Destroy(getTSM(), m_wall);
      }

      if(m_gear) {
        if(f<120) {
          m_gear->setRotateSpeed(0.0f);
        }
        if(f==120) {
          m_gear->setRotateSpeed(-15.0f);
        }
      }

      if(m_state==0) { // 入場 
        vector4 pos = getRelativePosition();
        pos.x-=400*(Sin90(1.0f/180*f)-Sin90(1.0f/180*(f-1)))*(m_ih?-1:1);
        setPosition(pos);
        if(f==180) {
          m_state = 1;
        }
      }
      else if(m_state==2) { // 落下 
        m_vel+=vector4(0, -0.02f, 0);
        setPosition(getRelativePosition()+m_vel);
      }

      if(m_gear) {
        if(m_presser) {
          m_presser->setPosition(vector4(50.0f/(360.0f*3.0f)*m_gear->getRotate(), 0, 0));
        }
        if(m_cap) {
          m_cap->setPosition(vector4(0, 40.0f/(360.0f*3.0f)*m_gear->getRotate(), 0));
        }
      };
    }
  };





  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    private:
      vector4 m_emission;

    public:
      Parts(Deserializer& s) : Super(s)
      {
        s >> m_emission;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_emission;
      }

    public:
      Parts() {}
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

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutMediumImpact(getCenter());
      }
    };

    class Core : public Inherit2(HaveInvincibleMode, Enemy)
    {
    typedef Inherit2(HaveInvincibleMode, Enemy) Super;
    public:
      Core(Deserializer& s) : Super(s) {}

      Core()
      {
        setLife(getMaxLife());
      }

      float getMaxLife()
      {
        float l[] = {900, 1050, 1350, 1500, 1800};
        return l[GetLevel()];
      }

      void drawLifeGauge()
      {}

      void draw()
      {
        if(!isDamaged()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f).v);
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

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(p && (IsFraction(p) || typeid(*p)==typeid(MediumBlock&) || typeid(*p)==typeid(LargeBlock&))) {
          SendDestroyMessage(0, p);
        }
      }

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<GLaser*>(m.getSource()) &&
           !dynamic_cast<Ray*>(m.getSource())) { // 自機の武装無効 
          Super::onDamage(m);
        }
      }

      int getFractionCount() { return 0; }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBossDestroyImpact(getCenter());
      }

      void onLifeZero(gobj_ptr from)
      {
        PutSmallExplode(getBox(), getMatrix(), getExplodeCount()/2);
        PutBigImpact(getCenter());
      }
    };


  private:
    static const int s_num_parts = 6;
    enum {
      APPEAR,
      ACTION1,
      ACTION2,
      DESTROY,
    };

    Layer *m_root_layer;
    Core *m_core;
    Parts *m_parts[s_num_parts];
    ArmBase *m_arm[2];
    FloatingTurret *m_turret[4];
    int m_arm_deadcount[2];
    int m_frame;
    int m_time;
    float m_rot;
    int m_action;

  public:
    Boss(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_root_layer);
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_arm);
      DeserializeLinkage(s, m_turret);
      s >> m_arm_deadcount >> m_frame >> m_time >> m_rot >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_root_layer);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_arm);
      SerializeLinkage(s, m_turret);
      s << m_arm_deadcount << m_frame << m_time << m_rot << m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_root_layer);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_arm);
      ReconstructLinkage(m_turret);
    }

  public:
    Boss() : m_frame(0), m_time(60*180), m_rot(0.0f), m_action(APPEAR)
    {
      m_root_layer = new Layer();
      m_root_layer->setPosition(vector4(0, 450, 0));

      gid group = Solid::createGroupID();
      m_core = new Core();
      m_core->setParent(m_root_layer);
      m_core->setBox(box(vector4(50)));
      m_core->setGroup(group);
      {
        box b[s_num_parts] = {
          box(vector4( 130,  60, 35), vector4(-130, 150, -35)), // 上側シールド 
          box(vector4(  50, 110, 25), vector4( 200,  50, -25)),
          box(vector4( -50, 110, 25), vector4(-200,  50, -25)),

          box(vector4( 100, -70, 25), vector4(-100,-120, -25)), // 下側シールド 
          box(vector4(  50, -90, 25), vector4( 180, -50, -25)),
          box(vector4( -50, -90, 25), vector4(-180, -50, -25)),
        };
        for(int i=0; i<s_num_parts; ++i) {
          Parts *g = new Parts();
          g->setParent(m_root_layer);
          g->setBox(b[i]);
          g->setGroup(group);
          m_parts[i] = g;
        }
      }

      for(int i=0; i<2; ++i) {
        m_arm[i] = 0;
        m_arm_deadcount[i] = 300;
      }

      ZeroClear(m_turret);
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


    ArmBase* createArm(bool invert)
    {
      ArmBase *a = 0;
      float l = m_core->getLife();
      float ml = m_core->getMaxLife();
      if(l > ml*0.55f) {
        SetCameraMovableArea(vector2(50, 50), vector2(-50, -200));
        a = new Arm1(invert);
      }
      else {
        SetCameraMovableArea(vector2(100, 50), vector2(-100, -200));
        a = new Arm2(invert);
      }

      return a;
    }


    void appear()
    {
      int f = m_frame;
      vector4 pos = m_root_layer->getRelativePosition();
      float pq = Sin90(1.0f/120*(f-1));
      float q = Sin90(1.0f/120*f);
      m_root_layer->setPosition(pos+vector4(0, -250, 0)*(q-pq));

      if(f==120) {
        m_action = ACTION1;
        m_frame = 0;
      }
    }

    void updateArms()
    {
      for(int i=0; i<2; ++i) {
        if(m_arm[i]) {
          if(m_arm[i]->isDead() || !m_arm[i]->getCore()) {
            m_arm[i] = 0;
          }
        }
        else {
          if(++m_arm_deadcount[i]>=240) {
            m_arm_deadcount[i] = 0;
            if(i==0) {
              m_arm[i] = createArm(false);
              m_arm[i]->setParent(m_root_layer);
            }
            else {
              m_arm[i] = createArm(true);
              m_arm[i]->setParent(m_root_layer);
            }
          }
        }
      }
    }

    void action1()
    {
      updateArms();

      int f = m_frame;
      if(f>240) {
        m_rot+=0.05f;
        vector4 pos = m_root_layer->getRelativePosition();
        pos.x = 75.0f*sinf(m_rot*ist::radian);
        m_root_layer->setPosition(pos);
      }
      if(f==60) {
        m_core->setInvincible(false);
      }

      f = f%1900;
      { // HeavyFighter ミサイル→レーザー 
        int kf[] = {300, 800, 1300, 1700};
        for(int i=0; i<4; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4( 120,-820,0), vector4(-120,-820,0),
            vector4(-750,-400,0), vector4( 750,-400,0),
          };
          vector4 dir[] = {
            vector4(0,1,0), vector4( 0,1,0),
            vector4(1,0,0), vector4(-1,0,0),
          };

          HeavyFighter *e = 0;
          if(i<2) {
            e = new HeavyFighter(new HeavyFighter_Missiles2(false));
          }
          else {
            e = new HeavyFighter(new HeavyFighter_Laser(i==3, false));
          }
          e->setPosition(pos[i]);
          e->setDirection(dir[i]);
        }
      }
      { // Fighterラッシュ 
        int kf[] = {
          250, 280, 310, 340,
          850, 880, 910, 940,
        };
        for(int i=0; i<8; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4( -70,-500,0), vector4(-150,-500,0), vector4(-230,-500,0), vector4(-310,-500,0),
            vector4(  70,-500,0), vector4( 150,-500,0), vector4( 230,-500,0), vector4( 310,-500,0),
          };

          Fighter_Rush *c = new Fighter_Rush(false);
          c->setLength(200.0f);

          Fighter *e = new Fighter(c);
          e->setPosition(pos[i]);
          e->setDirection(vector4(0,1,0));
        }
      }
      { // ザブラッシュ 
        int kf[] = {1500,1600, 1800,1899};
        for(int i=0; i<4; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos = vector4(-80+m_root_layer->getPosition().x, -150+50*i, 0);
          for(int j=0; j<3; ++j) {
            PutZab(pos, false);
            pos.x+=80.0f;
          }
        }
      }

      if(m_core->getLife() < m_core->getMaxLife()*0.55f) {
        m_action = ACTION2;
        m_frame = 0;
        for(int i=0; i<2; ++i) {
          SendDestroyMessage(0, m_arm[i], 1);
        }
      }
    }

    void action2()
    {
      updateArms();

      int f = m_frame;
      if(f==1) {
        for(int i=0; i<4; ++i) {
          m_turret[i] = 0;
        }
      }
      SweepDeadObject(m_turret);

      {
        m_rot+=0.05f;
        vector4 pos = m_root_layer->getRelativePosition();
        pos.x = 75.0f*sinf(m_rot*ist::radian);
        m_root_layer->setPosition(pos);
      }

      {
        int kf[] = {30,60,90,120};
        vector4 pos[] = {
          vector4(-330, -380, 0), vector4(-190, -440, 0), vector4(190, -440, 0), vector4(330, -380, 0),
        };
        for(int i=0; i<4; ++i) {
          if(!m_turret[i] && f%200==kf[i]) {
            FloatingTurret *e = new FloatingTurret(new FloatingTurret_Wait());
            e->setPosition(pos[i]+vector4(0,-200,0));
            e->setGroup(m_core->getGroup());
            m_turret[i] = e;
          }
        }
      }
      if(f%250==0) {
        FloatingTurret_Fall *c = new FloatingTurret_Fall();
        c->setMove(vector4(GetRand2()*20, -200, 30));

        FloatingTurret *e = new FloatingTurret(c);
        e->setPosition(m_core->getPosition()+vector4(0, 75, -30));
        e->setGroup(m_core->getGroup());
      }
      {
        int kf[] = {120,150, 260,290};
        vector4 pos[] = {
          vector4( 420, -400, 0), vector4( 420, -320, 0),
          vector4(-420, -400, 0), vector4(-420, -320, 0),
        };
        for(int i=0; i<4; ++i) {
          if(f%300==kf[i]) {
            PutZab(pos[i], false);
          }
        }
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
      }
    }

    void destroy()
    {
      if(m_frame==1) {
        InvincibleAllPlayers(600);
        for(int i=0; i<2; ++i) {
          SendDestroyMessage(0, m_arm[i], 1);
        }
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60 ? 1 : 0));
        IMusic::FadeOut(6000);
      }

      m_core->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
      for(int i=0; i<s_num_parts; ++i) {
        if(m_parts[i]) {
          m_parts[i]->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
        }
      }
      if(m_frame%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(m_frame==250) { SendDestroyMessage(0, m_parts[2], 1); }
      if(m_frame==300) { SendDestroyMessage(0, m_parts[5], 1); }
      if(m_frame==340) { SendDestroyMessage(0, m_parts[1], 1); }
      if(m_frame==370) { SendDestroyMessage(0, m_parts[4], 1); }
      if(m_frame==390) { SendDestroyMessage(0, m_parts[3], 1); }
      if(m_frame==400) { SendDestroyMessage(0, m_parts[0], 1); }

      if(m_frame==420) {
        SendDestroyMessage(0, m_core);
        SendKillMessage(0, this);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_parts);

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

      switch(m_action) {
      case APPEAR:  appear();  break;
      case ACTION1: action1(); break;
      case ACTION2: action2(); break;
      case DESTROY: destroy(); break;
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

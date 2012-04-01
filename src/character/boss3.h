#ifndef enemy_boss3_h
#define enemy_boss3_h

namespace exception {
namespace stage3 {


  class CellBlock : public Enemy
  {
  typedef Enemy Super;
  public:
    CellBlock(Deserializer& s) : Super(s) {}
    CellBlock()
    {
      setBox(box(vector4(29, 29, 29)));
      setLife(12.5f);
      setEnergy(2.5f);
      setBound(box(vector4(1000,1000,1000), vector4(-1000,-1000,-1000)));
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      setPosition(getRelativePosition()+GetGlobalScroll());
    }

    void onCollide(CollideMessage& m)
    {
      if(!IsSolid(m.getFrom())) {
        return;
      }

      if(typeid(*m.getFrom())==typeid(LargeBlock&)) {
         SendDestroyMessage(m.getFrom(), this);
         SendDamageMessage(this, m.getFrom(), 1, this);
      }
      else if(typeid(*m.getFrom())==typeid(MediumBlock&)) {
         SendDamageMessage(m.getFrom(), this, 7, this);
         SendDamageMessage(this, m.getFrom(), 1, this);
      }
    }

    void onDamage(DamageMessage& m)
    {
      if(IsFraction(m.getSource())) {
        SendDamageMessage(m.getFrom(), this, m.getDamage()*2.5f);
      }
      else {
        Super::onDamage(m);
      }
    }

    int getFractionCount() { return 0; }
    int getExplodeCount() { return 5; }
  };

  class CellGround : public Ground
  {
  typedef Ground Super;
  public:
    CellGround(Deserializer& s) : Super(s) {}
    CellGround()
    {
      setBox(box(vector4(29, 29, 29)));
      setBound(box(vector4(1000,1000,1000), vector4(-1000,-1000,-1000)));
    }

    void scroll()
    {
      setPosition(getRelativePosition()+GetGlobalScroll());
    }
  };


  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class ICore : public Inherit4(HaveSpinAttrib, HaveInvincibleMode, HaveDirection, Enemy)
    {
    typedef Inherit4(HaveSpinAttrib, HaveInvincibleMode, HaveDirection, Enemy) Super;
    public:
      ICore(Deserializer& s) : Super(s) {}
      ICore() {}
    };

    class Guard : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      ICore *m_core;
      bool m_start;
      int m_frame;
      float m_opa;

    public:
      Guard(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_start >> m_frame >> m_opa;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_start << m_frame << m_opa;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      Guard(gobj_ptr parent, ICore *core) : m_core(core), m_start(false), m_frame(0), m_opa(0)
      {
        setParent(parent);
        setGroup(m_core->getGroup());
        setLife(9999);
        enableCollision(false);
      }

      void start() { m_start=true; }

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
        if(m_opa==0.0f || !m_core) {
          return;
        }
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,1,m_opa).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_core->getEmission().v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,1.0f).v);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        SweepDeadObject(m_core);
        if(m_start) {
          m_opa+=(0.8f-m_opa)*0.005f;
          if(m_opa>0.5f) {
            enableCollision(true);
          }
        }
      }

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<GLaser*>(m.getSource()) && // 自機の攻撃2種無効 
           !dynamic_cast<Ray*>(m.getSource())) {
          SendDamageMessage(m.getFrom(), m_core, m.getDamage()*0.9f, this);
        }
      }

      void onCollide(CollideMessage& m)
      {
        solid_ptr s = ToSolid(m.getFrom());
        if(!s) {
          return;
        }

        if(typeid(*m.getFrom())==typeid(CellBlock&)) {
          SendDestroyMessage(this, m.getFrom());
        }
        else if(!IsFraction(s)) {
          SendDamageMessage(this, m.getFrom(), 0.4f);
        }
      }

      int getFractionCount() { return 0; }
      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBigImpact(getCenter());
      }
    };


    class GuardLayer : public Inherit2(HaveSpinAttrib, ChildLayer)
    {
    typedef Inherit2(HaveSpinAttrib, ChildLayer) Super;
    private:
      bool m_start;

    public:
      GuardLayer(Deserializer& s) : Super(s)
      {
        s >> m_start;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_start;
      }

    public:
      GuardLayer() : m_start(false)
      {
        setAxis(vector4(0,0,1));
        chain();
      }

      void start() { m_start=true; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_start) {
          float rots = getRotateSpeed();
          setRotateSpeed(rots+(0.5f-rots)*0.001f);
        }
      }
    };

    class GuardLayer2 : public Inherit2(HaveSpinAttrib, ChildLayer)
    {
    typedef Inherit2(HaveSpinAttrib, ChildLayer) Super;
    private:
      bool m_start;

    public:
      GuardLayer2(Deserializer& s) : Super(s)
      {
        s >> m_start;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_start;
      }

    public:
      GuardLayer2() : m_start(false)
      {
        setAxis(vector4(0,1,1));
        chain();
      }

      void start() { m_start=true; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_start) {
          float rots = getRotateSpeed();
          setRotateSpeed(rots+(0.7f-rots)*0.001f);
        }
      }
    };


    class Core : public ICore
    {
    typedef ICore Super;
    public:
      Core(Deserializer& s) : Super(s) {}
      Core()
      {
        setBox(box(vector4(25)));
        setLife(getMaxLife());

        setAxis(vector4(1,1,1).normal());
        setRotateSpeed(0.3f);
      }

      float getMaxLife()
      {
        float l[] = {1900, 2400, 3400, 4400, 5000};
        return l[GetLevel()];
      }

      float getDrawPriority() { return 1.1f; }

      void drawLifeGauge() {}

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

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<GLaser*>(m.getSource()) &&
           !dynamic_cast<Ray*>(m.getSource())) { // 自機の武装無効 
          Super::onDamage(m);
        }
      }

      void onCollide(CollideMessage& m)
      {
        if(IsFraction(m.getFrom()) || dynamic_cast<CellBlock*>(m.getFrom())) {
          SendDestroyMessage(0, m.getFrom());
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


    class PararelLaser1 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_center;
      vector4 m_vert;
      int m_frame;
      int m_count;

    public:
      PararelLaser1(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_center >> m_vert >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_center << m_vert << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser1(Core *core, const vector4& target, bool invert) : m_core(core), m_frame(0), m_count(0)
      {
        m_dir = (target-m_core->getPosition()).setZ(0).normal();
        m_center = m_core->getPosition()-m_dir*50.0f;
        m_vert = matrix44().rotateZ(invert ? 90 : -90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%10==1) {
          vector4 tar = m_center+(m_vert*(360-60*m_count));
          LaserBit::create(m_core, m_core->getPosition(), tar, m_dir, 40);

          if(++m_count==13) {
            SendKillMessage(0, this);
          }
        }
      }
    };

    class PararelLaser2 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_vert;
      int m_frame;
      int m_count;

    public:
      PararelLaser2(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_vert >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_vert << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser2(Core *core, const vector4& target) :
        m_core(core), m_frame(0), m_count(0)
      {
        m_dir = (target-m_core->getPosition()).setZ(0).normal();
        m_vert = matrix44().rotateZ(90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%20==1) {
          vector4 center1 = m_core->getPosition()+m_dir*50.0f;
          vector4 center2 = m_core->getPosition()-m_dir*100.0f;
          vector4 tar[4] = {
            center1+(m_vert* (20+60*m_count)),
            center1+(m_vert*-(20+60*m_count)),
            center2+(m_vert* (50+60*m_count)),
            center2+(m_vert*-(50+60*m_count)),
          };
          int delay[4] = {
            30, 30, 120, 120
          };
          for(int i=0; i<4; ++i) {
            LaserBit::create(m_core, m_core->getPosition(), tar[i], m_dir, delay[i]);
          }

          if(++m_count==5) {
            SendKillMessage(0, this);
          }
        }
      }
    };

    class PararelLaser3 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_center;
      vector4 m_vert;
      int m_frame;
      int m_count;

    public:
      PararelLaser3(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_center >> m_vert >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_center << m_vert << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser3(Core *core, const vector4& target, bool invert) : m_core(core), m_frame(0), m_count(0)
      {
        m_dir = matrix44().rotateZ(invert ? 45 : -45)*(target-m_core->getPosition()).setZ(0).normal();
        m_center = m_core->getPosition()-m_dir*125.0f;
        m_vert = matrix44().rotateZ(invert ? 90 : -90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%10==1) {
          vector4 tar = m_center+(m_vert*(120-60*m_count));
          LaserBit::create(m_core, m_core->getPosition(), tar, m_dir, 40);

          if(++m_count==13) {
            SendKillMessage(0, this);
          }
        }
      }
    };

    class PararelLaser4 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_vert;
      int m_frame;
      int m_count;

    public:
      PararelLaser4(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_vert >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_vert << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser4(Core *core, const vector4& target) : m_core(core), m_frame(0), m_count(0)
      {
        m_dir = (target-m_core->getPosition()).setZ(0).normal();
        m_vert = matrix44().rotateZ(90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%15==1) {
          vector4 center1 = m_core->getPosition()+m_dir*50.0f;
          vector4 center2 = m_core->getPosition()-m_dir*50.0f;
          vector4 tar[2] = {
            center1+(m_vert*( 320+20-80*m_count)),
            center2+(m_vert*(-320-20+80*m_count)),
          };
          for(int i=0; i<2; ++i) {
            LaserBit::create(m_core, m_core->getPosition(), tar[i], m_dir, 60);
          }

          if(++m_count==9) {
            SendKillMessage(0, this);
          }
        }
      }
    };



    class PararelLaser11 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_vert;
      float m_width;
      int m_n;
      int m_frame;
      int m_count;

    public:
      PararelLaser11(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_vert >> m_width >> m_n >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_vert << m_width << m_n << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser11(Core *core, const vector4& target, float width=50.0f, int n=8) :
        m_core(core), m_width(width), m_n(n), m_frame(0), m_count(0)
      {
        m_dir = (target-m_core->getPosition()).setZ(0).normal();
        m_vert = matrix44().rotateZ(90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%10==1) {
          vector4 center = m_core->getPosition()+m_dir*50.0f;
          if(m_count==0) {
            LaserBit::create(m_core, m_core->getPosition(), center, m_dir, 60);
          }
          else {
            vector4 tar[2] = {
              center+(m_vert* (m_width*m_count))-(m_dir*(m_width*m_count*0.5f)),
              center+(m_vert*-(m_width*m_count))-(m_dir*(m_width*m_count*0.5f)),
            };
            for(int i=0; i<2; ++i) {
              LaserBit::create(m_core, m_core->getPosition(), tar[i], m_dir, 60);
            }
          }

          if(++m_count==m_n) {
            SendKillMessage(0, this);
          }
        }
      }
    };

    class PararelLaser12 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      vector4 m_vert;
      int m_frame;
      int m_count;

    public:
      PararelLaser12(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_vert >> m_frame >> m_count;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_vert << m_frame << m_count;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser12(Core *core, const vector4& target) :
        m_core(core), m_frame(0), m_count(0)
      {
        m_dir = (target-m_core->getPosition()).setZ(0).normal();
        m_vert = matrix44().rotateZ(90)*m_dir;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%8==1) {
          vector4 center = m_core->getPosition();
          vector4 tar = center-m_dir*(GetRand()*150.0f)+m_vert*(GetRand()*800.0f-400.0f);
          LaserBit::create(m_core, m_core->getPosition(), tar, m_dir, 80);

        }
        if(m_frame==350) {
          SendKillMessage(0, this);
        }
      }
    };

    class PararelLaser13 : public Optional
    {
    typedef Optional Super;
    protected:
      Core *m_core;
      vector4 m_dir;
      int m_frame;
      int m_count;
      bool m_reverse;

    public:
      PararelLaser13(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_core);
        s >> m_dir >> m_frame >> m_count >> m_reverse;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_core);
        s << m_dir << m_frame << m_count << m_reverse;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_core);
      }

    public:
      PararelLaser13(Core *core, const vector4& target, bool reverse) :
        m_core(core), m_frame(0), m_count(0), m_reverse(reverse)
      {
        m_dir = matrix44().rotateZ(reverse ? 90 : -90)*(target-m_core->getPosition()).setZ(0).normal();
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getLife()<=0.0f) {
          SendKillMessage(0, this);
        }

        ++m_frame;
        if(m_frame%5==1) {
          vector4 center = m_core->getPosition()+m_dir*80.0f;
          LaserBit::create(m_core, m_core->getPosition(), center, m_dir, 40);
          m_dir = matrix44().rotateZ(m_reverse ? -15 : 15)*m_dir;

          if(++m_count==24) {
            SendKillMessage(0, this);
          }
        }
      }
    };


  private:
    enum {
      APPEAR,
      ACTION1,
      TO_ACTION2,
      ACTION2,
      DESTROY,
    };

    RotLayer *m_root_layer;
    ChildRotLayer *m_boss_layer;
    GuardLayer *m_gl;
    GuardLayer2 *m_gl2;
    Core *m_core;
    Guard *m_guard[4];
    int m_time;
    gid m_id_blocks;
    int m_frame;
    int m_block_frame;
    int m_block_count;
    vector4 m_core_pos;
    vector4 m_scroll;
    int m_action;

    vector4 m_block_dir;
    random_ptr m_brand;

    vector4 m_initial_pos;
    vector4 m_target_pos;

  public:
    Boss(Deserializer& s) : Super(s)
    {
      m_brand = new ist::Random(2);

      DeserializeLinkage(s, m_root_layer);
      DeserializeLinkage(s, m_boss_layer);
      DeserializeLinkage(s, m_gl);
      DeserializeLinkage(s, m_gl2);
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_guard);
      s >> m_time >> m_id_blocks >> m_frame >> m_block_frame >> m_block_count
        >> m_core_pos >> m_scroll >> m_action >> m_block_dir >> (*m_brand)
        >> m_initial_pos >> m_target_pos;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_root_layer);
      SerializeLinkage(s, m_boss_layer);
      SerializeLinkage(s, m_gl);
      SerializeLinkage(s, m_gl2);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_guard);
      s << m_time << m_id_blocks << m_frame << m_block_frame << m_block_count
        << m_core_pos << m_scroll << m_action << m_block_dir << (*m_brand)
        << m_initial_pos << m_target_pos;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_root_layer);
      ReconstructLinkage(m_boss_layer);
      ReconstructLinkage(m_gl);
      ReconstructLinkage(m_gl2);
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_guard);
    }

  private:
    float brand() { return m_brand->genReal()*2.0f-1.0f; }

    MediumBlock* putMediumBlock(const vector4& pos)
    {
      MediumBlock *e = new MediumBlock();
      e->setPosition(pos);
      e->setVel(m_block_dir+vector4(brand(), 0.0f, 0));
      e->setAccel(m_block_dir*0.02f);
      e->setAxis(vector4(brand(), brand(), brand()).normalize());
      return e;
    }

    LargeBlock* putLargeBlock(const vector4& pos)
    {
      LargeBlock *e = new LargeBlock();
      e->setPosition(pos);
      e->setVel(m_block_dir);
      e->setAccel(m_block_dir*0.02f);
      e->setAxis(vector4(brand(), brand(), brand()).normalize());
      return e;
    }


  public:
    Boss() :
      m_time(60*180), m_frame(0), m_block_frame(0), m_block_count(0),
      m_root_layer(0), m_boss_layer(0), m_gl(0), m_gl2(0),
      m_core(0), m_action(APPEAR)
    {
      m_brand = new ist::Random(2);

      ZeroClear(m_guard);
      m_block_dir = vector4(0, -1, 0).normal();

      m_id_blocks = Solid::createGroupID();
      m_core_pos = vector4(550, 0, 0);
      {
        m_root_layer = new RotLayer();
        m_root_layer->chain();

        m_boss_layer = new ChildRotLayer();
        m_boss_layer->setParent(m_root_layer);
        m_boss_layer->setPosition(m_core_pos);

        m_core = new Core();
        m_core->setParent(m_boss_layer);
      }
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
      m_scroll+=GetGlobalScroll();
      if(fabsf(m_scroll.y)>60.0f) {
        m_scroll.y+=m_scroll.y<0.0f ? 60.0f : -60.0f;
        ++m_block_count;
        for(size_t i=0; i<17; ++i) {
          if((m_action==TO_ACTION2 || m_action==ACTION2) && (
             (m_block_count%10==0 && (i==5 || i==11)) ||
             (m_block_count%10==5 && (i==4 || i==12)) )) {
            CellGround *e = new CellGround();
            e->setParent(m_root_layer);
            e->setGroup(m_id_blocks);
            e->setPosition(vector4(-480.0f+60.0f*i, m_scroll.y-540.0f, 0));
          }
          else {
            CellBlock *e = new CellBlock();
            e->setParent(m_root_layer);
            e->setGroup(m_id_blocks);
            e->setPosition(vector4(-480.0f+60.0f*i, m_scroll.y-540.0f, 0));
          }
        }
      }
    }


    void appear()
    {
      if(m_block_frame%220==0) {
        putLargeBlock(vector4(brand()*270.0f, 500.0f+brand()*30.0f, 0.0f));
      }
      if(m_block_frame%200==0) {
        putMediumBlock(vector4(brand()*350.0f, 480.0f+brand()*30.0f, 0.0f));
      }

      if(m_frame<=180) {
        float q = Sin90((1.0f/180.0f)*m_frame);
        vector4 pos = vector4(550, 0, 0)+(vector4(250, 0, 0)-vector4(550, 0, 0))*q;
        m_boss_layer->setPosition(pos);
      }
      if(m_frame==180) {
        m_gl = new GuardLayer();
        m_gl->setParent(m_boss_layer);
        m_gl2 = new GuardLayer2();
        m_gl2->setParent(m_gl);

        box b[4] = {
          box(vector4(25,170,10), vector4(-25,40,-10)),
          box(vector4(25,-170,10), vector4(-25,-40,-10)),
          box(vector4(130,25,10), vector4(55,-25,-10)),
          box(vector4(-130,25,10), vector4(-55,-25,-10)),
        };
        for(int i=0; i<2; ++i) {
          m_guard[i] = new Guard(m_gl, m_core);
          m_guard[i]->setBox(b[i]);
          m_guard[i]->start();
        }
        for(int i=2; i<4; ++i) {
          m_guard[i] = new Guard(m_gl2, m_core);
          m_guard[i]->setBox(b[i]);
          m_guard[i]->start();
        }
      }
      else if(m_frame==240) {
        m_gl2->start();
        m_gl->start();
      }

      if(m_frame==240) {
        m_frame = 0;
        m_action = ACTION1;
      }
    }

    void action1()
    {
      if(m_block_frame%220==0) {
        putLargeBlock(vector4(brand()*270.0f, 500.0f+brand()*30.0f, 0.0f));
      }
      if(m_block_frame%200==0) {
        putMediumBlock(vector4(brand()*350.0f, 480.0f+brand()*30.0f, 0.0f));
      }

      if(m_frame==1) {
        m_core_pos = m_core->getPosition();
        SetGlobalScroll(vector4(0.0f, 0.7f, 0.0f));
      }
      if(m_frame==120) {
        m_core->setInvincible(false);
      }

      m_core_pos = matrix44().rotateZ(-0.1f)*m_core_pos;
      m_boss_layer->move((m_core_pos-m_core->getPosition())*0.05f);

      const int phase1 = 460;
      const int phase2 = phase1+460;
      const int phase3 = phase2+550;
      const int phase4 = phase3+460;
      const int phase5 = phase4+460;
      const int phase6 = phase5+550;
      int f = m_frame%(phase6+1);
      if     (f==phase1) { new PararelLaser1(m_core, GetNearestPlayerPosition(getPosition()), false); }
      else if(f==phase2) { new PararelLaser1(m_core, GetNearestPlayerPosition(getPosition()), true); }
      else if(f==phase3) { new PararelLaser2(m_core, GetNearestPlayerPosition(getPosition())); }
      else if(f==phase4) { new PararelLaser3(m_core, GetNearestPlayerPosition(getPosition()), false); }
      else if(f==phase5) { new PararelLaser3(m_core, GetNearestPlayerPosition(getPosition()), true); }
      else if(f==phase6) { new PararelLaser4(m_core, GetNearestPlayerPosition(getPosition())); }

      if(m_core->getLife() < m_core->getMaxLife()*0.35f) {
        m_frame = 0;
        m_action = TO_ACTION2;
        m_core->setInvincible(true);
      }
    }

    void to_action2()
    {
      if(m_block_frame%220==0) {
        putLargeBlock(vector4(brand()*270.0f, 500.0f+brand()*30.0f, 0.0f));
      }
      if(m_block_frame%200==0) {
        putMediumBlock(vector4(brand()*350.0f, 480.0f+brand()*30.0f, 0.0f));
      }

      if(m_frame==1) {
        m_initial_pos = m_boss_layer->getRelativePosition();
        m_target_pos = vector4(0,-150,0);
      }
      if(m_frame<=240) {
        vector4 pos = m_initial_pos+(m_target_pos-m_initial_pos)*Cos180I((1.0f/240.0f)*m_frame);
        m_boss_layer->setPosition(pos);
      }
      if(m_frame==240) {
        m_frame = 0;
        m_action = ACTION2;
      }
    }

    void action2()
    {
      if(m_block_frame%220==0) {
        putLargeBlock(vector4(brand()*270.0f, 500.0f+brand()*30.0f, 0.0f));
      }
      if(m_block_frame%200==0) {
        putMediumBlock(vector4(brand()*350.0f, 480.0f+brand()*30.0f, 0.0f));
      }

      if(m_frame==100) {
        m_core->setInvincible(false);
      }
      if(m_frame>=480) {
        float v = GetGlobalScroll().y;
        v+=(0.45f-v)*0.01f;
        SetGlobalScroll(vector4(0.0f, v, 0.0f));
      }

      float rot = m_root_layer->getRotateSpeed();
      m_root_layer->setRotateSpeed(rot+(0.04f-rot)*0.001f);

      const int phase1 = 250;
      const int phase2 = phase1+220;
      const int phase3 = phase2+220;
      const int phase4 = phase3+500;
      const int phase5 = phase4+800;
      const int phase6 = phase5+360;
      int f = m_frame%(phase6+1);
      if     (f==phase1) { new PararelLaser11(m_core, GetNearestPlayerPosition(getPosition()), 50, 3); }
      else if(f==phase2) { new PararelLaser11(m_core, GetNearestPlayerPosition(getPosition()), 50, 6); }
      else if(f==phase3) { new PararelLaser11(m_core, GetNearestPlayerPosition(getPosition()), 50, 9); }
      else if(f==phase4) { new PararelLaser12(m_core, GetNearestPlayerPosition(getPosition())); }
      else if(f==phase5) { new PararelLaser13(m_core, GetNearestPlayerPosition(getPosition()), false); }
      else if(f==phase6) { new PararelLaser13(m_core, GetNearestPlayerPosition(getPosition()), true); }
    }


    void clearEnemy()
    {
      gobj_iter& i = GetAllObjects();
      while(i.has_next()) {
        gobj_ptr p = i.iterate();
        if(enemy_ptr e = ToEnemy(p)) {
          if(e!=m_core && !dynamic_cast<Guard*>(e)) {
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
      if(m_frame==1) {
        InvincibleAllPlayers(600);
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60 ? 1 : 0));
        IMusic::FadeOut(6000);
      }

      m_root_layer->setRotateSpeed(m_root_layer->getRotateSpeed()*0.99f);
      m_gl->setRotateSpeed(m_gl->getRotateSpeed()*0.99f);
      m_gl2->setRotateSpeed(m_gl2->getRotateSpeed()*0.99f);
      m_core->setRotateSpeed(m_core->getRotateSpeed()*0.99f);
      m_core->setEmission(vector4(1.2f/400*m_frame, 0.5f/400.0f*m_frame, 0));
      if(m_frame%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(m_frame==240) { SendDestroyMessage(0, m_guard[2], 1); }
      if(m_frame==280) { SendDestroyMessage(0, m_guard[3], 1); }
      if(m_frame==320) { SendDestroyMessage(0, m_guard[1], 1); }
      if(m_frame==360) { SendDestroyMessage(0, m_guard[0], 1); }

      if(m_frame==400) {
        SendDestroyMessage(0, m_root_layer, 1);
        SendKillMessage(0, this);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_guard);

      ++m_frame;
      ++m_block_frame;

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

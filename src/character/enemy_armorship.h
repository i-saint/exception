#ifndef enemy_turtle_h
#define enemy_turtle_h

namespace exception {


  class ArmorShip : public Inherit5(HaveBoxBound, HaveVelocity, HaveDirection, HaveControler, Layer)
  {
  typedef Inherit5(HaveBoxBound, HaveVelocity, HaveDirection, HaveControler, Layer) Super;
  public:

    class Core : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      vector4 m_emission;

    public:
      Core(Deserializer& s) : Super(s)
      {
        s >> m_emission;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_emission;
      }

    public:
      Core() : m_emission(0.2f, 0.2f, 1.0f)
      {
        setBound(box(vector4(2000)));
        setEnergy(150.0f);
      }

      void setEmission(const vector4& v) { m_emission=v; }
      const vector4& getEmission() { return m_emission; }

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
      }

      int getFractionCount()
      {
        return 0;
      }

      void onCollide(CollideMessage& m)
      {
        if(IsGround(m.getFrom())) {
          SendDestroyMessage(m.getFrom(), this);
        }
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        if(m.getStat()==0) {
          PutSmallImpact(getCenter());
        }
      }

      void onAccel(AccelMessage& m)
      {
        SendAccelMessage(m.getFrom(), getParent(), m.getAccel());
      }
    };


    class Parts : public Inherit2(HaveDirection, ChildEnemy)
    {
    typedef Inherit2(HaveDirection, ChildEnemy) Super;
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
      const vector4& getEmission() { return m_emission; }

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.1f, 0.1f, 0.3f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.7f).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f).v);
        glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.2f).v);
      }

      void onDamage(DamageMessage& m)
      {
        gobj_ptr src = m.getSource();
        if(!dynamic_cast<Laser*>(src) &&
           !dynamic_cast<GLaser*>(src) &&
           !dynamic_cast<Ray*>(src) ) { // 自機の武装と敵レーザー無効 
           Super::onDamage(m);
        }
      }

      void onCollide(CollideMessage& m)
      {
        if(IsGround(m.getFrom())) {
          SendDestroyMessage(m.getFrom(), this);
        }
      }
    };

    class Fort : public Inherit3(HaveDirection, HaveParent, Optional)
    {
    typedef Inherit3(HaveDirection, HaveParent, Optional) Super;
    private:
      Laser *m_laser;
      gid m_group;
      vector4 m_initial_dir;
      int m_cooldown;
      float m_opa;
      float m_range;
      int m_action;

    public:
      Fort(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_laser);
        s >> m_group >> m_initial_dir >> m_cooldown >> m_opa >> m_range >> m_action;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_laser);
        s << m_group << m_initial_dir << m_cooldown << m_opa << m_range << m_action;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_laser);
      }

    public:
      Fort() :
        m_laser(0), m_group(0), m_initial_dir(1,0,0,0),
        m_cooldown(240), m_opa(0.0f), m_range(90.0f), m_action(0)
      {}

      void setCooldown(int v) { m_cooldown=v; }
      void setGroup(gid v) { m_group=v; }

      // これを基準に探索範囲を決定 
      void setInitialDirection(const vector4& v)
      {
        vector4 n = v.normal();
        n.w = 0;
        m_initial_dir = n;
        setDirection(n);
      }
      void setSearchRange(float v) { m_range=v; }

      void draw()
      {
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        glColor4f(1, 0, 0, m_opa);
        glBegin(GL_LINES);
        glVertex3fv(getPosition().v);
        glVertex3fv((getPosition()+getParentMatrix()*getDirection()*1000.0f).v);
        glEnd();
        glColor4f(1,1,1,1);

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }

      void wait()
      {
        --m_cooldown;
        if(m_cooldown==60 && m_laser) {
          m_laser->fade();
          m_opa = 0.0f;
        }
        if(m_cooldown<=0) {
          ++m_action;
          m_cooldown = 0;
        }
      }

      void aim()
      {
        matrix44 pmat = getParentMatrix();
        matrix44 ipmat = matrix44(pmat).transpose();
        vector4 v = (GetNearestPlayerPosition(getPosition())-getPosition()).normal();
        if(v.dot(pmat*m_initial_dir)<cosf(m_range*ist::radian)) { // 範囲外なら何もしない 
          return;
        }

        vector4 dir = pmat*getDirection();
        float dot = v.dot(dir);
        float dot2 = (matrix44().rotateZ(90.0f)*v).dot(dir);
        const float rot_speed = 0.4f;
        if( (dot>=0.0f && dot2<=0.0f)
          ||(dot<=0.0f && dot2<=0.0f)) {
          dir = matrix44().rotateZ(rot_speed)*dir;
        }
        else {
          dir = matrix44().rotateZ(-rot_speed)*dir;
        }
        setDirection(ipmat*dir);

        if(v.dot(dir)>cosf(rot_speed*ist::radian)) {
          ++m_action;
        }
      }

      void charge()
      {
        ++m_cooldown;
        if(m_cooldown==90) {
          ++m_action;
        }
        if(m_opa<1.0f) {
          m_opa+=0.02f;
        }
      }

      void fire()
      {
        if(m_laser) {
          return;
        }
        Laser *l = new Laser(getParent());
        if(!IsSolid(getParent())) {
          l->setGroup(m_group);
        }
        l->setParent(this);
        l->setRadius(30.0f);
        l->setPower(1.0f);
        l->setSpeed(20.0f);
        m_laser = l;
        m_cooldown = 150;
        m_action = 0;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        SweepDeadObject(m_laser);
        if(m_opa<0.2f) {
          m_opa+=0.001f;
        }

        switch(m_action) {
        case 0: wait();   break;
        case 1: aim();    break;
        case 2: charge(); break;
        case 3: fire();   break;
        }
      }
    };

    ArmorShip(Deserializer& s) : Super(s) {}
    ArmorShip() {}
  };


  class Turtle : public ArmorShip
  {
  typedef ArmorShip Super;
  private:
    static const int s_num_parts = 14;
    static const int s_num_fort = 4;
    Core *m_core;
    Parts *m_parts[s_num_parts];
    Fort *m_fort[s_num_fort];
    gid m_group;
    int m_frame;
    int m_deadcount;

  public:
    Turtle(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_core);
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_fort);
      s >> m_group >> m_frame >> m_deadcount;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_core);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_fort);
      s << m_group << m_frame << m_deadcount;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_core);
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_fort);
    }

  public:
    Turtle(controler_ptr c) : m_group(0), m_frame(0), m_deadcount(0)
    {
      setControler(c);
      setAccelResist(0.0f);
      setBound(box(vector4(1500)));

      {
        m_core = new Core();
        m_core->setBox(box(vector4(15)));
        m_core->setParent(this);
        m_core->setBound(box(vector4(2000)));
        m_core->setLife(40.0f);
      }
      {
        vector4 dir[] = {
          vector4( 1, 1,0),
          vector4( 1,-1,0),
          vector4(-1, 1,0),
          vector4(-1,-1,0),
        };
        box b[] = {
          box(vector4( 130, 110, 30), vector4( 70, 50, -30)), // 4隅 
          box(vector4( 130,-110, 30), vector4( 70,-50, -30)),
          box(vector4(-130, 110, 30), vector4(-70, 50, -30)),
          box(vector4(-130,-110, 30), vector4(-70,-50, -30)),

          box(vector4( 100, 50, 15), vector4( 60,-50, -15)), // 左右枠 
          box(vector4(-100, 50, 15), vector4(-60,-50, -15)),

          box(vector4( 70, 80, 15), vector4( 35, 50, -15)), // 上枠 
          box(vector4(-70, 80, 15), vector4(-35, 50, -15)),
          box(vector4( 50, 80, 15), vector4( -0,100, -15)),
          box(vector4(-50, 50, 15), vector4(  0, 30, -15)),

          box(vector4( 70,-80, 15), vector4( 35, -50, -15)), // 下枠 
          box(vector4(-70,-80, 15), vector4(-35, -50, -15)),
          box(vector4( 50,-80, 15), vector4( -0,-100, -15)),
          box(vector4(-50,-50, 15), vector4(  0, -30, -15)),
        };
        for(int i=0; i<s_num_parts; ++i) {
          Parts *p = new Parts();
          p->setBox(b[i]);
          p->setParent(this);
          p->setLife(20);
          m_parts[i] = p;

          if(i<s_num_fort) {
            p->setLife(40);

            Fort *f = new Fort();
            f->setCooldown(180);
            f->setParent(p);
            f->setPosition(b[i].getCenter());
            f->setInitialDirection(dir[i]);
            m_fort[i] = f;
          }
        }
      }

      setGroup(Solid::createGroupID());
    }

    float getDrawPriority() { return 1.1f; }

    gid getGroup() { return m_group; }
    void setGroup(gid g)
    {
      m_group = g;
      SetGroup(m_core, g);
      SetGroup(m_parts, g);
    }

    Core* getCore() { return m_core; }
    Parts* getParts(int i) { return m_parts[i]; }
    int getPartsCount() { return s_num_parts; }

    void draw()
    {
      Super::draw();

      const vector4 bpos[4] = {
        vector4( 120, 100, -35),
        vector4( 120,-100, -35),
        vector4(-120, 100, -35),
        vector4(-120,-100, -35),
      };
      for(int i=0; i<4; ++i) {
        if(m_parts[i]) {
          DrawSprite("burner.png",
            getMatrix()*bpos[i],
            vector4(70.0f+::sinf(34.0f*m_frame*ist::radian)*6.0f),
            1.0f);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_core);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_fort);

      ++m_frame;
      if(!m_core) {
        ++m_deadcount;
        for(int i=0; i<s_num_parts; ++i) {
          if(m_parts[i]) {
            m_parts[i]->setEmission(vector4(1.2f/100*m_deadcount, 0.5f/100*m_deadcount, 0));
          }
        }
        if(m_deadcount==100) {
          SendDestroyMessage(0, this);
        }
      }
      setPosition(getRelativePosition()+getVel());
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      if(m.getStat()==0) {
        PutBigImpact(getPosition());
      }
    }
  };

  class Turtle_Controler : public TControler<Turtle>
  {
  typedef TControler<Turtle> Super;
  public:
    Turtle_Controler(Deserializer& s) : Super(s) {}
    Turtle_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getVel, const vector4&);
    Getter(getDirection, const vector4&);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setVel, const vector4&);
    Setter(setDirection, const vector4&);
  };



  // 2面ラスト用 
  class Turtle_Wait : public Turtle_Controler
  {
  typedef Turtle_Controler Super;
  private:
    vector4 m_move;
    int m_frame;
    float m_length;
    bool m_scroll;

  public:
    Turtle_Wait(Deserializer& s) : Super(s)
    {
      s >> m_move >> m_frame >> m_length >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_move << m_frame << m_length << m_scroll;
    }

  public:
    Turtle_Wait(float length, bool scroll) : m_frame(0), m_length(length), m_scroll(scroll)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_move = getDirection()*m_length;
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f<=240) {
        float pq = Sin90(1.0f/240*(f-1));
        float q = Sin90(1.0f/240*f);
        setPosition(getRelativePosition()+m_move*(q-pq));
      }
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  // 4面ラスト用 
  class Turtle_Sliding : public Turtle_Controler
  {
  typedef Turtle_Controler Super;
  private:
    vector4 m_move;
    int m_frame;
    bool m_ih;

  public:
    Turtle_Sliding(Deserializer& s) : Super(s)
    {
      s >> m_move >> m_frame >> m_ih;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_move << m_frame << m_ih;
    }

  public:
    Turtle_Sliding(bool ih) : m_frame(0), m_ih(ih)
    {}

    void onConstruct(ConstructMessage& m)
    {
      setDirection(vector4(0,1,0));
      m_move = vector4(450*(m_ih?-1:1), 0, 0);
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f<=300) {
        vector4 pos = getRelativePosition();
        float pq = Sin90(1.0f/300*(f-1));
        float q = Sin90(1.0f/300*f);
        setPosition(pos+m_move*(q-pq));
      }
      setPosition(getRelativePosition()+getParentIMatrix()*(GetGlobalScroll()*0.5f));
    }
  };




  // 4ボス浮遊砲台 
  class FloatingTurret : public ArmorShip
  {
  typedef ArmorShip Super;
  public:
    class HeadParts : public Parts
    {
    typedef Parts Super;
    public:
      HeadParts(Deserializer& s) : Super(s) {}
      HeadParts() {}
      void onAccel(AccelMessage& m)
      {
        SendAccelMessage(m.getFrom(), getParent(), m.getAccel());
      }
    };

  private:
    Fort *m_fort;
    ChildRotLayer *m_fan_layer;
    HeadParts *m_head;
    Parts *m_parts[2];
    Parts *m_fan[4];
    gid m_group;

  public:
    FloatingTurret(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_fort);
      DeserializeLinkage(s, m_fan_layer);
      DeserializeLinkage(s, m_head);
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_fan);
      s >> m_group;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_fort);
      SerializeLinkage(s, m_fan_layer);
      SerializeLinkage(s, m_head);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_fan);
      s << m_group;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_fort);
      ReconstructLinkage(m_fan_layer);
      ReconstructLinkage(m_head);
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_fan);
    }

  public:
    FloatingTurret(controler_ptr c)
    {
      setControler(c);
      setAccelResist(0.3f);
      setBound(box(vector4(1500)));

      {
        m_head = new HeadParts();
        m_head->setParent(this);
        m_head->setBox(box(vector4(20, 10, 30), vector4(-15,-10,-10)));
        m_head->setLife(20.0f);
        m_head->setEnergy(50.0f);
      }
      {
        m_fort = new Fort();
        m_fort->setParent(this);
        m_fort->setCooldown(120);
        m_fort->setSearchRange(180.0f);
      }
      {
        box b[] = {
          box(vector4(10, 10, 25), vector4(-10, 15, -5)),
          box(vector4(10,-10, 25), vector4(-10,-15, -5)),
        };
        for(int i=0; i<2; ++i) {
          Parts *p = new Parts();
          p->setParent(m_head);
          p->setBox(b[i]);
          m_parts[i] = p;
        }
      }
      {
        m_fan_layer = new ChildRotLayer();
        m_fan_layer->setParent(this);

        vector4 d[] = {
          vector4( 1, 0,0),
          vector4( 0, 1,0),
          vector4(-1, 0,0),
          vector4( 0,-1,0),
        };
        for(int i=0; i<4; ++i) {
          Parts *p = new Parts();
          p->setParent(m_fan_layer);
          p->setBox(box(vector4(40, 7.5f,-10), vector4(20,-7.5f,-20)));
          p->setDirection(d[i]);
          m_fan[i] = p;
        }
      }

      setGroup(Solid::createGroupID());
    }

    gid getGroup() { return m_group; }

    void setGroup(gid g)
    {
      m_group = g;
      SetGroup(m_head, g);
      SetGroup(m_fort, g);
      SetGroup(m_fan, g);
      SetGroup(m_parts, g);
    }

    Parts* getHead() { return m_head; }
    ChildRotLayer* getFanLayer() { return m_fan_layer; }
    Fort* getFort() { return m_fort; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_head);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_fan);
      SweepDeadObject(m_fan_layer);

      if(m_fan_layer) {
        float r = m_fan_layer->getRotateSpeed();
        m_fan_layer->setRotateSpeed(r+(2.0f-r)*0.005f);
      }
      if(m_head && m_fort) {
        m_head->setDirection(m_fort->getDirection());
      }

      if(!m_head) {
        SendDestroyMessage(0, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }
  };

  class FloatingTurret_Controler : public TControler<FloatingTurret>
  {
  typedef TControler<FloatingTurret> Super;
  public:
    typedef FloatingTurret::Parts Parts;
    typedef FloatingTurret::Fort Fort;

    FloatingTurret_Controler(Deserializer& s) : Super(s) {}
    FloatingTurret_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getDirection, const vector4&);
    Getter(getVel, const vector4&);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setDirection, const vector4&);
    Setter(setVel, const vector4&);
    Setter(setAccel, const vector4&);
    Setter(setMaxSpeed, float);

    Getter(getHead, Parts*);
    Getter(getFort, Fort*);
    Getter(getFanLayer, ChildRotLayer*);
  };


  // 4ボス用 
  class FloatingTurret_Fall : public FloatingTurret_Controler
  {
  typedef FloatingTurret_Controler Super;
  private:
    vector4 m_move;
    int m_frame;

  public:
    FloatingTurret_Fall(Deserializer& s) : Super(s)
    {
      s >> m_move >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_move << m_frame;
    }

  public:
    FloatingTurret_Fall() : m_frame(0)
    {}

    void setMove(const vector4& v) { m_move=v; }

    void onConstruct(ConstructMessage& m)
    {
      setDirection(vector4(0,-1,0));
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f<=150) {
        vector4 pos = getRelativePosition();
        float pq = Sin90(1.0f/150*(f-1));
        float q = Sin90(1.0f/150*f);
        setPosition(pos+m_move*(q-pq));
      }
      if(f==150) {
        setAccel(vector4(0, -0.005f, 0));
      }
      else if(f==300) {
        setAccel(vector4(0, 0, 0));
      }
      setPosition(getRelativePosition()+getVel());
      KillIfOutOfScreen(get(), rect(vector2(150)));
    }
  };

  // 4ボス用その2 
  class FloatingTurret_Wait : public FloatingTurret_Controler
  {
  typedef FloatingTurret_Controler Super;
  private:
    vector4 m_move;
    int m_frame;

  public:
    FloatingTurret_Wait(Deserializer& s) : Super(s)
    {
      s >> m_move >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_move << m_frame;
    }

  public:
    FloatingTurret_Wait() : m_frame(0)
    {
      m_move = vector4(0,300,0);
    }

    void setMove(const vector4& v) { m_move=v; }

    void onConstruct(ConstructMessage& m)
    {
      setDirection(vector4(0,1,0));
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f<=150) {
        vector4 pos = getRelativePosition();
        float pq = Sin90(1.0f/150*(f-1));
        float q = Sin90(1.0f/150*f);
        setPosition(pos+vector4(0,200,0)*(q-pq));
      }
      setPosition(getRelativePosition()+getVel());
    }
  };
}
#endif

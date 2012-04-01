#ifndef enemy_shell_h
#define enemy_shell_h

namespace exception {



  // 親オブジェクトにダメージを丸投げする 
  template<class T>
  class HaveMarunageAttrib : public T
  {
  typedef T Super;
  public:
    HaveMarunageAttrib(Deserializer& s) : Super(s) {}
    HaveMarunageAttrib() {}
    void drawLifeGauge() {}

    float getDeltaDamage()
    {
      if(BasicEnemy *p = dynamic_cast<BasicEnemy*>(this->getParent())) {
        return p->getDeltaDamage();
      }
      return 0.0f;
    }

    void onDamage(DamageMessage& m)
    {
      if(gobj_ptr p = this->getParent()) {
        this->SendDamageMessage(m.getFrom(), p, m.getDamage());
      }
    }
  };

  class Shell : public Inherit3(HaveVelocity, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveVelocity, HaveDirection, Enemy) Super;
  public:
    class Parts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts() {}
      void onCollide(CollideMessage& m)
      {
        if(IsGround(m.getFrom())) {
          SendDestroyMessage(0, this);
        }
        else {
          Scratch(this, m, 0.1f);
        }
      }
    };

    typedef HaveMarunageAttrib<Parts> DummyParts;

  private:
    static const int s_num_parts = 2;
    enum {
      NONE,
      OPEN,
      CLOSE,
    };

    Parts *m_shell[s_num_parts];
    DummyParts *m_shells[s_num_parts];
    int m_frame;
    int m_action;

  public:
    Shell(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_shell);
      DeserializeLinkage(s, m_shells);
      s >> m_frame >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_shell);
      SerializeLinkage(s, m_shells);
      s << m_frame << m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_shell);
      ReconstructLinkage(m_shells);
    }

  public:
    Shell(controler_ptr c) : m_frame(0), m_action(0)
    {
      setControler(c);
      setBox(box(vector4(10)));
      setLife(5.0f);
      setEnergy(20.0f);

      box bs[s_num_parts] = {
        box(vector4(15.0f, 20, 20), vector4(-17.5f, 0, -20)),
        box(vector4(15.0f,-20, 20), vector4(-17.5f, 0, -20)),
      };
      box bss[s_num_parts] = {
        box(vector4(35.0f, 15, 18), vector4(15.0f, 0, -18)),
        box(vector4(35.0f,-15, 18), vector4(15.0f, 0, -18)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        {
          Parts *s = new Parts();
          s->setParent(this);
          s->setBox(bs[i]);
          s->setLife(12.0f);
          m_shell[i] = s;
        }
        {
          DummyParts *s = new DummyParts();
          s->setParent(m_shell[i]);
          s->setBox(bss[i]);
          m_shells[i] = s;
        }
      }
      setGroup(getGroup());
    }

    float getDrawPriority() { return 1.1f; }

    void open()  { m_action = OPEN; }
    void close() { m_action = CLOSE; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_shell, v);
      SetGroup(m_shells, v);
    }

    void draw()
    {
      Super::draw();
      DrawSprite("burner.png",
        getMatrix()*vector4(-20.0f, 0, 0),
        vector4(35.0f+::sinf(53.0f*m_frame*ist::radian)*3.0f),
        1.0f);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_shell);
      SweepDeadObject(m_shells);

      ++m_frame;
      switch(m_action) {
      case OPEN:  _open(); break;
      case CLOSE: _close(); break;
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(IsGround(m.getFrom())) {
        SendDestroyMessage(0, this);
      }
      else {
        Scratch(this, m, 0.1f);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }


  private:
    void _open()
    {
      for(int i=0; i<s_num_parts; ++i) {
        if(m_shell[i]) {
          vector4 pos = m_shell[i]->getRelativePosition();
          if(i==0) {
            pos.y = std::min<float>(12.0f, pos.y+1.0f);
          }
          else if(i==1) {
            pos.y = std::max<float>(-12.0f, pos.y-1.0f);
          }
          if(fabsf(pos.y)==20.0f) { m_action=0; }
          m_shell[i]->setPosition(pos);
        }
      }
    }

    void _close()
    {
      for(int i=0; i<s_num_parts; ++i) {
        if(m_shell[i]) {
          vector4 pos = m_shell[i]->getRelativePosition();
          if(i==0) {
            pos.y = std::max<float>(0.0f, pos.y-1.0f);
          }
          else if(i==1) {
            pos.y = std::min<float>(0.0f, pos.y+1.0f);
          }
          if(pos.y==0.0f) { m_action=0; }
          m_shell[i]->setPosition(pos);
        }
      }
    }
  };



  class Shell_Controler : public TControler<Shell>
  {
  typedef TControler<Shell> Super;
  public:
    Shell_Controler(Deserializer& s) : Super(s) {}
    Shell_Controler() {}

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

    Caller(open);
    Caller(close);
  };

  // 入場→貫通弾3連射→そのまま直進して退場 
  class Shell_Blaster : public Shell_Controler
  {
  typedef Shell_Controler Super;
  private:
    int m_frame;
    int m_bullet;

  public:
    Shell_Blaster(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_bullet;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_bullet;
    }

  public:
    Shell_Blaster() : m_frame(0), m_bullet(3)
    {}

    void onConstruct(ConstructMessage& m)
    {
      setVel(getDirection()*1.3f);
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;

      setPosition(getRelativePosition()+getVel());

      const int phase1 = 250;
      const int phase2 = phase1+300;

      if(f<phase1) {
      }
      else if(f<phase2) {
        if(f==phase1) {
          open();
        }
        setVel(getDirection()*0.4f);

        if(f%80==40 && m_bullet) {
          Blaster *b = new Blaster(get(), getDirection());
          b->setPosition(getPosition()+getDirection()*15.0f);
          --m_bullet;
        }
      }
      else {
        close();
        setVel(getDirection()*2.0f);
      }
    }
  };


  class Shell_Launcher : public Shell_Controler
  {
  typedef Shell_Controler Super;
  protected:
    int m_frame;
    float m_length;
    vector4 m_back;
    bool m_scroll;
    int m_action;

  public:
    Shell_Launcher(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_length >> m_back >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_length << m_back << m_scroll << m_action;
    }

  public:
    Shell_Launcher(bool scroll=false) :
      m_frame(0), m_length(100.0f), m_scroll(scroll), m_action(0)
    {}

    void setLength(float v) { m_length=v; }

    void move()
    {
      int f = ++m_frame;
      int end = int(m_length/3.0f);
      vector4 move = getDirection()*m_length;
      float pq = Sin90(1.0f/end*(f-1));
      float q = Sin90(1.0f/end*f);
      setPosition(getRelativePosition()+move*(q-pq));

      if(f>=end) {
        setGroup(Solid::createGroupID());
        ++m_action;
        m_frame = 0;
      }
    }

    void aim()
    {
      int f = ++m_frame;
      vector4 dir = getParentIMatrix()*(GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      setDirection(getDirection()+dir*0.1f);
      if(f==30) {
        open();
      }
      if(f>=60) {
        ++m_action;
        m_frame = 0;
      }
    }

    virtual void createBullet()=0;

    void launch()
    {
      createBullet();
      ++m_action;
    }

    void wait()
    {
      int f = ++m_frame;
      if(f==30) {
        close();
      }
      if(f>=30) {
        m_back = matrix44().rotateZ(120.0f)*getDirection();
        ++m_action;
        m_frame = 0;
      }
    }

    void turn()
    {
      int f = ++m_frame;
      setDirection(getDirection()+m_back*0.1f);
      if(f==60) {
        ++m_action;
        m_frame = 0;
      }
    }

    void away()
    {
      setVel(getVel()+getDirection()*0.1f);
    }
      
    void onUpdate(UpdateMessage& m)
    {
      switch(m_action) {
      case 0: move();   break;
      case 1: aim();    break;
      case 2: launch(); break;
      case 3: wait();   break;
      case 4: turn();   break;
      case 5: away();   break;
      }

      setPosition(getRelativePosition()+getVel());
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };


  class Shell_BurstMissile : public Shell_Launcher
  {
  typedef Shell_Launcher Super;
  public:
    Shell_BurstMissile(Deserializer& s) : Super(s) {}
    Shell_BurstMissile(bool scroll) : Shell_Launcher(scroll) {}
    void createBullet()
    {
      Missile *b = new BurstMissile(m_scroll);
      b->setGroup(getGroup());
      b->setPosition(getPosition());
      b->setDirection(getDirection());
    }
  };

  class Shell_GravityMissile : public Shell_Launcher
  {
  typedef Shell_Launcher Super;
  public:
    Shell_GravityMissile(Deserializer& s) : Super(s) {}
    Shell_GravityMissile(bool scroll) : Shell_Launcher(scroll) {}
    void createBullet()
    {
      Missile *b = new GravityMissile(m_scroll);
      b->setGroup(getGroup());
      b->setPosition(getPosition());
      b->setDirection(getDirection());
    }
  };
}
#endif

#ifndef enemy_egg_h
#define enemy_egg_h

namespace exception {


  class Egg : public Inherit3(HaveVelocity, HaveDirection, Enemy)
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
        solid_ptr s = ToSolid(m.getFrom());
        if(s && s->getVolume()>=1000000.0f) {
          SendDestroyMessage(s, this);
        }
        else {
          Scratch(this, m, 0.1f);
        }
      }
    };

  private:
    static const int s_num_layer = 2;
    static const int s_num_parts = 6;
    enum {
      NONE = 0,
      OPEN = 1,
      CLOSE = 2,
    };

    ChildLayer *m_layer[s_num_layer];
    Parts *m_parts[s_num_parts];
    int m_frame;
    int m_action;

  public:
    Egg(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_layer);
      DeserializeLinkage(s, m_parts);
      s >> m_frame >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_layer);
      SerializeLinkage(s, m_parts);
      s << m_frame << m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_layer);
      ReconstructLinkage(m_parts);
    }

  public:
    Egg(controler_ptr c) : m_frame(0), m_action(0)
    {
      setControler(c);
      setBound(box(vector4(1500)));
      setBox(box(vector4(10)));
      setLife(5.0f);
      setEnergy(25.0f);
      setAccelResist(0.5f);

      for(int i=0; i<s_num_layer; ++i) {
        m_layer[i] = new ChildLayer();
        m_layer[i]->setParent(this);
        m_layer[i]->chain();
      }

      box b[s_num_parts] = {
        box(vector4( 15, 40, 25), vector4(-15, 10, -25)),
        box(vector4( 30, 30, 15), vector4( 10,  0, -15)),
        box(vector4(-30, 30, 15), vector4(-10,  0, -15)),

        box(vector4( 15,-40, 25), vector4(-15,-10, -25)),
        box(vector4( 30,-30, 15), vector4( 10,  0, -15)),
        box(vector4(-30,-30, 15), vector4(-10,  0, -15)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *a = new Parts();
        a->setParent(i<s_num_parts/2 ? m_layer[0] : m_layer[1]);
        a->setBox(b[i]);
        a->setLife(5.0f);
        m_parts[i] = a;
      }
      setGroup(getGroup());
    }

    void open() { m_action = OPEN; }
    void close() { m_action = CLOSE; }

    Parts* getParts(int i) { return m_parts[i]; }
    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }

    void draw()
    {
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.2f, 0.2f, 1.0f).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_parts);
      if(!AliveAny(m_parts)) {
        SendDestroyMessage(0, this);
        return;
      }

      ++m_frame;
      switch(m_action) {
      case OPEN: _open(); break;
      case CLOSE: _close(); break;
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(!AliveAny(m_parts)) {
        Scratch(this, m);
      }
    }

    void onKill(KillMessage& m)
    {
      Unchain(m_layer);
      Super::onKill(m);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }


  private:
    void _open()
    {
      for(int i=0; i<s_num_layer; ++i) {
        ChildLayer *l = m_layer[i];
        vector4 pos = l->getRelativePosition();
        pos.y+= i==0 ? 1.0f : -1.0f;
        if(fabsf(pos.y)>=25.0f) { m_action=0; }
        l->setPosition(pos);
      }
    }

    void _close()
    {
      for(int i=0; i<s_num_layer; ++i) {
        ChildLayer *l = m_layer[i];
        vector4 pos = l->getRelativePosition();
        if(i==0)      { pos.y = std::max<float>(0.0f, pos.y-1.0f); }
        else if(i==1) { pos.y = std::min<float>(0.0f, pos.y+1.0f); }
        if(pos.y==0.0f) { m_action=0; }
        l->setPosition(pos);
      }
    }
  };


  class Egg_Controler : public TControler<Egg>
  {
  typedef TControler<Egg> Super;
  public:
    Egg_Controler(Deserializer& s) : Super(s) {}
    Egg_Controler() {}

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

    Getter2(getParts, Egg::Parts*, int);
  };


  // ‰ñ“]‚µ‚È‚ª‚ç“Ë‚Áž‚ñ‚Å‚«‚Ä‹@—‹’u‚¢‚Ä‹Ž‚Á‚Ä‚¢‚­ 
  class Egg_Mine : public Egg_Controler
  {
  typedef Egg_Controler Super;
  protected:
    int m_frame;
    float m_length;
    vector4 m_move;
    vector4 m_initial_dir;
    bool m_scroll;
    int m_action;

  public:
    Egg_Mine(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_length >> m_move >> m_initial_dir >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_length << m_move << m_initial_dir << m_scroll << m_action;
    }

  public:
    Egg_Mine(bool scroll) : m_frame(0), m_length(300.0f), m_scroll(scroll), m_action(0)
    {}

    void setLength(float v) { m_length=v; }

    void onConstruct(ConstructMessage& m)
    {
      m_move = getDirection()*m_length;
      m_initial_dir = getDirection();
    }

    void move()
    {
      int f = ++m_frame;
      float pq = Sin90(1.0f/180.0f*(f-1));
      float q = Sin90(1.0f/180.0f*f);
      setPosition(getRelativePosition()+m_move*(q-pq));
      setDirection(matrix44().rotateZ(q*720.0f)*m_initial_dir);

      if(m_frame==180) {
        ++m_action;
        m_frame = 0;
      }
    }

    virtual void launch()
    {
      vector4 mpos[2] = {vector4(0, 25, 0), vector4(0, -25, 0)};
      int parts[2] = {0, 3};
      for(int i=0; i<2; ++i) {
        if(getParts(parts[i])) {
          MiniBurstMine *e = new MiniBurstMine();
          e->setParent(getParent());
          e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
          e->setGroup(getGroup());
        }
      }
    }

    void attack()
    {
      int f = ++m_frame;
      if(f==1) {
        open();
        launch();
      }
      else if(f==120) {
        close();
      }
      else if(f==220) {
        m_move = getDirection()*-300;
        ++m_action;
        m_frame = 0;
      }
    }

    void away()
    {
      int f = ++m_frame;
      float pq = Cos90I(1.0f/120.0f*(f-1));
      float q = Cos90I(1.0f/120.0f*f);
      setPosition(getRelativePosition()+m_move*(q-pq));

      if(f==120) {
        SendKillMessage(0, get());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      switch(m_action) {
      case 0: move();   break;
      case 1: attack(); break;
      case 2: away();   break;
      }
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  class Egg_Missile : public Egg_Mine
  {
  typedef Egg_Mine Super;
  public:
    Egg_Missile(Deserializer& s) : Super(s) {}
    Egg_Missile(bool scroll) : Egg_Mine(scroll)
    {}

    void launch()
    {
      vector4 mpos[2] = {vector4(-15, 25, 0), vector4(-15, -25, 0)};
      int parts[2] = {0, 3};
      for(int i=0; i<2; ++i) {
        if(getParts(parts[i])) {
          Missile *e = new BurstMissile(m_scroll);
          e->setParent(getParent());
          e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
          e->setDirection(getDirection());
          e->setGroup(getGroup());
        }
      }
    }
  };

  class Egg_Laser : public Egg_Mine
  {
  typedef Egg_Mine Super;
  public:
    Egg_Laser(Deserializer& s) : Super(s) {}
    Egg_Laser(bool scroll) : Egg_Mine(scroll)
    {}

    void launch()
    {
      vector4 mpos[2] = {vector4(-15, 25, 0), vector4(-15, -25, 0)};
      int parts[2] = {0, 3};
      for(int i=0; i<2; ++i) {
        if(getParts(parts[i])) {
          vector4 pos = getParentIMatrix()*getMatrix()*mpos[i];
          vector4 dir = getDirection();
          LaserBit::create(get(), pos, pos+dir*60.0f, dir, 40);
        }
      }
    }
  };

}
#endif

#ifndef enemy_hatch_h
#define enemy_hatch_h

namespace exception {


  class HatchBase : public Inherit2(HaveDirection, ChildEnemy)
  {
  typedef Inherit2(HaveDirection, ChildEnemy) Super;
  public:
    HatchBase(Deserializer& s) : Super(s) {}
    HatchBase() {}
    virtual void open()=0;
    virtual void close()=0;
  };


  class SmallHatch : public HatchBase
  {
  typedef HatchBase Super;
  private:
    static const int s_block_count = 7;
    enum {
      NONE,
      OPEN,
      CLOSE,
    };

    BoxModel *m_model[s_block_count];
    int m_action;
    int m_act_frame;
    bool m_opened;

  public:
    SmallHatch(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_model);
      s >> m_action >> m_act_frame >> m_opened;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_model);
      s << m_action << m_act_frame << m_opened;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_model);
    }

  public:
    SmallHatch(controler_ptr c=0) :  m_action(0), m_act_frame(0), m_opened(false)
    {
      setControler(c);
      setBox(box(vector4(50, 30, 15), vector4(0, -30, -15)));
      setLife(30.0f);
      setEnergy(40.0f);
      setDirection(vector4(0,1,0));
      setBound(box(vector4(1500)));

      box b[] = {
        box(vector4( 40, 20, 15), vector4( 0,-20,  12)),
        box(vector4( 50, 30,-15), vector4( 0,-30, -12)),
        box(vector4( 50, 20, 10), vector4(30,-20, -12)),
        box(vector4(  5, 20, 12), vector4( 0,-20, -12)),
        box(vector4( 50, 30, 12), vector4( 0, 20, -12)),
        box(vector4( 50,-30, 12), vector4( 0,-20, -12)),
        box(vector4( 25, 35,  8), vector4( 0,-35,  -8)),
      };
      for(int i=0; i<s_block_count; ++i) {
        BoxModel *db = new BoxModel(this);
        db->setBox(b[i]);
        m_model[i] = db;
      }
    }

    void drawModel()
    {
      // –{‘Ì‚Í•`‚©‚È‚¢ 
      for(size_t i=0; i<s_block_count; ++i) {
        m_model[i]->draw();
      }
    }


    void open()
    {
      if(!m_opened) {
        m_action = OPEN;
        m_opened = true;
      }
    }

    void close()
    {
      if(m_opened) {
        m_action = CLOSE;
        m_opened = false;
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      switch(m_action) {
      case OPEN:  _open();  break;
      case CLOSE: _close(); break;
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(IsGround(m.getFrom())) {
        SendDestroyMessage(m.getFrom(), this);
      }
      else {
        Scratch(this, m, 0.2f);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      if(m.getStat()==0) {
        PutSmallImpact(getPosition());
      }
    }

  private:
    void _open()
    {
      ++m_act_frame;
      vector4 pos = m_model[2]->getRelativePosition();
      pos.z = Sin90((1.0f/30.0f)*m_act_frame)*25.0f;
      m_model[2]->setPosition(pos);
      if(m_act_frame>=30) {
        m_act_frame = 0;
        m_action = 0;
      }
    }

    void _close()
    {
      ++m_act_frame;
      vector4 pos = m_model[2]->getRelativePosition();
      pos.z = Cos90((1.0f/30.0f)*m_act_frame)*25.0f;
      m_model[2]->setPosition(pos);
      if(m_act_frame>=30) {
        m_act_frame = 0;
        m_action = 0;
      }
    }
  };




  class LargeHatch : public HatchBase
  {
  typedef HatchBase Super;
  private:
    static const int s_block_count = 7;
    enum {
      NONE,
      OPEN,
      CLOSE,
    };

    BoxModel *m_model[s_block_count];
    int m_action;
    int m_act_frame;
    bool m_opened;

  public:
    LargeHatch(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_model);
      s >> m_action >> m_act_frame >> m_opened;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_model);
      s << m_action << m_act_frame << m_opened;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_model);
    }

  public:
    LargeHatch(controler_ptr c=0) : m_action(0), m_act_frame(0), m_opened(false)
    {
      setControler(c);
      setBox(box(vector4(100, 50, 15), vector4(0, -50, -15)));
      setLife(80.0f);
      setEnergy(80.0f);
      setDirection(vector4(0,1,0));
      setBound(box(vector4(1500)));

      box b[] = {
        box(vector4( 90, 40, 21), vector4( 0,-40, 12)),
        box(vector4(100, 50,-15), vector4( 0,-50,-12)),
        box(vector4(100, 40, 18), vector4(90,-40,-12)),
        box(vector4( 10, 40, 18), vector4( 0,-40,-12)),
        box(vector4(100, 50, 18), vector4( 0, 40,-12)),
        box(vector4(100,-50, 18), vector4( 0,-40,-12)),
        box(vector4( 65, 35, 25), vector4( 0,-35, 15)),
      };
      for(int i=0; i<s_block_count; ++i) {
        BoxModel *db = new BoxModel(this);
        db->setBox(b[i]);
        m_model[i] = db;
      }
    }

    void drawModel()
    {
      // –{‘Ì‚Í•`‚©‚È‚¢ 
      for(size_t i=0; i<s_block_count; ++i) {
        m_model[i]->draw();
      }
    }


    void open()
    {
      if(!m_opened) {
        m_action = OPEN;
        m_opened = true;
      }
    }

    void close()
    {
      if(m_opened) {
        m_action = CLOSE;
        m_opened = false;
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      switch(m_action) {
      case OPEN:  _open();  break;
      case CLOSE: _close(); break;
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(IsGround(m.getFrom())) {
        SendDamageMessage(m.getFrom(), this, 3.0f);
      }
      else {
        Scratch(this, m, 0.1f);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }

  private:
    void _open()
    {
      ++m_act_frame;
      vector4 pos = m_model[2]->getRelativePosition();
      pos.z = Sin90((1.0f/45.0f)*m_act_frame)*25.0f;
      m_model[2]->setPosition(pos);
      if(m_act_frame>=45) {
        m_act_frame = 0;
        m_action = 0;
      }
    }

    void _close()
    {
      ++m_act_frame;
      vector4 pos = m_model[2]->getRelativePosition();
      pos.z = Cos90((1.0f/45.0f)*m_act_frame)*25.0f;
      m_model[2]->setPosition(pos);
      if(m_act_frame>=45) {
        m_act_frame = 0;
        m_action = 0;
      }
    }
  };



  class Hatch_Controler : public TControler<HatchBase>
  {
  typedef TControler<HatchBase> Super;
  public:
    Hatch_Controler(Deserializer& s) : Super(s) {}
    Hatch_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getCenter, vector4);
    Getter(getPosition, const vector4&);
    Getter(getDirection, const vector4&);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setDirection, const vector4&);

    Caller(open);
    Caller(close);
  };


  class Hatch_GenRushFighter : public Hatch_Controler
  {
  typedef Hatch_Controler Super;
  private:
    int m_frame;
    int m_interval;
    int m_wait;
    bool m_scroll;

  public:
    Hatch_GenRushFighter(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_interval >> m_wait >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_interval << m_wait << m_scroll;
    }

  public:
    Hatch_GenRushFighter(int wait=120, bool scroll=false) :
      m_frame(0), m_interval(300), m_wait(wait), m_scroll(scroll)
    {}

    void setWait(int v) {m_wait=v; }
    void setInterval(int v) { m_interval=v; }

    virtual void generate()
    {
      Fighter *e = new Fighter(new Fighter_Rush(m_scroll));
      if(layer_ptr g=GetParentLayer(getParent())) {
        e->setParent(g);
        e->setPosition(g->getIMatrix()*getCenter());
        e->setDirection((g->getIMatrix()*getParentMatrix())*getDirection());
      }
      e->setGroup(get()->getGroup());
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_wait>0) {
        --m_wait;
        return;
      }

      int f = ++m_frame%(m_interval+1);
      if(f==m_interval) {
        generate();
      }
      else if(f==m_interval-50) {
        open();
      }
      else if(f==60) {
        close();
      }
    }
  };

  class Hatch_GenMissileShell : public Hatch_Controler
  {
  typedef Hatch_Controler Super;
  private:
    int m_frame;
    int m_interval;
    int m_wait;
    bool m_scroll;

  public:
    Hatch_GenMissileShell(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_interval >> m_wait >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_interval << m_wait << m_scroll;
    }

  public:
    Hatch_GenMissileShell(int wait=120, bool scroll=false) :
      m_frame(0), m_interval(300), m_wait(wait), m_scroll(scroll)
    {}

    void setWait(int v) {m_wait=v; }
    void setInterval(int v) { m_interval=v; }

    virtual void generate()
    {
      Shell *e = new Shell(new Shell_BurstMissile(m_scroll));
      if(layer_ptr g=GetParentLayer(getParent())) {
        e->setParent(g);
        e->setPosition(g->getIMatrix()*getCenter());
        e->setDirection((g->getIMatrix()*getParentMatrix())*getDirection());
      }
      e->setGroup(get()->getGroup());
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_wait>0) {
        --m_wait;
        return;
      }

      int f = ++m_frame%(m_interval+1);
      if(f==m_interval) {
        generate();
      }
      else if(f==m_interval-50) {
        open();
      }
      else if(f==60) {
        close();
      }
    }
  };


  class Hatch_Bolt : public Hatch_Controler
  {
  typedef Hatch_Controler Super;
  private:
    int m_frame;
    int m_wait;

  public:
    Hatch_Bolt(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_wait;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_wait;
    }

  public:
    Hatch_Bolt(int wait=240) : m_frame(0), m_wait(wait)
    {}

    virtual Bolt* generate()
    {
      return new Bolt(new Bolt_Rush());
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_wait>0) {
        --m_wait;
        return;
      }

      if(m_frame==0) {
        open();
      }
      if(m_frame>50 && m_frame<=600 && m_frame%30==0) {
        Bolt *e = generate();
        if(layer_ptr g=GetParentLayer(getParent())) {
          e->setParent(g);
          e->setPosition(g->getIMatrix()*getCenter());
          e->setDirection((g->getIMatrix()*getParentMatrix())*getDirection());
        }
        e->setGroup(get()->getGroup());
      }
      if(m_frame==650) {
        close();
      }
      ++m_frame;
    }
  };

  class Hatch_Bolt2 : public Hatch_Bolt
  {
  typedef Hatch_Bolt Super;
  public:
    Hatch_Bolt2(Deserializer& s) : Super(s) {}
    Hatch_Bolt2(int wait=240) : Super(wait) {}

    virtual Bolt* generate()
    {
     return new Bolt(new Bolt_Straight());
    }
  };


  class Hatch_MiniMine : public Hatch_Controler
  {
  typedef Hatch_Controler Super;
  private:
    int m_frame;
    int m_wait;

  public:
    Hatch_MiniMine(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_wait;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_wait;
    }

  public:
    Hatch_MiniMine(int wait=240) : m_frame(0), m_wait(wait)
    {}

    void onUpdate(UpdateMessage& m)
    {
      if(m_wait>0) {
        --m_wait;
        return;
      }

      if(m_frame==0) {
        open();
      }
      if(m_frame>50 && m_frame<=600 && m_frame%120==0) {
        MiniMine *e = new MiniBurstMine();
        if(layer_ptr g=GetParentLayer(getParent())) {
          e->setParent(g);
          e->setPosition(g->getIMatrix()*getCenter());
          e->setVel((g->getIMatrix()*getParentMatrix())*getDirection()*4.0f);
        }
        e->setGroup(get()->getGroup());
      }
      if(m_frame==600) {
        close();
      }
      ++m_frame;
    }
  };

  class Hatch_Laser : public Hatch_Controler
  {
  typedef Hatch_Controler Super;
  private:
    Laser *m_laser;
    int m_frame;
    int m_wait;

  public:
    Hatch_Laser(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_laser);
      s >> m_frame >> m_wait;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_laser);
      s << m_frame << m_wait;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_laser);
    }

  public:
    Hatch_Laser(int wait=240) : m_laser(0), m_frame(0), m_wait(wait)
    {}

    void start()
    {
      Laser *l = new Laser(get());
      l->setParent(get());
      l->setRadius(25.0f);
      l->setPower(0.5f);
      l->setSpeed(20.0f);
      m_laser = l;
    }

    void stop()
    {
      if(m_laser) {
        m_laser->fade();
        m_laser = 0;
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_wait>0) {
        --m_wait;
        return;
      }

      if(m_frame==0) {
        open();
      }
      if(m_frame==50) {
        start();
      }
      ++m_frame;
    }
  };

  namespace stage3 {
    class Hatch_GenFighterT1 : public Hatch_Controler
    {
    typedef Hatch_Controler Super;
    private:
      int m_frame;
      bool m_invh;
      int m_wait;

    public:
      Hatch_GenFighterT1(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_invh >> m_wait;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_invh << m_wait;
      }

    public:
      Hatch_GenFighterT1(bool invh, int wait=240) :
        m_frame(0), m_invh(invh), m_wait(wait)
      {}

      virtual void generate()
      {
        Fighter *e = new Fighter(new Fighter_Turns1(m_invh));
        if(layer_ptr g=GetParentLayer(getParent())) {
          e->setParent(g);
          e->setPosition(g->getIMatrix()*getCenter());
        }
        e->setGroup(get()->getGroup());
      }

      void onUpdate(UpdateMessage& m)
      {
        if(m_wait>0) {
          --m_wait;
          return;
        }

        if(m_frame==0) {
          open();
        }
        if(m_frame>50 && m_frame<=1350 && m_frame%80==0) {
          generate();
        }
        if(m_frame==1400) {
          close();
        }
        ++m_frame;
      }
    };
  } // namespace stage3 

  namespace stage4 {
    class Hatch_GenFighterT1 : public Hatch_Controler
    {
    typedef Hatch_Controler Super;
    private:
      int m_frame;
      float m_turnpoint;
      bool m_invh;
      int m_wait;
      int m_interval;

    public:
      Hatch_GenFighterT1(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_turnpoint >> m_invh >> m_wait >> m_interval;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_turnpoint << m_invh << m_wait << m_interval;
      }

    public:
      Hatch_GenFighterT1(float turnpoint, bool invh, int wait=240, int interval=80) :
        m_frame(0), m_turnpoint(turnpoint), m_invh(invh), m_wait(wait), m_interval(interval)
      {}

      virtual void generate()
      {
        Fighter *e = new Fighter(new Fighter_Turns1(m_turnpoint, m_invh));
        if(layer_ptr g=GetParentLayer(getParent())) {
          e->setParent(g);
          e->setPosition(g->getIMatrix()*getCenter());
          e->setDirection(getDirection());
        }
        e->setGroup(get()->getGroup());
      }

      void onUpdate(UpdateMessage& m)
      {
        if(m_wait>0) {
          --m_wait;
          return;
        }

        if(m_frame==0) {
          open();
        }
        if(m_frame>50 && m_frame<=1350 && m_frame%m_interval==0) {
          generate();
        }
        if(m_frame==1400) {
          close();
        }
        ++m_frame;
      }
    };
  } // namespace stage4 
}
#endif

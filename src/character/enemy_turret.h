#ifndef enemy_fort_h
#define enemy_fort_h

namespace exception {


  class LaserTurret : public Inherit2(HaveDirection, ChildGround)
  {
  typedef Inherit2(HaveDirection, ChildGround) Super;
  public:
    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts() {}
      void onDamage(DamageMessage& m)
      {
        if(IsFraction(m.getSource())) {
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

  private:
    static const int s_num_layer = 2;
    static const int s_num_parts = 6;
    enum {
      NONE,
      OPEN,
      CLOSE,
      EMIT,
    };

    ChildLayer *m_layer[s_num_layer];
    Parts *m_parts[s_num_parts];
    int m_frame;
    vector4 m_emission;
    int m_action;

  public:
    LaserTurret(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_layer);
      DeserializeLinkage(s, m_parts);
      s >> m_frame >> m_emission >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_layer);
      SerializeLinkage(s, m_parts);
      s << m_frame << m_emission << m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_layer);
      ReconstructLinkage(m_parts);
    }

  public:
    LaserTurret(controler_ptr c) : m_frame(0), m_action(NONE)
    {
      setControler(c);
      setBound(box(vector4(1500)));
      setBox(box(vector4(25, 10, 10), vector4(0, -10, -10)));

      for(int i=0; i<s_num_layer; ++i) {
        m_layer[i] = new ChildLayer();
        m_layer[i]->setParent(this);
        m_layer[i]->chain();
      }

      box b[s_num_parts] = {
        box(vector4(35, 20, 15), vector4(0,  0,-15)),
        box(vector4(20, 15, 25), vector4(0,-10,  0)),
        box(vector4(20, 15,-25), vector4(0,-10,  0)),

        box(vector4(35,-20, 15), vector4(0,  0,-15)),
        box(vector4(20,-15, 25), vector4(0, 10,  0)),
        box(vector4(20,-15,-25), vector4(0, 10,  0)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *a = new Parts();
        a->setParent(i<3 ? m_layer[0] : m_layer[1]);
        a->setBox(b[i]);
        m_parts[i] = a;
      }
      setGroup(getGroup());
    }

    void open()  { m_action = OPEN; }
    void close() { m_action = CLOSE; }

    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
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
      SweepDeadObject(m_parts);

      ++m_frame;
      if(!AliveAny(m_parts)) {
        SendDestroyMessage(0, this);
        return;
      }

      switch(m_action) {
      case OPEN:  _open();  break;
      case CLOSE: _close(); break;
      case EMIT:  _emit();  break;
      }
    }

    void onDamage(DamageMessage& m)
    {
      if(IsFraction(m.getSource())) {
        Super::onDamage(m);
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

      if(m.getStat()==0) {
        PutSmallImpact(getPosition());
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(gobj_ptr p = getParent()) {
        SendCollideMessage(m.getFrom(), p, m.getPosition(), m.getNormal(), m.getDistance());
      }
      if(IsGround(m.getFrom())) {
        SendDestroyMessage(m.getFrom(), this);
      }
    }

  private:
    void _open()
    {
      for(int i=0; i<s_num_layer; ++i) {
        ChildLayer *l = m_layer[i];
        vector4 pos = l->getRelativePosition();
        pos.y+= i==0 ? 1.0f : -1.0f;
        if(fabsf(pos.y)>=15.0f) { m_action = EMIT; }
        l->setPosition(pos);
      }
    }

    void _emit()
    {
      m_emission = m_emission+(vector4(0.5f, 0.5f, 1.0f)-m_emission)*0.01f;
      if(m_emission.z>=0.99f) {
        m_action=0;
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



  class LaserTurret_Controler : public TControler<LaserTurret>
  {
  typedef TControler<LaserTurret> Super;
  public:
    LaserTurret_Controler(Deserializer& s) : Super(s) {}
    LaserTurret_Controler() {}

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


  class LaserTurret_Normal : public LaserTurret_Controler
  {
  typedef LaserTurret_Controler Super;
  private:
    Laser *m_laser;
    int m_frame;
    int m_wait;

  public:
    LaserTurret_Normal(Deserializer& s) : Super(s)
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
    LaserTurret_Normal(int wait=240) :
      m_laser(0), m_frame(0), m_wait(wait)
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
      if(m_frame==120) {
        start();
      }
      ++m_frame;
    }
  };

}
#endif

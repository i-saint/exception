#ifndef enemy_bolt_h
#define enemy_bolt_h

namespace exception {

  class Bolt : public Inherit3(HaveVelocity, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveVelocity, HaveDirection, Enemy) Super;
  private:
    static const int s_num_parts = 1;
    BoxModel *m_model[s_num_parts];
    int m_frame;

  public:
    Bolt(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_model);
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_model);
      s << m_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_model);
    }

  public:
    Bolt(controler_ptr c) : m_frame(0)
    {
      setControler(c);
      setLife(2.0f);
      setEnergy(5.0f);
      setBox(box(vector4(15,7,7)));

      const box b[s_num_parts] = {
        box(vector4(5,10,10)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        BoxModel *db = new BoxModel(this);
        db->setBox(b[i]);
        m_model[i] = db;
      }
    }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      mat.rotateX(2.0f*m_frame);
    }

    void drawModel()
    {
      Super::drawModel();
      for(size_t i=0; i<s_num_parts; ++i) {
        m_model[i]->draw();
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      int f = ++m_frame;
      modMatrix();
      setPosition(getRelativePosition()+getVel());
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      gobj_ptr from = m.getFrom();
      if(IsSolid(from) && !IsFraction(from)) {
        SendDamageMessage(this, from, 5.0f);
        SendDestroyMessage(from, this);
      }
    }

    int getFractionCount() { return 1; }
    int getExplodeCount() { return 1; }
  };


  class Bolt_Controler : public TControler<Bolt>
  {
  typedef TControler<Bolt> Super;
  public:
    Bolt_Controler(Deserializer& s) : Super(s) {}
    Bolt_Controler() {}

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
    Setter(setAccel, const vector4&);
    Setter(setMaxSpeed, float);
    Setter(setVelAttenuation, float);
    Setter(setDirection, const vector4&);
  };


  class Bolt_Straight : public Bolt_Controler
  {
  typedef Bolt_Controler Super;
  private:
    StopWatch m_sw;
    float m_speed;
    float m_pre_sin;
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_action;

  public:
    Bolt_Straight(Deserializer& s) : Super(s)
    {
      s >> m_sw >> m_speed >> m_pre_sin >> m_initial_pos >> m_target_pos >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_sw << m_speed << m_pre_sin << m_initial_pos << m_target_pos << m_action;
    }

  public:
    Bolt_Straight() : m_speed(0.0f), m_pre_sin(0.0f), m_action(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_initial_pos = getRelativePosition();
      m_target_pos = m_initial_pos+getDirection()*60.0f;
      m_sw.run(30);
    }

    void move()
    {
      vector4 pos = getRelativePosition();
      float q = Sin90(1.0f/30.0f*m_sw.getPast());
      setPosition(pos+(m_target_pos-m_initial_pos)*(q-m_pre_sin));
      m_pre_sin = q;

      if(m_sw.isFinished()) {
        setAccel(getDirection()*0.05f);
        setMaxSpeed(3.0f);
        ++m_action;
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      setPosition(getRelativePosition() + (getParentIMatrix()*GetGlobalScroll()) + getVel());
      switch(m_action) {
      case 0: move(); break;
      }
    }
  };


  class Bolt_Rush : public Bolt_Controler
  {
  typedef Bolt_Controler Super;
  private:
    StopWatch m_sw;
    float m_speed;
    float m_pre_sin;
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_action;

  public:
    Bolt_Rush(Deserializer& s) : Super(s)
    {
      s >> m_sw >> m_speed >> m_pre_sin >> m_initial_pos >> m_target_pos >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_sw << m_speed << m_pre_sin << m_initial_pos << m_target_pos << m_action;
    }

  public:
    Bolt_Rush() : m_speed(0.0f), m_pre_sin(0.0f), m_action(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_initial_pos = getRelativePosition();
      m_target_pos = m_initial_pos+getDirection()*60.0f;
      m_sw.run(30);
    }

    void move()
    {
      vector4 pos = getRelativePosition();
      float q = Sin90(1.0f/30.0f*m_sw.getPast());
      setPosition(pos+(m_target_pos-m_initial_pos)*(q-m_pre_sin));
      m_pre_sin = q;

      if(m_sw.isFinished()) {
        ++m_action;
        m_sw.run(40);
      }
    }

    void target()
    {
      vector4 dir = getParentIMatrix()*(GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      setDirection(getDirection()+dir*0.1f);
      if(m_sw.isFinished()) {
        setAccel(getDirection()*0.03f);
        setMaxSpeed(2.0f);
        ++m_action;
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      setPosition(getRelativePosition() + (getParentIMatrix()*GetGlobalScroll()) + getVel());
      switch(m_action) {
      case 0: move();   break;
      case 1: target(); break;
      }
    }
  };

} //  exception
#endif

#ifndef enemy_gear_h
#define enemy_gear_h

namespace exception {

  class IGear : public Inherit2(HaveSpinAttrib, ChildGround)
  {
  typedef Inherit2(HaveSpinAttrib, ChildGround) Super;
  private:
    int m_stat;
    float m_max_rotate_speed;
    float m_min_rotate;
    float m_max_rotate;
    float m_reverse_speed;

  public:
    IGear(Deserializer& s) : Super(s)
    {
      s >> m_stat >> m_max_rotate_speed >> m_min_rotate >> m_max_rotate >> m_reverse_speed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_stat << m_max_rotate_speed << m_min_rotate << m_max_rotate << m_reverse_speed;
    }

  public:
    IGear() :
      m_stat(0), m_max_rotate_speed(10.0f),
      m_min_rotate(-100000.0f), m_max_rotate(100000.0f), m_reverse_speed(0.0f)
    {
      this->setAxis(vector4(0,0,1));
    }

    void setIncrementOnly(bool v) { m_stat = v?1:0; }
    void setDecrementOnly(bool v) { m_stat = v?2:0; }

    void accelRotate(float v)
    {
      if(m_stat==0 || (m_stat==1 && v>0) || (m_stat==2 && v<0)) {
        setRotateSpeed(getRotateSpeed()+v);
      }
    }

    void setRotateSpeed(float v)
    {
      if(fabsf(v)>=m_max_rotate_speed) {
        v = v<0.0f ? -m_max_rotate_speed : m_max_rotate_speed;
      }
      this->Super::setRotateSpeed(v);
    }

    float getMinRotate() { return m_min_rotate; }
    float getMaxRotate() { return m_max_rotate; }

    void setMaxRotateSpeed(float v) { m_max_rotate_speed=fabsf(v); }
    void setMinRotate(float v) { m_min_rotate=v; }
    void setMaxRotate(float v) { m_max_rotate=v; }
    void setReverseSpeed(float v) { m_reverse_speed=v; }

    void rotate(float v)
    {
      float old = this->getRotate();
      float r = std::max<float>(m_min_rotate, std::min<float>(old+v, m_max_rotate));
      if(r==m_min_rotate || r==m_max_rotate) {
        this->setRotateSpeed(0.0f);
      }
      this->setRotate(r);
    }

    virtual void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.1f, 0.1f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.7f).v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      float r = getRotate();
      if(r>0.2f) {
        setRotateSpeed(getRotateSpeed()-m_reverse_speed);
      }
      else if(r<-0.2f) {
        setRotateSpeed(getRotateSpeed()+m_reverse_speed);
      }
    }

    int getFractionCount() { return 0; }
  };


  class GearParts : public ChildGround
  {
  typedef ChildGround Super;
  private:
    float m_link_speed;

  public:
    GearParts(Deserializer& s) : Super(s)
    {
      s >> m_link_speed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_link_speed;
    }

  public:
    GearParts() : m_link_speed(0.0f)
    {
      setBound(box(vector4(2500), vector4(-2500)));
    }

    void setLinkSpeed(float v) { m_link_speed=v; }

    bool isReverse() { return (getMatrix()*vector4(0,0,1)).z<0.0f; }

    void accelRotate(float v)
    {
      if(IGear *parent = static_cast<IGear*>(getParent())) {
        parent->accelRotate(v*(isReverse()?-1:1));
      }
    }

    virtual void accel(const vector4& v)
    {
      vector4 dir = matrix44().rotateZ(90)*(getParentMatrix()*getDirection());

      float dot = dir.dot(v.normal());
      if(dot<-0.5f) {
        accelRotate(-v.norm());
      }
      else if(dot>0.5f) {
        accelRotate(v.norm());
      }
    }

    virtual void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.1f, 0.1f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.7f).v);
    }

    void onCollide(CollideMessage& m)
    {
      float accel = m_link_speed;
      if(IsFraction(m.getFrom())) {
        accel*=0.02f;
      }

      IGear *parent = static_cast<IGear*>(getParent());
      vector4 dir = matrix44().rotateZ(90)*(getParentMatrix()*getDirection());

      float dot = dir.dot(m.getNormal());
      if(dot<-0.5f) {
        accelRotate(-accel);
      }
      else if(dot>0.5f) {
        accelRotate(accel);
      }
    }

    void onAccel(AccelMessage& m)
    {
      accel(m.getAccel());
    }

    int getFractionCount() { return 0; }
  };


  class SmallGear : public IGear
  {
  typedef IGear Super;
  private:
    static const int s_num_parts = 4;
    GearParts *m_parts[s_num_parts];

  public:
    SmallGear(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
    }

  public:
    SmallGear()
    {
      setMaxRotateSpeed(15.0f);
      setBox(box(vector4(9, 9, 15)));

      vector4 dir(1,0,0,0);
      for(int i=0; i<s_num_parts; ++i) {
        GearParts *p = new GearParts();
        p->setParent(this);
        p->setBox(box(vector4(9,6,10), vector4(30,-6,-10)));
        p->setDirection(dir);
        p->setLinkSpeed(0.3f);
        dir = matrix44().rotateZ(360.0f/s_num_parts)*dir;
        m_parts[i] = p;
      }
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      float rs = getRotateSpeed();
      rs*=0.99f;
      setRotateSpeed(rs);
    }
  };


  class LargeGear : public IGear
  {
  typedef IGear Super;
  private:
    static const int s_num_parts = 6;
    GearParts *m_parts[s_num_parts];

  public:
    LargeGear(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
    }

  public:
    LargeGear()
    {
      setBox(box(vector4(40, 40, 40)));
      setMaxRotateSpeed(0.5f);
      setBound(box(vector4(1500)));

      vector4 dir(1,0,0,0);
      for(int i=0; i<s_num_parts; ++i) {
        GearParts *p = new GearParts();
        p->setParent(this);
        p->setBox(box(vector4(30,25,25), vector4(250,-25,-25)));
        p->setDirection(dir);
        p->setLinkSpeed(0.005f);
        dir = matrix44().rotateZ(360.0f/s_num_parts)*dir;
        m_parts[i] = p;
      }
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      float rs = getRotateSpeed();
      rs*=0.996f;
      setRotateSpeed(rs);
    }
  };




  class LaserAmp : public ChildGround
  {
  typedef ChildGround Super;
  private:
    Laser *m_laser;
    int m_frame;
    int m_wait;
    vector4 m_emission;

  public:
    LaserAmp(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_laser);
      s >> m_frame >> m_wait >> m_emission;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_laser);
      s << m_frame << m_wait << m_emission;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_laser);
    }

  public:
    LaserAmp() : m_frame(0), m_wait(0), m_laser(0)
    {}

    void setWait(int v) { m_wait=v; }

    virtual void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.1f, 0.1f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.7f).v);
    }

    void draw()
    {
      glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
    }

    void fire()
    {
      if(m_laser) {
        return;
      }
      Laser *l = new Laser(this);
      l->setParent(this);
      l->setRadius(20.0f);
      l->setPower(0.5f);
      l->setSpeed(10.0f);
      m_laser = l;
    }

    void stop()
    {
      if(m_laser) {
        m_laser->fade();
        m_laser = 0;
        m_wait = 99999;
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_laser);

      if(m_wait>0) {
        --m_wait;
        m_emission = m_emission+(vector4()-m_emission)*0.02f;
      }
      else {
        m_emission = m_emission+(vector4(0.5f, 0.5f, 1.0f)-m_emission)*0.01f;
        ++m_frame;
        if(m_frame==150) {
          fire();
        }
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(gobj_ptr p = getParent()) {
        SendCollideMessage(m.getFrom(), p, m.getPosition(), m.getNormal(), m.getDistance());
      }
    }
  };

  class LaserGear : public IGear
  {
  typedef IGear Super;
  private:
    static const int s_num_parts = 3;
    GearParts *m_parts[s_num_parts];
    LaserAmp *m_amp[s_num_parts];

  public:
    LaserGear(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_amp);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_amp);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_amp);
    }

  public:
    LaserGear(int wait)
    {
      setMaxRotateSpeed(2.0f);
      setBox(box(vector4(18, 18, 25)));

      vector4 dir(1,0,0,0);
      for(int i=0; i<s_num_parts; ++i) {
        GearParts *p = new GearParts();
        p->setParent(this);
        p->setBox(box(vector4(10,20,20), vector4(70,-20,-20)));
        p->setDirection(dir);
        p->setLinkSpeed(0.06f);
        m_parts[i] = p;
        dir = matrix44().rotateZ(360.0f/s_num_parts)*dir;

        LaserAmp *l = new LaserAmp();
        l->setParent(p);
        l->setWait(wait);
        l->setBox(box(vector4(0,15,15), vector4(15,-15,-15)));
        l->setPosition(vector4(70,0,0));
        l->setDirection(vector4(1,0,0));
        m_amp[i] = l;
      }
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
      SetGroup(m_amp, v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      float rs = getRotateSpeed();
      rs*=0.98f;
      setRotateSpeed(rs);
    }
  };

}
#endif

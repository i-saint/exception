#ifndef enemy_defenser_h
#define enemy_defenser_h

namespace exception {


  class Defenser : public Inherit2(HaveDirection, ChildGround)
  {
  typedef Inherit2(HaveDirection, ChildGround) Super;
  public:
    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    public:
      void onCollide(CollideMessage& m)
      {
        if(IsGround(m.getFrom())) {
          SendDestroyMessage(m.getFrom(), this);
        }
      }
    };

  private:
    static const int s_num_shields = 6;
    static const int s_num_parts = 3;
    RotLayer *m_shield_layer;
    Parts *m_shields[s_num_shields];
    Parts *m_parts[s_num_parts];
    int m_frame;
    void (Defenser::*m_action)();

  public:
    Defenser(controler_ptr c) : m_frame(0), m_action(0)
    {
      setControler(c);
      setBound(box(vector4(1500)));
      setBox(box(vector4(10), vector4(-10)));

      box b[s_num_parts] = {
        box(vector4(35,-20, 15), vector4(0,  0,-15)),
        box(vector4(20,-15, 25), vector4(0, 10,  0)),
        box(vector4(20,-15,-25), vector4(0, 10,  0)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *a = new Parts();
        a->setParent(this);
        a->setBox(b[i]);
        m_parts[i] = a;
      }

      m_shield_layer = new RotLayer();
      m_shield_layer->setAxis(vector4(0,1,0));
      m_shield_layer->setParent(this);
      m_shield_layer->chain();
      box sb(vector4(20, 0, 15), vector4(-20, 10,-15));
      matrix44 rot;
      for(int i=0; i<s_num_shields; ++i) {
        Parts *a = new Parts();
        a->setParent(m_shield_layer);
        a->setBox(sb);
        a->setDirection(rot*vector4(1,0,0));
        a->setPosition(a->getDirection()*40.0f);
        m_shields[i] = a;
        rot.rotateZ(360.0f/s_num_shields);
      }
      setGroup(getGroup());
    }

    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
      SetGroup(m_shields, v);
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

      ++m_frame;
      SweepDeadObject(m_parts);
      SweepDeadObject(m_shields);
      if(!AliveAny(m_parts)) {
        SendDestroyMessage(0, this);
        return;
      }

      if(m_action) {
        (this->*m_action)();
      }
    }

    void onKill(KillMessage& m)
    {
      Unchain(m_shield_layer);
      Super::onKill(m);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
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



    void aiming(const vector4& tpos)
    {
      matrix44 pmat = getParentMatrix();
      matrix44 ipmat = matrix44(pmat).transpose();
      vector4 v = (tpos-getPosition()).normal();

      vector4 dir = pmat*getDirection();
      float dot = v.dot(dir);
      float dot2 = (matrix44().rotateZ(90.0f)*v).dot(dir);
      const float rot_speed = 0.8f;
      if( (dot>=0.0f && dot2<=0.0f)
        ||(dot<=0.0f && dot2<=0.0f)) {
        dir = matrix44().rotateZ(rot_speed)*dir;
      }
      else {
        dir = matrix44().rotateZ(-rot_speed)*dir;
      }
      setDirection(ipmat*dir);

      if(v.dot(dir)>cosf(rot_speed*ist::radian)) {
      //  m_action = &Defenser::charge;
      }
    }

  private:

  };



  class Defenser_Controler : public TControler<Defenser>
  {
  public:
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
  };


  class Defenser_Normal : public Defenser_Controler
  {
  private:
    int m_frame;
    int m_wait;
    Laser *m_laser;

  public:
    Defenser_Normal(int wait=240) :
      m_frame(0), m_wait(wait), m_laser(0)
    {
    }

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

      ++m_frame;
    }
  };

}
#endif

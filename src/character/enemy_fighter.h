#ifndef enemy_fighter_h
#define enemy_fighter_h

namespace exception {

  class Fighter : public Inherit3(HaveVelocity, HaveDirection, Enemy)
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
        Super::onCollide(m);
        gobj_ptr from = m.getFrom();
        if(IsSolid(from) && !IsFraction(from)) {
          SendDamageMessage(this, from, 2.0f);
          SendDestroyMessage(from, this);
        }
      }
    };

  private:
    static const int s_num_parts = 2;

    Parts *m_parts[s_num_parts];
    int m_frame;
    float m_spin;

  public:
    Fighter(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      s >> m_frame >> m_spin;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      s << m_frame << m_spin;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
    }

  public:
    Fighter(controler_ptr c) : m_frame(0), m_spin(0.0f)
    {
      setControler(c);
      setLife(5.0f);
      setEnergy(15.0f);
      setBox(box(vector4(20.0f, 12.5f, 12.5f), vector4(-5.0f, -12.5f, -12.5f)));

      const box pbox[s_num_parts] = {
        box(vector4(5.0f,  25.0f, 5.0f), vector4(-30.0f,  2.0f, -12.5f)),
        box(vector4(5.0f, -25.0f, 5.0f), vector4(-30.0f, -2.0f, -12.5f)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *p = new Parts();
        p->setParent(this);
        p->setLife(2.0f);
        p->setBox(pbox[i]);
        m_parts[i] = p;
      }
    }

    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }


    void draw()
    {
      Super::draw();

      const vector4 bpos[2] = {
        vector4(-35.0f,  13.5, 0),
        vector4(-35.0f, -13.5, 0)
      };
      for(int i=0; i<2; ++i) {
        if(m_parts[i]) {
          DrawSprite("burner.png",
            getMatrix()*bpos[i],
            vector4(27.0f+::sinf(53.0f*m_frame*ist::radian)*2.5f),
            1.0f);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      int f = ++m_frame;
      SweepDeadObject(m_parts);
      if(getControler() && DeadAny(m_parts)) {
        setControler(0);
      }

      if(getControler()==0) {
        setVel(getVel()+getParentIMatrix()*GetGlobalAccel());
        if(!m_parts[0] && m_parts[1]) {
          m_spin = std::min<float>(3.0f, m_spin+0.1f);
        }
        else if(!m_parts[1] && m_parts[0]) {
          m_spin = std::max<float>(-3.0f, m_spin-0.1f);
        }
        setDirection(matrix44().rotateZ(m_spin)*getDirection());
      }

      setPosition(getRelativePosition()+getVel());
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      gobj_ptr from = m.getFrom();
      if(IsSolid(from) && !IsFraction(from)) {
        SendDamageMessage(this, from, 2.0f);
        SendDestroyMessage(from, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }
  };



  class Fighter_Controler : public TControler<Fighter>
  {
  typedef TControler<Fighter> Super;
  public:
    Fighter_Controler(Deserializer& s) : Super(s) {}
    Fighter_Controler() {}

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


  class Fighter_Straight : public Fighter_Controler
  {
  typedef Fighter_Controler Super;
  private:
    float m_speed;
    bool m_scroll;

  public:
    Fighter_Straight(Deserializer& s) : Super(s)
    {
      s >> m_speed >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_speed << m_scroll;
    }

  public:
    Fighter_Straight(float speed=2.0f, bool scroll=false) :
      m_speed(speed), m_scroll(scroll)
    {}

    void onConstruct(ConstructMessage& m)
    {
      setVel(getDirection()*m_speed);
    }

    void onUpdate(UpdateMessage& m)
    {
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  class Fighter_Straight2 : public Fighter_Straight
  {
  typedef Fighter_Straight Super;
  public:
    Fighter_Straight2(Deserializer& s) : Super(s) {}
    Fighter_Straight2(float speed=2.0f) : Fighter_Straight(speed)
    {}

    void onUpdate(UpdateMessage& m)
    {
      setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
    }
  };

  class Fighter_Rush : public Fighter_Controler
  {
  typedef Fighter_Controler Super;
  private:
    int m_frame;
    float m_length;
    bool m_scroll;
    int m_action;

  public:
    Fighter_Rush(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_length >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_length << m_scroll << m_action;
    }

  public:
    Fighter_Rush(bool scroll) :
      m_frame(0), m_length(90.0f), m_scroll(scroll), m_action(0)
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
        ++m_action;
        m_frame = 0;
        setGroup(Solid::createGroupID());
      }
    }

    void aim()
    {
      int f = ++m_frame;
      vector4 dir = getParentIMatrix()*(GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      setDirection(getDirection()+dir*0.1f);
      if(f>=40) {
        ++m_action;
        m_frame = 0;
      }
    }

    void rush()
    {
      setVel(getVel()+getDirection()*0.1f);
    }

    void onUpdate(UpdateMessage& m)
    {
      switch(m_action) {
      case 0: move(); break;
      case 1: aim();  break;
      case 2: rush(); break;
      }

      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };


  namespace stage1 {
    class Fighter_Turn : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      float m_speed;
      float m_bezier;
      bool m_invh;
      bool m_invv;

    public:
      Fighter_Turn(Deserializer& s) : Super(s)
      {
        s >> m_speed >> m_bezier >> m_invh >> m_invv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_speed << m_bezier << m_invh << m_invv;
      }

    public:
      Fighter_Turn(bool invh=false, bool invv=false) :
        m_speed(3.0f), m_bezier(0.0f),
        m_invh(invh), m_invv(invv)
      {}

      void setSpeed(float v) { m_speed=v; }

      void onUpdate(UpdateMessage& m)
      {
        static spline motion;
        if(motion.empty()) {
          motion.resize(5);
          motion[0] = spline::point_t(vector2(0,0));
          motion[1] = spline::point_t(vector2(-500,0));
          motion[2] = spline::point_t(vector2(-620,0), vector2(-620,-120), vector2(-620,-240));
          motion[3] = spline::point_t(vector2(-500,-240));
          motion[4] = spline::point_t(vector2(0,-240));
        }

        float pb = m_bezier;
        m_bezier = motion.advance(m_bezier, m_speed);
        vector2 pre = motion.bezierS(pb);
        vector2 now = motion.bezierS(m_bezier);
        vector2 prog = now-pre;
        if(m_invh) { prog.x = -prog.x; }
        if(m_invv) { prog.y = -prog.y; }

        vector4 ppos = getRelativePosition();
        vector4 pos = ppos+vector4(prog.x, prog.y, 0);
        setVel(getVel()*0.98f);
        setDirection(pos-ppos);
        setPosition(pos);

        if(pb==m_bezier) {
          SendKillMessage(0, get());
        }
      }
    };

    class Fighter_Cross : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      float m_speed;
      float m_bezier;
      bool m_invh;
      bool m_invv;

    public:
      Fighter_Cross(Deserializer& s) : Super(s)
      {
        s >> m_speed >> m_bezier >> m_invh >> m_invv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_speed << m_bezier << m_invh << m_invv;
      }

    public:
      Fighter_Cross(bool invh=false, bool invv=false) :
        m_speed(3.0f), m_bezier(0.0f),
        m_invh(invh), m_invv(invv)
      {}

      void setSpeed(float v) { m_speed=v; }

      void onUpdate(UpdateMessage& m)
      {
        static spline motion;
        if(motion.empty()) {
          motion.resize(4);
          motion[0] = spline::point_t(vector2(0,0));
          motion[1] = spline::point_t(vector2(-350,0), vector2(-350,0), vector2(-500,0));
          motion[2] = spline::point_t(vector2(-500,-240), vector2(-650,-240), vector2(-650,-240));
          motion[3] = spline::point_t(vector2(-1000,-240));
        }

        float pb = m_bezier;
        m_bezier = motion.advance(m_bezier, m_speed);
        vector2 pre = motion.bezierS(pb);
        vector2 now = motion.bezierS(m_bezier);
        vector2 prog = now-pre;
        if(m_invh) { prog.x = -prog.x; }
        if(m_invv) { prog.y = -prog.y; }

        vector4 ppos = getRelativePosition();
        vector4 pos = ppos+vector4(prog.x, prog.y, 0);
        setVel(vector4());
        setDirection(pos-ppos);
        setPosition(pos);

        if(pb==m_bezier) {
          SendKillMessage(0, get());
        }
      }
    };
  } // namespace stage1 

  namespace stage2 {
    class Fighter_Turn : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      float m_speed;
      bool m_invv;
      int m_state;
      int m_frame;

    public:
      Fighter_Turn(Deserializer& s) : Super(s)
      {
        s >> m_speed >> m_invv >> m_state >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_speed << m_invv << m_state << m_frame;
      }

    public:
      Fighter_Turn(bool invv=false) : m_invv(invv), m_state(0), m_frame(0)
      {}

      void onUpdate(UpdateMessage& m)
      {
        ++m_frame;
        if(m_state!=1) {
          vector4 prog = getDirection()*3.5f;
          setPosition(getRelativePosition()+prog);
        }
        if(m_state==0) {
          if(m_frame==180) {
            m_frame = 0;
            ++m_state;
          }
        }
        else if(m_state==1) {
          vector4 dir = getDirection();
          setDirection(matrix44().rotateZ(m_invv ? 3 : -3)*dir);
          if(m_frame==30) {
            m_frame = 0;
            ++m_state;
          }
        }
      }
    };
  } // namespace stage2 

  namespace stage3 {
    class Fighter_Turns1 : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      int m_frame;
      vector4 m_dir;
      bool m_iv;

    public:
      Fighter_Turns1(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_dir >> m_iv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_dir << m_iv;
      }

    public:
      Fighter_Turns1(bool iv) :
        m_frame(0), m_iv(iv)
      {}

      void onConstruct(ConstructMessage& m)
      {
        setDirection(vector4(0,1,0));
        m_dir = vector4(0,1,0);
      }

      void setDirection(const vector4& v)
      {
        if(m_iv) {
          Super::setDirection(vector4(v.x, -v.y, v.z));
        }
        else {
          Super::setDirection(v);
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        int f = ++m_frame;
        const float vel = 2.0f;
        const int phase1 = int(150.0f/vel);
        const int phase2 = phase1+20;
        const int phase3 = phase2+int(700.0f/vel);
        const int phase_end = phase3;

        vector4 pos = getRelativePosition();
        pos+=getParentIMatrix()*GetGlobalScroll();

        if     (f<phase1) { pos+=getDirection()*vel; }
        else if(f<phase2) { setDirection(matrix44().rotateZ(Cos180I(1.0f/20.0f*(f-phase1))*90.0f)*m_dir); }
        else if(f<phase3) { pos+=getDirection()*vel; }

        setPosition(pos);
        if(f==phase_end) {
          SendKillMessage(0, get());
        }
      }
    };
  } // namespace starge3 

  namespace stage4 {
    class Fighter_Turns1 : public Fighter_Controler
    {
    typedef Fighter_Controler Super;
    private:
      int m_frame;
      vector4 m_dir;
      float m_turnpoint;
      bool m_iv;

    public:
      Fighter_Turns1(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_dir >> m_turnpoint >> m_iv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_dir << m_turnpoint << m_iv;
      }

    public:
      Fighter_Turns1(float turnpoint, bool iv) :
        m_frame(0), m_turnpoint(turnpoint), m_iv(iv)
      {}

      void onConstruct(ConstructMessage& m)
      {
        m_dir = getDirection();
      }

      void onUpdate(UpdateMessage& m)
      {
        int f = ++m_frame;
        const float vel = 2.0f;
        const int phase1 = int(m_turnpoint/vel);
        const int phase2 = phase1+20;
        const int phase3 = phase2+int(700.0f/vel);
        const int phase_end = phase3;

        vector4 pos = getRelativePosition();
        pos+=getParentIMatrix()*GetGlobalScroll();

        if     (f<phase1) { pos+=getDirection()*vel; }
        else if(f<=phase2) {
          setDirection(matrix44().rotateZ(Cos180I(1.0f/20.0f*(f-phase1))*90.0f*(m_iv?-1:1))*m_dir);
        }
        else if(f<phase3) { pos+=getDirection()*vel; }

        setPosition(pos);
        if(f==phase_end) {
          SendKillMessage(0, get());
        }
      }
    };
  } // namespace starge4 

}
#endif

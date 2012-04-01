#ifndef enemy_zab_h
#define enemy_zab_h

namespace exception {

  class Zab : public Inherit2(HaveVelocity,  Enemy)
  {
  typedef Inherit2(HaveVelocity,  Enemy) Super;
  public:
    class Parts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      vector4 m_emission;
      float m_opa;

    public:
      Parts(Deserializer& s) : Super(s)
      {
        s >> m_emission >> m_opa;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_emission << m_opa;
      }

    public:
      Parts() : m_opa(0)
      {
        enableCollision(false);
      }

      void setEmission(const vector4& v) { m_emission=v; }
      void setOpacity(float v)
      {
        m_opa = v;
        if(m_opa>0.8f) {
          enableCollision(true);
        }
      }

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,m_opa).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,1.0f).v);
      }

      void onDamage(DamageMessage& m)
      {
        if(!dynamic_cast<Laser*>(m.getSource())) { // 敵レーザー無効 
          Super::onDamage(m);
        }
      }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr from = m.getFrom();
        if(IsGround(from)) {
          SendDestroyMessage(from, this);
        }
      }
    };

  private:
    static const int s_num_parts = 4;

    Parts *m_parts[s_num_parts];
    player_ptr m_blower;
    vector4 m_emission;
    float m_opa;

  public:
    Zab(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_blower);
      s >> m_emission >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_blower);
      s << m_emission << m_opa;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_blower);
    }

  public:
    Zab(controler_ptr c) : m_blower(0), m_opa(0.0f)
    {
      setControler(c);
      setLife(1.0f);
      setBox(box(vector4(10,10,10)));
      setEnergy(25.0f);
      enableCollision(false);
      m_emission = vector4(0.8f, 0.8f, 1.5f);

      const box pbox[s_num_parts] = {
        box(vector4( 25,  25, 15), vector4( 2, 2, -15)),
        box(vector4(-25,  25, 15), vector4(-2, 2, -15)),
        box(vector4(-25, -25, 15), vector4(-2,-2, -15)),
        box(vector4( 25, -25, 15), vector4( 2,-2, -15)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *p = new Parts();
        p->setParent(this);
        p->setLife(3.0f);
        p->setBox(pbox[i]);
        m_parts[i] = p;
      }
    }

    float getOpacity() { return m_opa; }
    const vector4& getEmission() { return m_emission; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }

    void draw()
    {
      if(m_opa==0.0f) {
        return;
      }
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,m_opa).v);
      glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,1.0f).v);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_blower);

      m_opa+=(1.0f-m_opa)*0.03f;
      if(m_opa>0.8f) {
        enableCollision(true);
      }
      if(m_opa>0.8f && m_emission.z>0.5f) {
        m_emission*=0.98f;
      }
      setPosition(getRelativePosition()+getVel());

      for(int i=0; i<s_num_parts; ++i) {
        if(Parts *p=m_parts[i]) {
          p->setEmission(m_emission);
          p->setOpacity(m_opa);
        }
      }

      KillIfOutOfScreen(this, rect(vector2(300,300)));
    }

    void onDamage(DamageMessage& m)
    {
      if(!dynamic_cast<Laser*>(m.getSource())) { // 敵レーザー無効 
        Super::onDamage(m);
      }
    }

    void onCollide(CollideMessage& m)
    {
      gobj_ptr from = m.getFrom();
      if(!IsSolid(from) || IsFraction(from)) {
        return;
      }

      if(IsGround(from)) {
        SendDestroyMessage(from, this);
      }
      else if(IsEnemy(from) && getVel().norm()>4.0f) {
        SendDestroyMessage(m_blower, this);
        SendDamageMessage(m_blower, from, 10.0f, this);
      }
    }

    void onAccel(AccelMessage& m)
    {
      Super::onAccel(m);
      if(player_ptr pl=ToPlayer(m.getFrom())) {
        m_blower = pl;
      }
    }
  };

  class Zab_Controler : public TControler<Zab>
  {
  typedef TControler<Zab> Super;
  public:
    Zab_Controler(Deserializer& s) : Super(s) {}
    Zab_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getVel, const vector4&);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setVel, const vector4&);
    Setter(setAccel, const vector4&);
  };

  class Zab_Rush : public Zab_Controler
  {
  typedef Zab_Controler Super;
  private:
    int m_frame;
    bool m_scroll;

  public:
    Zab_Rush(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_scroll;
    }

  public:
    Zab_Rush(bool scroll=false) : m_frame(0), m_scroll(scroll)
    {}

    virtual vector4 getTargetPosition()
    {
      return GetNearestPlayerPosition(getPosition());
    }

    void onUpdate(UpdateMessage& m)
    {
      ++m_frame;
      if(m_frame==60) {
        setAccel(getParentIMatrix()*((getTargetPosition()-getPosition()).normal()*0.1f));
      }
      if(m_frame==79) {
        setAccel(vector4());
      }
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  class Zab_Straight : public Zab_Controler
  {
  typedef Zab_Controler Super;
  private:
    int m_frame;
    vector4 m_dir;
    bool m_scroll;

  public:
    Zab_Straight(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_dir >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_dir << m_scroll;
    }

  public:
    Zab_Straight(const vector4& dir, bool scroll=false) :
      m_frame(0), m_scroll(scroll)
    {
      m_dir = dir.normal();
    }

    void onUpdate(UpdateMessage& m)
    {
      ++m_frame;
      if(m_frame==60) {
        setAccel(getParentIMatrix()*(m_dir*0.1f));
      }
      if(m_frame==75) {
        setAccel(vector4());
      }
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };


  inline Zab* PutZab(const vector4& pos, bool scroll)
  {
    Zab *e = new Zab(new Zab_Rush(scroll));
    e->setPosition(pos);
    return e;
  }

  inline Zab* PutStraightZab(const vector4& pos, const vector4& dir, bool scroll)
  {
    Zab *e = new Zab(new Zab_Straight(dir, scroll));
    e->setPosition(pos);
    return e;
  }

} //  exception
#endif

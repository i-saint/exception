#ifndef enemy_block_h
#define enemy_block_h

namespace exception {


  template<class T>
  class SpinningBlockAttrib : public Inherit3(HaveVelocity, HaveSpinAttrib, T)
  {
  typedef Inherit3(HaveVelocity, HaveSpinAttrib, T) Super;
  private:
    bool m_scroll;

  public:
    SpinningBlockAttrib(Deserializer& s) : Super(s)
    {
      s >> m_scroll;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_scroll;
    }

  public:
    SpinningBlockAttrib() : m_scroll(false)
    {
      this->setMaxSpeed(3);
      this->setVelAttenuation(0.98f);
    }

    void setScroll(bool v) { m_scroll=v; }

    virtual void progress()
    {
      vector4 pos = this->getRelativePosition();
      pos+=this->getVel();
      pos.z*=0.98f;
      this->setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      this->Super::onUpdate(m);
      progress();
      if(m_scroll) {
        this->setPosition(this->getRelativePosition()+this->getParentIMatrix()*GetGlobalScroll());
      }
    }
  };


  class MediumBlock : public Inherit2(SpinningBlockAttrib, Enemy)
  {
  typedef Inherit2(SpinningBlockAttrib, Enemy) Super;
  public:
    MediumBlock(Deserializer& s) : Super(s) {}
    MediumBlock()
    {
      setLife(50.0f);
      setEnergy(30.0f);
      setBox(box(vector4(25)));
      setAxis(vector4(GetRand()-0.5f, GetRand()-0.5f, GetRand()-0.5f).normalize());
      setRotateSpeed(1.0f);
      setMaxSpeed(1.4f);
      setVelAttenuation(0.98f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      if(!s) {
        return;
      }

      vector4 vel = getVel();
      if(!IsFraction(s) && vel.norm()>getMaxSpeed()+2.0f) { // 高速で衝突したらダメージ食らわせて自壊 
        SendDamageMessage(m.getFrom(), s, 50.0f, this);
        SendDestroyMessage(m.getFrom(), this);
      }
      else {
        float max_speed = std::max<float>(getMaxSpeed(), vel.norm());
        vector4 n = getParentIMatrix()*m.getNormal();
        vector4 pos = getRelativePosition();
        if(IsGround(s)) {
          vel = matrix44().rotateA(n, 180)*-vel;
          vel+=n*0.2f;
          vel.z = 0;
          pos+=n*1.0f;
        }
        else if(IsFraction(s)) {
          vel+=n*0.003f;
          vel.z = 0;
        }
        else if(IsEnemy(s)) {
          pos+=n*(vel.norm()+0.5f);
          vel+=n*0.8f;
          vel.z = 0;
        }
        if(vel.norm()>max_speed) {
          vel = vel.normalize()*max_speed;
        }
        setVel(vel);
        setPosition(pos);
      }
    }

    virtual void explode()
    {
      PutSmallImpact(getPosition());
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        explode();
      }
    }
  };


  class LargeBlock : public MediumBlock
  {
  typedef MediumBlock Super;
  public:
    LargeBlock(Deserializer& s) : Super(s) {}
    LargeBlock()
    {
      setLife(100.0f);
      setEnergy(60.0f);
      setBox(box(vector4(40.0f)));
      setAxis(vector4(GetRand()-0.5f, GetRand()-0.5f, GetRand()-0.5f).normalize());
      setRotateSpeed(0.3f);
      setMaxSpeed(1.0f);
      setVelAttenuation(0.96f);
      setAccelResist(0.5f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      enemy_ptr e = ToEnemy(m.getFrom());
      if(!s) {
        return;
      }

      vector4 n = getParentIMatrix()*m.getNormal();
      vector4 vel = getVel();
      vector4 pos = getRelativePosition();
      if(IsGround(m.getFrom()) || s->getVolume()>=getVolume() || (e && e->getLife()>100.0f)) {
        vel = matrix44().rotateA(n, 180.0f)*-vel;
        vel.z = 0;
        pos+=n*2.0f;
      }
      else if(IsFraction(s)) {
        vel+=n*0.001f;
        vel.z = 0;
      }
      setVel(vel);
      setPosition(pos);
    }

    void explode()
    {
      PutMediumImpact(getPosition());
    }
  };


  class CrashGround : public Inherit2(SpinningBlockAttrib, Ground)
  {
  typedef Inherit2(SpinningBlockAttrib, Ground) Super;
  public:
    CrashGround(Deserializer& s) : Super(s) {}
    CrashGround()
    {
      setBox(box(vector4(25)));
      setAxis(vector4(GetRand()-0.5f, GetRand()-0.5f, GetRand()-0.5f).normalize());
      setRotateSpeed(1.0f);
      setMaxSpeed(1.4f);
      setVelAttenuation(0.98f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      if(!s) {
        return;
      }

      vector4 vel = getVel();
      if(!IsFraction(s) && vel.norm()>getMaxSpeed()+2.0f) { // 高速で衝突したらダメージ食らわせて自壊 
        SendDamageMessage(m.getFrom(), s, 50.0f, this);
        SendDestroyMessage(m.getFrom(), this);
      }
      else {
        float max_speed = std::max<float>(getMaxSpeed(), vel.norm());
        vector4 n = getParentIMatrix()*m.getNormal();
        vector4 pos = getRelativePosition();
        if(IsGround(s)) {
          vel = matrix44().rotateA(n, 180)*-vel;
          vel+=n*0.2f;
          vel.z = 0;
          pos+=n*1.0f;
        }
        else if(IsFraction(s)) {
          vel+=n*0.003f;
          vel.z = 0;
        }
        else if(IsEnemy(s)) {
          pos+=n*(vel.norm()+0.5f);
          vel+=n*0.8f;
          vel.z = 0;
        }
        if(vel.norm()>max_speed) {
          vel = vel.normalize()*max_speed;
        }
        setVel(vel);
        setPosition(pos);
      }
    }

    virtual void explode()
    {
      PutSmallImpact(getPosition());
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        explode();
      }
    }
  };

  class BigCrashGround : public CrashGround
  {
  typedef CrashGround Super;
  public:
    BigCrashGround(Deserializer& s) : Super(s) {}
    BigCrashGround()
    {
      setBox(box(vector4(40.0f)));
      setAxis(vector4(GetRand()-0.5f, GetRand()-0.5f, GetRand()-0.5f).normalize());
      setRotateSpeed(0.3f);
      setMaxSpeed(1.0f);
      setVelAttenuation(0.96f);
      setAccelResist(0.5f);
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr s = ToSolid(m.getFrom());
      enemy_ptr e = ToEnemy(m.getFrom());
      if(!s) {
        return;
      }

      vector4 n = getParentIMatrix()*m.getNormal();
      vector4 vel = getVel();
      vector4 pos = getRelativePosition();
      if(IsGround(m.getFrom()) || s->getVolume()>=getVolume() || (e && e->getLife()>100.0f)) {
        vel = matrix44().rotateA(n, 180.0f)*-vel;
        vel.z = 0;
        pos+=n*2.0f;
      }
      else if(IsFraction(s)) {
        vel+=n*0.001f;
        vel.z = 0;
      }
      setVel(vel);
      setPosition(pos);
    }

    void explode()
    {
      PutMediumImpact(getPosition());
    }
  };





  template<class T>
  class HaveFallAttrib : public T
  {
  typedef T Super;
  private:
    bool m_pushable;
    float m_bounce;

  public:
    HaveFallAttrib(Deserializer& s) : Super(s)
    {
      s >> m_pushable >> m_bounce;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_pushable << m_bounce;
    }

  public:
    HaveFallAttrib() : m_pushable(false), m_bounce(0.1f)
    {}

    void setPushable(bool v) { m_pushable=v; }
    void setBounce(float v) { m_bounce=v; }

    virtual void scroll()
    {}

    virtual void fall()
    {
      vector4 pos = this->getRelativePosition();
      pos+=this->getParentIMatrix()*(this->getVel()+GetGlobalScroll());
      pos.z*=0.98f;
      this->setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      fall();
    }

    void onCollide(CollideMessage& m)
    {
      solid_ptr from = ToSolid(m.getFrom());
      if(!from || IsFraction(from) || IsPlayer(from)) {
        return;
      }

      vector4 pos = this->getRelativePosition();
      vector4 vel = this->getVel();
      vector4 n = m.getNormal();

      float dot = n.dot(vel.normal());
      if(dot < -0.6f) { // 正面衝突 
        if(from->getVolume()>20000.0f) {
          pos+=this->getParentIMatrix()*(n*(vel.norm()+0.3f));
          vel = (matrix44().rotateA(n, 180)*-vel);
          vel*=m_bounce;
        }
      }
      else if(fabsf(dot) < 0.6f) { // 横から押された 
        if(m_pushable && from->getVolume()*27.0f > this->getVolume()) {
          vel*=0.9f;
          vel+=n*0.1f;
          pos+=this->getParentIMatrix()*(n*2.0f);
        }
      }
      else { // 追突 
        if(from->getVolume()*27.0f > this->getVolume()) {
          vel+=n*0.3f;
          pos+=this->getParentIMatrix()*(n*0.5f);
        }
      }
      vel.z = 0;
      this->setPosition(pos);
      this->setVel(vel);
    }
  };


  class DynamicBlock : public Inherit3(HaveFallAttrib, HaveVelocity, Enemy)
  {
  typedef Inherit3(HaveFallAttrib, HaveVelocity, Enemy) Super;
  public:
    DynamicBlock(Deserializer& s) : Super(s) {}
    DynamicBlock()
    {
      setAccel(vector4(0, -0.025f, 0));
    }
  };


  class PillerBlock : public DynamicBlock
  {
  typedef DynamicBlock Super;
  public:
    PillerBlock(Deserializer& s) : Super(s) {}
    PillerBlock()
    {
      setAccelResist(0.0f);
      setEnergy(50);
    }

    int getFractionCount() { return 0; }
  };



  class StaticGround : public Ground
  {
  typedef Ground Super;
  public:
    StaticGround(Deserializer& s) : Super(s) {}
    StaticGround()
    {
      setBound(box(vector4(1500,1000,1000), vector4(-1500,-1500,-1000)));
    }
  };

  class DynamicGround : public Inherit3(HaveFallAttrib, HaveVelocity, Ground)
  {
  typedef Inherit3(HaveFallAttrib, HaveVelocity, Ground) Super;
  public:
    DynamicGround(Deserializer& s) : Super(s) {}
    DynamicGround()
    {
      setAccel(vector4(0, -0.025f, 0));
      setAccelResist(0.0f);
      setBound(box(vector4(1000,1000,1000), vector4(-1000,-1500,-1000)));
    }
  };

  // 地形に衝突したら壊れるフタ用ブロック 
  class CoverGround : public Ground
  {
  typedef Ground Super;
  public:
    CoverGround(Deserializer& s) : Super(s) {}
    CoverGround()
    {
      setBound(box(vector4(1500,1000,1000), vector4(-1500,-1500,-1000)));
    }

    void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.1f, 0.1f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.6f).v);
    }

    int getFractionCount() { return 0; }

    void onCollide(CollideMessage& m)
    {
      if(IsGround(m.getFrom())) {
        SendDestroyMessage(m.getFrom(), this);
      }
    }
  };

}

#endif

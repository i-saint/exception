namespace exception
{


  template<class T>
  class HaveVelocity : public T
  {
  typedef T Super;
  private:
    vector4 m_vel;
    vector4 m_accel;

    float m_accel_resist;
    float m_vel_attr;
    float m_max_speed;

  public:
    HaveVelocity(Deserializer& s) : Super(s)
    {
      s >> m_vel >> m_accel >> m_accel_resist >> m_vel_attr >> m_max_speed;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_vel << m_accel << m_accel_resist << m_vel_attr << m_max_speed;
    }

  public:
    HaveVelocity() :
      m_accel_resist(1.0f), m_vel_attr(0.98f), m_max_speed(20.0f)
    {}

    const vector4& getVel()   { return m_vel; }
    const vector4& getAccel() { return m_accel; }
    virtual void setVel(const vector4& v)   { m_vel=v;   m_vel.w=0.0f; }
    virtual void setAccel(const vector4& v) { m_accel=v; m_accel.w=0.0f; }

    float getAccelResist()    { return m_accel_resist; }
    float getMaxSpeed()       { return m_max_speed; }
    float getVelAttenuation() { return m_vel_attr; }
    virtual void setAccelResist(float v)   { m_accel_resist=v; }
    virtual void setMaxSpeed(float v)      { m_max_speed=v; }
    virtual void setVelAttenuation(float v){ m_vel_attr=v; }

    virtual void accel(const vector4& v)
    {
      if(layer_ptr g=GetParentLayer(this->getParent())) {
        setVel(getVel()+g->getIMatrix()*vector4(v.x, v.y, v.z, 0.0f)*getAccelResist());
      }
      else {
        setVel(getVel()+v*getAccelResist());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      vector4 vel = getVel()+getAccel();
      float speed = vel.norm();
      if(speed>=getMaxSpeed()) {
        vel = (vel/speed)*std::max<float>(speed*getVelAttenuation(), getMaxSpeed());
      }
      setVel(vel);
    }

    void onAccel(AccelMessage& m)
    {
      accel(m.getAccel());
    }


    bool call(const string& name, const boost::any& value)
    {
      if(name=="setVel")        setVel(*any_cast<vector4>(&value));
      else if(name=="setAccel") setAccel(*any_cast<vector4>(&value));
      else if(name=="accel")    accel(*any_cast<vector4>(&value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& vel = getVel();
      const vector4& acell = getAccel();
      sprintf(buf, "  vel: %.2f, %.2f, %.2f\n", vel.x, vel.y, vel.z);
      r+=buf;
      sprintf(buf, "  accel: %.2f, %.2f, %.2f\n", acell.x, acell.y, acell.z);
      r+=buf;
      return r;
    }
  };

  // ì¡íËï˚å¸Çå¸Ç≠ 
  template<class T>
  class HaveDirection : public T
  {
  typedef T Super;
  private:
    vector4 m_dir;

  public:
    HaveDirection(Deserializer& s) : Super(s)
    {
      s >> m_dir;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_dir;
    }

  public:
    HaveDirection()
    {
      setDirection(vector4(1, 0, 0));
    }

    const vector4& getDirection() { return m_dir; }

    virtual void setDirection(const vector4& v)
    {
      this->modMatrix();
      m_dir = v.normal();
      if(fabsf(m_dir.x)==1.0f) {
        m_dir.y = 0.001f;
      }
      m_dir.normalize();
      m_dir.w=0.0f;
    }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      mat.aimVector2(m_dir);
    }

    bool call(const string& name, const boost::any& value)
    {
      if(name=="setDirection") setDirection(*any_cast<vector4>(&value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& dir = getDirection();
      sprintf(buf, "  dir: %.2f, %.2f, %.2f\n", dir.x, dir.y, dir.z);
      r+=buf;
      return r;
    }
  };


  // îCà”é≤âÒì]Ç∑ÇÈ 
  template<class T>
  class HaveSpinAttrib : public T
  {
  typedef T Super;
  private:
    vector4 m_axis;
    float m_rot;
    float m_rot_speed;

  public:
    HaveSpinAttrib(Deserializer& s) : Super(s)
    {
      s >> m_axis >> m_rot >> m_rot_speed;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_axis << m_rot << m_rot_speed;
    }

  public:
    HaveSpinAttrib() : m_rot(0.0f), m_rot_speed(0.0f)
    {
      setAxis(vector4(GetRand2(), GetRand2(), GetRand2()));
    }

    const vector4& getAxis() { return m_axis; }
    float getRotate()        { return m_rot; }
    float getRotateSpeed()   { return m_rot_speed; }
    virtual void setAxis(const vector4& v) { this->modMatrix(); m_axis=v.normal(); m_axis.w=0; }
    virtual void setRotate(float v)        { this->modMatrix(); m_rot=v; }
    virtual void setRotateSpeed(float v)   { m_rot_speed=v; }

    virtual void rotate(float r)
    {
      setRotate(getRotate()+r);
    }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      mat.rotateA(m_axis, m_rot);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      rotate(getRotateSpeed());
    }

    virtual bool call(const string& name, const boost::any& value)
    {
      if     (name=="setAxis")        setAxis(*any_cast<vector4>(&value));
      else if(name=="setRotate")      setRotate(any_cast<float>(value));
      else if(name=="setRotateSpeed") setRotateSpeed(any_cast<float>(value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& axis = getAxis();
      sprintf(buf, "  axis: %.2f, %.2f, %.2f\n", axis.x, axis.y, axis.z);
      r+=buf;
      sprintf(buf, "  rot: %.2f\n", getRotate());
      r+=buf;
      sprintf(buf, "  rotspeed: %.2f\n", getRotateSpeed());
      r+=buf;
      return r;
    }
  };

  // 2é≤âÒì] 
  template<class T>
  class HaveSpinAttrib2 : public T
  {
  typedef T Super;
  private:
    vector4 m_axis1;
    vector4 m_axis2;
    float m_rot;
    float m_rot_speed;

  public:
    HaveSpinAttrib2(Deserializer& s) : Super(s)
    {
      s >> m_axis1 >> m_axis2 >> m_rot >> m_rot_speed;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_axis1 << m_axis2 << m_rot << m_rot_speed;
    }

  public:
    HaveSpinAttrib2() : m_rot(0.0f), m_rot_speed(0.0f)
    {
      setAxis1(vector4(GetRand2(), GetRand2(), GetRand2()));
      setAxis2(vector4(GetRand2(), GetRand2(), GetRand2()));
    }

    float getRotate()         { return m_rot; }
    float getRotateSpeed()    { return m_rot_speed; }
    const vector4& getAxis1() { return m_axis1; }
    const vector4& getAxis2() { return m_axis2; }

    void setRotate(float v)      { this->modMatrix(); m_rot=v; }
    void setRotateSpeed(float v) { m_rot_speed=v; }
    void setAxis1(const vector4& v) { this->modMatrix(); m_axis1=v.normal(); m_axis1.w=0; }
    void setAxis2(const vector4& v) { this->modMatrix(); m_axis2=v.normal(); m_axis2.w=0; }

    virtual void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_axis1 = matrix44().rotateA(m_axis2, m_rot_speed) * m_axis1;
      setRotate(getRotate()+m_rot_speed);
    }

    virtual void updateMatrix(matrix44& mat)
    {
      mat.translate(this->getPosition().v);
      mat.rotateA(m_axis1, m_rot);
    }
  };

  template<class T>
  class SpinningCube : public Inherit2(HaveSpinAttrib2, T)
  {
  typedef Inherit2(HaveSpinAttrib2, T) Super;
  public:
    SpinningCube(Deserializer& s) : Super(s) {}
    SpinningCube()
    {
      this->setRotateSpeed(1.0f);
    }

    virtual void drawModel()
    {
      static IVertexBufferObject *model = GetVBO("cube").get();
      ist::MatrixSaver msaver(GL_MODELVIEW_MATRIX);
      glMultMatrixf(this->getMatrix().v);
      model->assign();
      model->draw();
      model->disassign();
    }

    void draw()
    {
      drawModel();
    }
  };


  // ñ≥ìGèÛë‘ÇéùÇ¬ 
  template<class T>
  class HaveInvincibleMode : public T
  {
  typedef T Super;
  private:
    bool m_invincible;
    vector4 m_emission;

  public:
    HaveInvincibleMode(Deserializer& s) : Super(s)
    {
      s >> m_invincible >> m_emission;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_invincible << m_emission;
    }

  public:
    HaveInvincibleMode() : m_invincible(true)
    {}

    virtual void setEmission(const vector4& v) { m_emission=v; }
    const vector4& getEmission() { return m_emission; }

    virtual void updateEmission()
    {
      if(m_invincible) {
        m_emission+=(vector4(0.4f, 0.4f, 0.5f)-m_emission)*0.05f;
      }
      else {
        m_emission+=(vector4(0, 0, 0)-m_emission)*0.05f;
      }
    }

    virtual void updateEmission(bool v)
    {
      if(!v) {
        m_emission = vector4(2.0f, 2.0f, 0.0f);
      }
      else if(!m_invincible) {
        m_emission = vector4(1.5f, 1.5f, 1.5f);
      }
    }

    void setInvincible(bool v)
    {
      updateEmission(v);
      m_invincible = v;
    }

    bool isInvincible() { return m_invincible; }

    void draw()
    {
      if(m_emission.x > 0.01f) {
        glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
      }
      else {
        Super::draw();
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      updateEmission();
    }

    void onDamage(DamageMessage& m)
    {
      if(!m_invincible) {
        Super::onDamage(m);
      }
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      sprintf(buf, "  invincible: %s\n", isInvincible() ? "true" : "false");
      r+=buf;
      return r;
    }
  };
}

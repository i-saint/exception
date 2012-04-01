#ifndef enemy_fraction_h
#define enemy_fraction_h

namespace exception {

  class Fraction : public Inherit3(SpinningCube, HavePosition, IFraction)
  {
  typedef Inherit3(SpinningCube, HavePosition, IFraction) Super;
  public:
    typedef TCache<Fraction> Factory;
    friend class TCache<Fraction>;
    void release()
    {
      Factory::insertNotUsed(this);
    }


    class Drawer : public BatchDrawer<Fraction>
    {
    typedef BatchDrawer<Fraction> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 1.0f; }
      void draw()
      {
        static IVertexBufferObject *model = GetVBO("cube").get();
        glShadeModel(GL_FLAT);
        glDisable(GL_BLEND);
        model->assign();
        Super::draw();
        model->disassign();
        glEnable(GL_BLEND);
        glShadeModel(GL_SMOOTH);
      }
    };

  private:
    player_ptr m_blower;
    player_ptr m_destroyed_by;
    point_collision m_collision;
    vector4 m_vel;
    vector4 m_accel;
    float m_emission;
    matrix44 m_draw_matrix;
    vector4 m_next_pos;
    bool m_next_pos_updated;
    bool m_increment_hit_counter;

  private:
    Fraction() :
      m_blower(0), m_destroyed_by(0), m_emission(0.0f),
      m_next_pos_updated(false), m_increment_hit_counter(false)
    {
      Register(this);
      Drawer::instance()->insert(this);

      setAccel(GetGlobalAccel());
      m_collision.setRadius(6.0f);
    }

    Fraction(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_blower);
      DeserializeLinkage(s, m_destroyed_by);
      s >> m_collision >> m_vel >> m_accel >> m_emission >> m_draw_matrix
        >> m_next_pos >> m_next_pos_updated >> m_increment_hit_counter;
    }

  public:
    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_blower);
      SerializeLinkage(s, m_destroyed_by);
      s << m_collision << m_vel << m_accel << m_emission << m_draw_matrix
        << m_next_pos << m_next_pos_updated << m_increment_hit_counter;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_blower);
      ReconstructLinkage(m_destroyed_by);
    }

  public:
    float getDrawPriority() { return -1.0f; }
    float getLife() { return m_vel.norm()>5.0f ? 100.0f : 1.0f; }
    virtual float getVolume() { return m_collision.getSphere().getVolume(); }
    const collision& getCollision() { return m_collision; }

    const vector4& getVel()        { return m_vel; }
    const vector4& getAccel()      { return m_vel; }
    void setVel(const vector4& v)  { m_vel=v; }
    void setAccel(const vector4& v){ m_accel=v; }

    void accel(const vector4& a)
    {
      m_vel+=a;
      if(m_vel.norm()>5.0f) {
        m_collision.setRadius(0.0f);
      }
    }

    void update()
    {
      if(isDestroyed()) {
        PutCubeExplode(getPosition());

        static ISound *s = GetSound("explosion1.wav").get();
        PlaySound(s, 0);

        if(m_increment_hit_counter) {
          AddHitCount(1);
        }
        if(m_destroyed_by) {
          m_destroyed_by->setEnergy(m_destroyed_by->getEnergy()+1.0f);
          AddScore(1.0f);
        }
      }
      else {
        m_collision.setPosition(getPosition());
        m_draw_matrix = getMatrix();
        KillIfOutOfScreen(this, rect(vector2(300)));
      }
    }

    void asyncupdate()
    {
      doCollide();
      processMessageQueue();
    }

    void doCollide()
    {
      message_queue& mq = getMessageQueue();

      float speed = m_vel.norm();
      vector4 col_normal;
      float col_dist = 0;
      int col_num = 0;

      for(size_t i=0; i<mq.size(); ++i) {
        if(mq[i]->getType()!=Message::COLLIDE) {
          continue;
        }
        CollideMessage& m = static_cast<CollideMessage&>(*mq[i]);
        gobj_ptr from = m.getFrom();
        col_dist+=m.getDistance();
        col_normal+=m.getNormal();
        ++col_num;

        if(speed > 5.0f) { // çÇë¨Ç≈è’ìÀÇµÇΩÇÁÉ_ÉÅÅ[ÉWó^Ç¶ÇÈ 
          SendDamageMessage(m_blower, from, GetFractionDamage(), this);
        }
      }

      if(col_num==0) {
        return;
      }

      if(speed > 5.0f) { // çÇë¨Ç≈è’ìÀÇµÇƒÇΩÇÁé©âÛ 
        SendDestroyMessage(m_blower, this);
        if(m_blower) {
          m_increment_hit_counter = true;
        }
        return;
      }

      for(size_t i=0; i<mq.size(); ++i) {
        if(mq[i]->getType()!=Message::COLLIDE || !IsSolid(mq[i]->getFrom()) || IsFraction(mq[i]->getFrom())) {
          continue;
        }
        for(size_t j=i+1; j<mq.size(); ++j) {
          if(mq[j]->getType()!=Message::COLLIDE || !IsSolid(mq[j]->getFrom()) || IsFraction(mq[j]->getFrom())) {
            continue;
          }
          CollideMessage& m1 = static_cast<CollideMessage&>(*mq[i]);
          CollideMessage& m2 = static_cast<CollideMessage&>(*mq[j]);
          if(m1.getNormal().dot(m2.getNormal()) < 0.0f) { // ã≤Ç‹ÇÍÇƒÇΩÇÁé©âÛ 
            SendDestroyMessage(0, this);
            goto hold_check_end;
          }
        }
      }
hold_check_end:;


      col_normal/=float(col_num);
      col_dist/=float(col_num);
      m_next_pos = getPosition() + col_normal*col_dist;
      m_next_pos_updated = true;
      float v = m_vel.norm();
      if(v > 0.5f && col_normal.dot(m_vel/v) < 0.0f) { // ê≥ñ è’ìÀÇµÇΩÇÁíµÇÀï‘Ç∑ 
        const float bounce = 0.4f;
        m_vel = (matrix44().rotateA(col_normal, 180.0f))*m_vel*(bounce*-1.0f);
      }
      else if(speed < 4.8f) { // í«ìÀÇµÇΩÇÁâ¡ë¨ 
        m_vel+=(col_normal*0.5f);
        if(m_vel.norm()>4.8f) {
          m_vel = m_vel.normalize()*4.8f;
        }
      }
    }



    virtual void drawModel()
    {
      static IVertexBufferObject *model = GetVBO("cube").get();
      ist::MatrixSaver msaver(GL_MODELVIEW_MATRIX);
      glMultMatrixf(m_draw_matrix.v);

      // assign/diasssign ÇÕDrawerë§Ç≈àÍäáèàóù 
      model->draw();
    }

    void draw()
    {
      if(m_emission < 0.02f) {
        drawModel();
      }
      else {
        float4 emission(m_emission, m_emission/4.0f, 0.0f, 1.0f);
        glMaterialfv(GL_FRONT, GL_EMISSION,  emission.v);
        drawModel();
        glMaterialfv(GL_FRONT, GL_EMISSION,  vector4().v);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_blower);

      m_vel+=m_accel;
      m_vel.z = 0.0f;
      float v = m_vel.norm();

      if(v > 5.0f) {
        m_collision.setRadius(0.0f);
        m_emission = std::min<float>(m_emission+0.12f, 1.2f);
      }
      else {
        m_collision.setRadius(6.0f);
        m_emission-=0.01f;
      }

      if(v > 3.0f) {
        m_vel*=0.985f;
      }

      vector4 pos = m_next_pos_updated ? m_next_pos : getPosition();
      m_next_pos_updated = false;
      pos+=m_vel;
      pos.z*=0.95f;
      setPosition(pos);
      m_collision.setPosition(pos);
    }

    void onDamage(DamageMessage& m)
    {
      if(m_vel.norm()<=5.0f) {
        SendDestroyMessage(m.getFrom(), this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(player_ptr pl=ToPlayer(m.getFrom())) {
        m_destroyed_by = pl;
      }
    }

    void onAccel(AccelMessage& m)
    {
      accel(m.getAccel());
      if(player_ptr pl=ToPlayer(m.getFrom())) {
        m_blower = pl;
      }
    }

    bool call(const string& name, const boost::any& value) {
      if(name=="setVel")    setVel(*any_cast<vector4>(&value));
      else if(name=="accel")accel(*any_cast<vector4>(&value));
      return false;
    }
  };




}
#endif

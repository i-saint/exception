#ifndef enemy_missile_h
#define enemy_missile_h

namespace exception {




  class Gravity : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    bool m_active;
    float m_strength;
    float m_radius;
    int m_lifetime;

  public:
    Gravity(Deserializer& s) : Super(s)
    {
      s >> m_active >> m_strength >> m_radius >> m_lifetime;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_active << m_strength << m_radius << m_lifetime;
    }

  public:
    Gravity() : m_active(true), m_strength(0.035f), m_radius(300.0f), m_lifetime(360)
    {
      Register(this);
    }

    float getDrawPriority() { return 6.0f; }

    float getStrength() {return m_strength; }
    float getRadius() { return m_radius; }
    int getLifetime() { return m_lifetime; }
    void setStrength(float v){ m_strength=v; }
    void setRadius(float v)  { m_radius=v; }
    void setLifetime(int v)  { m_lifetime=v; }

    void draw()
    {
      glDisable(GL_DEPTH_TEST);
      glColor4f(1,1,1, std::min<float>(float(m_lifetime)/60.0f, 1.0f)*0.3f);
      DrawSprite("gravity.png", getPosition(), vector4(m_radius), float(-m_lifetime*3.0f));
      glColor4f(1,1,1,1);
      glEnable(GL_DEPTH_TEST);
    }

    void absorb()
    {
      point_collision pc;
      cdetector cd;

      sphere vol(getPosition(), getRadius());
      gobj_iter& it = GetObjects(vol);
      while(it.has_next()) {
        if(solid_ptr p = ToSolid(it.iterate())) {
          const vector4& tpos = p->getPosition();
          if(vol.isInner(tpos)) {
            vector4 accel = -(tpos-getPosition()).normal()*getStrength();
            accel.z = 0.0f;
            SendAccelMessage(0, p, accel);
          }
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      absorb();

      if(--m_lifetime<=0) {
        SendDestroyMessage(0, this);
      }
    }
  };


  class MineBurst : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    int m_frame;
    int m_lifetime;
    float m_radius;
    float m_power;

  public:
    MineBurst(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_lifetime >> m_radius >> m_power;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_lifetime << m_radius << m_power;
    }

  public:
    MineBurst() : m_frame(0), m_lifetime(60), m_radius(30.0f), m_power(0.5f)
    {
      Register(this);
    }

    void setLifeTime(int v) { m_lifetime=v; }
    void setRadius(float v) { m_radius=v; }
    void setPower(float v)  { m_power=v; }

    void draw()
    {
      float opa = 1.0f;
      if(m_frame>=m_lifetime) {
        opa-=1.0f/5.0f*float(m_frame-m_lifetime);
      }
      glColor4f(1,1,1, opa);
      DrawSprite("burnerb.png", getPosition(), vector4(m_radius*1.5f+((m_radius/5.0f)*sinf(float(m_frame)*78.0f))) );
      glColor4f(1,1,1,1);
    }

    void attack()
    {
      cdetector cd;
      point_collision pc;
      pc.setPosition(getPosition());
      pc.setRadius(m_radius);
      gobj_iter &iter = GetObjects(pc.getBoundingBox());
      while(iter.has_next()) {
        solid_ptr s = ToSolid(iter.iterate());
        if(s && !IsFraction(s) && cd.detect(pc, s->getCollision())) {
          SendDamageMessage(0, s, m_power, this);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      if(m_frame < m_lifetime) {
        attack();
      }
      if(m_frame>=m_lifetime+5) {
        SendDestroyMessage(0, this);
      }
    }
  };



  class MineBase : public Inherit3(HaveSpinAttrib, HaveVelocity, Enemy)
  {
  typedef Inherit3(HaveSpinAttrib, HaveVelocity, Enemy) Super;
  protected:
    static const int s_box_count = 3;
    BoxModel *m_model[s_box_count];
    int m_frame;

  public:
    MineBase(Deserializer& s) : Super(s)
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
    MineBase() : m_frame(0)
    {
      setRotateSpeed(1.0f);
      setMaxSpeed(1.0f);
      setVelAttenuation(0.95f);
    }

    int getFrame() { return m_frame; }

    void drawModel()
    {
      Super::drawModel();
      for(size_t i=0; i<s_box_count; ++i) {
        m_model[i]->draw();
      }
    }

    virtual void move()
    {
      if(player_ptr p=GetNearestPlayer(getPosition())) {
        vector4 vel = getVel();
        vector4 dir = getParentIMatrix()*(p->getPosition()-getPosition()).normal();
        vel = vel+dir*0.025f;
        setVel(vel);
      }

      vector4 pos = getRelativePosition();
      pos+=getVel();
      pos.z*=0.95f;
      setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      ++m_frame;
      move();
      KillIfOutOfScreen(this, rect(vector2(50.0f)));
    }

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      gobj_ptr from = m.getFrom();
      if(IsSolid(from) && !IsFraction(from)) {
        SendDestroyMessage(from, this);
      }
    }

    virtual void effect()=0;
    virtual void explode()=0;

    int getFractionCount() { return 0; }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      effect();
      if(m.getStat()==0) {
        explode();
      }
    }
  };

  class Mine : public MineBase
  {
  typedef MineBase Super;
  public:
    Mine(Deserializer& s) : Super(s) {}
    Mine()
    {
      setBox(box(vector4(15.0f)));
      setLife(15.0f);

      box b[s_box_count] = {
        box(vector4(22.5f, 10.0f, 10.0f)),
        box(vector4(10.0f, 22.5f, 10.0f)),
        box(vector4(10.0f, 10.0f, 22.5f)),
      };
      for(int i=0; i<s_box_count; ++i) {
        m_model[i] = new BoxModel(this);
        m_model[i]->setBox(b[i]);
      }
    }

    void explode()
    {
      PutSmallImpact(getPosition());
    }
  };

  class GravityMine : public Mine
  {
  typedef Mine Super;
  public:
    GravityMine(Deserializer& s) : Super(s) {}
    GravityMine() {}

    void draw()
    {
      float emission = (sinf(float(getFrame()*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission,0,emission).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);
    }

    void effect()
    {
      Gravity *g = new Gravity();
      vector4 pos = getPosition();
      pos.z = 0.0f;
      g->setPosition(pos);
    }

    void onCollide(CollideMessage& m)
    {
      setPosition(getPosition()+m.getNormal());
      if(IsPlayer(m.getFrom())) {
        SendDestroyMessage(0, this);
      }
    }
  };

  class BurstMine : public Mine
  {
  typedef Mine Super;
  public:
    BurstMine(Deserializer& s) : Super(s) {}
    BurstMine() {}

    void draw()
    {
      float emission = (sinf(float(getFrame()*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission,emission*0.25f,0).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);
    }

    void effect()
    {
      MineBurst *g = new MineBurst();
      g->setPosition(getPosition());
      g->setRadius(35.0f);
      g->setLifeTime(60);
    }
  };


  class MiniMine : public MineBase
  {
  typedef MineBase Super;
  public:
    MiniMine(Deserializer& s) : Super(s) {}
    MiniMine()
    {
      setBox(box(vector4(10.0f)));
      setLife(5.0f);

      box b[s_box_count] = {
        box(vector4(15, 7, 7)),
        box(vector4(7, 15, 7)),
        box(vector4(7, 7, 15)),
      };
      for(int i=0; i<s_box_count; ++i) {
        m_model[i] = new BoxModel(this);
        m_model[i]->setBox(b[i]);
      }
    }

    virtual void explode()
    {
      PutTinyImpact(getTSM(), getPosition());
    }
  };

  class MiniBurstMine : public MiniMine
  {
  typedef MiniMine Super;
  public:
    MiniBurstMine(Deserializer& s) : Super(s) {}
    MiniBurstMine() {}

    void draw()
    {
      float emission = (sinf(float(getFrame()*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission,emission*0.25f,0).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);
    }

    void effect()
    {
      MineBurst *g = new MineBurst();
      g->setPosition(getPosition());
      g->setRadius(25);
      g->setLifeTime(30);

      PutSmallRedRing(getPosition());
    }
  };






  class Missile : public Inherit3(HaveVelocity, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveVelocity, HaveDirection, Enemy) Super;
  protected:
    int m_frame;
    float m_tail;

    vector4 m_move_target;
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_move_time;
    float m_pre_sin;
    bool m_scroll;

  public:
    Missile(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_tail >> m_move_target >> m_initial_pos >> m_target_pos
        >> m_move_time >> m_pre_sin >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_tail << m_move_target << m_initial_pos << m_target_pos
        << m_move_time << m_pre_sin << m_scroll;
    }

  public:
    Missile(bool scroll) :
      m_frame(0), m_tail(30.0f), m_move_time(30), m_pre_sin(0.0f), m_scroll(scroll)
    {
      setBox(box(vector4(30, 10, 10), vector4(0, -10, -10)));
      setLife(3.0f);
    }

    void setMoveTarget(const vector4& v) { m_move_target=v; }
    void setMoveTime(int v) { m_move_time=v; }

    void drawBurner()
    {
      DrawSprite("burner.png",
        getMatrix()*vector4(-5.0f, 0, 0),
        vector4(30.0f+::sinf(73.0f*m_frame*ist::radian)*8.0f),
        1.0f);

      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);
      {
        static ist::Texture *tex = GetTexture("ray.png").get();
        vector4 vertex[2];
        vertex[0] = getPosition()-(getParentMatrix()*getDirection()*m_tail);
        vertex[1] = getPosition();

        tex->assign();
        draw_polyline(vertex, vertex+2, GetCamera().getPosition(), 60);
        tex->disassign();
      }
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
    }

    void onConstruct(ConstructMessage& m)
    {
      m_initial_pos = getRelativePosition();
      m_target_pos = m_initial_pos+m_move_target;
      setAccel(getDirection()*0.04f);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      m_tail = std::min<float>(120, m_tail+1.0f);

      vector4 pos = getRelativePosition();
      if(m_frame<=m_move_time) {
        float q = sinf(90.0f/m_move_time*m_frame*ist::radian);
        pos+=(m_target_pos-m_initial_pos)*(q-m_pre_sin);
        m_pre_sin = q;
      }
      pos+=getVel();
      if(m_scroll) {
        pos+=getParentIMatrix()*GetGlobalScroll();
      }
      setPosition(pos);

      KillIfOutOfScreen(this, rect(vector2(300, 300)));
    }

    int getFractionCount() { return 0; }

    virtual void effect()=0;

    void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      gobj_ptr from = m.getFrom();
      if(IsSolid(from) && !IsFraction(from)) {
        SendDestroyMessage(from, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      effect();
    }
  };

  class BurstMissile : public Missile
  {
  typedef Missile Super;
  public:
    BurstMissile(Deserializer& s) : Super(s) {}
    BurstMissile(bool scroll) : Missile(scroll) {}

    void draw()
    {
      float emission = (sinf(float(m_frame*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission*0.8f, emission*0.2f,0).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);

      drawBurner();
    }

    void effect()
    {
      vector4 pos = getMatrix()*getBox().getCenter();
      PutTinyImpact(getTSM(), pos);

      MineBurst *g = new MineBurst();
      g->setPosition(pos);
      g->setRadius(50);
      g->setLifeTime(30);

      PutSmallRedRing(pos);
    }
  };

  class GravityMissile : public Missile
  {
  typedef Missile Super;
  public:
    GravityMissile(Deserializer& s) : Super(s) {}
    GravityMissile(bool scroll) : Missile(scroll) {}

    void draw()
    {
      float emission = (sinf(float(m_frame*5)*ist::radian)+1.0f)/2.0f;
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(emission,0,emission).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0,0,0).v);

      drawBurner();
    }

    void effect()
    {
      Gravity *g = new Gravity();

      vector4 pos = getPosition();
      pos.z = 0.0f;
      g->setPosition(pos);
      g->setRadius(200.0f);
    }
  };

}
#endif

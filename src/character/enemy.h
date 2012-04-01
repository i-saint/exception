#include "enemy_util.h"
#include "enemy_attrib.h"

namespace exception {


  template<class T>
  class Breakable : public T
  {
  typedef T Super;
  private:
    float m_delta_damage;
    float m_life;
    gobj_ptr m_from;

  public:
    Breakable() : m_delta_damage(0), m_life(1.0f), m_from(0) {}
    Breakable(Deserializer& s) : Super(s)
    {
      gid from;
      s >> m_delta_damage >> m_life >> from;
      PushLinkage(from);
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      m_from = PopLinkage();
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_delta_damage << m_life << ToID(m_from);
    }

    virtual bool isDamaged()       { return getDeltaDamage()>0.0f; }
    virtual float getDeltaDamage() { return m_delta_damage; }
    virtual float getLife()        { return m_life; }

    virtual void setDeltaDamage(float v) { m_delta_damage=v; }
    virtual void setLife(float v)        { m_life=v; }

    void update()
    {
      m_delta_damage = 0.0f;
      m_from = 0;
      message_queue& mq = this->getMessageQueue();
      for(message_queue::iterator p=mq.begin(); p!=mq.end(); ++p) {
        message_ptr m = *p;
        if(m->getType()==Message::DAMAGE) {
          gobj_ptr from = m->getFrom();
          if(IsPlayer(m->getFrom())) {
            m_from = from;
          }
        }
      }
      Super::update();
    }

    virtual void damage(float d)
    {
      if(d>0.0f) {
        m_delta_damage+=d;
        m_life-=d;
      }
    }

    virtual void onLifeZero(gobj_ptr from)
    {
      SendDestroyMessage(from, this);
    }

    void onDamage(DamageMessage& m)
    {
      Super::onDamage(m);
      if(m_life>0.0f) {
        damage(m.getDamage());
        if(m_life<=0.0f) {
          onLifeZero(m_from ? m_from : m.getFrom());
        }
      }
    }


    bool call(const string& name, const boost::any& value)
    {
      if(name=="setLife") setLife(any_cast<float>(value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      sprintf(buf, "  life: %.2f\n", getLife());
      return r+buf;
    }
  };


  template<class T>
  class HaveLifeGauge : public T
  {
  typedef T Super;
  private:
    size_t m_frame_damaged;

  public:
    HaveLifeGauge() : m_frame_damaged(0) {}
    HaveLifeGauge(Deserializer& s) : Super(s)
    {
      s >> m_frame_damaged;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame_damaged;
    }

    void drawGauge()
    {
      if(m_frame_damaged+60>=GetPast()) {
        vector2 center = GetProjectedPosition(this->getPosition());
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        DrawRect(center, center+vector2(this->getLife()/5.0f, 5));
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      }
    }

    virtual void draw()
    {
      drawGauge();
    }

    virtual void onDamage(DamageMessage& m)
    {
      Super::onDamage(m);
      m_frame_damaged = GetPast();
    }
  };


  template<class T>
  class HaveHitFlash : public T
  {
  typedef T Super;
  private:
    size_t m_flash_cycle;
    size_t m_frame_damaged;

  public:
    HaveHitFlash() : m_flash_cycle(0), m_frame_damaged(0) {}
    HaveHitFlash(Deserializer& s) : Super(s)
    {
      s >> m_flash_cycle >> m_frame_damaged;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_flash_cycle << m_frame_damaged;
    }

    virtual void drawLifeGauge()
    {
      vector2 center = GetProjectedPosition(this->getCenter());
      glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
      DrawRect(center, center+vector2(this->getLife()/5.0f, 5));
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    void draw()
    {
      if(this->isDamaged()) {
        if(m_flash_cycle%4<2) {
          if(this->getDeltaDamage()>1.0f) {
            glMaterialfv(GL_FRONT, GL_EMISSION, vector4(1.0f, 0.0f, 0.0f).v);
          }
          else {
            glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.8f, 0.3f, 0.0f).v);
          }
          Super::draw();
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        }
        else {
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.5f, 0.3f, 0.0f).v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        }
      }
      else {
        Super::draw();
      }

      if(this->isDamaged()) {
        ++m_flash_cycle;
      }
      else {
        m_flash_cycle = 0;
      }

      if(m_frame_damaged+60>=GetPast()) {
        drawLifeGauge();
      }
    }

    virtual void damage(float d)
    {
      Super::damage(d);
      if(d>0.0f) {
        m_frame_damaged = GetPast();
      }
    }
  };


  controler_ptr DeserializeControler(Serializer& s);

  template<class T>
  class HaveControler : public T
  {
  typedef T Super;
  private:
    controler_ptr m_controler;

  public:
    HaveControler() {}
    HaveControler(Deserializer& s) : Super(s)
    {
      m_controler = DeserializeControler(s);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      if(m_controler) {
        m_controler->reconstructLinkage();
      }
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeControler(s, m_controler);
    }

    controler_ptr getControler() { return m_controler; }
    virtual void setControler(controler_ptr v)
    {
      if(v) { v->setObject(this); }
      m_controler = v;
    }

    virtual void onConstruct(ConstructMessage& m)
    {
      Super::onConstruct(m);
      if(m_controler) { m_controler->onConstruct(m); }
    }

    virtual void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      if(m_controler) { m_controler->onUpdate(m); }
    }

    virtual void onKill(KillMessage& m)
    {
      if(m_controler) {
        m_controler->onKill(m);
        m_controler = 0;
      }
      Super::onKill(m);
    }

    virtual void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      if(m_controler) { m_controler->onDestroy(m); }
    }

    virtual void onDamage(DamageMessage& m)
    {
      Super::onDamage(m);
      if(m_controler) { m_controler->onDamage(m); }
    }

    virtual void onCollide(CollideMessage& m)
    {
      Super::onCollide(m);
      if(m_controler) { m_controler->onCollide(m); }
    }

    virtual void onAccel(AccelMessage& m)
    {
      Super::onAccel(m);
      if(m_controler) { m_controler->onAccel(m); }
    }

    virtual void onCall(CallMessage& m)
    {
      Super::onCall(m);
      if(m_controler) { m_controler->onCall(m); }
    }

    virtual bool call(const string& name, const boost::any& value)
    {
      if(Super::call(name, value)) {
        return true;
      }
      else if(m_controler) {
        return m_controler->call(name, value);
      }
      return false;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      if(controler_ptr c = getControler()) {
        sprintf(buf, "  controler: %s\n", typeid(*c).name());
      }
      else {
        sprintf(buf, "  controler: none\n");
      }
      return r+buf;
    }
  };


  template<class T>
  class HaveBoxCollision : public T
  {
  typedef T Super;
  public:
    class collision_t : public box_collision
    {
    typedef box_collision Super;
    private:
      int m_type;

    public:
      collision_t()
      {
        m_type = ist::CM_BOX;
      }

      void enableCollision(bool v)
      {
        m_type = v ? ist::CM_BOX : ist::CM_FALSE;
      }

      int getType() const { return m_type; }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_type;
      }

      void deserialize(Deserializer& s)
      {
        Super::deserialize(s);
        s >> m_type;
      }
    };

  private:
    collision_t m_collision;

  protected:
    HaveBoxCollision() {}
    HaveBoxCollision(Deserializer& s) : Super(s)
    {
      m_collision.deserialize(s);
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      m_collision.serialize(s);
    }

    virtual void updateCollisionMatrix()
    {
      const matrix44& m = this->getMatrix();
      matrix44 im = m;
      im.transpose();

      m_collision.setMatrix(m, im);
      m_collision.setBox(this->getBox());
    }

    virtual void enableCollision(bool v) { m_collision.enableCollision(v); }

  public:
    virtual const collision& getCollision() { return m_collision; }
  };

  template<class T>
  class HavePointCollision : public T
  {
  typedef T Super;
  private:
    point_collision m_collision;
    float m_radius;

  protected:
    virtual void updateCollisionMatrix()
    {
      m_collision.setPosition(this->getPosition());
      m_collision.setRadius(m_radius);
    }

  public:
    HavePointCollision() : m_radius(0.0f)
    {}

    HavePointCollision(Deserializer& s) : Super(s)
    {
      s >> m_collision >> m_radius;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_collision << m_radius;
    }


    virtual void setRadius(float v) { m_radius=v; }
    virtual float getRadius() { return m_radius; }

    virtual const point_collision& getCollision() { return m_collision; }
  };



  template<class T>
  class TPointCollision : public T
  {
  typedef T Super;
  private:
    gobj_ptr m_parent;
    point_collision m_col;

  public:
    TPointCollision() : m_parent(0)
    {}

    TPointCollision(Deserializer& s) : Super(s)
    {
      gid parent;
      s >> parent >> m_col;
      PushLinkage(parent);
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      m_parent = PopLinkage();
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << ToID(m_parent) << m_col;
    }

    const collision& getCollision() { return m_col; }
    float getVolume() { return m_col.getSphere().getVolume(); }
    const vector4& getPosition() { return m_parent->getPosition(); }
    const matrix44& getMatrix() { return m_parent->getMatrix(); }

    void setParent(gobj_ptr v) { m_parent=v; }
    gobj_ptr getParent() { return m_parent; }
    void setRadius(float v) { m_col.setRadius(v); }
    float getRadius() { return m_col.getRadius(); }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(!m_parent || m_parent->isDead()) {
        SendKillMessage(0, this);
        return;
      }

      m_col.setPosition(getPosition());
    }
  };


  template<class T>
  class TBoxCollision : public T
  {
  typedef T Super;
  private:
    gobj_ptr m_parent;
    box_collision m_col;

  public:
    TBoxCollision() : m_parent(0)
    {}

    TBoxCollision(Deserializer& s) : Super(s)
    {
      gid parent;
      s >> parent >> m_col;
      PushLinkage(parent);
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      m_parent = PopLinkage();
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << ToID(m_parent) << m_col;
    }

    const collision& getCollision() { return m_col; }
    float getVolume() { return m_col.getBox().getVolume(); }
    const vector4& getPosition() { return m_parent->getPosition(); }
    const matrix44& getMatrix() { return m_parent->getMatrix(); }

    void setParent(gobj_ptr v) { m_parent=v; }
    gobj_ptr getParent() { return m_parent; }
    void setBox(const box& v) { m_col.setBox(v); }
    void setBox(const vector4& ur, const vector4& bl) { setBox(box(ur, bl)); }
    const box& getBox() { return m_col.getBox(); }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(!m_parent || m_parent->isDead()) {
        SendKillMessage(0, this);
        return;
      }
      const matrix44& mat = getMatrix();
      matrix44 im = mat;
      im.transpose();
      m_col.setMatrix(mat, im);
    }
  };


  template<class T>
  class HaveParent : public T
  {
  typedef T Super;
  private:
    gobj_ptr m_parent;
    vector4 m_tpos;
    matrix44 m_pmat;
    bool m_modpos;

  protected:
    void modPosition() { m_modpos=true; }


    virtual void updateTPosition(vector4& tpos)
    {
      tpos = getParentMatrix()*getRelativePosition();
    }

  public:
    HaveParent() : m_parent(0), m_modpos(false)
    {}

    HaveParent(Deserializer& s) : Super(s), m_parent(0)
    {
      gid parent;
      s >> parent >> m_tpos >> m_pmat >> m_modpos;
      PushLinkage(parent);
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      m_parent = PopLinkage();
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << ToID(m_parent) << m_tpos << m_pmat << m_modpos;
    }


    void setParent(gobj_ptr p)
    {
      if(layer_ptr old = ToLayer(getParent())) {
        old->unchain();
      }
      if(layer_ptr g = ToLayer(p)) {
        g->chain();
      }
      m_parent = p;

      if(p) {
        setParentMatrix(p->getMatrix());
      }
    }

    virtual void setParentMatrix(const matrix44& v)
    {
      m_pmat = v;
      modPosition();
      this->modMatrix();
    }

    virtual void setPosition(const vector4& v)
    {
      this->Super::setPosition(v);
      modPosition();
    }

    virtual gobj_ptr getParent() { return m_parent; }
    virtual const matrix44& getParentMatrix() { return m_pmat; }
    virtual const matrix44& getParentIMatrix()
    {
      if(layer_ptr g=ToLayer(getParent())) {
        return g->getIMatrix();
      }
      else {
        static matrix44 mat;
        mat = matrix44(getParentMatrix()).invert();
        return mat;
      }
    }

    virtual const vector4& getRelativePosition() { return Super::getPosition(); }
    virtual const vector4& getPosition()
    {
      if(m_modpos) {
        m_modpos = false;
        updateTPosition(m_tpos);
      }
      return m_tpos;
    }

    virtual void updateMatrix(matrix44& mat)
    {
      mat*=getParentMatrix();
      mat.translate(getRelativePosition().v);
    }

    virtual void onParentDestroyed() { SendDestroyMessage(0, this); }
    virtual void onParentKilled() { SendKillMessage(0, this); }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      if(m_parent) {
        setParentMatrix(m_parent->getMatrix());
        if(m_parent->isDestroyed()) {
          onParentDestroyed();
          setParent(0);
        }
        else if(m_parent->isKilled()) {
          onParentKilled();
          setParent(0);
        }
      }
    }

    void onKill(KillMessage& m)
    {
      if(layer_ptr old = ToLayer(getParent())) {
        old->unchain();
      }
      Super::onKill(m);
    }

    virtual bool call(const string& name, const any& value)
    {
      if(name=="setParent") setParent(any_cast<gobj_ptr>(value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& pos = getRelativePosition();
      sprintf(buf, "  relpos: %.2f, %.2f, %.2f\n", pos.x, pos.y, pos.z);
      r+=buf;
      if(gobj_ptr p = getParent()) {
        sprintf(buf, "  parent: %s\n", typeid(*p).name());
      }
      else {
        sprintf(buf, "  parent: none\n");
      }
      r+=buf;
      return r;
    }
  };


  template<class T>
  class HaveBoxBound : public T
  {
  typedef T Super;
  private:
    box m_bound;

  public:
    HaveBoxBound() : m_bound(vector4(1500))
    {}

    HaveBoxBound(Deserializer& s) : Super(s)
    {
      s >> m_bound;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_bound;
    }

    const box& getBound() { return m_bound; }
    void setBound(const box& v) { m_bound=v; }

    virtual void checkBound()
    {
      KillIfOutOfBox(this, m_bound);
    }

    void onUpdate(UpdateMessage& m)
    {
      checkBound(); // getPosition()‚Ì‹y‚Ú‚·‰e‹¿‚Ì“s‡ãA‚±‚ê‚ªæ 
      Super::onUpdate(m);
    }
  };






  class Optional : public Inherit2(HavePosition, GameObject)
  {
  typedef Inherit2(HavePosition, GameObject) Super;
  public:
    Optional()
    {
      Register(this);
    }

    Optional(Deserializer& s) : Super(s)
    {}
  };


  class LayerBase : public Inherit2(HavePosition, ILayer)
  {
  typedef Inherit2(HavePosition, ILayer) Super;
  private:
    int m_ref;
    matrix44 m_imat;

  public:
    LayerBase() : m_ref(0)
    {
      Register(this);
    }

    LayerBase(Deserializer& s) : Super(s)
    {
      s >> m_ref >> m_imat;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_ref << m_imat;
    }

    virtual void chain() { ++m_ref; }
    virtual void unchain() { if(--m_ref==0) { SendKillMessage(0, this); } }

    const matrix44& getIMatrix() { return m_imat; }

    void update()
    {
      Super::update();
      m_imat = matrix44(getMatrix()).invert();
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[32];
      sprintf(buf, "  ref: %d\n", m_ref);
      r+=buf;
      return r;
    }
  };

  class ChildLayer : public Inherit2(HaveParent, LayerBase)
  {
  typedef Inherit2(HaveParent, LayerBase) Super;
  public:
    ChildLayer() {}
    ChildLayer(Deserializer& s) : Super(s) {}
  };

  class Layer : public ChildLayer
  {
  typedef ChildLayer Super;
  public:
    Layer()
    {
      setParent(GetGlobals());
    }
    Layer(Deserializer& s) : Super(s) {}

    void setParent(gobj_ptr p)
    {
      Super::setParent(GetParentLayer(p));
    }
  };


  class RotLayer : public Inherit2(HaveSpinAttrib, Layer)
  {
  typedef Inherit2(HaveSpinAttrib, Layer) Super;
  public:
    RotLayer()
    {
      setAxis(vector4(0, 0, 1));
    }
    RotLayer(Deserializer& s) : Super(s) {}
  };

  class ChildRotLayer : public Inherit2(HaveSpinAttrib, ChildLayer)
  {
  typedef Inherit2(HaveSpinAttrib, ChildLayer) Super;
  public:
    ChildRotLayer()
    {
      setAxis(vector4(0, 0, 1));
    }
    ChildRotLayer(Deserializer& s) : Super(s) {}
  };

  class ScrolledLayer : public Inherit2(HaveBoxBound, Layer)
  {
  typedef Inherit2(HaveBoxBound, Layer) Super;
  public:
    ScrolledLayer()
    {
      setBound(box(vector4(2000), vector4(-2000)));
    }
    ScrolledLayer(Deserializer& s) : Super(s) {}

    virtual void scroll()
    {
      vector4 pos = getRelativePosition();
      pos+=getParentIMatrix()*GetGlobalScroll();
      setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      scroll();
    }
  };




  typedef Inherit2(TPointCollision, IGround) PointCollision;
  typedef Inherit2(TBoxCollision, IGround) BoxCollision;
  typedef Inherit3(TPointCollision, Breakable, IEnemy) BreakablePointCollision;
  typedef Inherit3(TBoxCollision, Breakable, IEnemy) BreakableBoxCollision;


  typedef Inherit9(HaveBoxBound, HaveParent, HaveBoxCollision, HaveHitFlash, Box, HaveControler, HavePosition, Breakable, IEnemy) EnemyBase;
  class BasicEnemy : public EnemyBase
  {
  typedef EnemyBase Super;
  private:
    float m_energy;

  public:
    BasicEnemy() : m_energy(0.0f)
    {
      Register(this);
    }

    BasicEnemy(Deserializer& s) : Super(s)
    {
      s >> m_energy;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_energy;
    }

    void setEnergy(float v) { m_energy=v; }
    float getEnergy() { return m_energy; }

    void update()
    {
      Super::update();
      updateCollisionMatrix();
    }

    void checkBound()
    {
      if(!getBound().isInner(getCenter())) {
        SendKillMessage(0, this);
      }
    }

    virtual int getFractionCount()
    {
      return int(getVolume()/powf(37.5f, 3.0f)*GetFractionRate());
    }

    virtual int getExplodeCount()
    {
      return int(getVolume()/8000.0f);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(player_ptr p=ToPlayer(m.getFrom())) {
        p->setEnergy(p->getEnergy()+getEnergy());
      }

      if(m.getStat()==0) {
        SplinkleCube(getBox(), getMatrix(), getFractionCount());
      }
      PutSmallExplode(getBox(), getMatrix(), getExplodeCount());
    }
  };

  class ChildEnemy : public BasicEnemy
  {
  typedef BasicEnemy Super;
  public:
    ChildEnemy() {}
    ChildEnemy(Deserializer& s) : Super(s) {}

    void checkBound()
    {
      if(!GetParentSolid(this)) {
        Super::checkBound();
      }
    }

    void setParent(gobj_ptr p)
    {
      Super::setParent(p);
      if(solid_ptr s=ToSolid(p)) {
        setGroup(s->getGroup());
      }
    }
  };


  class Enemy : public BasicEnemy
  {
  typedef BasicEnemy Super;
  public:
    Enemy()
    {
      setParent(GetGlobals());
    }

    Enemy(Deserializer& s) : Super(s) {}

    void setParent(gobj_ptr p)
    {
      Super::setParent(GetParentLayer(p));
    }
  };



  class BoxModel : public Inherit4(HaveParent, Box, HavePosition, IEffect)
  {
  typedef Inherit4(HaveParent, Box, HavePosition, IEffect) Super;
  public:
    BoxModel(gobj_ptr parent)
    {
      Register(this);
      setParent(parent);
    }

    BoxModel(Deserializer& s) : Super(s)
    {}

    float getDrawPriority() { return -1.0f; } // –¾Ž¦“I‚Édraw‚µ‚È‚¢‚Æ•`‚©‚È‚¢ 

    void updateMatrix(matrix44& mat)
    {
      mat.translate(getRelativePosition().v);
    }
  };



#define Getter(name, type) virtual type name() { return get()->name(); }
#define Getter2(name, type, arg) virtual type name(arg v) { return get()->name(v); }
#define Setter(name, type) virtual void name(type v) { get()->name(v); }
#define Caller(name) virtual void name() { get()->name(); }

  template<class T>
  class TControler : public Inherit2(HaveThreadSpecificMethod, IControler)
  {
  typedef Inherit2(HaveThreadSpecificMethod, IControler) Super;
  public:
    typedef T parent_type;
  private:
    parent_type *m_parent;

  public:
    TControler() : m_parent(0) {}
    TControler(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parent);
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parent);
      if(m_parent) {
        setTSM(m_parent->getTSM());
      }
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parent);
    }

    parent_type* get() { return m_parent; }

    void setObject(gobj_ptr v)
    {
      m_parent = dynamic_cast<parent_type*>(v);
      if(v && !m_parent) { // Œ^‚ª‡‚í‚È‚©‚Á‚½‚ç—áŠO 
        throw Error("TControler::setObject()");
      }
      else if(m_parent) {
        setTSM(m_parent->getTSM());
      }
    }
  };




  template<class T>
  class TCache : public CacheInfo
  {
  private:
    typedef std::vector<T*> container;
    container m_all;
    container m_not_used;

    TCache()
    {}

    ~TCache()
    {
      for(size_t i=0; i<m_all.size(); ++i) {
        delete m_all[i];
      }
    }

    void insertNotUsed_(T *f)
    {
      f->~T();
      m_not_used.push_back(f);
    }

    T* create_()
    {
      T *f = 0;
      if(m_not_used.empty()) {
        f = new T();
        m_all.push_back(f);
      }
      else {
        f = m_not_used.back();
        new(f) T();
        m_not_used.pop_back();
      }
      return f;
    }

    T* create_(Deserializer& s)
    {
      T *f = 0;
      if(m_not_used.empty()) {
        f = new T(s);
        m_all.push_back(f);
      }
      else {
        f = m_not_used.back();
        new(f) T(s);
        m_not_used.pop_back();
      }
      return f;
    }

    static TCache& instance()
    {
      static TCache s_f;
      return s_f;
    }

  public:
    static void insertNotUsed(T *f)
    {
      return instance().insertNotUsed_(f);
    }

    static T* create()
    {
      return instance().create_();
    }

    static T* create(Deserializer& s)
    {
      return instance().create_(s);
    }

    string p()
    {
      char buf[256];
      sprintf(buf, "%s : %d\n", typeid(T).name(), m_all.size());
      return buf;
    }
  };
}

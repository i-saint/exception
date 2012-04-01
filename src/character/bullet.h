namespace exception {

  class BlueBlur : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    static BlueBlur *s_inst;
    fbo_ptr m_fbo;
    gobj_vector m_draw;
    vector2 m_center;

  public:
    static BlueBlur& instance()
    {
      if(!s_inst) {
        new BlueBlur();
      }
      return *s_inst;
    }

    BlueBlur()
    {
      if(s_inst) {
        throw Error("BlueBlur::BlueBlur()");
      }
      s_inst = this;
      Register(this);
    }

    ~BlueBlur()
    {
      s_inst = 0;
    }

    BlueBlur(Deserializer& s) : Super(s)
    {
      s_inst = this;
      DeserializeLinkageContainer(s, m_draw);
      s >> m_center;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkageContainer(m_draw);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkageContainer(s, m_draw);
      s << m_center;
    }

    float getDrawPriority() { return 1.1f; }
    void setCenter(const vector4& pos) { m_center = GetProjectedPosition(pos); }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_draw.clear();
    }

    void draw()
    {
      if(!GetConfig()->shader) {
        return;
      }
      if(!m_fbo) {
        m_fbo = new ist::FrameBufferObject(320, 240);
        m_fbo->enable();
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        DrawRect(vector2(640,480), vector2(0,0));
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        m_fbo->disable();
      }
      {
        // 前フレを色を薄めて拡大 
        float str = 5.0f;
        vector2 ur = (vector2(640.0f, 480.0f)-m_center)/vector2(640.0f, 480.0f) * str;
        vector2 bl = vector2(str, str)-ur;
        m_fbo->assign();
        m_fbo->enable();
        glEnable(GL_TEXTURE_2D);
        DrawRect(vector2(640.0f, 480.0f)+ur, vector2(0.0f, 0.0f)-bl);
        glDisable(GL_TEXTURE_2D);

        glColor4f(0.0f, 0.0f, 0.0f, 0.075f);
        DrawRect(vector2(640,480), vector2(0,0));
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        m_fbo->disable();
        m_fbo->disassign();

        m_fbo->enable();
        for(gobj_vector::iterator p=m_draw.begin(); p!=m_draw.end(); ++p) {
          (*p)->draw();
        }
        m_fbo->disable();
      }

      glEnable(GL_TEXTURE_2D);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      m_fbo->assign();
      glColor4f(0.1f, 0.1f, 1.0f, 0.7f);
      DrawRect(vector2(640.0f, 480.0f), vector2(0.0f, 0.0f));
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      m_fbo->disassign();
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
    }

    void append(gobj_ptr p)
    {
      m_draw.push_back(p);
    }
  };



  class Ray : public Inherit3(PolyLine, HavePosition, IBullet)
  {
  typedef Inherit3(PolyLine, HavePosition, IBullet) Super;
  private:
    gobj_ptr m_owner;
    solid_ptr m_target;
    point_collision m_collision;
    size_t m_past;
    float m_life;
    vector4 m_vel;

  public:
    Ray(gobj_ptr owner, const vector4& pos, const vector4& vel, solid_ptr target) :
      m_owner(owner), m_target(target), m_past(0), m_life(3.0f)
    {
      Register(this);

      setTexture("ray.png");
      m_collision.setRadius(5.0f);
      setSize(20);
      setWidth(15.0f);
      setVel(vel);
      setPosition(pos);
    }

    Ray(Deserializer& s) : Super(s)
    {
      setTexture("ray.png");
      DeserializeLinkage(s, m_owner);
      DeserializeLinkage(s, m_target);
      s >> m_collision >> m_past >> m_life >> m_vel;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
      ReconstructLinkage(m_target);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_owner);
      SerializeLinkage(s, m_target);
      s << m_collision << m_past << m_life << m_vel;
    }

    float getDrawPriority() { return 9.0f; }

    void setPosition(const vector4& pos)
    {
      Super::setPosition(pos);
      m_collision.setPosition(pos);
    }

    const collision& getCollision() { return m_collision; }


    gobj_ptr getOwner() { return m_owner; }
    gobj_ptr getTarget() { return m_target; }
    const vector4& getVel() { return m_vel; }
    void setOwner(gobj_ptr v) { m_owner=v; }
    void setTarget(solid_ptr v) { m_target=v; }
    void setVel(const vector4& v) { m_vel=v; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_past;
      if(m_owner && m_owner->isDead()) {
        m_owner = 0;
      }

      if(!m_target || m_target->isDead()) {
        m_target = nearest();
        if(!m_target) {
          SendDestroyMessage(0, this);
        }
      }
      if(m_target && !m_target->isDead()) {
        m_vel = ((m_vel*0.875f) + (m_target->getCenter()-getPosition()).normalize()*1.5f);

        vector4 v = (m_target->getPosition()-getPosition()).normal();
        vector4 dir = m_vel.normal();
        float dot = v.dot(dir);
        float dot2 = (matrix44().rotateZ(90.0f)*v).dot(dir);
        const float rot_speed = 1.0f;
        if( (dot>=0.0f && dot2<=0.0f)
          ||(dot<=0.0f && dot2<=0.0f)) {
          dir = matrix44().rotateZ(rot_speed)*dir;
        }
        else {
          dir = matrix44().rotateZ(-rot_speed)*dir;
        }
        m_vel = dir*m_vel.norm();
      }
      setPosition(getPosition()+m_vel);
      hit();

      if(m_life<=0 || m_past>240 || !IsInnerScreen(getPosition(), vector2(300, 300))) {
        SendDestroyMessage(0, this);
      }
      BlueBlur::instance().append(this);
    }

    enemy_ptr nearest()
    {
      enemy_ptr nearest = 0;
      float dist = 0.0f;
      for(int i=0; i<3; ++i) {
        gobj_iter& it = GetObjects(sphere(getPosition(), 100.0f+(200.0f*i)));
        while(it.has_next()) {
          enemy_ptr e = ToEnemy(it.iterate());
          if(!e) {
            continue;
          }
          if(!nearest) {
            dist = (e->getPosition()-getPosition()).norm();
            nearest = e;
          }
          else {
            float d = (e->getPosition()-getPosition()).norm();
            if(d < dist) {
              nearest = e;
              dist = d;
            }
          }
        }
        if(nearest) {
          break;
        }
      }
      return nearest;
    }

    virtual void hit()
    {
      cdetector cd;
      const collision& c = getCollision();
      gobj_iter& it = GetObjects(c.getBoundingBox());
      while(it.has_next()) {
        solid_ptr p = ToSolid(it.iterate());
        if(!p || IsPlayer(p)) {
          continue;
        }
        if(cd.detect(c, p->getCollision())) {
          setPosition(cd.getPosition());
          PutFlash(getPosition(), 40.0f);
          SendDamageMessage(m_owner, p, 1.0f, this);

          fraction_ptr fp = ToFraction(p);
          if(fp && fp->getLife()<=1.0f) { // 破片が対象の場合lifeがある限り貫通 
            SendDamageMessage(p, this, 1.0f);
          }
          else {
            SendDestroyMessage(p, this);
          }
          break;
        }
      }
    }

    void onDamage(DamageMessage& m)
    {
      Super::onDamage(m);
      m_life-=m.getDamage();
      if(m_life<=0.0f) {
        SendDestroyMessage(m.getFrom(), this);
      }
    }

    void onKill(KillMessage& m)
    {
      m_life = 0;
      Super::onKill(m);
    }
  };


  // 自機レーザー 
  class GLaser : public Inherit3(HaveTexture, HavePosition, IBullet)
  {
  typedef Inherit3(HaveTexture, HavePosition, IBullet) Super;
  public:
    class Point
    {
    public:
      point_collision col;
      vector4 dir;
      int pierce;

      Point() : pierce(2) {}

      void serialize(ist::bostream& b) const
      {
        b << col << dir << pierce;
      }

      void deserialize(ist::bistream& b)
      {
        b >> col >> dir >> pierce;
      }
    };

  private:
    std::deque<Point> m_points;
    gobj_ptr m_owner;
    gid m_group;
    vector4 m_dir;
    float m_power;
    float m_radius;
    float m_speed;
    bool m_stop;

  public:
    GLaser(gobj_ptr parent) :
        m_owner(parent), m_group(0),
        m_dir(1.0f, 0.0f, 0.0f), m_power(1.0f), m_radius(5.0f),
        m_speed(20.0f), m_stop(false)
    {
      Register(this);

      setTexture("gl.png");
      float power[5] = {
        1.0f, 1.0f, 0.6f, 0.45f, 0.4f,
      };
      m_power = power[GetPlayerCount()];
    }

    GLaser(Deserializer& s) : Super(s)
    {
      setTexture("gl.png");
      ist::deserialize_object_container(s, m_points);
      DeserializeLinkage(s, m_owner);
      s >> m_group >> m_dir >> m_power >> m_radius >> m_speed >> m_stop;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      ist::serialize_object_container(s, m_points);
      SerializeLinkage(s, m_owner);
      s << m_group << m_dir << m_power << m_radius << m_speed << m_stop;
    }

    float getDrawPriority() { return 9.0f; }
    const vector4& getDirection() { return m_dir; }
    gobj_ptr getOwner() { return m_owner; }
    float getPower()  { return m_power; }
    float getSpeed()  { return m_speed; }

    void setDirection(const vector4& v) { m_dir=v; }
    void setOwner(gobj_ptr v) { m_owner=v; }
    void setPower(float v) { m_power=v; }
    void setSpeed(float v) { m_speed=v; }
    void stop() { m_stop=true; }

    void draw()
    {
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);
      getTexture()->assign();

      static std::vector<vector4> vertex;
      for(std::deque<Point>::iterator q=m_points.begin(); q!=m_points.end(); ++q) {
        Point& pt = *q;
        if(pt.pierce<=0) {
          draw_polyline_fade(vertex.begin(), vertex.end(), GetCamera().getPosition(), 20.0f);
          vertex.clear();
        }
        else {
          vertex.push_back(pt.col.getPosition());
        }
      }
      draw_polyline_fade(vertex.begin(), vertex.end(), GetCamera().getPosition(), 20.0f);
      vertex.clear();

      getTexture()->disassign();
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
    }

    void hit()
    {
      cdetector cd;
      box bound(vector4(1000.0f));
      for(std::deque<Point>::iterator q=m_points.begin(); q!=m_points.end(); ++q) {
        Point& pt = *q;
        if(pt.pierce==0) {
          continue;
        }
        else if(!bound.isInner(pt.col.getPosition())) {
          pt.pierce = 0;
          continue;
        }

        gobj_iter& it = GetObjects(pt.col.getBoundingBox());
        while(it.has_next() && pt.pierce>0) {
          solid_ptr p = ToSolid(it.iterate());
          if(  !p
            || (m_group && p->getGroup()==m_group)
            || IsPlayer(p)
            || !cd.detect(pt.col, p->getCollision())) {
            continue;
          }

          PutFlash(cd.getPosition(), 30.0f);
          for(int i=0; i<2; ++i) {
            BlueParticle *bp = BlueParticle::Factory::create();
            bp->setPosition(cd.getPosition());
            bp->setLifeTime(5.0f+GenRand()*10.0f);
            bp->setVel((-cd.getNormal()+vector4(GenRand()*2-1.0f, GenRand()*2-1.0f, GenRand()*2-1.0f))*(1.5f+GenRand()*2.0f));
          }

          fraction_ptr fp = ToFraction(p);
          if(fp && fp->getLife()<=1.0f) {
            --pt.pierce;
          }
          else {
            pt.pierce = 0;
          }
          SendCollideMessage(this, p, cd.getPosition(), cd.getNormal(), cd.getDistance());
          SendDamageMessage(m_owner, p, m_power, this);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(m_owner && m_owner->isDead()) {
        m_owner = 0;
        m_stop = true;
      }

      for(int i=0; i<2; ++i) {
        for(std::deque<Point>::iterator q=m_points.begin(); q!=m_points.end(); ++q) {
          Point& pt = *q;
          pt.col.setPosition(pt.col.getPosition()+pt.dir*(m_speed*0.5f));
        }
        hit();
      }

      static std::vector<vector4> ppos;
      for(std::deque<Point>::iterator q=m_points.begin(); q!=m_points.end(); ++q) {
        Point& pt = *q;
        vector4 pos = pt.col.getPosition();
        ppos.push_back(pos);
        pt.col.setPosition(pos);
        if(!IsInnerScreen(pos, vector2(300.0f))) {
          pt.pierce = 0;
        }
      }
      for(size_t i=0; i<m_points.size(); ++i) {
        if(  i > 1
          && (m_points[i-1].pierce<=0 || m_points[i-2].pierce<=0 )
          && (i==m_points.size()-1 || m_points[i+1].pierce<=0)) {
          m_points[i].pierce = 0;
          m_points[i-1].pierce = 0;
        }
        if(i==0 || i==m_points.size()-1) {
          continue;
        }
        vector4 center = (ppos[i+1]+ppos[i-1])/2.0f;
        m_points[i].col.setPosition(ppos[i]+(center-ppos[i])*0.5f);
      }
      ppos.clear();

      while(!m_points.empty() && m_points.back().pierce==0) {
        m_points.pop_back();
      }

      if(!m_stop) {
        Point pt;
        pt.dir = m_dir;
        pt.col.setPosition(getPosition());
        pt.col.setRadius(5.0f);
        m_points.push_front(pt);
      }
      if(m_stop && m_points.empty()) {
        SendDestroyMessage(0, this);
      }

      BlueBlur::instance().append(this);
    }
  };


  class Laser : public Inherit4(HaveTexture, HaveParent, HavePosition, IBullet)
  {
  typedef Inherit4(HaveTexture, HaveParent, HavePosition, IBullet) Super;
  private:
    typedef std::vector<vector4> vertex_cont;
    vertex_cont m_vertex;
    gobj_ptr m_owner;
    gid m_group;
    float m_power;
    float m_length;
    float m_radius;
    float m_speed;
    int m_pierce;

    bool m_fade;
    float m_opa;

  protected:
    float getLength() { return m_length; }
    void setLength(float v){ m_length=v; }

  public:
    Laser(gobj_ptr owner) :
        m_owner(owner), m_group(0),
        m_power(1.5f), m_length(0.0f), m_radius(5.0f),
        m_speed(10.0f), m_pierce(2),
        m_fade(false), m_opa(1.0f)
    {
      Register(this);
      setTexture("laser.png");
    }

    Laser(Deserializer& s) : Super(s)
    {
      setTexture("laser.png");
      ist::deserialize_container(s, m_vertex);
      DeserializeLinkage(s, m_owner);
      s >> m_group >> m_power >> m_length >> m_radius >> m_speed >> m_pierce >> m_fade >> m_opa;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      ist::serialize_container(s, m_vertex);
      SerializeLinkage(s, m_owner);
      s << m_group << m_power << m_length << m_radius << m_speed << m_pierce << m_fade << m_opa;
    }

    float getDrawPriority() { return 9.0f; }
    gobj_ptr getOwner() { return m_owner; }
    float getPower()  { return m_power; }
    float getRadius() { return m_radius; }
    float getSpeed()  { return m_speed; }
    int getPierce() { return m_pierce; }

    void setOwner(gobj_ptr v) { m_owner=v; }
    void setPower(float v) { m_power=v; }
    void setRadius(float v){ m_radius=v; }
    void setSpeed(float v) { m_speed=v; }
    void setPierce(int v)  { m_pierce=v; }
    void setGroup(gid v)   { m_group=v; }

    void fade() { m_fade=true; }

    void draw()
    {
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);
      getTexture()->assign();
      draw_polyline_fade(m_vertex.begin(), m_vertex.end(), GetCamera().getPosition(), getRadius()*2.5f, m_opa);
      getTexture()->disassign();
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
    }

    void onConstruct(ConstructMessage& m)
    {
      PutSmallBlueRing(getPosition());
    }

    void hit()
    {
      box bound(vector4(1000.0f));
      int hits = 0;
      float span = m_radius;
      vector4 dir = getMatrix()*vector4(1,0,0,0);
      vector4 pos = getPosition();
      cdetector cd;
      ist::PointCollision c;
      c.setRadius(getRadius());
      c.setPosition(pos);

      m_vertex.clear();
      static std::vector<void*> checked;
      bool stop = false;
      for(int i=0; i<=m_length/span; ++i) {
        gobj_iter& it = GetObjects(c.getBoundingBox());
        while(it.has_next()) {
          solid_ptr p = ToSolid(it.iterate());
          if(  !p
            || (m_group && p->getGroup()==m_group)
            || p==m_owner
            || !cd.detect(c, p->getCollision())
            || std::find(checked.begin(), checked.end(), p)!=checked.end()) {
            continue;
          }
          checked.push_back(p);

          if(!m_fade) {
            PutFlash(cd.getPosition(), 5.0f+getRadius()*2.0f);
            SendCollideMessage(this, p, cd.getPosition(), cd.getNormal(), cd.getDistance());
            SendDamageMessage(m_owner, p, m_power, this);
          }

          ++hits;
          fraction_ptr fp = ToFraction(p);
          if(hits>m_pierce || (!fp || fp->getLife()>1.0f)) {
            m_length = span*i;
            stop = true;
            break;
          }
          if(stop) { break; }
        }
        m_vertex.push_back(pos);
        pos+=(dir*span);
        c.setPosition(pos);
        if(stop) { break; }
        else if(!bound.isInner(pos)) {
          m_length = span*i;
          break;
        }
      }
      checked.clear();
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(solid_ptr s=ToSolid(m_owner)) {
        m_group = s->getGroup();
      }

      m_length+=m_speed;
      hit();
      if(m_fade) {
        m_opa-=0.02f;
        if(m_opa<0.2f) {
          SendDestroyMessage(0, this);
        }
      }

      if(m_owner && m_owner->isDead()) {
        m_owner = 0;
        fade();
      }
      BlueBlur::instance().append(this);
    }

    void onParentDestroyed()
    {
      fade();
    }

    void onParentKilled()
    {
      fade();
    }
  };


  class LaserBit : public Inherit3(HaveDirection, HavePosition, IEffect)
  {
  typedef Inherit3(HaveDirection, HavePosition, IEffect) Super;
  private:
    Laser *m_laser;
    solid_ptr m_owner;
    int m_frame;
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_move_frame;

  public:
    LaserBit(solid_ptr owner) : m_laser(0), m_owner(owner), m_frame(0), m_move_frame(20)
    {
      Register(this);
    }

    LaserBit(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_laser);
      DeserializeLinkage(s, m_owner);
      s >> m_frame >> m_initial_pos >> m_target_pos >> m_move_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_laser);
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_laser);
      SerializeLinkage(s, m_owner);
      s << m_frame << m_initial_pos << m_target_pos << m_move_frame;
    }

    static LaserBit* create(solid_ptr owner, const vector4& pos, const vector4& tar, const vector4& dir, int f=20)
    {
      LaserBit *o = new LaserBit(owner);
      o->setPosition(pos);
      o->setTargetPosition(tar);
      o->setDirection(dir);
      o->setMoveFrame(f);
      return o;
    }

    float getDrawPriority() { return 1.1f; }
    void setTargetPosition(const vector4& v) { m_target_pos=v; }
    void setMoveFrame(int v) { m_move_frame=v; }

    virtual Laser* fire()
    {
      Laser *l = new Laser(m_owner);
      l->setParent(this);
      l->setRadius(25.0f);
      l->setPower(0.5f);
      l->setSpeed(20.0f);
      return l;
    }

    void draw()
    {
      glDisable(GL_DEPTH_TEST);
      DrawSprite("burnerb.png",
        getPosition(),
        vector4(35.0f+::sinf(m_frame*23.0f*ist::radian)*6.0f));
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;

      if(m_frame==1) {
        m_initial_pos = getPosition();
      }
      if(m_frame<=m_move_frame) {
        float p = float(m_frame)/float(m_move_frame)*90.0f*ist::radian;
        setPosition(m_initial_pos+(m_target_pos-m_initial_pos)*sinf(p));

      }
      else if(m_frame==m_move_frame+30) {
        m_laser = fire();
      }
      else if(m_frame==m_move_frame+30+80) {
        m_laser->fade();
      }

      if(m_owner && m_owner->isDead()) {
        SendDestroyMessage(0, this);
      }
      if(m_laser && m_laser->isDead()) {
        SendDestroyMessage(0, this);
      }
    }
  };


  // 敵レイ 
  class RedRay : public Inherit4(HaveVelocity, PolyLine, HavePosition, IGround)
  {
  typedef Inherit4(HaveVelocity, PolyLine, HavePosition, IGround) Super;
  private:
    gobj_ptr m_owner;
    player_ptr m_blower;
    point_collision m_collision;
    int m_frame;
    float m_life;

  public:
    RedRay(gobj_ptr owner) : m_owner(owner), m_blower(0), m_frame(0), m_life(5.0f)
    {
      Register(this);
      m_collision.setRadius(3.0f);
      setSize(20);
      setWidth(25.0f);
      setTexture("rray.png");
    }

    RedRay(Deserializer& s) : Super(s)
    {
      setTexture("rray.png");
      DeserializeLinkage(s, m_owner);
      DeserializeLinkage(s, m_blower);
      s >> m_collision >> m_frame >> m_life;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
      ReconstructLinkage(m_blower);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_owner);
      SerializeLinkage(s, m_blower);
      s << m_collision << m_frame << m_life;
    }

    float getDrawPriority() { return 9.0f; }

    void setPosition(const vector4& pos)
    {
      Super::setPosition(pos);
      m_collision.setPosition(pos);
    }

    const collision& getCollision() { return m_collision; }
    float getVolume() { return m_collision.getSphere().getVolume(); }

    player_ptr getBlower() { return m_blower; }

    void draw()
    {
      glDisable(GL_DEPTH_TEST);
      Super::draw();
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      SweepDeadObject(m_blower);
      if(m_owner && m_owner->isDead()) {
        m_owner = 0;
        SendDestroyMessage(0, this);
      }

      if(m_frame<=60) {
        vector4 vel = getVel();
        vel = ((vel*0.95f) + (GetNearestPlayerPosition(getPosition())-getPosition()).normalize()*0.6f);

        vector4 v = (GetNearestPlayerPosition(getPosition())-getPosition()).normal();
        vector4 dir = vel.normal();
        float dot = v.dot(dir);
        float dot2 = (matrix44().rotateZ(90.0f)*v).dot(dir);
        const float rot_speed = 0.25f;
        if( (dot>=0.0f && dot2<=0.0f)
          ||(dot<=0.0f && dot2<=0.0f)) {
          dir = matrix44().rotateZ(rot_speed)*dir;
        }
        else {
          dir = matrix44().rotateZ(-rot_speed)*dir;
        }
        vel = dir*vel.norm();

        setVel(vel);
      }
      else {
        vector4 vel = getVel();
        setVel(vel+vel.normal()*0.05f);
      }
      setPosition(getPosition()+getVel());

      KillIfOutOfScreen(this, rect(vector2(150, 150)));
    }

    void onCollide(CollideMessage& m)
    {
      gobj_ptr from = m.getFrom();
      if(m_life<=0.0f || IsFraction(from)) {
        return;
      }
      m_life-=1.0f;
      if(m_life<=0.0f) {
        SendDestroyMessage(0, this);
        return;
      }

      PutFlash(getPosition(), 40.0f);
      if(IsPlayer(from)) {
        SendDamageMessage(this, from, 3.0f);
        SendDestroyMessage(0, this);
      }
      else if(IsFraction(from)) {
        SendDestroyMessage(0, from);
      }
      else if(IsGround(from) && !dynamic_cast<RedRay*>(from)) {
        vector4 vel = getVel();
        vel = (matrix44().rotateA(m.getNormal(), 180.0f))*vel*(0.98f*-1.0f);
        setVel(vel);
      }
    }

    void onAccel(AccelMessage& m)
    {
      const vector4& v = m.getAccel();
      accel(v*2.0f);
      if(player_ptr pl=ToPlayer(m.getFrom())) {
        setGroup(Solid::createGroupID());
        m_blower = pl;
      }
    }
  };

  class RedRayBit : public Inherit3(HaveDirection, HavePosition, IEffect)
  {
  typedef Inherit3(HaveDirection, HavePosition, IEffect) Super;
  private:
    solid_ptr m_owner;
    int m_frame;
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_move_frame;

  public:
    RedRayBit(solid_ptr owner) : m_owner(owner), m_frame(0), m_move_frame(20)
    {
      Register(this);
    }

    RedRayBit(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_owner);
      s >> m_frame >> m_initial_pos >> m_target_pos >> m_move_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_owner);
      s << m_frame << m_initial_pos << m_target_pos << m_move_frame;
    }

    static RedRayBit* create(solid_ptr owner, const vector4& pos, const vector4& tar, int f=20)
    {
      RedRayBit *o = new RedRayBit(owner);
      o->setPosition(pos);
      o->setTargetPosition(tar);
      o->setMoveFrame(f);
      return o;
    }

    float getDrawPriority() { return 1.1f; }
    void setTargetPosition(const vector4& v) { m_target_pos=v; }
    void setMoveFrame(int v) { m_move_frame=v; }

    void draw()
    {
      glDisable(GL_DEPTH_TEST);
      DrawSprite("burner.png",
        getPosition(),
        vector4(35.0f+::sinf(m_frame*23.0f*ist::radian)*6.0f));
      glEnable(GL_DEPTH_TEST);
    }

    virtual void fire()
    {
      vector4 dir = (GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      vector4 vel = dir*-15.0f;
      matrix44 mat = matrix44().rotateZ(10.0f);
      matrix44 mirror = matrix44().rotateA(dir, 180.0f);
      vector4 pos = getPosition()+dir*20.0f;
      for(int i=0; i<15; ++i) {
        RedRay *b = new RedRay(m_owner);
        b->setPosition(getPosition());
        b->setVel(vel);
        b->setGroup(m_owner->getGroup());

        if(i%2==0) {
          vel = mat*vel;
        }
        vel = mirror*vel;
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;

      if(m_frame==1) {
        m_initial_pos = getPosition();
      }
      if(m_frame<=m_move_frame) {
        float p = float(m_frame)/float(m_move_frame)*90.0f*ist::radian;
        setPosition(m_initial_pos+(m_target_pos-m_initial_pos)*sinf(p));

      }
      else if(m_frame==m_move_frame+30) {
        fire();
        PutSmallRedRing(getPosition());
        SendDestroyMessage(0, this);
      }

      if(m_owner && m_owner->isDead()) {
        SendDestroyMessage(0, this);
      }
      else if(enemy_ptr e = ToEnemy(m_owner)) {
        if(e->getLife()<0.0f) {
          SendDestroyMessage(0, this);
        }
      }
    }
  };

  class Blaster : public Inherit2(HavePosition, IBullet)
  {
  typedef Inherit2(HavePosition, IBullet) Super;
  private:
    gobj_ptr m_owner;
    vector4 m_dir;
    float m_speed;
    float m_accel;
    int m_frame;

  public:
    Blaster(gobj_ptr owner, const vector4& dir) :
        m_owner(owner), m_dir(dir), m_speed(0.0f), m_accel(0.02f), m_frame(0)
    {
      Register(this);
    }

    Blaster(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_owner);
      s >> m_dir >> m_speed >> m_accel >> m_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_owner);
      s << m_dir << m_speed << m_accel << m_frame;
    }

    gobj_ptr getOwner() { return m_owner; }

    void draw()
    {
      DrawSprite("flare.png",
        getPosition(),
        vector4(28.0f+::sinf(m_frame*34.0f*ist::radian)*3.0f));
    }

    void hit()
    {
      cdetector cd;
      point_collision c;
      c.setPosition(getPosition());
      c.setRadius(15.0f);

      gobj_iter& it = GetObjects(c.getBoundingBox());
      while(it.has_next()) {
        solid_ptr p = ToSolid(it.iterate());
        if(!p || p==m_owner) {
          continue;
        }
        if(solid_ptr s = ToSolid(m_owner)) {
          if(p->getGroup()==s->getGroup()) {
            continue;
          }
        }
        if(cd.detect(c, p->getCollision())) {
          if(fraction_ptr pp = ToFraction(p)) {
            vector4 n = cd.getNormal();
            n.z = 0;
            n.normalize();
            pp->setVel(pp->getVel()+n*0.2f);
          }
          else if(IsGround(p)) {
            SendDestroyMessage(0, this);
          }
          else {
            SendDamageMessage(m_owner, p, 1.0f, this);
          }
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      if(m_owner && m_owner->isDead()) { m_owner=0; }

      m_speed+=m_accel;
      vector4 pos = getPosition();
      pos+=m_dir*m_speed;
      setPosition(pos);
      hit();

      if(m_frame%5==0) {
        PutCubeExplode(getPosition());
      }

      if(!IsInnerScreen(getPosition(), vector2(20))) {
        SendKillMessage(0, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      for(int i=0; i<3; ++i) {
        PutCubeExplode(getPosition());
      }
    }
  };

}

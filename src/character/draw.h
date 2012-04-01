
namespace exception {


  template<class T>
  class HavePosition : public T
  {
  typedef T Super;
  private:
    vector4 m_pos;
    matrix44 m_matrix;
    bool m_mod;

  protected:
    void modMatrix() { m_mod=true; }

  public:
    HavePosition(Deserializer& s) : Super(s)
    {
      s >> m_pos >> m_matrix >> m_mod;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_pos << m_matrix << m_mod;
    }

  public:
    HavePosition() : m_mod(true) {}
    virtual const vector4& getPosition()       { return m_pos; }
    virtual void setPosition(const vector4& v) { modMatrix(); m_pos=v; }
    virtual void move(const vector4& v)        { setPosition(getPosition()+v); }

    virtual void updateMatrix(matrix44& mat)
    {
      mat.translate(getPosition().v);
    }

    virtual const matrix44& getMatrix()
    {
      if(m_mod) {
        m_mod = false;
        m_matrix.identity();
        updateMatrix(m_matrix);
      }
      return m_matrix;
    }

    virtual bool call(const string& name, const boost::any& value)
    {
      if     (name=="setPosition") setPosition(*any_cast<vector4>(&value));
      else if(name=="move")        move(*any_cast<vector4>(&value));
      else return Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& pos = getPosition();
      sprintf(buf, "  pos: %.2f, %.2f, %.2f\n", pos.x, pos.y, pos.z);
      r+=buf;
      return r;
    }
  };


  template<class T>
  class HaveTransform : public T
  {
  typedef T Super;
  private:
    vector4 m_pos;
    vector4 m_rot;
    vector4 m_scl;
    matrix44 m_matrix;
    bool m_mod;

  protected:
    void modMatrix() { m_mod=true; }

  public:
    HaveTransform(Deserializer& s) : Super(s)
    {
      s >> m_pos >> m_rot >> m_scl >> m_matrix >> m_mod;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_pos << m_rot << m_scl << m_matrix << m_mod;
    }

  public:
    HaveTransform() : m_mod(true), m_scl(1.0f, 1.0f, 1.0f) {}
    virtual const vector4& getPosition()       { return m_pos; }
    virtual const vector4& getRotate()         { return m_rot; }
    virtual const vector4& getScale()          { return m_scl; }
    virtual void setPosition(const vector4& v) { modMatrix(); m_pos=v; }
    virtual void setRotate(const vector4& v)   { modMatrix(); m_rot=v; }
    virtual void setScale(const vector4& v)    { modMatrix(); m_scl=v; }
    virtual void move(const vector4& v)        { setPosition(getPosition()+v); }

    virtual void updateMatrix(matrix44& mat)
    {
      mat.translate(getPosition().v);
      mat.rotate(getRotate().v);
      mat.scale(getScale().v);
    }

    virtual const matrix44& getMatrix()
    {
      if(m_mod) {
        m_mod = false;
        m_matrix.identity();
        updateMatrix(m_matrix);
      }
      return m_matrix;
    }

    virtual bool call(const string& name, const boost::any& value)
    {
      if     (name=="setPosition")setPosition(*any_cast<vector4>(&value));
      else if(name=="setRotate")  setRotate(*any_cast<vector4>(&value));
      else if(name=="setScale")   setScale(*any_cast<vector4>(&value));
      else if(name=="move")       move(*any_cast<vector4>(&value));
      else return this->Super::call(name, value);
      return true;
    }

    virtual string p()
    {
      string r = Super::p();
      char buf[256];
      const vector4& pos = getPosition();
      const vector4& rot = getRotate();
      const vector4& scl = getScale();
      sprintf(buf, "  pos: %.2f, %.2f, %.2f\n", pos.x, pos.y, pos.z);
      sprintf(buf, "  rot: %.2f, %.2f, %.2f\n", rot.x, rot.y, rot.z);
      sprintf(buf, "  scl: %.2f, %.2f, %.2f\n", scl.x, scl.y, scl.z);
      r+=buf;
      return r;
    }
  };




  template<class T>
  class HaveVBO : public T
  {
  private:
    vbo_ptr m_vbo;

  public:
    HaveVBO(Deserializer& s) : Super(s) {}
    HaveVBO() {}
    vbo_ptr getVBO() { return m_vbo; }
    void setVBO(vbo_ptr v)          { m_vbo=v; }
    void setVBO(const string& name) { setVBO(GetVBO(name)); }

    void drawModel()
    {
      if(vbo_ptr model = getVBO()) {
        ist::MatrixSaver msaver(GL_MODELVIEW_MATRIX);
        glMultMatrixf(this->getMatrix().v);
        model->draw();
      }
    }

    virtual void draw()
    {
      drawModel();
    }
  };


  template<class T>
  class HaveTexture : public T
  {
  typedef T Super;
  private:
    texture_ptr m_tex;

  public:
    HaveTexture(Deserializer& s) : Super(s) {}
    HaveTexture() {}
    texture_ptr getTexture() { return m_tex; }
    void setTexture(texture_ptr v)         { m_tex=v; }
    void setTexture(const string& filename){ setTexture(GetTexture(filename)); }

    virtual void draw()
    {
      glEnable(GL_TEXTURE_2D);
      texture_ptr tex = getTexture();
      tex->assign();
      Super::draw();
      tex->disassign();
      glDisable(GL_TEXTURE_2D);
    }
  };


  template<class T>
  class Billboard : public Inherit2(HaveTexture, T)
  {
  typedef Inherit2(HaveTexture, T) Super;
  private:
  public:
    Billboard(Deserializer& s) : Super(s) {}
    Billboard() {}

    void updateMatrix(matrix44& mat)
    {
      mat.translate(this->getPosition().v);
      mat*=GetAimCameraMatrix();
      mat.rotate(this->getRotate().v);
      mat.scale(this->getScale().v);
    }

    void draw()
    {
      static IVertexBufferObject *model = GetVBO("plane").get();
      ist::MatrixSaver ms(GL_MODELVIEW_MATRIX);
      glMultMatrixf(this->getMatrix().v);

      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);
      this->getTexture()->assign();
      model->assign();
      model->draw();
      model->disassign();
      this->getTexture()->disassign();
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      this->modMatrix();
    }
  };

  class Sprite
  {
  private:
    vector2 m_tur;
    vector2 m_tbl;
    GLbyte m_color[4];
    matrix44 m_mat;

  public:
    Sprite()
    {
      setTexcoord(vector2(1,1), vector2(0,0));
      setColor(vector4(1,1,1,1));
    }

    const float* getVertexArray() const
    {
      static float va[6*4]; // T2 C4UB V3 *4 

      const vector2 texcoord[4] = {
        vector2(m_tbl.x, m_tbl.y),
        vector2(m_tbl.x, m_tur.y),
        vector2(m_tur.x, m_tur.y),
        vector2(m_tur.x, m_tbl.y),
      };
      for(int i=0; i<4; ++i) {
        std::copy(texcoord[i].v, texcoord[i].v+2, va+(6*i));
      }

      for(int i=0; i<4; ++i) {
        va[(6*i)+2] = *((float*)m_color);
      }

      const vector4 vertex[4] = {
        vector4(0.0f, 1.0f, 1.0f),
        vector4(0.0f, -1.0f, 1.0f),
        vector4(0.0f, -1.0f, -1.0f),
        vector4(0.0f, 1.0f, -1.0f),
      };
      for(int i=0; i<4; ++i) {
        vector4 v = m_mat*vertex[i];
        std::copy(v.v, v.v+3, va+(6*i)+3);
      }

      return va;
    }

    void setColor(const vector4& col)
    {
      m_color[0] = GLbyte(col.x*255.0f);
      m_color[1] = GLbyte(col.y*255.0f);
      m_color[2] = GLbyte(col.z*255.0f);
      m_color[3] = GLbyte(col.w*255.0f);
    }

    void setTexcoord(const vector2& ur, const vector2& bl)
    {
      m_tur = ur;
      m_tbl = bl;
    }

    void setTransform(const matrix44& mat)
    {
      m_mat = mat;
    }
  };


  template<class T>
  class HaveSprite : public T
  {
  typedef T Super;
  private:
    Sprite m_sprite;

  public:
    HaveSprite(Deserializer& s) : Super(s) {}
    HaveSprite() {}

    Sprite& getSprite() { return m_sprite; }

    virtual void updateSprite(Sprite& sp)
    {
      sp.setTransform(this->getMatrix());
    }

    virtual void updateMatrix(matrix44& mat)
    {
      mat.translate(this->getPosition().v);
      mat*=GetAimCameraMatrix();
      mat.rotate(this->getRotate().v);
      mat.scale(this->getScale().v);
    }
  };


  class Particle : public Inherit3(HaveSprite, HaveTransform, Object)
  {
  typedef Inherit3(HaveSprite, HaveTransform, Object) Super;
  private:
    bool m_dead;

  public:
    Particle(Deserializer& s) : Super(s)
    {
      s >> m_dead;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_dead;
    }

  public:
    Particle() : m_dead(false) {}
    void draw() {}
    bool isDead() { return m_dead; }
    void kill() { m_dead=true; }
  };


  class SpriteBatch
  {
  private:
    typedef std::vector<Sprite*> sprite_cont;
    sprite_cont m_sprite;

  public:
    void insert(Sprite& p) { m_sprite.push_back(&p); }
    void clear() { m_sprite.clear(); }
    bool empty() { return m_sprite.empty(); }

    void draw()
    {
      if(m_sprite.empty()) {
        return;
      }

      static std::vector<float> vertex;
      for(size_t i=0; i<m_sprite.size(); ++i) {
        const float *v = m_sprite[i]->getVertexArray();
        vertex.insert(vertex.end(), v, v+24);
      }

      glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertex[0]);
      glDrawArrays(GL_QUADS, 0, m_sprite.size()*4);
      glColor4f(1,1,1,1);

      vertex.clear();
    }
  };

  template<class C>
  class ParticleSet : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  protected:
    typedef C particle_type;
    typedef typename C::Drawer conclete_type;
    typedef typename C::Factory factory_type;
    typedef particle_type* particle_ptr;
    typedef std::vector<particle_ptr> particle_cont;

    particle_cont m_particle;
    particle_cont m_dead;
    SpriteBatch m_sb;

    ParticleSet()
    {
      pinstance() = this;
      Register(this);
    }

    ~ParticleSet()
    {
      for(size_t i=0; i<m_particle.size(); ++i) {
        m_particle[i]->release();
      }
      pinstance() = 0;
    }

  public:
    ParticleSet(Deserializer& s) : Super(s)
    {
      pinstance() = this;
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_particle.push_back(factory_type::create(s));
      }
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_dead.push_back(factory_type::create(s));
      }
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_particle.size();
      for(size_t i=0; i<m_particle.size(); ++i) {
        m_particle[i]->serialize(s);
      }
      s << m_dead.size();
      for(size_t i=0; i<m_dead.size(); ++i) {
        m_dead[i]->serialize(s);
      }
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      for(size_t i=0; i<m_particle.size(); ++i) {
        m_particle[i]->reconstructLinkage();
      }
      for(size_t i=0; i<m_dead.size(); ++i) {
        m_dead[i]->reconstructLinkage();
      }
    }

  private:
    static ParticleSet*& pinstance()
    {
      static ParticleSet *inst;
      return inst;
    }

  public:
    static ParticleSet* instance()
    {
      if(!pinstance()) {
        new conclete_type();
      }
      return pinstance();
    }

    void insert(particle_ptr p)
    {
      m_particle.push_back(p);
    }


    struct less_id2
    {
      gid m_id;
      less_id2(gid id) : m_id(id) {}
      bool operator()(particle_ptr l, particle_ptr r) const
      {
        return l->getID() < m_id;
      }
    };

    particle_ptr getObject(gid id)
    {
      if(id==0) {
        return particle_ptr();
      }
      particle_cont::iterator p = std::lower_bound(m_particle.begin(), m_particle.end(), particle_ptr(0), less_id2(id));
      if(p!=m_particle.end() && (*p)->getID()==id) {
        return *p;
      }
      return particle_ptr();
    }

    virtual void drawSprite()
    {
      m_sb.draw();
    }

    void draw()
    {
      for(size_t i=0; i<m_particle.size(); ++i) {
        particle_ptr p = m_particle[i];
        p->updateSprite(p->getSprite());
        m_sb.insert(p->getSprite());
      }
      if(!m_sb.empty()) {
        drawSprite();
        m_sb.clear();
      }
      glColor4f(1,1,1,1);
    }


    struct dead
    {
      bool operator()(particle_ptr p) const
      {
        return p->isDead();
      }
    };

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      for(size_t i=0; i<m_particle.size(); ++i) {
        particle_ptr p = m_particle[i];
        p->update();
        if(p->isDead()) {
          m_dead.push_back(p);
        }
      }

      m_particle.erase(std::remove_if(m_particle.begin(), m_particle.end(), dead()), m_particle.end());
      for(size_t i=0; i<m_dead.size(); ++i) {
        m_dead[i]->release();
      }
      m_dead.clear();
    }

    void onDestroy(DestroyMessage& m)
    {
      // KillMessageëóêMñhé~ 
    }
  };



  template<class C>
  class BatchDrawer : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  protected:
    typedef C object_type;
    typedef object_type* object_ptr;
    typedef typename object_type::Drawer conclete_type;
    typedef std::vector<object_ptr> obj_cont;
    obj_cont m_objs;

    BatchDrawer()
    {
      pinstance() = this;
      Register(this);
    }

    ~BatchDrawer()
    {
      pinstance() = 0;
    }

    static BatchDrawer*& pinstance()
    {
      static BatchDrawer *inst;
      return inst;
    }

  public:
    BatchDrawer(Deserializer& s) : Super(s)
    {
      pinstance() = this;
      DeserializeLinkageContainer(s, m_objs);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkageContainer(s, m_objs);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkageContainer(m_objs);
    }

  public:
    static BatchDrawer* instance()
    {
      if(!pinstance()) {
        new conclete_type();
      }
      return pinstance();
    }

    void insert(object_ptr p)
    {
      m_objs.push_back(p);
    }

    void draw()
    {
      for(size_t i=0; i<m_objs.size(); ++i) {
        object_ptr o = m_objs[i];
        if(!o->isDead()) { // êÊÇ…updateÇ≥ÇÍÇÈä÷åWè„ÅAéÄÇÒÇ≈ÇÈÉIÉuÉWÉFÉNÉgÇä‹ÇﬁÇ±Ç∆Ç™Ç†ÇÈ 
          o->draw();
        }
      }
    }


    struct dead
    {
      bool operator()(object_ptr p) const
      {
        return p->isDead();
      }
    };

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      m_objs.erase(std::remove_if(m_objs.begin(), m_objs.end(), dead()), m_objs.end());
      if(m_objs.empty()) {
        SendKillMessage(0, this);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      // KillMessageëóêMñhé~ 
    }
  };





  inline void DrawSprite(const string& texfile, const vector4& pos, const vector4& scale, float rot=0.0f)
  {
    static IVertexBufferObject *model = GetVBO("plane").get();
    ist::MatrixSaver ms(GL_MODELVIEW_MATRIX);
    matrix44 mat;
    mat.translate(pos.v);
    mat*=GetAimCameraMatrix();
    mat.rotateX(rot);
    mat.scale(scale.v);
    glMultMatrixf(mat.v);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    texture_ptr tex = GetTexture(texfile);
    tex->assign();
    model->assign();
    model->draw();
    model->disassign();
    tex->disassign();
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
  }





  template<class T>
  class Box : public T
  {
  typedef T Super;
  private:
    float m_vertex[24*8];
    bool m_init;
    box m_box;

  public:
    Box(Deserializer& s) : Super(s)
    {
      s >> m_vertex >> m_init >> m_box;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_vertex << m_init << m_box;
    }

  public:
    Box() : m_init(false) {}
    const box& getBox() { return m_box; }
    virtual void setBox(const box& box) { m_box=box; initVertex(); }

    float getVolume() { return m_box.getVolume(); }
    vector4 getCenter() { return this->getMatrix()*m_box.getCenter(); }

    virtual void initPosition(vector3 *vertex, const vector4& ur, const vector4& bl)
    {
      vertex[0] = vector3(ur.x, ur.y, bl.z);
      vertex[1] = vector3(ur.x, ur.y, ur.z);
      vertex[2] = vector3(ur.x, bl.y, ur.z);
      vertex[3] = vector3(ur.x, bl.y, bl.z);

      vertex[4] = vector3(bl.x, ur.y, bl.z);
      vertex[5] = vector3(bl.x, bl.y, bl.z);
      vertex[6] = vector3(bl.x, bl.y, ur.z);
      vertex[7] = vector3(bl.x, ur.y, ur.z);

      vertex[8] = vector3(ur.x, ur.y, ur.z);
      vertex[9] = vector3(ur.x, ur.y, bl.z);
      vertex[10] = vector3(bl.x, ur.y, bl.z);
      vertex[11] = vector3(bl.x, ur.y, ur.z);

      vertex[12] = vector3(ur.x, bl.y, bl.z);
      vertex[13] = vector3(ur.x, bl.y, ur.z);
      vertex[14] = vector3(bl.x, bl.y, ur.z);
      vertex[15] = vector3(bl.x, bl.y, bl.z);

      vertex[16] = vector3(ur.x, ur.y, ur.z);
      vertex[17] = vector3(bl.x, ur.y, ur.z);
      vertex[18] = vector3(bl.x, bl.y, ur.z);
      vertex[19] = vector3(ur.x, bl.y, ur.z);

      vertex[20] = vector3(bl.x, ur.y, bl.z);
      vertex[21] = vector3(ur.x, ur.y, bl.z);
      vertex[22] = vector3(ur.x, bl.y, bl.z);
      vertex[23] = vector3(bl.x, bl.y, bl.z);
    }

    virtual void initNormal(vector3 *normal, const vector4& ur, const vector4& bl)
    {
      vector3 n;
      n = vector3(1.0f, 0.0f, 0.0f);
      normal[0] = n;
      normal[1] = n;
      normal[2] = n;
      normal[3] = n;

      n = vector3(-1.0f, 0.0f, 0.0f);
      normal[4] = n;
      normal[5] = n;
      normal[6] = n;
      normal[7] = n;

      n = vector3(0.0f, 1.0f, 0.0f);
      normal[8] = n;
      normal[9] = n;
      normal[10] = n;
      normal[11] = n;

      n = vector3(0.0f, -1.0f, 0.0f);
      normal[12] = n;
      normal[13] = n;
      normal[14] = n;
      normal[15] = n;

      n = vector3(0.0f, 0.0f, 1.0f);
      normal[16] = n;
      normal[17] = n;
      normal[18] = n;
      normal[19] = n;

      n = vector3(0.0f, 0.0f, -1.0f);
      normal[20] = n;
      normal[21] = n;
      normal[22] = n;
      normal[23] = n;
    }

    virtual void initTexcoord(vector2 *tex, const vector4& ur, const vector4& bl)
    {
      tex[0] = vector2(1.0f, 1.0f);
      tex[1] = vector2(1.0f, 0.0f);
      tex[2] = vector2(0.0f, 0.0f);
      tex[3] = vector2(0.0f, 1.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+0, tex+4);
      }

      tex[4] = vector2(1.0f, 1.0f);
      tex[5] = vector2(1.0f, 0.0f);
      tex[6] = vector2(0.0f, 0.0f);
      tex[7] = vector2(0.0f, 1.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+4, tex+8);
      }

      tex[8] = vector2(1.0f, 1.0f);
      tex[9] = vector2(1.0f, 0.0f);
      tex[10] = vector2(0.0f, 0.0f);
      tex[11] = vector2(0.0f, 1.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+8, tex+12);
      }

      tex[12] = vector2(1.0f, 0.0f);
      tex[13] = vector2(1.0f, 1.0f);
      tex[14] = vector2(0.0f, 1.0f);
      tex[15] = vector2(0.0f, 0.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+12, tex+16);
      }

      tex[16] = vector2(1.0f, 1.0f);
      tex[17] = vector2(1.0f, 0.0f);
      tex[18] = vector2(0.0f, 0.0f);
      tex[19] = vector2(0.0f, 1.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+16, tex+20);
      }

      tex[20] = vector2(1.0f, 1.0f);
      tex[21] = vector2(1.0f, 0.0f);
      tex[22] = vector2(0.0f, 0.0f);
      tex[23] = vector2(0.0f, 1.0f);
      if(GenRand()<0.5f) {
        std::reverse(tex+20, tex+24);
      }
    }

    void initVertex()
    {
      const vector4& ur = m_box.getUpperRight();
      const vector4& bl = m_box.getBottomLeft();
      vector3 vertex[24];
      vector3 normal[24];
      vector2 tex[24];
      initPosition(vertex, ur, bl);
      initNormal(normal, ur, bl);
      initTexcoord(tex, ur, bl);

      for(size_t i=0; i<24; ++i) {
        float *dst = &m_vertex[0]+(8*i);
        std::copy(tex[i].v, tex[i].v+2, dst+0);
        std::copy(normal[i].v, normal[i].v+3, dst+2);
        std::copy(vertex[i].v, vertex[i].v+3, dst+5);
      }
      m_init = true;
    }


    virtual void drawModel()
    {
      if(m_init) {
        glInterleavedArrays(GL_T2F_N3F_V3F, 0, &m_vertex[0]);
        glDrawArrays(GL_QUADS, 0, 24);
      }
    }

    void drawBasic()
    {
      ist::MatrixSaver msaver(GL_MODELVIEW_MATRIX);
      glMultMatrixf(this->getMatrix().v);
      drawModel();
    }

    virtual void draw()
    {
      drawBasic();
    }

    virtual bool call(const string& name, const any& value)
    {
      if(name=="setBox") { setBox(*any_cast<box>(&value)); }
      else { return Super::call(name, value); }
      return true;
    }
  };


  template<class Container>
  void _insert(Container& cont, typename Container::value_type* p, int num)
  {
    cont.insert(cont.end(), p, p+num);
  }


  template<class Iter>
  void draw_polyline(Iter begin, Iter end, const vector4& camera, float width)
  {
    size_t size = std::distance(begin, end);
    if(size<=1) {
      return;
    }
    vector2 t1 = vector2(0.0f, 0.0f);
    vector2 t2 = ist::vector2(1.0f, 1.0f);
    float tv = t2.y;
    static std::vector<float> vertex;

    for(Iter p=begin; p!=end; ++p) {
      vector4 tangent;
      if(p==begin)      tangent = *(p+1)-*p;
      else if(p==end-1) tangent = *p-*(p-1);
      else              tangent = *(p+1)-*(p-1);
      tangent.normalize();

      vector4 aim_camera= (camera-(*p)).normalize();
      vector4 distance  = (vector4().cross(tangent, aim_camera))*(width/2.0f);

      vector4 right= *p+distance;
      vector4 left = *p-distance;

      _insert(vertex, vector2(t1.x, tv).v, 2);
      _insert(vertex, vector3(left.v).v, 3);

      _insert(vertex, vector2(t2.x, tv).v, 2);
      _insert(vertex, vector3(right.v).v, 3);

      tv-=(t2.y-t1.y)/(size-1);
    }

    glInterleavedArrays(GL_T2F_V3F, 0, &vertex[0]);
    glDrawArrays(GL_QUAD_STRIP, 0, GLsizei(vertex.size()/5));
    vertex.clear();
  }

  template<class Iter>
  void draw_polyline_fade(Iter begin, Iter end, const vector4& camera, float width, float opa=1.0f)
  {
    size_t size = std::distance(begin, end);
    if(size<=1) {
      return;
    }
    vector2 t1 = vector2(0.0f, 0.0f);
    vector2 t2 = ist::vector2(1.0f, 1.0f);
    float tv = t2.y;
    static std::vector<float> vertex;

    for(Iter p=begin; p!=end; ++p) {
      vector4 tangent;
      if(p==begin)      tangent = *(p+1)-*p;
      else if(p==end-1) tangent = *p-*(p-1);
      else              tangent = *(p+1)-*(p-1);
      tangent.normalize();

      vector4 aim_camera= (camera-(*p)).normalize();
      vector4 distance  = (vector4().cross(tangent, aim_camera))*(width/2.0f);

      vector4 right= *p+distance;
      vector4 left = *p-distance;

      uchar color[4] = {255, 255, 255, uchar(255.0f*opa)};
      if(p==begin || p==end-1) { color[3] = 0; }
      _insert(vertex, vector2(t1.x, tv).v, 2);
      _insert(vertex, (float*)color, 1);
      _insert(vertex, vector3(left.v).v, 3);

      _insert(vertex, vector2(t2.x, tv).v, 2);
      _insert(vertex, (float*)color, 1);
      _insert(vertex, vector3(right.v).v, 3);

      tv-=(t2.y-t1.y)/(size-1);
    }

    glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertex[0]);
    glDrawArrays(GL_QUAD_STRIP, 0, GLsizei(vertex.size()/6));
    vertex.clear();
    glColor4f(1,1,1,1);
  }





  template<class T>
  class PolyLine : public T
  {
  typedef T Super;
  public:
    typedef std::deque<vector4> vertex_cont;
  private:
    vertex_cont m_line;
    size_t m_size;
    float m_width;
    texture_ptr m_texture;

  public:
    PolyLine(Deserializer& s) : Super(s)
    {
      ist::deserialize_container(s, m_line);
      s >> m_size >> m_width;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      ist::serialize_container(s, m_line);
      s << m_size << m_width;
    }

  public:
    PolyLine() : m_size(20), m_width(15.0f)
    {}

    virtual void drawPolyline()
    {
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDepthMask(GL_FALSE);

      m_texture->assign();
      draw_polyline(m_line.begin(), m_line.end(), GetCamera().getPosition(), m_width);
      m_texture->disassign();

      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
    }

    virtual void draw()
    {
      drawPolyline();
    }

    void setSize(size_t v) { m_size=v; }
    void setWidth(float v) { m_width=v; }
    void setTexture(const std::string& filename) { setTexture(GetTexture(filename)); }
    void setTexture(texture_ptr v) { m_texture=v; }
    void setPosition(const vector4& pos)
    {
      Super::setPosition(pos);
      if(m_line.size()!=m_size) {
        m_line.resize(m_size, pos);
      }
      else {
        m_line.push_back(pos);
        m_line.pop_front();
      }
    }

    vertex_cont& getVertex() { return m_line; }
  };


}

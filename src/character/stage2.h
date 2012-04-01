#ifndef stage2_h
#define stage2_h

namespace exception {
namespace stage2 {


  class Background : public BackgroundBase
  {
  typedef BackgroundBase Super;
  private:

    class Block : public Inherit4(Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit4(Box, HavePosition, Dummy, RefCounter) Super;
    private:
      vector4 m_vel;

    public:
      Block(Deserializer& s) : Super(s)
      {
        s >> m_vel;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_vel;
      }

    public:
      Block() : m_vel(-4.0f, 0.0f, 0.0f)
      {
        setBox(box(vector4(50.0f)));
      }

      void setVel(const vector4& v) { m_vel=v; }
      const vector4& getVel() const { return m_vel; }

      void update()
      {
        vector4 pos = getPosition();
        pos+=m_vel+GetGlobalScroll();
        if(m_vel.x<0.0f && pos.x<-1200.0f) { pos.x+=2400.0f; }
        if(m_vel.x>0.0f && pos.x> 1200.0f) { pos.x-=2400.0f; }
        if(m_vel.y<0.0f && pos.y<-1000.0f) { pos.y+=2000.0f; }
        if(m_vel.y>0.0f && pos.y> 1000.0f) { pos.y-=2000.0f; }
        setPosition(pos);
      }
    };
    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    block_cont m_blocks;
    int m_frame;

  public:
    Background(Deserializer& s) : Super(s)
    {
      s >> m_cam >> m_fog >> m_bgmat;
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_blocks.push_back(new Block(s));
      }
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_cam << m_fog << m_bgmat;
      s << m_blocks.size();
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->serialize(s);
      }
      s << m_frame;
    }

  public:
    Background() : m_frame(0)
    {
      m_cam.setPosition(vector4(-300.0f, 600.0f, 600.0f));
      m_cam.setTarget(vector4(0.0f, 0.0f, -500.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setDiffuse(vector4(0.5f, 0.5f, 0.7f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<100; ++i) {
        Block *block = new Block();
        block->setPosition(vector4(GetRand2()*1200.0f, GetRand2()*1000.0f, GetRand()*600.0f-800.0f));
        float f = GetRand();
        float v = GetRand()*2.0f+1.0f;
        if(f<0.25f)      { block->setVel(vector4( v,0,0)); }
        else if(f<0.5f)  { block->setVel(vector4(-v,0,0)); }
        else if(f<0.75f) { block->setVel(vector4(0, v,0)); }
        else             { block->setVel(vector4(0,-v,0)); }
        m_blocks.push_back(block);
      }
    }


    void draw()
    {
      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;
        m_cam.look();

        m_fog.enable();
        m_bgmat.assign();
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->draw();
        }
        ist::Material().assign();
        m_fog.disable();
      }

      drawFadeEffect();

      glClear(GL_DEPTH_BUFFER_BIT);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      m_cam.setPosition(vector4(500.0f, 600.0f, 500.0f) + GetCamera().getTarget());
      m_cam.setTarget(vector4(0.0f, 0.0f, -500.0f) + GetCamera().getTarget());

      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }

      ++m_frame;
    }
  };






  class LineTexture : public RefCounter
  {
  private:
    fbo_ptr m_fbo;

  public:
    LineTexture()
    {
      m_fbo = new ist::FrameBufferObject(128, 128);
      m_fbo->enable();
      glClearColor(0,0,0,0);
      glClear(GL_COLOR_BUFFER_BIT);
      draw();
      m_fbo->disable();
    }

    void draw()
    {
      ScreenMatrix sm(m_fbo->getWidth(), m_fbo->getHeight());

      glDisable(GL_LIGHTING);
      glColor4f(0,0,0,1);
      glLineWidth(3.0f);

      for(int j=0; j<5; ++j) {
        const float speed = 1.0f;
        vector2 pos;
        vector2 dir(speed, 0);

        glBegin(GL_LINE_STRIP);
        glVertex2fv(pos.v);
        for(int i=0; i<250; ++i) {
          pos+=dir;
          glVertex2fv(pos.v);
          if(GenRand() < 0.05f) {
            float f = float(GenRand());
            if(f<0.5f) {
              dir = vector2(0.0f, speed);
            }
            else {
              dir = vector2(speed, 0.0f);
            }
          }
        }
        glEnd();
      }

      glLineWidth(1.0f);
      glColor4f(1,1,1,1);
      glEnable(GL_LIGHTING);
    }

    void assign() { m_fbo->assign(); }
    void disassign() { m_fbo->disassign(); }
  };
  typedef intrusive_ptr<LineTexture> ltex_ptr;


  class ILightDirection
  {
  public:
    virtual ~ILightDirection() {}
    virtual int getLightDirection()=0;
  };

  class LightGroundDrawer : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    typedef std::vector<gobj_ptr> obj_cont;

    ltex_ptr m_ltex[4];
    po_ptr m_hline;
    po_ptr m_glow;
    fbo_ptr m_fbo_tmp;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;
    obj_cont m_obj;
    int m_frame;

    static LightGroundDrawer*& pinstance()
    {
      static LightGroundDrawer *inst;
      return inst;
    }

  public:
    LightGroundDrawer(Deserializer& s) : Super(s)
    {
      DeserializeLinkageContainer(s, m_obj);
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkageContainer(s, m_obj);
      s << m_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkageContainer(m_obj);
    }

  public:
    static LightGroundDrawer* instance()
    {
      if(!pinstance()) {
        new LightGroundDrawer();
      }
      return pinstance();
    }

    void insert(gobj_ptr v)
    {
      m_obj.push_back(v);
    }


    LightGroundDrawer() : m_frame(0)
    {
      pinstance() = this;
      Register(this);
    }

    ~LightGroundDrawer()
    {
      pinstance() = 0;
    }

    float getDrawPriority() { return 1.1f; }


    void swap(fbo_ptr& l, fbo_ptr& r)
    {
      fbo_ptr t = l;
      l = r;
      r = t;
    }

    void drawRect(const vector2& ur, const vector2& bl)
    {
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(bl.x, bl.y);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(bl.x, ur.y);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(ur.x, ur.y);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(ur.x, bl.y);
      glEnd();
    }

    void draw()
    {
      // まずは普通に描画 
      for(size_t i=0; i<m_obj.size(); ++i) {
        gobj_ptr o = m_obj[i];
        if(!o->isDead()) {
          o->draw();
        }
      }

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }
    }

    void draw_gl20()
    {
      // シェーダ&FBO初期化 
      if(!m_hline) {
        for(int i=0; i<4; ++i) {
          m_ltex[i] = new LineTexture();
        }

        m_hline = new ist::ProgramObject();
        m_hline->attach(GetVertexShader("stage2.vsh"));
        m_hline->attach(GetFragmentShader("stage2.fsh"));
        m_hline->link();

        m_glow = new ist::ProgramObject();
        m_glow->attach(GetFragmentShader("glow.fsh"));
        m_glow->link();

        m_fbo_lines = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT);
        m_fbo_tmp = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo_glow = new ist::FrameBufferObject(256, 256);
        m_fbo_glow_b = new ist::FrameBufferObject(256, 256);
      }

      glDisable(GL_LIGHTING);

      matrix44 icam;
      glGetFloatv(GL_MODELVIEW_MATRIX, icam.v);

      // 溝を描画 
      m_fbo_tmp->enable();
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_TEXTURE_2D);
      m_hline->enable();
      m_hline->setMatrix4fv("icam", 1, false, icam.invert().v);
      m_hline->setUniform1f("freq", float(m_frame));

      for(obj_cont::iterator p=m_obj.begin(); p!=m_obj.end(); ++p) {
        gobj_ptr o = *p;
        int ld = 0;
        if(ILightDirection *il = dynamic_cast<ILightDirection*>(o)) {
          ld = il->getLightDirection();
        }
        m_hline->setUniform1i("dir", ld);

        m_ltex[o->getID()%4]->assign();
        if(Enemy *e=dynamic_cast<Enemy*>(o)) {
          e->drawBasic();
        }
        else {
          o->draw();
        }
        m_ltex[o->getID()%4]->disassign();
      }
      m_hline->disable();
      glDisable(GL_TEXTURE_2D);
      m_fbo_tmp->disable();


      // フィードバックブラー 
      m_fbo_lines->enable();
      if(m_frame==1) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
      }
      {
        ScreenMatrix sm;

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        m_fbo_lines->assign();
        glEnable(GL_TEXTURE_2D);
        vector2 str(2.0f, 2.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        drawRect(vector2(640.0f, 480.0f)+str*3.0f, vector2(0.0f, 0.0f)-str*3.0f);
        drawRect(vector2(640.0f, 480.0f)+str*2.0f, vector2(0.0f, 0.0f)-str*2.0f);
        drawRect(vector2(640.0f, 480.0f)+str*1.0f, vector2(0.0f, 0.0f)-str*1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_TEXTURE_2D);
        m_fbo_lines->disassign();

        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        drawRect(vector2(640,480), vector2(0,0));
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        m_fbo_tmp->assign();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
        drawRect(vector2(640,480), vector2(0,0));
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        m_fbo_tmp->disassign();

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
      }
      m_fbo_lines->disable();

      glEnable(GL_LIGHTING);


      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        // テクスチャ張った状態を合成 
        m_fbo_lines->assign();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_fbo_lines->disassign();

        // テクスチャ張った状態をぼかす(グローエフェクト化) 
        m_fbo_lines->assign();
        m_glow->enable();
        m_glow->setUniform1f("width", float(m_fbo_glow->getWidth()));
        m_glow->setUniform1f("height", float(m_fbo_glow->getHeight()));
        for(int i=0; i<2; ++i) {
          if(i!=0) {
            swap(m_fbo_glow, m_fbo_glow_b);
            m_fbo_glow_b->assign();
          }
          m_fbo_glow->enable();
          m_glow->setUniform1i("pass", i+1);
          DrawRect(vector2(640,480), vector2(0,0));
          m_fbo_glow->disable();
          m_fbo_glow->disassign();
        }
        m_glow->disable();


        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // グローエフェクトを描画 
        m_fbo_glow->assign();
        glColor4f(0.3f, 0.3f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1,1,1,1);
        m_fbo_glow->disassign();
        
      // 確認用 
      //  m_fbo_lines->assign();
      //  DrawRect(vector2(float(m_fbo_lines->getWidth()), float(m_fbo_lines->getHeight())), vector2(0.0f, 0.0f));
      //  m_fbo_lines->disassign();

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      m_obj.erase(std::remove_if(m_obj.begin(), m_obj.end(), is_dead()), m_obj.end());
      if(m_obj.empty()) {
        SendKillMessage(0, this);
      }
    }
  };



  class LGround : public Ground, public ILightDirection
  {
  typedef Ground Super;
  private:
    int m_lightdir;

  public:
    LGround(Deserializer& s) : Super(s)
    {
      s >> m_lightdir;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_lightdir;
    }

  public:
    LGround() : m_lightdir(0)
    {
      LightGroundDrawer::instance()->insert(this);
    }

    float getDrawPriority() { return -1.0f; }
    int getLightDirection() { return m_lightdir; }
    void setLightDirection(int v) { m_lightdir=v; }
  };



  class MiniFallBlock : public DynamicBlock, public ILightDirection
  {
  typedef DynamicBlock Super;
  private:
    int m_lightdir;

  public:
    MiniFallBlock(Deserializer& s) : Super(s)
    {
      s >> m_lightdir;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_lightdir;
    }

  public:
    MiniFallBlock() : m_lightdir(0)
    {
      LightGroundDrawer::instance()->insert(this);
      setBounce(0.05f);
    }

    float getDrawPriority() { return -1.0f; }
    int getLightDirection() { return m_lightdir; }

    void onConstruct(ConstructMessage& m)
    {
      vector4 n = getAccel().normal();
      if(fabsf(n.x)>0.1f) {
        m_lightdir = n.x>0 ? 0 : 1;
      }
      else if(fabsf(n.y)>0.1f) {
        m_lightdir = n.y>0 ? 2 : 3;
      }
    }

    int getExplodeCount() { return Super::getExplodeCount()/2; }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);
      GetSound("explosion2.wav")->play(1);
    }
  };

  class FallBlock : public MiniFallBlock
  {
  typedef MiniFallBlock Super;
  public:
  private:
    int m_frame;
    bool m_freezed;

  public:
    FallBlock(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_freezed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_freezed;
    }

  public:
    FallBlock() : m_frame(0), m_freezed(false)
    {}

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(!m_freezed) {
        int f = ++m_frame;
        if(f>600 && getVel().norm()<0.1f) {
          setVel(vector4());
          setAccel(vector4());
          setBound(box(vector4(1000)));
          m_freezed = true;
        }
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(m_freezed) {
        return;
      }

      gobj_ptr from = m.getFrom();
      if(dynamic_cast<MediumBlock*>(from) ||dynamic_cast<LargeBlock*>(from)) {
        SendDamageMessage(this, from, 0.2f);
      }
      if(IsGround(from) || dynamic_cast<FallBlock*>(from)) {
        Super::onCollide(m);
      }
    }
  };


  class MiniFallGround : public DynamicGround, public ILightDirection
  {
  typedef DynamicGround Super;
  private:
    int m_lightdir;

  public:
    MiniFallGround(Deserializer& s) : Super(s)
    {
      s >> m_lightdir;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_lightdir;
    }

  public:
    MiniFallGround() : m_lightdir(0)
    {
      LightGroundDrawer::instance()->insert(this);
      setBounce(0.05f);
    }

    float getDrawPriority() { return -1.0f; }
    int getLightDirection() { return m_lightdir; }

    void onConstruct(ConstructMessage& m)
    {
      vector4 n = getAccel().normal();
      if(fabsf(n.x)>0.1f) {
        m_lightdir = n.x>0 ? 0 : 1;
      }
      else if(fabsf(n.y)>0.1f) {
        m_lightdir = n.y>0 ? 2 : 3;
      }
    }

    void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.0f, 0.0f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.5f).v);
    }
  };

  class FallGround : public MiniFallGround
  {
  typedef MiniFallGround Super;
  private:
    int m_frame;
    bool m_freezed;

  public:
    FallGround(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_freezed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_freezed;
    }

  public:
    FallGround() : m_frame(0), m_freezed(false)
    {}

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(!m_freezed) {
        int f = ++m_frame;
        if(f>600 && getVel().norm()<0.1f) {
          setVel(vector4());
          setAccel(vector4());
          setBound(box(vector4(1000)));
          m_freezed = true;
        }
      }
    }

    void onCollide(CollideMessage& m)
    {
      if(m_freezed) {
        return;
      }

      gobj_ptr from = m.getFrom();
      if(dynamic_cast<MediumBlock*>(from) ||dynamic_cast<LargeBlock*>(from)) {
        SendDamageMessage(this, from, 0.2f);
      }
      if(dynamic_cast<FallGround*>(from) || dynamic_cast<LGround*>(from) || dynamic_cast<FallBlock*>(from)) {
        Super::onCollide(m);
      }
    }
  };



  class Scene : public SceneBase
  {
  typedef SceneBase Super;
  protected:
    static float brand() { return float(s_brand->genReal())*2.0f-1.0f; }
    static Background* getBackground() { return s_bg; }

    virtual LGround* putStaticGround(const vector4& pos, const box& b)
    {
      LGround *e = new LGround();
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(2000)));
      return e;
    }

  private:
    static random_ptr s_brand;
    static Background *s_bg;

  public:
    Scene(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, s_bg);
      s_brand = new ist::Random(1);
      s >> (*s_brand);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, s_bg);
      s << (*s_brand);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(s_bg);
    }

  public:
    static void init()
    {
      s_brand = new ist::Random(1);
      s_bg = new Background();
    }

    static void quit()
    {
      s_brand = 0;
      s_bg = 0;
    }

    Scene() {}
  };
  random_ptr Scene::s_brand = 0;
  Background* Scene::s_bg = 0;




  class Scroller : public Optional
  {
  typedef Optional Super;
  private:
    int m_frame;
    int m_endframe;
    vector4 m_move;
    vector4 m_scroll;

  public:
    Scroller(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_endframe >> m_move >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_endframe << m_move << m_scroll;
    }

  public:
    Scroller(int endframe, const vector4& move) :
      m_frame(0), m_endframe(endframe), m_move(move)
    {
      m_scroll = m_move*-(1.0f/m_endframe);
      SetGlobalScroll(GetGlobalScroll()+m_scroll);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      if(m_frame==m_endframe) {
        SetGlobalScroll(GetGlobalScroll()-m_scroll);
        SendKillMessage(0, this);
      }
    }
  };


  class Scene1 : public Scene
  {
  typedef Scene Super;
  public:

    virtual MediumBlock* putMediumBlock(const vector4& pos)
    {
      MediumBlock *b = new MediumBlock();
      b->setBound(box(vector4(1000,1000,200), vector4(-1000,-500,-200)));
      b->setPosition(pos);
      b->setVel(vector4(0,-1,0)+vector4(brand(), 0.0f, 0));
      b->setAccel(vector4(0,-1,0)*0.02f);
      b->setAxis(vector4(brand(), brand(), brand()).normalize());
      return b;
    }

    virtual LargeBlock* putLargeBlock(const vector4& pos)
    {
      LargeBlock *b = new LargeBlock();
      b->setBound(box(vector4(1000,1000,200), vector4(-1000,-500,-200)));
      b->setPosition(pos);
      b->setVel(vector4(0,-1,0)+vector4(brand(), 0.0f, 0));
      b->setAccel(vector4(0,-1,0)*0.02f);
      b->setAxis(vector4(brand(), brand(), brand()).normalize());
      return b;
    }

  public:
    Scene1(Deserializer& s) : Super(s) {}
    Scene1() {}

    void progress(int f)
    {
      if(f%90==0) {
        putMediumBlock(vector4(80.0f+brand()*150.0f, 320.0f+brand()*30.0f, 0.0f));
      }
      {
        int kf[] = {
          120, 150, 180,
          240, 270, 300,
        };
        for(int i=0; i<6; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4(-400,  80,0),
            vector4(-400, 160,0),
            vector4(-400, 240,0),

            vector4(-400, -80,0),
            vector4(-400,-160,0),
            vector4(-400,-240,0),
          };

          Fighter_Rush *c = new Fighter_Rush(false);
          c->setLength(150.0f);

          Fighter *e = new Fighter(c);
          e->setPosition(pos[i]);
          e->setDirection(vector4(1,0,0));
        }
      }
      if(f==360) {
        HeavyFighter *e = new HeavyFighter(new HeavyFighter_Missiles(false));
        e->setPosition(vector4(-600, 0, 0));
        e->setDirection(vector4(1, 0, 0));
      }
      {
        int kf[] = {
          600,600,
          650,650,
        };
        for(int i=0; i<4; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4(-500, 160,0),
            vector4(-500,-160,0),
            vector4(-450, 260,0),
            vector4(-450,-260,0),
          };

          Shell_BurstMissile *c = new Shell_BurstMissile(false);
          c->setLength(300.0f);

          Shell *e = new Shell(c);
          e->setPosition(pos[i]);
          e->setDirection(vector4(1,0,0));
        }
      }

      if(f==900) {
        SendKillMessage(0, this);
      }
    }
  };


  // キューブ地帯 
  class Scene2 : public Scene
  {
  typedef Scene Super;
  private:

    FallGround* putGround(const box& b, const vector4& pos, const vector4& g, float v=2.0f)
    {
      FallGround *e = new FallGround();
      vector4 n = g.normal();
      e->setAccel(n*0.02f);
      e->setVel(n*v);

      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(1500)));
      e->setAccelResist(0.0f);
      e->setPushable(false);
      e->setMaxSpeed(9.0f);
      return e;
    }

    FallGround* putGround(const vector4& pos, const vector4& g, float v=2.0f)
    {
      return putGround(box(vector4(50, 50, 25)), pos, g, v);
    }

    FallGround* putDeepGround(const vector4& pos, const vector4& g, float v=2.0f)
    {
      return putGround(box(vector4(50, 50, 25), vector4(-50,-50,-75)), pos, g, v);
    }

    FallBlock* putBlock(const vector4& pos, const vector4& g, float v=2.0f)
    {
      FallBlock *e = new FallBlock();
      e->setLife(50.0f);
      e->setEnergy(50.0f);

      vector4 n = g.normal();
      e->setAccel(n*0.02f);
      e->setVel(n*v);

      e->setPosition(pos);
      e->setBox(box(vector4(50, 50, 25)));
      e->setBound(box(vector4(1500)));
      e->setAccelResist(0.0f);
      e->setPushable(false);
      e->setMaxSpeed(9.0f);
      return e;
    }

    MiniFallBlock* putMiniBlock(const vector4& pos, const vector4& g, float v=2.0f)
    {
      MiniFallBlock *e = new MiniFallBlock();
      e->setLife(10.0f);
      e->setEnergy(20.0f);

      vector4 n = g.normal();
      e->setAccel(n*0.02f);
      e->setVel(n*v);

      e->setPosition(pos);
      e->setBox(box(vector4(24.5f, 24.5f, 16.0f)));
      e->setBound(box(vector4(1500)));
      e->setAccelResist(0.025f);
      e->setMaxSpeed(9.0f);
      return e;
    }

    MiniFallGround* putMiniGround(const vector4& pos, const vector4& g, float v=2.0f)
    {
      MiniFallGround *e = new MiniFallGround();
      vector4 n = g.normal();
      e->setAccel(n*0.02f);
      e->setVel(n*v);

      e->setPosition(pos);
      e->setBox(box(vector4(24.5f, 24.5f, 16.0f)));
      e->setBound(box(vector4(1500)));
      e->setAccelResist(0.025f);
      e->setMaxSpeed(9.0f);
      return e;
    }

  private:
    gobj_ptr m_stopper;
    vector4 m_scroll;
    int m_pframe;
    int m_action;

  public:
    Scene2(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_stopper);
      s >> m_scroll >> m_pframe >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_stopper);
      s << m_scroll << m_pframe << m_action;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_stopper);
    }

  public:
    Scene2() : m_stopper(0), m_pframe(0), m_action(1)
    {
      SetGlobalScroll(vector4(0, 0, 0));
    //  SetCameraMovableArea(vector2(1000,1000), vector2(-1000,-1000));
    }

    void phase1()
    {
      int f = ++m_pframe;

      // ストッパー 
      if(f==1) {
        m_stopper = putStaticGround(vector4(), box(vector4(-300, -300, 25), vector4(-400, -400,-25)));
      }

      // 左壁 
      // (-400,-400) (-300, 0)
      if(f==1 || f==60 || f==120) {
        FallGround *b = putGround(vector4(-350, 400, 0), vector4(0,-1,0));
      }

      // 下壁 
      // (400, -200) (-300, -300)
      if(f==120 || f==160 || f==200 || f==240 || f==280 || f==320 || f==360) {
        putGround(vector4(500, -250, 0), vector4(-1,0,0), 3);
      }

      {
        int kf[] = {
          120, 150, 180, 210, 240, 300,
        };
        for(int i=0; i<6; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4(400,-160,0),
            vector4(400,-80,0),
            vector4(400,0,0),
            vector4(400,80,0),
            vector4(400,160,0),
            vector4(400,240,0),
          };

          Fighter_Rush *c = new Fighter_Rush(false);
          c->setLength(150.0f);

          Fighter *e = new Fighter(c);
          e->setPosition(pos[i]);
          e->setDirection(vector4(-1,0,0));
        }
      }

      if(f==360) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase2()
    {
      int f = ++m_pframe;

      // 右壁 
      // (400, 300) (300, -200) 
      if(f==1 || f==40 || f==80 || f==120 || f==160) {
        putGround(vector4(350, 400, 0), vector4(0,-1,0), 3);
      }

      // 上壁 
      // (-300, 300), (300, 200) 
      if(f==160 || f==200 || f==240 || f==280 || f==320 || f==360) {
        putGround(vector4(-800, 250, 0)+m_scroll, vector4(1,0,0), 3);
      }

      {
        int kf[] = {
          60, 90, 120, 150, 180, 210,
        };
        for(int i=0; i<6; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 pos[] = {
            vector4( 200,350,0),
            vector4( 120,350,0),
            vector4(  40,350,0),
            vector4( -40,350,0),
            vector4(-120,350,0),
            vector4(-200,350,0),
          };

          Fighter_Rush *c = new Fighter_Rush(false);
          c->setLength(150.0f);

          Fighter *e = new Fighter(c);
          e->setPosition(pos[i]);
          e->setDirection(vector4(0,-1,0));
        }
      }

      // 右ザコブロック 
      // (-400,   0) (-300,200)
      if(f==100) {
        vector4 base = vector4(-325,350, 0);
        for(int i=0; i<2; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(-50*i,50*j, 0)+m_scroll;
            vector4 dir = vector4(0,-1,0);
            putMiniBlock(pos, dir);
          }
        }
      }

      if(f==360) {
        SendKillMessage(0, m_stopper);

        ++m_action;
        m_pframe = 0;
      }
    }


    // 左スクロール地帯 
    void phase3()
    {
      int f = ++m_pframe;

      // vector4(-900, 0, 0) 
      if(f==1) {
        new Scroller(1200, vector4(-900, 0, 0));
      }

      // 上壁 
      // (-700, 300) (-300, 200)
      if(f==1 || f==60 || f==120 || f==180) {
        putGround(vector4(-1100, 250, 0)+m_scroll, vector4(1,0,0), 2.0f);
      }

      // 右ザコブロック 
      // (-600,   0) (-400,-100)
      if(f==50) {
        vector4 base = vector4(-1100,-25, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<2; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(1,0,0);
            putMiniBlock(pos, dir);
          }
        }
      }


      // 上突起 
      // (-700, -200) (-600, 200)
      if(f==180 || f==220 || f==260) {
        putGround(vector4(-650,-500, 0)+m_scroll, vector4(0,1,0), 3);
      }

      // 右ザコブロック 
      // (-600,   0) (-400,-100)
      if(f==250) {
        vector4 base = vector4(-1500,-25, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<2; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(1,0,0);
            if(i==3) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }


      // 下壁 
      // (-1000,-200) (-400,-300)
      if(f==260 || f==300 || f==360 || f==420 || f==480 || f==540) {
        putGround(vector4(-1500,-250, 0)+m_scroll, vector4(1,0,0), 2);
      }

      // 下突起 
      // (-1000,   0) (-900,-200)
      if(f==660 || f==700) {
        putGround(vector4(-950, 500, 0)+m_scroll, vector4(0,-1,0), 3);
      }


      if(f==700) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase4() // 下スクロール地帯 
    {
      int f = ++m_pframe;

      // vector4(-900,-350, 0) 
      if(f==500) {
        new Scroller(600, vector4(0,-350, 0));
      }

      // 上壁続き 
      // (-1300, 300) (-700, 200)
      if(f==1 || f==40 || f==80 || f==120 || f==160 || f==200) {
        putGround(vector4(-1600, 250, 0)+m_scroll, vector4(1,0,0), 3);
      }

      // 上ザコブロック 
      // (-1200, 400) (-1000,200)
      if(f==90) {
        vector4 base = vector4(-1025,-800, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<5; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==4) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 右ザコブロック 
      // (-1200,-100) (-1000,-300)
      if(f==400) {
        vector4 base = vector4(-1500,-125, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(1,0,0);
            if(i==3) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 左壁 
      // (-1300,-600) (-1200,  200)
      if(f==200 || f==240 || f==560 || f==600 || f==640 || f==680 || f==720 || f==760) {
        putGround(vector4(-1250,-800, 0)+m_scroll, vector4(0,1,0), 3);
      }

      // 上ザコブロック 
      // (-900,-500) (-700,-300)
      if(f==700) {
        vector4 base = vector4(-875,-800, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            putMiniBlock(pos, dir);
          }
        }
      }

      if(f==900) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase5() // 右下スクロール地帯 
    {
      int f = ++m_pframe;

      // vector4(-550,-500, 0) 
      if(f==200) {
        new Scroller(600, vector4(400,-150,0));
      }

      // 左ザコブロック 
      // (-1200, -50) (-800,-300)
      if(f==1) {
        vector4 base = vector4(-300,-325, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(-1,0,0);
            if(i==3) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 下壁 
      // (-1200,-500) (-500, -600)
      if(f==1 || f==40 || f==80 || f==120 || f==160 || f==200 || f==240) {
        putGround(vector4(-300, -550, 0)+m_scroll, vector4(-1,0,0), 3.0f);
      }
      // 左ザコブロック 
      // (-500,-500) (-300,-600)
      if(f==300) {
        vector4 base = vector4(-200,-525, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<2; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(-1,0,0);
            putMiniBlock(pos, dir);
          }
        }
      }

      // 右壁 
      // (-100,-300) ( -200,-900) 
      if(f==400 || f==440 || f==480 || f==520 || f==560 || f==600) {
        putGround(vector4(-250,-1200, 0)+m_scroll, vector4(0,1,0), 3.0f);
      }

      // 上ザコブロック 
      // (-500,-600) (-300,-800)
      if(f==300) {
        vector4 base = vector4(-475,-1200, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==3) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }
      // 右ザコブロック 
      // (-500,-500) (-300,-600)
      if(f==330) {
        vector4 base = vector4(-1600,-625, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(1,0,0);
            putMiniBlock(pos, dir);
          }
        }
      }

      // 上ザコブロック 
      // (-1000,-600) (-900,-800)
      if(f==500) {
        vector4 base = vector4(-975,-1200, 0);
        for(int i=0; i<2; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==3) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }


      if(f==800) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase6() // 左下スクロール地帯 
    {
      int f = ++m_pframe;

      // vector4(-900,-650, 0) 
      if(f==1) {
        new Scroller(600, vector4(-400,-150,0));
      }


      // 下壁 
      // (-1000,-800) ( -100,-900) 
      if(f==1 || f==40 || f==80 || f==120 || f==160 || f==200 || f==240) {
        putGround(vector4(-1600, -850, 0)+m_scroll, vector4(1,0,0), 3);
      }

      // 左壁 
      // (-1300,-600) (-1200,-900)
      if(f==300 || f==350 || f==400) {
        putGround(vector4(-1250,-1600, 0)+m_scroll, vector4(0,1,0), 3);
      }

      // 上ザコブロック 
      // (-1200,-600) (-1000,-1100)
      if(f==300) {
        vector4 base = vector4(-1025,-1600, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<10; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==9) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 左ザコブロック 
      // (-1000,-600) (-800,-800)
      if(f==550) {
        vector4 base = vector4(-400,-925, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(-1,0,0);
            putMiniBlock(pos, dir);
          }
        }
      }

      if(f==600) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase7() // 下スクロール地帯 
    {
      int f = ++m_pframe;

      // vector4(-900,-1450, 0) 
      if(f==1) {
        new Scroller(2000, vector4(0,-1000,0));
      }


      // 左壁 
      // (-1300,-900) (-1200,-1200)
      if(f==1 || f==50 || f==100) {
        putGround(vector4(-1250,-1600, 0)+m_scroll, vector4(0,1,0), 3);
      }


      // 左突起 
      // (-1200,-1100) (-800,-1200)
      if(f==120 || f==160 || f==200 || f==240) {
        putGround(vector4(-400,-1150, 0)+m_scroll, vector4(-1,0,0), 3);
      }

      // 右壁 
      //(-600,-900) (-500,-1500)
      if(f==240 || f==280 || f==320 || f==360 || f==400 || f==440) {
        putGround(vector4(-550,-1700, 0)+m_scroll, vector4(0,1,0), 3);
      }

      // 詰めザコブロック 
      //(-800,-900) (-600,-1400)
      if(f==300) {
        vector4 base = vector4(-625,-1750, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<10; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==9) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 右突起 
      // (-1000,-1400) (-700,-1500)
      if(f==650 || f==700 || f==750 || f==800) {
        FallGround *b = putGround(vector4(-1600,-1450, 0)+m_scroll, vector4(1,0,0), 3);
      }

      // 詰めザコブロック 
      //(-1200,-1200) (-800,-1400)
      if(f==600) {
        vector4 base = vector4(-1600,-1225, 0);
        for(int i=0; i<8; ++i) {
          for(int j=0; j<4; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(1,0,0);
            putMiniBlock(pos, dir);
          }
        }
      }
      // 詰めザコブロック 
      //(-1200,-1400) (-1000,-1500)
      if(f==900) {
        vector4 base = vector4(-1025,-1800, 0);
        for(int i=0; i<4; ++i) {
          for(int j=0; j<6; ++j) {
            vector4 pos = base+vector4(-50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==5) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      // 左壁 
      // (-1300,-1200) (-1200,-1900)
      if(f==900 || f==950 || f==1000|| f==1050|| f==1100 || f==1150 || f==1200) {
        putGround(vector4(-1250,-2000, 0)+m_scroll, vector4(0,1,0), 3);
      }

      // 詰めザコブロック 
      //(-900,-1500) (-600,-1800)
      if(f==1400) {
        vector4 base = vector4(-775,-1800, 0);
        for(int i=0; i<6; ++i) {
          for(int j=0; j<6; ++j) {
            vector4 pos = base+vector4(50*i,-50*j, 0)+m_scroll;
            vector4 dir = vector4(0,1,0);
            if(j==5) {
              putMiniGround(pos, dir);
            }
            else {
              putMiniBlock(pos, dir);
            }
          }
        }
      }

      if(f==1500) {
        ++m_action;
        m_pframe = 0;
      }
    }

    void phase8()
    {
      int f = ++m_pframe;

      if(f==501) {
        SetGlobalScroll(vector4(-0.6f, 0, 0));
      }

      // 下壁 
      // (-1200,-1800) ( -500,-1900)
      if(f==1 || f==40 || f==80|| f==120|| f==160 || f==200 || f==240) {
        putGround(vector4(-400,-1850, 0)+m_scroll, vector4(-1,0,0), 3);
      }

      if(f==800) {
        ++m_action;
        m_pframe = 0;
      }
    }


    // Turtle死亡をトリガーにザコ編隊出す 
    class Action1 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_trigger;
      int m_frame;
      vector4 m_dir;

    public:
      Action1(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_trigger);
        s >> m_frame >> m_dir;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_trigger);
        s << m_frame << m_dir;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_trigger);
      }

    public:
      Action1(const vector4& dir, gobj_ptr t) : m_trigger(t), m_frame(0),  m_dir(dir)
      {}

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_trigger);

        if(!m_trigger) {
          int f = ++m_frame;
          if(f==60) {
            IControler *ic[3];
            vector4 pos[3];

            pos[0] = getPosition();
            pos[1] = pos[0]+(matrix44().rotateZ( 90)*m_dir)*80.0f;
            pos[2] = pos[0]+(matrix44().rotateZ(-90)*m_dir)*80.0f;
            {
              Shell_GravityMissile *c = new Shell_GravityMissile(true);
              c->setLength(300.0f);
              ic[0] = c;
            }
            for(int i=1; i<3; ++i) {
              Shell_BurstMissile *c = new Shell_BurstMissile(true);
              c->setLength(300.0f);
              ic[i] = c;
            }

            for(int i=0; i<3; ++i) {
              Shell *e = new Shell(ic[i]);
              e->setPosition(pos[i]);
              e->setDirection(m_dir);
            }

            SendKillMessage(0, this);
          }
        }

        if(getPosition().x<-300.0f) {
          SendKillMessage(0, this);
        }
      }
    };

    // 後ろからFighter出し続ける 
    class Action2 : public Actor
    {
    typedef Actor Super;
    private:
      int m_frame;

    public:
      Action2(Deserializer& s) : Super(s)
      {
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame;
      }

    public:
      Action2() : m_frame(0)
      {}

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        int f = ++m_frame;
        if(f%100==0) {
          Fighter *e = new Fighter(new Fighter_Straight(1.0f));
          e->setDirection(vector4(1,0,0));
          e->setPosition(vector4(-450,0,0));
        }
        if(f>=2500) {
          SendKillMessage(0, this);
        }
      }
    };


    void phase9() // 右スクロール亀地帯 
    {
      int f = ++m_pframe;

      if(f==501) {
        SetGlobalScroll(vector4(-0.7f, 0, 0));
      }

      {
        int kf[] = {1, 400,1100, 1400};
        for(int i=0; i<4; ++i) {
          if(f!=kf[i]) { continue; }
          vector4 dir[] = {
            vector4(0,-1,0),
            vector4(0, 1,0),
            vector4(0,-1,0),
            vector4(0,-1,0),
          };
          vector4 pos[] = {
            vector4(-350,-1190, 0),
            vector4( 100,-2120, 0),
            vector4( 520,-1200, 0),
            vector4( 780,-1150, 0),
          };

          Turtle *e = new Turtle(new Turtle_Wait(450.0f, true));
          e->setPosition(pos[i]+m_scroll);
          e->setDirection(dir[i]);
          e->setBound(box(vector4(1000.0f)));

          Action1 *a = new Action1(dir[i], e);
          a->setPosition(pos[i]+m_scroll);
        }
      }

      // 下壁 
      // (-500,-1800) ( -100,-1900)
      if(f==1 || f==60 || f==120 || f==180) {
        putGround(vector4(  0,-1850, 0)+m_scroll, vector4(-1,0,0));
      }

      // 右壁 
      // (-200,-1400) ( -100,-1800)
      if(f==180 || f==240 || f==300 || f==360 || f==420) {
        vector4 pos = vector4(-150,-1200, 0)+m_scroll;
        vector4 dir = vector4(0,-1,0);
        if(f==180 || f==300) {
          FallGround *g = putDeepGround(pos, dir);
          SmallHatch *h = new SmallHatch(0);
          h->setParent(g);
          h->setDirection(vector4(1,0,0));
          h->setPosition(vector4(50,0,0));
        }
        else {
          box b(vector4(50,50,25));
          if(f==240) { b=box(vector4(50,50,-25), vector4(-50,-50,-75)); }
          putGround(b, pos, dir);
        }
      }

      if(f==700) {
        new Action2();
      }

      // 上壁 
      // (-100,-1400) ( 400,-1500)
      if(f==480 || f==540 || f==600 || f==660 || f==720) {
        putGround(vector4(600,-1450, 0)+m_scroll, vector4(-1,0,0));
      }

      // 右壁 
      // (300,-1200) ( 400,-1800)
      if(f==1040 || f==1100 || f==1160 || f==1220 || f==1280) {
        vector4 pos = vector4(350,-2200, 0)+m_scroll;
        vector4 dir = vector4(0,1,0);
        if(f==1040 || f==1160) {
          FallGround *g = putDeepGround(pos, dir);
          SmallHatch *h = new SmallHatch(new Hatch_Laser(180));
          h->setParent(g);
          h->setDirection(vector4(-1,0,0));
          h->setPosition(vector4(-50,0,0));
        }
        else {
          box b(vector4(50,50,25));
          if(f==1100) { b=box(vector4(50,50,-25), vector4(-50,-50,-75)); }
          putGround(b, pos, dir);
        }
      }

      // 下壁 
      // (400,-1800) (1000,-1900)
      if(f==1280 || f==1340 || f==1400 || f==1460 || f==1520 || f==1580) {
        putGround(vector4(1100,-1850, 0)+m_scroll, vector4(-1,0,0));
      }

      // 右壁 
      // (900,-1200) (1000,-1800)
      if(f==1800 || f==1860 || f==1920 || f==1980 || f==2040) {
        vector4 pos = vector4(950,-1200, 0)+m_scroll;
        vector4 dir = vector4(0,-1,0);
        if(f==1800 || f==1920) {
          FallGround *g = putDeepGround(pos, dir);
          SmallHatch *h = new SmallHatch(new Hatch_Laser(120));
          h->setParent(g);
          h->setDirection(vector4(1,0,0));
          h->setPosition(vector4(50,0,0));
        }
        else {
          box b(vector4(50,50,25));
          if(f==1860) { b=box(vector4(50,50,-25), vector4(-50,-50,-75)); }
          putGround(b, pos, dir);
        }
      }

      // 上下壁 
      // (1000, ) (2400, )
      if(f==2000 || f==2060 || f==2120 || f==2180 || f==2240 )
      {
        putGround(vector4(1600,-1450, 0)+m_scroll, vector4(-1,0,0));
        putGround(vector4(1600,-1850, 0)+m_scroll, vector4(-1,0,0));
      }
      if(f==2500 || f==2560 || f==2620 || f==2680 || f==2740)
      {
        putGround(vector4(2100,-1450, 0)+m_scroll, vector4(-1,0,0));
        putGround(vector4(2100,-1850, 0)+m_scroll, vector4(-1,0,0));
      }
      if(f==3060 || f==3120 || f==3180 || f==3240) {
        putGround(vector4(2600,-1450, 0)+m_scroll, vector4(-1,0,0));
        putGround(vector4(2600,-1850, 0)+m_scroll, vector4(-1,0,0));
      }

      // Turtle*2 
      if(f==2100) {
        Turtle *e = new Turtle(new Turtle_Wait(0.0f, true));
        e->setPosition(vector4(1250,-1650, 0)+m_scroll);
        e->setDirection(vector4(0,1,0));
      }
      if(f==2300) {
        Turtle *e = new Turtle(new Turtle_Wait(0.0f, true));
        e->setPosition(vector4(1500,-1650, 0)+m_scroll);
        e->setDirection(vector4(0,1,0));
      }

      if(f==3360) {
        m_pframe = 0;
        SendKillMessage(0, this);
      }
    }

    void progress(int f)
    {
      m_scroll+=GetGlobalScroll();
      switch(m_action) {
      case 1: phase1(); break;
      case 2: phase2(); break;
      case 3: phase3(); break;
      case 4: phase4(); break;
      case 5: phase5(); break;
      case 6: phase6(); break;
      case 7: phase7(); break;
      case 8: phase8(); break;
      case 9: phase9(); break;
      }
      SweepDeadObject(m_stopper);
    }
  };


  class SceneBoss : public Scene
  {
  typedef Scene Super;
  private:
    gobj_ptr m_boss;

  public:
    SceneBoss(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_boss);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_boss);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_boss);
    }

  public:
    SceneBoss() : m_boss(0)
    {}

    void progress(int f)
    {
      if(f==0) {
        DestroyAllEnemy(getTSM());
        IMusic::FadeOut(2500);
        new Warning(L"\"function\"");
      }
      else if(f==200) {
        IMusic::WaitFadeOut();
        GetMusic("boss.ogg")->play();
        m_boss = CreateBoss2();
        SetBossMode(true);
      }
      else if(m_boss && m_boss->isDead()) {
        DestroyAllEnemy(getTSM());
        SetBossMode(false);
        IMusic::FadeOut(3000);
        SendKillMessage(0, this);
      }
    }
  };


  class SceneEnd : public Scene
  {
  typedef Scene Super;
  public:
    SceneEnd(Deserializer& s) : Super(s) {}
    SceneEnd() {}
    void progress(int f)
    {
      Background *bg = getBackground();
      if(f==240) {
        bg->fadeout();
        ShowStageResult();
      }
      if(f==700) {
        SendKillMessage(0, this);
      }
    }
  };



  class Stage : public StageBase
  {
  typedef StageBase Super;
  public:
    Stage(Deserializer& s) : Super(s) {}
    Stage()
    {
      Scene::init();
      GetMusic("stage2.ogg")->play();
      SetCameraMovableArea(vector2(0,0), vector2(0,0));
      SetGlobalScroll(vector4(0, 0, 0));
    }

    ~Stage()
    {
      Scene::quit();
    }

    scene_ptr changeScene(int scene)
    {
      if     (scene==1) { return new Scene1(); }
      else if(scene==2) { return new Scene2(); }
      else if(scene==3) { return new SceneBoss(); }
      else if(scene==4) { return new SceneEnd(); }
      return 0;
    }
  };

} // stage2
} // exception
#endif

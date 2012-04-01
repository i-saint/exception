#ifndef stage3_h
#define stage3_h


namespace exception {
namespace stage3 {


  class Background : public BackgroundBase
  {
  typedef BackgroundBase Super;
  private:

    class LineTexture : public RefCounter
    {
    private:
      fbo_ptr m_fbo;
      line_cont m_lines;
      int m_rest_time;

    public:
      LineTexture() : m_rest_time(int(GenRand()*100.0f))
      {
        m_fbo = new ist::FrameBufferObject(128, 128);
        for(size_t i=0; i<5; ++i) {
          m_lines.push_back(new Lines(100, 1.5f));
        }
      }

      void update()
      {
        if(--m_rest_time>0) {
          return;
        }
        for(line_cont::iterator p=m_lines.begin(); p!=m_lines.end(); ++p) {
          (*p)->update();
        }
      }

      void draw()
      {
        ScreenMatrix sm(m_fbo->getWidth(), m_fbo->getHeight());
        m_fbo->enable();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        for(line_cont::iterator p=m_lines.begin(); p!=m_lines.end(); ++p) {
          (*p)->draw();
        }
        m_fbo->disable();
      }

      void assign() { m_fbo->assign(); }
    };

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
      Block(const vector4 vel) : m_vel(vel)
      {
        setBox(box(vector4(50.0f)));
      }

      void update()
      {
        vector4 pos = getPosition();
        pos+=m_vel;
        pos.y+=GetGlobalScroll().y;
        if(pos.y>1000.0f) {
          pos.y-=2000.0f;
        }
        setPosition(pos);
      }
    };

    typedef intrusive_ptr<LineTexture> ltex_ptr;
    typedef std::vector<ltex_ptr> ltex_cont;

    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

  private:
    po_ptr m_glow;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;
    ltex_cont m_ltex;
    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    block_cont m_blocks;
    float m_rot;
    int m_scene;
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
      s >> m_rot >> m_scene >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_cam << m_fog << m_bgmat;
      s << m_blocks.size();
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->serialize(s);
      }
      s << m_rot << m_scene << m_frame;
    }

  public:
    Background() : m_scene(0), m_frame(0), m_rot(0.0f)
    {
      m_cam.setPosition(vector4(0.0f, 0.0f, 1000.0f));
      m_cam.setTarget(vector4(0.0f, 0.0f, 0.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setDiffuse(vector4(0.5f, 0.5f, 0.7f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<100; ++i) {
        vector4 pos = matrix44().rotateY(GenRand()*360.0f)*vector4(
          GenRand()*250.0f,
          GenRand()*2000.0f-1000.0f,
          GenRand()*250.0f);
        float d = vector2(pos.x, pos.z).norm();
        Block *block = new Block(vector4(0.0f, 0.5f+d/1000.0f, 0.0f));
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<15; ++i) {
        Block *block = new Block(vector4(0.0f, 2.5f, 0.0f));
        block->setPosition(
          matrix44().rotateY(GenRand()*360.0f)*vector4(
            GenRand()*150.0f+200.0f,
            GenRand()*2000.0f-1000.0f,
            GenRand()*150.0f+200.0f));
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<50; ++i) {
        Block *block = new Block(vector4(0.0f, 0.0f, 0.0f));
        block->setPosition(
          matrix44().rotateY(GenRand()*360.0f)*vector4(
            GenRand()*500.0f+1000.0f,
            GenRand()*2000.0f-1000.0f,
            GenRand()*500.0f-1000.0f));
        m_blocks.push_back(block);
      }
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
      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();
        m_fog.enable();
        m_bgmat.assign();
        // まずは普通に描画 
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->draw();
        }
        ist::Material().assign();
        m_fog.disable();
      }

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }

      drawFadeEffect();

      glClear(GL_DEPTH_BUFFER_BIT);
    }


    void swap(fbo_ptr& l, fbo_ptr& r)
    {
      fbo_ptr t = l;
      l = r;
      r = t;
    }

    void draw_gl20()
    {
      if(!m_glow) {
        m_glow = new ist::ProgramObject();
        m_glow->attach(GetFragmentShader("glow.fsh"));
        m_glow->link();
        m_fbo_lines = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo_glow = new ist::FrameBufferObject(256, 256);
        m_fbo_glow_b = new ist::FrameBufferObject(256, 256);

        for(size_t i=0; i<5; ++i) {
          LineTexture *lt = new LineTexture();
          m_ltex.push_back(lt);
        }
      }

      {
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        for(size_t i=0; i<m_ltex.size(); ++i) {
          m_ltex[i]->draw();
        }
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
      }
      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();

        // テクスチャ張った状態をfboに描画 
        glDisable(GL_LIGHTING);
        m_fbo_lines->enable();
        if(m_frame==1) {
          glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else {
          glClear(GL_DEPTH_BUFFER_BIT);
        }

        // フィードバックブラー 
        {
          ScreenMatrix sm;

          glDisable(GL_DEPTH_TEST);
          glDepthMask(GL_FALSE);

          m_fbo_lines->assign();
          glEnable(GL_TEXTURE_2D);
          vector2 str(4.0f, 1.5f);
          glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
          drawRect(vector2(640.0f, 480.0f)+str*3.0f, vector2(0.0f, 0.0f)-str*3.0f);
          drawRect(vector2(640.0f, 480.0f)+str*2.0f, vector2(0.0f, 0.0f)-str*2.0f);
          drawRect(vector2(640.0f, 480.0f)+str*1.0f, vector2(0.0f, 0.0f)-str*1.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glDisable(GL_TEXTURE_2D);
          m_fbo_lines->disassign();

          glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
          drawRect(vector2(640,480), vector2(0,0));
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

          glDepthMask(GL_TRUE);
          glEnable(GL_DEPTH_TEST);
        }

        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_ltex[i%m_ltex.size()]->assign();
          m_blocks[i]->draw();
        }
        m_fbo_lines->disable();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
      }
      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        // テクスチャ張った状態を加算合成 
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

        // グローエフェクトを描画 
        m_fbo_glow->assign();
        glColor3f(0.3f, 0.3f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0f, 1.0f, 1.0f);
        m_fbo_glow->disassign();
        
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      m_rot+=0.01f-GetGlobalScroll().x/5.0f;
      m_cam.setPosition(matrix44().rotateY(m_rot)*(vector4(0.0f, 0.0f, 1000.0f)+GetCamera().getTarget()));
      vector4 t = GetCamera().getTarget();
      t.x = 0;
      m_cam.setTarget(t);

      for(size_t i=0; i<m_ltex.size(); ++i) {
        m_ltex[i]->update();
      }
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
    }
  };







  class Scene : public SceneBase
  {
  typedef SceneBase Super;
  protected:
    static float brand() { return float(s_brand->genReal()-0.5f)*2.0f; }
    static Background* getBackground() { return s_bg; }


    virtual PillerBlock* putPillerBlock(const vector4& pos, const box& b, float life=100)
    {
      PillerBlock *e = new PillerBlock();
      e->setParent(m_group);
      e->setPosition(pos);
      e->setBox(b);
      e->setLife(life);
      e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1500,-1000)));
      return e;
    }

    virtual DynamicGround* putDynamicGround(const vector4& pos, const box& b)
    {
      DynamicGround *e = new DynamicGround();
      e->setParent(m_group);
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1500,-1000)));
      return e;
    }

    virtual StaticGround* putStaticGround(const vector4& pos, const box& b)
    {
      StaticGround *e = new StaticGround();
      e->setParent(m_group);
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1500,-1000)));
      return e;
    }

    virtual CoverGround* putCoverGround(const vector4& pos, const box& b)
    {
      CoverGround *e = new CoverGround();
      e->setParent(m_group);
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(1000,1000,1000), vector4(-1000,-1500,-1000)));
      return e;
    }



    virtual Egg* putMineEgg(const vector4& pos, const vector4& dir, bool scroll=false)
    {
      Egg *e = new Egg(new Egg_Mine(scroll));
      e->setParent(m_group);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    virtual Egg* putMissileEgg(const vector4& pos, const vector4& dir, bool scroll=false)
    {
      Egg *e = new Egg(new Egg_Missile(scroll));
      e->setParent(m_group);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    virtual Fighter* putFighter(const vector4& pos, const vector4& dir, float speed=2.5f)
    {
      Fighter *e = new Fighter(new Fighter_Straight(speed));
      e->setParent(m_group);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

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


    Fighter* putFighter2(const vector4& pos, const vector4& dir, float speed=2.0f)
    {
      Fighter *e = new Fighter(new Fighter_Straight2(speed));
      e->setParent(m_group);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    SmallHatch* putBoltHatch(gobj_ptr parent, const vector4& pos, const vector4& dir, int wait=300)
    {
      SmallHatch *e = new SmallHatch(new Hatch_Bolt(wait));
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    SmallHatch* putLaserHatch(gobj_ptr parent, const vector4& pos, const vector4& dir, int wait=300)
    {
      SmallHatch *e = new SmallHatch(new Hatch_Laser(wait));
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    LargeHatch* putLargeHatch(gobj_ptr parent, const vector4& pos, const vector4& dir, int wait=300)
    {
      LargeHatch *e = new LargeHatch(new Hatch_GenRushFighter(wait, true));
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

  protected:
    static random_ptr s_brand;
    static Background *s_bg;
    RotLayer *m_group;

  public:
    Scene(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, s_bg);
      DeserializeLinkage(s, m_group);
      s_brand = new ist::Random(1);
      s >> (*s_brand);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, s_bg);
      SerializeLinkage(s, m_group);
      s << (*s_brand);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(s_bg);
      ReconstructLinkage(m_group);
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

    Scene()
    {
      m_group = new RotLayer();
      m_group->chain();
    }

    void onKill(KillMessage& m)
    {
      Unchain(m_group);
      Super::onKill(m);
    }
  };
  random_ptr Scene::s_brand = 0;
  Background* Scene::s_bg = 0;




  // 編隊地帯 
  class Scene1 : public Scene
  {
  typedef Scene Super;
  private:
    HeavyFighter *m_hfighter[2];
    int m_hfighter_destroyed;

  public:
    Scene1(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_hfighter);
      s >> m_hfighter_destroyed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_hfighter);
      s << m_hfighter_destroyed;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_hfighter);
    }

  public:
    Scene1() :  m_hfighter_destroyed(0)
    {
      ZeroClear(m_hfighter);
      SetGlobalScroll(vector4(0, 0.4f, 0));
      SetCameraMovableArea(vector2(0,50), vector2(0,-50));
    }

    HeavyFighter* putHeavyFighter1(const vector4& pos, const vector4& dir)
    {
      HeavyFighter *e = new HeavyFighter(new HeavyFighter_PutMines());
      e->setParent(m_group);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    void segment1(int f)
    {
      if(f%100==0) {
        putMediumBlock(vector4(brand()*320.0f, 360.0f+brand()*30.0f, 0.0f));
      }

      if(f%25==0 && f<=550 ) {
        if(f%50==0) {
          putFighter(vector4(400, 50, 0), vector4(-1,0,0));
        }
        else {
          putFighter(vector4(400,-50, 0), vector4(-1,0,0));
        }
      }

      if(f==150) {
        putMineEgg(vector4(500, 200, 0), vector4(-1,0,0));
        putMineEgg(vector4(500,-200, 0), vector4(-1,0,0));
      }
      if(f==400) {
        putMineEgg(vector4(-500, 200, 0), vector4(1,0,0));
        putMineEgg(vector4(-500,-200, 0), vector4(1,0,0));
      }
    }

    void segment2(int f)
    {
      if(f%300==0 && f<1300) {
        putMediumBlock(vector4(brand()*320.0f, 360.0f+brand()*30.0f, 0.0f));
      }

      if(f%100==0) {
        int ec = f/100;
        vector4 pos = matrix44().rotateZ(ec*37.0f)*vector4(580, 0, 0);
        putMineEgg(pos, -pos);
      }

      for(int i=0; i<2; ++i) {
        if(m_hfighter[i] && m_hfighter[i]->isDead()) {
          m_hfighter_destroyed++;
        }
      }
      SweepDeadObject(m_hfighter);

      if(f>=200 && f<=1300 && !m_hfighter[0]) {
        m_hfighter[0] = putHeavyFighter1(vector4(-700, -130, 0), vector4(1,0,0));
        m_hfighter[0]->setBound(box(vector4(900)));
      }
      if(f>=500 && f<=1600 && !m_hfighter[1]) {
        m_hfighter[1] = putHeavyFighter1(vector4(700, 130, 0), vector4(-1,0,0));
        m_hfighter[1]->setBound(box(vector4(900)));
      }

      if(f>300) {
        m_group->setRotate(Cos180I(1.0f/1400.0f*(f-300))*90.0f);
      }
    }

    void progress(int f)
    {
      const int phase1 = 30;
      const int phase2 = phase1 + 660;
      const int phase3 = phase2 + 1700;
      const int phase_end = phase3 + 60;

      if(f>=phase1 && f<=phase2) {
        segment1(f-phase1);
      }
      if(f>=phase2 && f<=phase3) {
        segment2(f-phase2);
      }
      if(f==phase_end) {
        SendKillMessage(0, this);
      }
    }
  };



  // ドリラー地帯 
  class Scene2 : public Scene
  {
  typedef Scene Super;
  private:
    Egg *m_egg[2];
    gobj_ptr m_stopper;
    vector4 m_scroll;
    vector4 m_scroll_speed;
    int m_phase;

  public:
    Scene2(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_egg);
      DeserializeLinkage(s, m_stopper);
      s >> m_scroll >> m_scroll_speed >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_egg);
      SerializeLinkage(s, m_stopper);
      s << m_scroll << m_scroll_speed << m_phase;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_egg);
      ReconstructLinkage(m_stopper);
    }

  public:
    Scene2() : m_stopper(0), m_scroll_speed(0, 0.4f, 0), m_phase(0)
    {
      SetGlobalAccel(vector4(0, -0.01f, 0));
      SetGlobalScroll(m_scroll_speed);
      SetCameraMovableArea(vector2(80,0), vector2(-80,0));
      ZeroClear(m_egg);
    }

    void killStopper()
    {
      if(m_stopper) {
        SendKillMessage(0, m_stopper);
        m_stopper = 0;
      }
    }

    void progress(int f)
    {
      SweepDeadObject(m_egg);

      m_scroll+=GetGlobalScroll();
      float base = 0.0f;
      gobj_ptr p=0;


      if(f>=60 && f<=420 && f%60==0) {
        putMissileEgg(vector4( 450-150*(f/60-1), 450, 0), vector4(0,-1,0));
      }

      if(f>=1500 && f<=2200) {
        if(!m_egg[0]) {
          m_egg[0] = putMissileEgg(vector4(720, m_scroll.y-850, 0), vector4(-1,0,0), true);
        }
        if(!m_egg[1]) {
          m_egg[1] = putMissileEgg(vector4(720, m_scroll.y-1000, 0), vector4(-1,0,0), true);
        }
      }

      if(f>=3300 && f<=4000) {
        if(!m_egg[0]) {
          m_egg[0] = putMissileEgg(vector4(720, m_scroll.y-1250, 0), vector4(-1,0,0), true);
        }
        if(!m_egg[1]) {
          m_egg[1] = putMissileEgg(vector4(720, m_scroll.y-1400, 0), vector4(-1,0,0), true);
        }
      }


      if(m_phase==0) {
        ++m_phase;

        float h = -500+m_scroll.y;

        putDynamicGround(vector4(0,h +50,0), box(vector4( 250,-100,40), vector4( 150,0,-40))); // 上置物 
      p=putDynamicGround(vector4(0,h -50,0), box(vector4(-100, -50,40), vector4( 470,0,-40))); // 天板 
        putBoltHatch(p,  vector4( 100,   0,0), vector4(0, 1,0), 600);
        putBoltHatch(p,  vector4(  40, -50,0), vector4(0,-1,0), 1450);

      p=putDynamicGround(vector4(0,h-100,0), box(vector4(-200, -50,40), vector4(   0,0,-40))); // 小天板 
        putBoltHatch(p, vector4(-150,  0,0), vector4(0, 1,0), 700);
        putPillerBlock(  vector4(0,h-300,0), box(vector4( -40, 150,30), vector4(   0,0,-30)), 50);
        putPillerBlock(  vector4(0,h-300,0), box(vector4( 160, 200,30), vector4( 200,0,-30)), 50);

        h = -800;
      p=putDynamicGround(vector4(0,h+150,0), box(vector4(-350,-150,30), vector4(-500,0,-30))); // 左ブロック 
        putBoltHatch(p,  vector4(-385,  0,0), vector4(0,1,0), 2600);
        putBoltHatch(p,  vector4(-455,  0,0), vector4(0,1,0), 2600);

        putDynamicGround(vector4(0,h  -0,0), box(vector4(-550, -50,40), vector4(-350,0,-40)));
      p=putDynamicGround(vector4(0,h  -0,0), box(vector4(-349, -50,40), vector4( 199,0,-40))); // 床 
        putBoltHatch(p, vector4(-250,  0,0), vector4(0, 1,0), 1200);
        putBoltHatch(p, vector4( 120,  0,0), vector4(0, 1,0), 1300);
        putBoltHatch(p, vector4(-315,-50,0), vector4(0,-1,0), 2300);

        putDynamicGround(vector4(0,h-100,0), box(vector4( 200,150,40), vector4( 250,0,-40))); // 右ストッパー 
        putDynamicGround(vector4(0,h-200,0), box(vector4( 150,100,40), vector4( 250,0,-40)));
        putPillerBlock(  vector4(0,h-300,0), box(vector4( 180,100,30), vector4( 220,0,-30)), 50);

      p=putDynamicGround(vector4(0,h-300,0), box(vector4(-200,100,40), vector4(   0,0,-40))); // 中央左置物 
        putBoltHatch(p , vector4(0,   50,0), vector4(1,0,0), 2100);
        putPillerBlock(  vector4(0,h -50,0), box(vector4(-150,-150,30), vector4(-110,0,-30)), 50);
        putPillerBlock(  vector4(0,h -50,0), box(vector4(-425,-250,30), vector4(-375,0,-30)), 50); // 左大支柱 

        m_stopper = putStaticGround(vector4(0,h-350,0), box(vector4(-500,50,50), vector4( 500,0,-50)));
      }
      else if(m_phase==1 && m_scroll.y>500.0f) {
        ++m_phase;
        float h = -1100+m_scroll.y;
        killStopper();

        putCoverGround(  vector4(0,h  -0,0), box(vector4(-470,-100,40), vector4(-346,0,-40))); //
        putDynamicGround(vector4(0,h  -0,0), box(vector4(-345,-100,40), vector4(-101,0,-40))); //
        putDynamicGround(vector4(0,h  -0,0), box(vector4(-100, -50,40), vector4( 500,0,-40))); // 天板 

        putPillerBlock(  vector4(0,h -50,0), box(vector4( 250,-200,40), vector4( 300,0,-40)), 50); //
        putPillerBlock(  vector4(0,h-250,0), box(vector4( 250,-150,40), vector4( 300,0,-40)), 50); // 右支柱 

      p=putDynamicGround(vector4(0,h-100,0), box(vector4(-250,-200,40), vector4(-200,0,-40))); // 左入り口 
        putLaserHatch(p, vector4(-250,-100, 0), vector4(-1,0,0), 1200);
        putLaserHatch(p, vector4(-250,-165, 0), vector4(-1,0,0), 1200);
        putPillerBlock(  vector4(0,h-300,0), box(vector4(-250,-100,30), vector4(-210,0,-30)), 50);

        putDynamicGround(vector4(0,h -50,0), box(vector4(-100,-100,30), vector4( 100,0,-30))); // 上重石 
        putPillerBlock(  vector4(0,h-150,0), box(vector4( -80,-100,30), vector4( -30,0,-30)), 20);
        putPillerBlock(  vector4(0,h-150,0), box(vector4(  80,-100,30), vector4(  30,0,-30)), 20);

      p=putCoverGround(  vector4(0,h-250,0), box(vector4(-199, -50,40), vector4( 150,0,-40)));
        putLaserHatch(p, vector4(-120,-50, 0), vector4(0,-1,0), 1600);
        putLaserHatch(p, vector4(   0,-50, 0), vector4(0,-1,0), 1700);
        putLaserHatch(p, vector4( 120,-50, 0), vector4(0,-1,0), 1800);


      p=putCoverGround(  vector4(0,h-400,0), box(vector4(-149, -49,40), vector4( 149,0,-40)));
      //  putLaserHatch(p, );

        putDynamicGround(vector4(0,h-400,0), box(vector4( 150,-50,40), vector4( 470,0,-40)));
        putDynamicGround(vector4(0,h-400,0), box(vector4(-150,-50,40), vector4(-470,0,-40)));

        m_stopper = putStaticGround(vector4(0,h-450,0), box(vector4(-500,-50,50), vector4( 500,0,-50)));
      }
      else if(m_phase==2 && m_scroll.y>1000.0f) {
        ++m_phase;
        float h = -1500+m_scroll.y;
        killStopper();

      p=putStaticGround( vector4(0,h -50,0), box(vector4( 250,-445,50), vector4( 600,0,-50)));
        putLargeHatch(p, vector4( 260, -60, 0), vector4(-1,0,0), 1240);
        putLargeHatch(p, vector4( 260,-170, 0), vector4(-1,0,0), 1320);
        putLargeHatch(p, vector4( 260,-280, 0), vector4(-1,0,0), 1400);
        putLargeHatch(p, vector4( 260,-390, 0), vector4(-1,0,0), 1480);
      p=putStaticGround( vector4(0,h-495,0), box(vector4( 250,-350,50), vector4( 600,0,-50)));
        putLargeHatch(p, vector4( 260, -55, 0), vector4(-1,0,0), 1560);
        putLargeHatch(p, vector4( 260,-165, 0), vector4(-1,0,0), 1640);

      p=putStaticGround( vector4(0,h -50,0), box(vector4(-250,-445,50), vector4(-600,0,-50)));
        putLargeHatch(p, vector4(-260, -60, 0), vector4( 1,0,0), 1240);
        putLargeHatch(p, vector4(-260,-170, 0), vector4( 1,0,0), 1320);
        putLargeHatch(p, vector4(-260,-280, 0), vector4( 1,0,0), 1400);
        putLargeHatch(p, vector4(-260,-390, 0), vector4( 1,0,0), 1480);
      p=putStaticGround( vector4(0,h-495,0), box(vector4(-250,-350,50), vector4(-600,0,-50)));
        putLargeHatch(p, vector4(-260, -55, 0), vector4( 1,0,0), 1560);
        putLargeHatch(p, vector4(-260,-165, 0), vector4( 1,0,0), 1640);
      }

      int end_frame = 4500;
      if(f>end_frame) {
        m_scroll_speed.y = std::max<float>(0, m_scroll_speed.y-0.004);
        SetGlobalScroll(m_scroll_speed);
      }
      if(f>end_frame+100 && f<=end_frame+300) {
        m_group->setRotate(Cos180I(1.0f/200.0f*(f-end_frame-100))*90.0f);
      }
      if(f==end_frame+400) {
        SendKillMessage(0, this);
      }
    }
  };


  // ブロック地帯 
  class Scene3 : public Scene
  {
  typedef Scene Super;
  public:

    LargeHatch *putLargeHatch2(gobj_ptr parent, const vector4& pos, const vector4& dir, bool inv, int wait=300)
    {
      LargeHatch *e = new LargeHatch(new Hatch_GenFighterT1(inv, wait));
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

  private:
    HeavyFighter *m_hfighter[5];
    Egg *m_egg[2];
    vector4 m_scroll_total;
    vector4 m_scroll;
    vector4 m_scroll_speed;
    int m_phase;

  public:
    Scene3(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_hfighter);
      DeserializeLinkage(s, m_egg);
      s >> m_scroll_total >> m_scroll >> m_scroll_speed >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_hfighter);
      SerializeLinkage(s, m_egg);
      s << m_scroll_total << m_scroll << m_scroll_speed << m_phase;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_hfighter);
      ReconstructLinkage(m_egg);
    }

  public:
    Scene3() : m_scroll_speed(-0.6f, 0, 0), m_phase(0)
    {
      ZeroClear(m_hfighter);
      ZeroClear(m_egg);

      SetGlobalAccel(vector4(0, -0.01f, 0));
      SetGlobalScroll(m_scroll_speed);
      SetCameraMovableArea(vector2(0,50), vector2(0,-50));
    }

    void progress(int f)
    {
      SweepDeadObject(m_hfighter);
      SweepDeadObject(m_egg);

      m_scroll_total+=GetGlobalScroll();
      m_scroll+=GetGlobalScroll();
      vector4 base;

      if(f>=int(660.0f/0.6f) && f<=int(1200.0f/0.6f) && !m_hfighter[0]) {
        HeavyFighter *e = new HeavyFighter(new HeavyFighter_Turns1());
        e->setParent(m_group);
        e->setPosition(m_scroll_total+vector4(20+1000,500,0));
        m_hfighter[0] = e;
      }
      if(f>=int(1300.0f/0.6f) && f<=int(1700.0f/0.6f) && !m_hfighter[1]) {
        HeavyFighter *e = new HeavyFighter(new HeavyFighter_Turns2());
        e->setParent(m_group);
        e->setPosition(m_scroll_total+vector4(-100+1550,500,0));
        m_hfighter[1] = e;
      }
      if(f>=int(1500.0f/0.6f) && f<=int(1700.0f/0.6f) && !m_hfighter[2]) {
        HeavyFighter *e = new HeavyFighter(new HeavyFighter_Turns3());
        e->setParent(m_group);
        e->setPosition(m_scroll_total+vector4(-100+1550,-500,0));
        m_hfighter[2] = e;
      }
      if(f>=int(1900.0f/0.6f) && f<=int(2350.0f/0.6f) && (f-int(1900.0f/0.6f))%300==0) {
        HeavyFighter *b = new HeavyFighter(new HeavyFighter_Turns4());
        b->setParent(m_group);
        b->setPosition(m_scroll_total+vector4(450+1550,500,0));
      }


      if(f>=int(175.0f/0.6f) && f<=(550.0f/0.6f)) {
        if(!m_egg[0]) {
          m_egg[0] = putMissileEgg(vector4(-450, 100, 0), vector4(1,0,0));
        }
        if(!m_egg[1]) {
          m_egg[1] = putMissileEgg(vector4(-450,-100, 0), vector4(1,0,0));
        }
      }

      if(f>(500.0f/0.6f) && f<(1100.0f/0.6f) && f%120==0) {
        putFighter2(m_scroll_total+vector4(20+1000,400,0), vector4(0,-1,0));
      }
      if(f>(900.0f/0.6f) && f<(1500.0f/0.6f) && f%120==0) {
        putFighter2(m_scroll_total+vector4(-100+1550,400,0), vector4(0,-1,0));
      }

      gobj_ptr p;
      if(m_phase==0) {
        ++m_phase;

      p=putStaticGround(base+vector4(500, 0, 0), box(vector4(0,250,50), vector4(400,600,-50)));
        putBoltHatch(p, vector4(50,250,0), vector4(0,-1,0), 360);
        putBoltHatch(p, vector4(350,250,0), vector4(0,-1,0), 840);
      p=putStaticGround(base+vector4(500, 0, 0), box(vector4(0,-250,50), vector4(400,-600,-50)));
      p=putStaticGround(base+vector4(700, 0, 0), box(vector4(0, 100,40), vector4(200,-100,-40)));
      p=putStaticGround(base+vector4(625, 0, 0), box(vector4(0,-100,40), vector4(200,-250,-40)));
        putBoltHatch(p, vector4(35,-100,0), vector4(0,1,0), 360);
        putBoltHatch(p, vector4(200,-140,0), vector4(1,0,0), 1300);
        putBoltHatch(p, vector4(200,-210,0), vector4(1,0,0), 1300);
      }
      else if(m_phase==1 && m_scroll.x<-500.0f) {
        ++m_phase;
        m_scroll.x+=500.0f;
        base = m_scroll;

        putStaticGround(base+vector4(500, 0, 0), box(vector4(-100,-300,50), vector4(300,-600,-50)));

        putDynamicGround(base+vector4(650, 0, 0), box(vector4(0,0,40), vector4(150,350,-40)));
        for(int i=0; i<5; ++i) {
          DynamicGround *e = putDynamicGround(base+vector4(695,-300+60*i, 0), box(vector4(0,60,30), vector4(60,0,-30)));
          e->setAccelResist(0.5f);
          e->setBound(box(vector4(1000,800,1000)));
          e->setPushable(true);
        }
      }
      else if(m_phase==2 && m_scroll.x<-550.0f) {
        ++m_phase;
        m_scroll.x+=550.0f;
        base = m_scroll;

        putStaticGround(base+vector4(550, 0, 0),  box(vector4(-50,-300,50), vector4(250,-400,-50)));
        for(int i=0; i<4; ++i) {
          DynamicGround *e = putDynamicGround(base+vector4(620,-300+60*i, 0), box(vector4(0,60,30), vector4(60,0,-30)));
          e->setAccelResist(0.5f);
          e->setBound(box(vector4(1000,800,1000)));
          e->setPushable(true);
        }

      p=putDynamicGround(base+vector4(550, -60, 0), box(vector4(-50,150,40), vector4(250,0,-40)));
        putBoltHatch(p, vector4(-20,150,0), vector4(0, 1,0), 300);
        putBoltHatch(p, vector4(-20,  0,0), vector4(0,-1,0), 300);

        for(int i=0; i<4; ++i) {
          DynamicGround *e = putDynamicGround(base+vector4(620,90+60*i, 0), box(vector4(0,60,30), vector4(60,0,-30)));
          e->setAccelResist(0.5f);
          e->setBound(box(vector4(1000,800,1000)));
          e->setPushable(true);
        }
        putDynamicGround(base+vector4(550,330, 0), box(vector4(0,0,40), vector4(200,600,-40)));
        putStaticGround(base+vector4(750,300, 0), box(vector4(0,0,50), vector4(50,200,-50)));
      }
      else if(m_phase==3 && m_scroll.x<-500.0f) {
        ++m_phase;
        m_scroll.x+=500.0f;
        base = m_scroll;

        putStaticGround(base+vector4(550, 0, 0), box(vector4(0,-100,40), vector4(150,-600,-50)));
        putStaticGround(base+vector4(550, 0, 0), box(vector4(0, 100,40), vector4(150, 600,-50)));
      p=putStaticGround(base+vector4(700, 0, 0), box(vector4(0, 250,40), vector4(400, 350,-40)));
        putLargeHatch2(p, vector4( 60, 250, 0), vector4(0,-1,0), true, 10);
        putLargeHatch( p, vector4(170, 250, 0), vector4(0,-1,0), 1200);
        putLargeHatch( p, vector4(280, 250, 0), vector4(0,-1,0), 1350);
      p=putStaticGround(base+vector4(700, 0, 0), box(vector4(0,-250,40), vector4(400,-350,-40)));
        putLargeHatch2(p, vector4( 60, -250, 0), vector4(0,1,0), false, 10);
        putLargeHatch( p, vector4(170, -250, 0), vector4(0,1,0), 1200);
        putLargeHatch( p, vector4(280, -250, 0), vector4(0,1,0), 1350);
      }

      const int end_frame = 3600;
      if(f>end_frame) {
        m_scroll_speed.x = std::min<float>(0.0f, m_scroll_speed.x+0.006f);
        SetGlobalScroll(m_scroll_speed);
      }
      if(f>end_frame+100 && f<=end_frame+400) {
        m_group->setRotate(Cos180I(1.0f/300.0f*(f-(end_frame+100)))*-90.0f);
      }
      if(f==end_frame+400) {
        SendKillMessage(0, this);
      }
    }
  };


  // レーザーハッチ地帯 
  class Scene4 : public Scene
  {
  typedef Scene Super;
  private:
    Egg *m_egg[5];
    vector4 m_scroll_total;
    vector4 m_scroll;
    vector4 m_scroll_speed;
    int m_phase;

  public:
    Scene4(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_egg);
      s >> m_scroll_total >> m_scroll >> m_scroll_speed >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_egg);
      s << m_scroll_total << m_scroll << m_scroll_speed << m_phase;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_egg);
    }

  public:
    Scene4() : m_scroll_speed(0, 0.45f, 0), m_phase(0)
    {
      ZeroClear(m_egg);

      SetGlobalAccel(vector4(0, -0.01f, 0));
      SetGlobalScroll(m_scroll_speed);
      SetCameraMovableArea(vector2(40,0), vector2(-40,0));
    }

    void progress(int f)
    {
      SweepDeadObject(m_egg);

      m_scroll_total+=GetGlobalScroll();
      m_scroll+=GetGlobalScroll();
      vector4 base;

      if(f<=4200 && f%300==290) {
        HeavyFighter *b = new HeavyFighter(new HeavyFighter_Straight());
        b->setParent(m_group);
        b->setPosition(vector4(0,500,0));
        b->setDirection(vector4(0,-1,0));
        b->setBound(box(vector4(1000,1000,1000), vector4(-1000,-700,-1000)));
      }

      if(m_scroll.y>675 && m_scroll.y<=1125) {
        if(!m_egg[0]) {
          m_egg[0] = putMissileEgg(vector4(600, m_scroll.y-875, 0), vector4(-1,0,0), true);
        }
      }
      if(m_scroll.y>1000 && m_scroll.y<=1575) {
        if(!m_egg[1]) {
          m_egg[1] = putMissileEgg(vector4(-600, m_scroll.y-1075, 0), vector4(1,0,0), true);
        }
      }
      if(m_scroll.y>1450 && m_scroll.y<=2000) {
        if(!m_egg[3]) {
          m_egg[3] = putMissileEgg(vector4(600, m_scroll.y-1425, 0), vector4(-1,0,0), true);
        }
      }

      gobj_ptr p;
      if(m_phase==0) {
        ++m_phase;

        float h = -400;
        putStaticGround(base+vector4(0,h  -0, 0), box(vector4( -75,0,40), vector4(-250, -50,-40)));
        putStaticGround(base+vector4(0,h  -0, 0), box(vector4(  75,0,40), vector4( 250, -50,-40)));
        putStaticGround(base+vector4(0,h  -0, 0), box(vector4(-150,0,40), vector4(-250,-200,-40)));
        putStaticGround(base+vector4(0,h  -0, 0), box(vector4( 150,0,40), vector4( 250,-200,-40)));
        putStaticGround(base+vector4(0,h-200, 0), box(vector4(  75,0,40), vector4( 250,-200,-40)));
      }
      else if(m_phase==1 && m_scroll.y>300.0f) {
        ++m_phase;
        float h = -600+m_scroll.y;

        putStaticGround( base+vector4(0,h  -0, 0), box(vector4(-150,0,30), vector4(-350, -50,-30)));

        putDynamicGround(base+vector4(0,h-100, 0), box(vector4( -75,0,30), vector4(-299, -50,-30)));
        putPillerBlock(  base+vector4(0,h-150, 0), box(vector4(-200,0,30), vector4(-350, -50,-30)), 50);
      p=putStaticGround( base+vector4(0,h-200, 0), box(vector4(-300,0,30), vector4(-351,-150,-30)));
        putLaserHatch(p, vector4(-300, -35, 0), vector4(1,0,0), 400);
        putLaserHatch(p, vector4(-300,-115, 0), vector4(1,0,0), 400);
        putStaticGround( base+vector4(0,h-350, 0), box(vector4(-150,0,30), vector4(-350, -50,-30)));
        putStaticGround( base+vector4(0,h  -0, 0), box(vector4(-350,0,30), vector4(-450,-400,-30)));

        h-=350;
        putStaticGround( base+vector4(0,h  -0, 0), box(vector4(  75,0,30), vector4( 300, -50,-30)));
        putDynamicGround(base+vector4(0,h-200, 0), box(vector4(  75,0,30), vector4( 230, -50,-30)));
        putPillerBlock(  base+vector4(0,h-250, 0), box(vector4( 190,0,30), vector4( 230,-100,-30)), 50);
        putStaticGround( base+vector4(0,h-350, 0), box(vector4( 150,0,30), vector4( 300, -50,-30)));
      p=putStaticGround( base+vector4(0,h  -0, 0), box(vector4( 300,0,30), vector4( 450,-400,-30)));
        putLaserHatch(p, vector4(300, -85, 0), vector4(-1,0,0), 800);
        putLaserHatch(p, vector4(300,-165, 0), vector4(-1,0,0), 800);
      }
      else if(m_phase==2 && m_scroll.y>800.0f) {
        ++m_phase;
        float h = -1300+m_scroll.y;

      p=putStaticGround( base+vector4(0,h  -0, 0), box(vector4( -75,0,30), vector4(-450, -50,-30)));
        putLargeHatch(p, vector4(-130,-10, 0), vector4(0,1,0), 1500);
        putLargeHatch(p, vector4(-250,-10, 0), vector4(0,1,0), 1600);
        putLargeHatch(p, vector4(-370,-10, 0), vector4(0,1,0), 1700);
        putDynamicGround(base+vector4(0,h-200, 0), box(vector4( -75,0,30), vector4(-230, -50,-30)));
        putPillerBlock(  base+vector4(0,h-250, 0), box(vector4(-190,0,30), vector4(-230,-100,-30)), 50);
        putStaticGround( base+vector4(0,h-350, 0), box(vector4(-150,0,30), vector4(-300, -50,-30)));
      p=putStaticGround( base+vector4(0,h -50, 0), box(vector4(-300,0,30), vector4(-450,-350,-30)));
        putLaserHatch(p, vector4(-300, -35, 0), vector4(1,0,0), 700);
        putLaserHatch(p, vector4(-300,-115, 0), vector4(1,0,0), 700);

        h-=350;
      p=putStaticGround( base+vector4(0,h  -0, 0), box(vector4( 75,0,30), vector4(450, -50,-30)));
        putLargeHatch(p, vector4(130,-10, 0), vector4(0,1,0), 2000);
        putLargeHatch(p, vector4(250,-10, 0), vector4(0,1,0), 2100);
        putLargeHatch(p, vector4(370,-10, 0), vector4(0,1,0), 2200);
        putStaticGround( base+vector4(0,h-250, 0), box(vector4(150,0,30), vector4(400, -50,-30)));
      p=putStaticGround( base+vector4(0,h -50, 0), box(vector4(400,0,30), vector4(450,-250,-30)));
        putLaserHatch(p, vector4(400, -35, 0), vector4(-1,0,0), 1000);
        putLaserHatch(p, vector4(400,-115, 0), vector4(-1,0,0), 1000);
      }


      if(f==5000) {
        SendKillMessage(0, this);
      }
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
        new Warning(L"\"iterator\"");
      }
      else if(f==200) {
        IMusic::WaitFadeOut();
        GetMusic("boss.ogg")->play();
        m_boss = CreateBoss3();
        SetBossMode(true);
        SetCameraMovableArea(vector2(20,50), vector2(-20,-50));
      }
      if(m_boss && m_boss->isDead()) {
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
      GetMusic("stage3.ogg")->play();
      SetCameraMovableArea(vector2(0,0), vector2(0,0));
      SetGlobalAccel(vector4(0, -0.01f, 0));
      SetGlobalScroll(vector4(0, 0.4f, 0));
    }

    ~Stage()
    {
      Scene::quit();
    }

    scene_ptr changeScene(int scene)
    {
      if     (scene==1) { return new Scene1(); }
      else if(scene==2) { return new Scene2(); }
      else if(scene==3) { return new Scene3(); }
      else if(scene==4) { return new Scene4(); }
      else if(scene==5) { return new SceneBoss(); }
      else if(scene==6) { return new SceneEnd(); }
      return 0;
    }
  };

}
}

#endif

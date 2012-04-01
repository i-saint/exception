#ifndef stagex_h
#define stagex_h

namespace exception {


namespace stagex {


  class Background : public BackgroundBase
  {
  typedef BackgroundBase Super;

    class LineTexture : public RefCounter
    {
    private:
      fbo_ptr m_fbo;

    public:
      LineTexture()
      {
        m_fbo = new ist::FrameBufferObject(64, 128);
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
              if(f<0.35f) {
                dir = vector2(speed, 0.0f);
              }
              else {
                dir = vector2(0.0f, speed);
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
  
    class Block : public Inherit5(HaveVelocity, Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit5(HaveVelocity, Box, HavePosition, Dummy, RefCounter) Super;
    private:
      ltex_ptr m_lt;
      float m_initial_y;
      float m_freq;
      float m_str;
      float m_cycle;
      vector4 m_emission;

    public:
      Block() : m_initial_y(0.0f), m_freq(GenRand()*360.0f), m_str(GenRand()*75.0f), m_cycle(GenRand()*0.1f)
      {
        setBox(box(vector4(50, 100, 50)));
      }

      const vector4& getEmission() { return m_emission; }
      void setEmission(const vector4& v) { m_emission=v; }

      void setLineTexture(ltex_ptr v) { m_lt=v; }
      void assign() { m_lt->assign(); }
      void disassign() { m_lt->disassign(); }

      struct invert_x_coord
      { vector2 operator()(const vector2& v) { return vector2(1.0f-v.x, v.y); } };

      struct invert_y_coord
      { vector2 operator()(const vector2& v) { return vector2(v.x, 1.0f-v.y); } };

      void initTexcoord(vector2 *tex, const vector4& ur, const vector4& bl)
      {
        tex[0] = vector2(1.0f, 1.0f);
        tex[1] = vector2(0.0f, 1.0f);
        tex[2] = vector2(0.0f, 0.0f);
        tex[3] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_y_coord()); }

        tex[4] = vector2(1.0f, 1.0f);
        tex[5] = vector2(1.0f, 0.0f);
        tex[6] = vector2(0.0f, 0.0f);
        tex[7] = vector2(0.0f, 1.0f);
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_y_coord()); }

        tex[16] = vector2(1.0f, 1.0f);
        tex[17] = vector2(0.0f, 1.0f);
        tex[18] = vector2(0.0f, 0.0f);
        tex[19] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_y_coord()); }

        tex[20] = vector2(1.0f, 1.0f);
        tex[21] = vector2(0.0f, 1.0f);
        tex[22] = vector2(0.0f, 0.0f);
        tex[23] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_y_coord()); }
      }

      void update()
      {
        float pf = m_freq;
        m_freq+=m_cycle;
        vector4 pos = getPosition();
        pos.y+= (sinf(m_freq*ist::radian)-sinf(pf*ist::radian))*m_str;
        setPosition(pos+getVel());
      }

      void draw()
      {
        if(m_emission.x>0.0f) {
          glMaterialfv(GL_FRONT, GL_EMISSION, m_emission.v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
        }
        else {
          Super::draw();
        }
      }
    };

    
    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

  private:
    ltex_ptr m_ltex[4];
    int m_scene;
    int m_frame;
    po_ptr m_hline;
    po_ptr m_glow;
    fbo_ptr m_fbo_tmp;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;
    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;

    vector4 m_basecolor;
    block_cont m_blocks;

    int m_aframe;
    void (Background::*m_action)();
    float m_rot;

  public:
    Background() : m_scene(0), m_frame(0), m_aframe(0), m_action(0), m_rot(0.0f)
    {
      m_basecolor = vector4(1.0f, 1.0f, 2.0f);

      m_cam.setPosition(vector4(0.0f, 500.0f, 1200.0f));
      m_cam.setTarget(vector4(0.0f, 200.0f, 000.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(0.0f);

      m_bgmat.setDiffuse(vector4(0.5f, 0.5f, 0.7f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<50; ++i) {
        vector4 pos = matrix44().rotateY(GenRand()*360.0f)*vector4(
          GenRand()*250.0f,
          GenRand()*2000.0f-1000.0f,
          0.0f);
        float d = vector2(pos.x, pos.z).norm();
        Block *block = new Block();
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<150; ++i) {
        float x = GenRand()*1500.0f+250.0f;
        vector4 pos = matrix44().rotateY(GenRand()*360.0f)*vector4(
          x,
          GenRand()*100.0f-500.0f+(x*x/5000.0f),
          0.0f);
        float d = vector2(pos.x, pos.z).norm();
        Block *block = new Block();
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
    }

    void destroy() { m_action=&Background::_destroy; }
    void setBaseColor(const vector4& v) { m_basecolor=v; }


    float getDrawPriority() { return 0.0f; }

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


    struct less_dist
    {
      vector4 m_pos;

      less_dist(const vector4& pos) : m_pos(pos)
      {}

      bool operator()(block_ptr l, block_ptr r)
      {
        return (l->getPosition()-m_pos).square() < (r->getPosition()-m_pos).square();
      }
    };

    void draw()
    {
      glClearColor(0,0,0,0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      std::sort(m_blocks.begin(), m_blocks.end(), less_dist(m_cam.getPosition()));

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
    }


    void swap(fbo_ptr& l, fbo_ptr& r)
    {
      fbo_ptr t = l;
      l = r;
      r = t;
    }

    void draw_gl20()
    {
      if(!m_hline) {
        for(int i=0; i<4; ++i) {
          m_ltex[i] = new LineTexture();
        }
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->setLineTexture(m_ltex[i%4]);
        }

        m_hline = new ist::ProgramObject();
        m_hline->attach(GetVertexShader("title.vsh"));
        m_hline->attach(GetFragmentShader("title.fsh"));
        m_hline->link();

        m_glow = new ist::ProgramObject();
        m_glow->attach(GetFragmentShader("glow.fsh"));
        m_glow->link();

        m_fbo_lines = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT);
        m_fbo_tmp = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo_glow = new ist::FrameBufferObject(256, 256);
        m_fbo_glow_b = new ist::FrameBufferObject(256, 256);
      }

      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();
        matrix44 icam;
        glGetFloatv(GL_MODELVIEW_MATRIX, icam.v);

        glDisable(GL_LIGHTING);

        // 溝を描画 
        m_fbo_tmp->enable();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        m_hline->enable();
        m_hline->setMatrix4fv("icam", 1, false, icam.invert().v);
        m_hline->setUniform1f("freq", float(m_frame));
        m_hline->setUniform4f("basecolor", m_basecolor.x, m_basecolor.y, m_basecolor.z, m_basecolor.w);
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->assign();
          m_blocks[i]->draw();
          m_blocks[i]->disassign();
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
          glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
          for(int i=3; i>0; --i) {
            vector2 str = vector2(6,4)*i;
            drawRect(vector2(640.0f, 480.0f)+str, vector2(0.0f, 0.0f)-str);
          }
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glDisable(GL_TEXTURE_2D);
          m_fbo_lines->disassign();

          glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
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
      }
   
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
      //  m_fbo_glow->assign();
      //  DrawRect(vector2(float(m_fbo_glow->getWidth()), float(m_fbo_glow->getHeight())), vector2(0.0f, 0.0f));

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      {
        float f = m_fog.getFar();
        m_fog.setFar(f+(2500.0f-f)*0.004f);
      }

      m_rot+=0.05f;
      m_cam.setPosition(matrix44().rotateY(m_rot)*(vector4(0.0f, 500.0f, 1200.0f)+GetCamera().getTarget()));
      m_cam.setTarget(GetCamera().getTarget());
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
      if(m_action) {
        (this->*m_action)();
      }
    }

  private:
    void _destroy()
    {
      int f = ++m_aframe;
      for(int i=0; i<m_blocks.size(); ++i) {
        block_ptr b = m_blocks[i];
        if((b->getPosition()-vector4(0,-1000,0)).norm()<6.6f*f) {
          b->accel(vector4(0, -GenRand()*0.25f, 0));
        }
      }
    }
  };

  class Scene : public SceneBase
  {
  typedef SceneBase Super;
  private:
    static random_ptr s_brand;
    static Background *s_bg;

  protected:
    static float brand() { return float(s_brand->genReal())*2.0f-1.0f; }


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

    static void setBackground(Background *v) { s_bg = v; }
    static Background* getBackground() { return s_bg; }
  };
  random_ptr Scene::s_brand = 0;
  Background* Scene::s_bg = 0;


  class Scene1 : public Scene
  {
  typedef Scene Super;
  private:

  public:
    Scene1()
    {}

    void progress(int f)
    {
      if(f==1800) {
        SendKillMessage(0, this);
      }
    }
  };


  class Scene2 : public Scene
  {
  typedef Scene Super;
  private:

  public:
    Scene2()
    {}

    void progress(int f)
    {
      if(f==60) {
        SendKillMessage(0, this);
      }
    }
  };




  class SceneEnd : public Scene
  {
  typedef Scene Super;
  public:
    void progress(int f)
    {
      if(f==1) {
        if(Background *bg = getBackground()) {
          bg->fadeout();
        }
      }
      if(f==180) {
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
    Stage()
    {
      Scene::init();
      GetMusic("stage4.ogg")->play();
      SetCameraMovableArea(vector2(50, 50), vector2(-50, -50));

      SetGlobalAccel(vector4(0, -0.01f, 0));
      SetGlobalScroll(vector4(0, 0.0f, 0));
    }

    ~Stage()
    {
      Scene::quit();
    }

    scene_ptr changeScene(int scene)
    {
      if     (scene==1) { return new Scene1(); }
      else if(scene==2) { return new Scene2(); }
      return 0;
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      int f = getFrame();
      if(f==1) {
        SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      }
      else if(f==60) {
        new Warning(L"\"kernel\"");
      }
    }
  };


} // stagex
} // exception
#endif

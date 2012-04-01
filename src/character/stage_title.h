#ifndef stage_title_h
#define stage_title_h

namespace exception {
namespace title {


  class Background : public Inherit2(HavePosition, Object)
  {
  typedef Inherit2(HavePosition, Object) Super;

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

      bool isDead() { return true; }

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
  
    class Block : public Inherit4(Box, HavePosition, Dummy, RefCounter)
    {
    private:
      ltex_ptr m_lt;
      float m_initial_y;
      float m_freq;
      float m_str;
      float m_cycle;

    public:
      Block() : m_initial_y(0.0f), m_freq(GenRand()*360.0f), m_str(GenRand()*75.0f), m_cycle(GenRand()*0.1f)
      {
        setBox(box(vector4(50, 100, 50)));
      }

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
        m_freq+=m_cycle;
        vector4 pos = getPosition();
        if(m_initial_y==0) {
          m_initial_y = pos.y;
        }
        pos.y = m_initial_y+sinf(m_freq*ist::radian)*m_str;
        setPosition(pos);
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
    ist::Light m_light;
    ist::Fog m_fog;

    block_cont m_blocks;

  public:
    Background() : m_scene(0), m_frame(0)
    {
      m_cam.setPosition(vector4(0.0f, 500.0f, 1200.0f));
      m_cam.setTarget(vector4(0.0f, 200.0f, 000.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_light.setDiffuse(vector4(0.6f, 0.6f, 0.6f));
      m_light.setPosition(vector4(0.0f, -500.0f, 0.0f));

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(2500.0f);

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
      m_light.enable();
      std::sort(m_blocks.begin(), m_blocks.end(), less_dist(m_cam.getPosition()));

      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();
        m_fog.enable();
        // まずは普通に描画 
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.6f, 0.6f, 0.8f, 1.0f).v);
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->draw();
        }
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f, 0.8f, 0.8f, 1.0f).v);
        m_fog.disable();
      }

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }
      m_light.disable();
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
        m_glow->attach(GetFragmentShader("title_glow.fsh"));
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
        m_hline->setUniform4f("basecolor", 1.0f, 1.0f, 2.0f, 1.0f);
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
            vector2 str = vector2(3,2)*i;
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


        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // グローエフェクトを描画 
        m_fbo_glow->assign();
        glColor4f(0.3f, 0.3f, 1.0f, 1.0f*GetConfig()->bloom);
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


    void update()
    {
      ++m_frame;
      static matrix44 rot = matrix44().rotateY(0.05f);
      m_cam.setPosition(rot * m_cam.getPosition());
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
    }
  };


} // title
} // exception
#endif

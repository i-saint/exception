#ifndef stage4_h
#define stage4_h


namespace exception {
namespace stage4 {


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
        m_fbo = new ist::FrameBufferObject(128, 64);
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
  
    class Block : public Inherit4(Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit4(Box, HavePosition, Dummy, RefCounter) Super;
    private:
      ltex_ptr m_lt;
      float m_initial_y;
      float m_freq;
      float m_str;
      float m_cycle;

    public:
      Block(Deserializer& s) : Super(s)
      {
        s >> m_initial_y >> m_freq >> m_str >> m_cycle;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_initial_y << m_freq << m_str << m_cycle;
      }

    public:
      Block() : m_initial_y(0.0f), m_freq(GenRand()*360.0f), m_str(GenRand()*75.0f), m_cycle(GenRand()*0.1f)
      {
        setBox(box(vector4(100, 50, 50)));
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
        tex[0] = vector2(0.0f, 0.0f);
        tex[1] = vector2(0.0f, 0.0f);
        tex[2] = vector2(0.0f, 0.0f);
        tex[3] = vector2(0.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_y_coord()); }

        tex[4] = vector2(0.0f, 0.0f);
        tex[5] = vector2(0.0f, 0.0f);
        tex[6] = vector2(0.0f, 0.0f);
        tex[7] = vector2(0.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_y_coord()); }

        tex[8] = vector2(1.0f, 1.0f);
        tex[9] = vector2(0.0f, 1.0f);
        tex[10] = vector2(0.0f, 0.0f);
        tex[11] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_y_coord()); }

        tex[12] = vector2(1.0f, 1.0f);
        tex[13] = vector2(1.0f, 0.0f);
        tex[14] = vector2(0.0f, 0.0f);
        tex[15] = vector2(0.0f, 1.0f);
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_y_coord()); }

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
        pos+=GetGlobalScroll()*2.0f;
        if(pos.x<-2000) {
          pos.x+=4000;
        }
        if(pos.y<-1500) {
          pos.y+=3000;
        }
        setPosition(pos);
      }
    };

    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

  private:
    ltex_ptr m_ltex[4];
    po_ptr m_hline;
    po_ptr m_glow;
    fbo_ptr m_fbo_tmp;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;
    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    block_cont m_blocks;
    int m_frame;
    vector4 m_scroll;

  public:
    Background(Deserializer& s) : Super(s)
    {
      s >> m_cam >> m_fog >> m_bgmat;
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_blocks.push_back(new Block(s));
      }
      s >> m_frame >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_cam << m_fog << m_bgmat;
      s << m_blocks.size();
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->serialize(s);
      }
      s << m_frame << m_scroll;
    }

  public:
    Background() : m_frame(0)
    {
      m_cam.setPosition(vector4(0.0f, 0.0f, 1000.0f));
      m_cam.setTarget(vector4(0.0f, 0.0f, 0.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(2000.0f);

      m_bgmat.setDiffuse(vector4(0.5f, 0.5f, 0.7f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<90; ++i) {
        vector4 pos = vector4(
          GenRand()*1000-2000,
          GenRand()*3000-1500,
          GenRand()*1000-1000);
        Block *block = new Block();
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<90; ++i) {
        vector4 pos = vector4(
          GenRand()*1000-   0,
          GenRand()*3000-1500,
          GenRand()*1000-1000);
        Block *block = new Block();
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<70; ++i) {
        vector4 pos = vector4(
          GenRand()*4000-2000,
          GenRand()* 750-1500-1075,
          GenRand()*1000-1000);
        Block *block = new Block();
        block->setPosition(pos);
        m_blocks.push_back(block);
      }
      for(size_t i=0; i<70; ++i) {
        vector4 pos = vector4(
          GenRand()*4000-2000,
          GenRand()* 750-   0-1075,
          GenRand()*1000-1000);
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
        m_hline->attach(GetVertexShader("stage4.vsh"));
        m_hline->attach(GetFragmentShader("stage4.fsh"));
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
        m_hline->setUniform1f("sx", float(m_scroll.x));
        m_hline->setUniform1f("sy", float(m_scroll.y));
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
          vector2 str(4.0f, 3.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
          drawRect(vector2(640.0f, 480.0f)+str*3.0f, vector2(0.0f, 0.0f)-str*3.0f);
          drawRect(vector2(640.0f, 480.0f)+str*2.0f, vector2(0.0f, 0.0f)-str*2.0f);
          drawRect(vector2(640.0f, 480.0f)+str*1.0f, vector2(0.0f, 0.0f)-str*1.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glDisable(GL_TEXTURE_2D);
          m_fbo_lines->disassign();

          glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
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
      m_cam.setPosition(vector4(0.0f, 0.0f, 1000.0f) + GetCamera().getTarget());
      m_cam.setTarget(GetCamera().getTarget());
      m_scroll+=GetGlobalScroll()+vector4(0.8f, 0.8f, 0.0f);
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
    }
  };






  class Elevator : public ScrolledLayer
  {
  typedef ScrolledLayer Super;
  public:

    class Parts : public ChildGround
    {
    typedef ChildGround Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts()
      {
        setBound(box(vector4(3000), vector4(-3000)));
      }

      int getFractionCount() { return 0; }

      void onCollide(CollideMessage& m)
      {
        // Elevatorへ衝突を伝達 
        gobj_ptr p = getParent()->getParent();
        SendCollideMessage(m.getFrom(), p, m.getPosition(), m.getNormal(), m.getDistance());
      }
    };

  private:
    static const int s_num_parts = 4;
    Layer *m_layer;
    Parts *m_parts[s_num_parts];
    gid m_group;

  public:
    Elevator(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_layer);
      DeserializeLinkage(s, m_parts);
      s >> m_group;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_layer);
      SerializeLinkage(s, m_parts);
      s << m_group;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_layer);
      ReconstructLinkage(m_parts);
    }

  public:
    Elevator()
    {
      m_layer = new Layer();
      m_layer->setParent(this);

      box b[s_num_parts] = {
        box(vector4( 99, 99, 20), vector4(-99, 70, -30)),
        box(vector4( 99,-99, 20), vector4(-99,-70, -30)),
        box(vector4( 90, 70,-20), vector4( 40,-70, -30)),
        box(vector4(-40, 70,-20), vector4(-90,-70, -30)),
      };
      for(int i=0; i<s_num_parts; ++i) {
        Parts *p = new Parts();
        p->setParent(m_layer);
        p->setBox(b[i]);
        m_parts[i] = p;
      }

      setGroup(Solid::createGroupID());
    }

    gid getGroup() { return m_group; }

    void setGroup(gid v)
    {
      m_group = v;
      SetGroup(m_parts, v);
    }

    void checkBound()
    {
      if(m_layer) {
        if(!getBound().isInner(m_layer->getPosition())) {
          SendKillMessage(0, this);
        }
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_layer);
      if(!m_layer) {
        SendKillMessage(0, this);
      }
    }

    Parts* getParts(int i) { return m_parts[i]; }
    Layer* getPartsLayer() { return m_layer; }
  };

  class VElevator : public Elevator
  {
  typedef Elevator Super;
  private:
    SmallGear *m_gear;

  public:
    VElevator(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_gear);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_gear);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_gear);
    }

  public:
    VElevator()
    {
      m_gear = new SmallGear();
      m_gear->setParent(getPartsLayer());
      m_gear->setGroup(getGroup());
      m_gear->setPosition(vector4(0,-35, 0));
    }

    SmallGear* getGear() { return m_gear; }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_gear);

      if(m_gear && getPartsLayer()) {
        getPartsLayer()->setPosition(vector4(0, m_gear->getRotate()*0.1f, 0));
      }
    }

    void onCollide(CollideMessage& m)
    {
      gobj_ptr from = m.getFrom();
      if(!IsSolid(from) || IsFraction(from)) {
        return;
      }

      const float r = 0.01f;
      if(m_gear) {
        float rs = m_gear->getRotateSpeed();
        vector4 dir(0, rs>0.0f?1:-1, 0, 0);
        float dot = dir.dot(m.getNormal());
        if(dot < -0.8f) {
          if(dynamic_cast<StaticGround*>(from)) {
            m_gear->setRotateSpeed(dir.y>0.0f ? std::min<float>(0.0f, rs-r) : std::max<float>(0.0f, rs+r));
          }
          else {
            m_gear->setRotateSpeed(dir.y>0.0f ? rs-r : rs+r);
          }
        }
        else if(dot > 0.8f) {
          m_gear->setRotateSpeed(dir.y>0.0f ? rs+r : rs-r);
        }
      }
    }
  };

  class VHElevator : public Elevator
  {
  typedef Elevator Super;
  private:
    SmallGear *m_vgear;
    SmallGear *m_hgear;

  public:
    VHElevator(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_vgear);
      DeserializeLinkage(s, m_hgear);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_vgear);
      SerializeLinkage(s, m_hgear);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_vgear);
      ReconstructLinkage(m_hgear);
    }

  public:
    VHElevator()
    {
      m_vgear = new SmallGear();
      m_vgear->setParent(getPartsLayer());
      m_vgear->setGroup(getGroup());
      m_vgear->setPosition(vector4(-50, 35, 0));
      m_vgear->setIncrementOnly(true);

      m_hgear = new SmallGear();
      m_hgear->setParent(getPartsLayer());
      m_hgear->setGroup(getGroup());
      m_hgear->setPosition(vector4( 50,-35, 0));
      m_hgear->setDecrementOnly(true);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_vgear);
      SweepDeadObject(m_hgear);

      if(Layer *l = getPartsLayer()) {
        vector4 pos = l->getRelativePosition();
        if(m_vgear) {
          pos.y = m_vgear->getRotate()*0.1f;
        }
        if(m_hgear) {
          pos.x = -m_hgear->getRotate()*0.1f;
        }
        l->setPosition(pos);
      }
    }

    void onCollide(CollideMessage& m)
    {
      gobj_ptr from = m.getFrom();
      if(!IsSolid(from) || IsFraction(from)) {
        return;
      }

      const float r = 0.02f;
      if(m_vgear) {
        float rs = m_vgear->getRotateSpeed();
        vector4 dir(0, rs>0.0f?1:-1, 0, 0);
        float dot = dir.dot(m.getNormal());
        if(dot < -0.8f) {
          if(dynamic_cast<StaticGround*>(from)) {
            m_vgear->setRotateSpeed(dir.y>0.0f ? std::min<float>(0.0f, rs-r) : std::max<float>(0.0f, rs+r));
          }
          else {
            m_vgear->setRotateSpeed(dir.y>0.0f ? rs-r : rs+r);
          }
        }
        else if(dot > 0.8f) {
          m_vgear->setRotateSpeed(dir.y>0.0f ? rs+r : rs-r);
        }
      }
      if(m_hgear) {
        float rs = m_hgear->getRotateSpeed();
        vector4 dir(rs>0.0f?-1:1, 0, 0, 0);
        float dot = dir.dot(m.getNormal());
        if(dot < -0.8f) {
          if(dynamic_cast<StaticGround*>(from)) {
            m_hgear->setRotateSpeed(dir.x>0.0f ? std::max<float>(0.0f, rs+r) : std::min<float>(0.0f, rs-r));
          }
          else {
            m_hgear->setRotateSpeed(dir.x>0.0f ? rs+r : rs-r);
          }
        }
        else if(dot > 0.8f) {
          m_hgear->setRotateSpeed(dir.x>0.0f ? rs-r : rs+r);
        }
      }
    }
  };





  // キャラクタ同士の相互作用を表現するclass 
  class Linkage : public Actor
  {
  typedef Actor Super;
  private:
    gobj_ptr m_linker;
    gobj_ptr m_linked;

  public:
    Linkage(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_linker);
      DeserializeLinkage(s, m_linked);
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_linker);
      SerializeLinkage(s, m_linked);
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_linker);
      ReconstructLinkage(m_linked);
    }

  public:
    Linkage(gobj_ptr gear, gobj_ptr g) : m_linker(gear), m_linked(g)
    {}

    gobj_ptr getLinker() { return m_linker; }
    gobj_ptr getLinked() { return m_linked; }

    virtual void updateLinkage() {}
    virtual void onDisconnected() {}

    void onUpdate(UpdateMessage& m)
    {
      SweepDeadObject(m_linker);
      SweepDeadObject(m_linked);
      if(!m_linker || !m_linked) {
        onDisconnected();
        SendKillMessage(0, this);
      }
      else {
        updateLinkage();
      }
    }
  };

  // リンク元が死んだらリンク先も死ぬリンケージ 
  class DestroyLinkage : public Linkage
  {
  typedef Linkage Super;
  public:
    DestroyLinkage(Deserializer& s) : Super(s) {}
    DestroyLinkage(gobj_ptr linker, gobj_ptr linked) : Super(linker, linked)
    {}

    void onDisconnected()
    {
      if(getLinked()) {
        SendDestroyMessage(0, getLinked());
      }
    }
  };





  class Scene : public SceneBase
  {
  typedef SceneBase Super;
  protected:
    static float brand() { return float(s_brand->genReal()-0.5f)*2.0f; }
    static Background* getBackground() { return s_bg; }

    virtual StaticGround* putStaticGround(const vector4& pos, const box& b)
    {
      StaticGround *e = new StaticGround();
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(1500,1500,1000), vector4(-1000,-1500,-1000)));
      return e;
    }

    virtual SmallHatch* putSmallHatch(gobj_ptr parent, const vector4& pos, const vector4& dir, controler_ptr c)
    {
      SmallHatch *e = new SmallHatch(c);
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    virtual LargeHatch *putLargeHatch(gobj_ptr parent, const vector4& pos, const vector4& dir, controler_ptr c)
    {
      LargeHatch *e = new LargeHatch(c);
      e->setParent(parent);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    virtual LaserTurret* putLaserTurret(gobj_ptr p, const vector4& pos, const vector4& dir, int wait=0)
    {
      LaserTurret *e = new LaserTurret(new LaserTurret_Normal(wait));
      e->setParent(p);
      e->setPosition(pos);
      e->setDirection(dir);
      return e;
    }

    virtual SmallGear* putSmallGear(const vector4& pos)
    {
      ScrolledLayer *l = new ScrolledLayer();

      SmallGear *g = new SmallGear();
      g->setParent(l);
      g->setPosition(pos);
      return g;
    }

    virtual LargeGear* putLargeGear(const vector4& pos)
    {
      ScrolledLayer *l = new ScrolledLayer();

      LargeGear *g = new LargeGear();
      g->setParent(l);
      g->setPosition(pos);
      return g;
    }

    virtual LaserGear* putLaserGear(const vector4& pos, int wait)
    {
      ScrolledLayer *l = new ScrolledLayer();

      LaserGear *g = new LaserGear(wait);
      g->setParent(l);
      g->setPosition(pos);
      return g;
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
    {
      SetGlobalScroll(vector4(0, -0.4f, 0));
    }

    void progress(int f)
    {
      if(f==0) {
        DestroyAllEnemy(getTSM());
        IMusic::FadeOut(2500);
        new Warning(L"\"thread\"");
      }
      else if(f==200) {
        IMusic::WaitFadeOut();
        GetMusic("boss.ogg")->play();
        m_boss = CreateBoss4();
        SetBossMode(true);
        SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
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




  class LeftScrollScene : public Scene
  {
  typedef Scene Super;
  protected:
    StaticGround* putStaticGround(const vector4& pos, const box& b)
    {
      StaticGround *e = Super::putStaticGround(pos, b);
      e->setBound(box(vector4(3000,3000,1000), vector4(-800,-1500,-1000)));
      return e;
    }

    LaserTurret* putLaserTurret(gobj_ptr p, const vector4& pos, const vector4& dir, int wait=0)
    {
      LaserTurret *e = Super::putLaserTurret(p, pos, dir, wait);
      e->setBound(box(vector4(3000,3000,1000), vector4(-800,-1500,-1000)));
      return e;
    }

  public:
    LeftScrollScene(Deserializer& s) : Super(s) {}
    LeftScrollScene() {}
  };




  // 入り口 
  class Scene1 : public LeftScrollScene
  {
  typedef LeftScrollScene Super;
  public:

    // 入り口前敵ラッシュ 
    class Act1 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      Fighter* putFighter(const vector4& pos, const vector4& dir)
      {
        Fighter *e = new Fighter(new Fighter_Straight(2.5f, true));
        e->setBound(box(vector4(1000,2000,1000), vector4(-600,-2000,-1000)));
        e->setPosition(pos);
        e->setDirection(dir);
        return e;
      }

    private:
      int m_frame;

    public:
      Act1(Deserializer& s) : Super(s)
      {
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame;
      }

    public:
      Act1(const vector4& pos) : m_frame(0)
      {
        setPosition(pos);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        int f = ++m_frame;
        if(f==60) {
          vector4 pos[] = {
            vector4( 500,  350, 0),
            vector4( 400, -450, 0),
            vector4( 300,  550, 0),
            vector4( 200, -650, 0),
            vector4( 100,  750, 0),
            vector4(   0, -850, 0),
            vector4(-100,  950, 0),
          };
          for(int i=0; i<7; ++i) {
            for(int j=0; j<5; ++j) {
              vector4 dir(0, (i%2)?1:-1, 0);
              putFighter(getPosition()+pos[i], dir);
              pos[i]-=dir*80;
            }
          }
        }
        else if(f==550) {
          vector4 pos[] = {
            vector4(-600, 130, 0),
            vector4(-600,-130, 0),
          };
          for(int i=0; i<2; ++i) {
            HeavyFighter *b = new HeavyFighter(new HeavyFighter_Missiles(false));
            b->setPosition(pos[i]);
            b->setDirection(vector4(1, 0, 0));
          }
        }

        if(f==600) {
          SendKillMessage(0, this);
        }
      }
    };

    // 入り口、ハッチ破壊後ザブ出現 
    class Act2 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_hatch[4];
      int m_frame;

    public:
      Act2(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_hatch);
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_hatch);
        s << m_frame;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_hatch);
      }

    public:
      Act2(const vector4& pos, gobj_ptr (&h)[4]) : m_frame(0)
      {
        for(int i=0; i<4; ++i) {
          m_hatch[i] = h[i];
        }
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;

        int keyframe[] = {
           60, 90, 120, 150, 180, 210, 240
        };
        for(int i=0; i<7; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }
          vector4 pos[] = {
            vector4(250,  200, 0),
            vector4(180,  200, 0),
            vector4(110,  200, 0),
            vector4( 60,  200, 0),
            vector4( 60,  150, 0),
            vector4( 60,  100, 0),
            vector4( 60,   50, 0),
          };
          PutZab(getPosition()+pos[i], true);
          pos[i].y = -pos[i].y;
          PutZab(getPosition()+pos[i], true);

          if(i==6) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_hatch);
        if(!AliveAny(m_hatch)) {
          action();
        }

        if(getPosition().x<-600.0f) {
          SendKillMessage(0, this);
        }
      }
    };

    // 大ギア地帯、ハッチ破壊でザブ出現 
    class Act3 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_hatch;
      int m_frame;
      bool m_ih;
      bool m_iv;

    public:
      Act3(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_hatch);
        s >> m_frame >> m_ih >> m_iv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_hatch);
        s << m_frame << m_ih << m_iv;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_hatch);
      }

    public:
      Act3(const vector4& pos, gobj_ptr h, bool ih, bool iv) :
        m_frame(0), m_hatch(h), m_ih(ih), m_iv(iv)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;

        int keyframe[] = {30, 70, 70, 110, 110, 110};
        for(int i=0; i<6; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }
          vector4 pos[] = {
            vector4(  0,  0,0),
            vector4( 60,  0,0),
            vector4(  0, 60,0),
            vector4( 60, 60,0),
            vector4(120,  0,0),
            vector4(  0,120,0),
          };
          if(m_ih) { pos[i].x = -pos[i].x; }
          if(m_iv) { pos[i].y = -pos[i].y; }
          PutZab(getPosition()+pos[i], true);

          if(i==5) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_hatch);
        if(!AliveAny(m_hatch)) {
          action();
        }
        if(getPosition().x<-600.0f) {
          SendKillMessage(0, this);
        }
      }
    };


    // 入り口リンケージ 
    class Linkage1 : public Linkage
    {
    typedef Linkage Super;
    private:
      SmallGear *m_gear;
      StaticGround *m_ground;
      Layer *m_layer;

    public:
      Linkage1(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_gear);
        DeserializeLinkage(s, m_ground);
        DeserializeLinkage(s, m_layer);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_gear);
        SerializeLinkage(s, m_ground);
        SerializeLinkage(s, m_layer);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_gear);
        ReconstructLinkage(m_ground);
        ReconstructLinkage(m_layer);
      }

    public:
      Linkage1(SmallGear *gear, StaticGround *g) :
        Super(gear, g), m_gear(gear), m_ground(g)
      {
        m_layer = new Layer();
        m_ground->setParent(m_layer);

        m_gear->setMinRotate(0.0f);
        m_gear->setMaxRotate(360.0f*3.0f);
        m_gear->setReverseSpeed(0.04f);
        m_gear->setIncrementOnly(true);
      }

      void updateLinkage()
      {
        m_layer->setPosition(vector4(0, 100.0f/(360.0f*3.0f)*m_gear->getRotate(), 0, 0));
      }
    };

  private:
    vector4 m_scroll;
    int m_phase;

  public:
    Scene1(Deserializer& s) : Super(s)
    {
      s >> m_scroll >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_scroll << m_phase;
    }

  public:
    Scene1() : m_phase(0)
    {
      SetGlobalScroll(vector4(-0.5f, 0, 0));
      SetCameraMovableArea(vector2(50, 100), vector2(-50, -100));
    }

    void progress(int f)
    {
      m_scroll+=GetGlobalScroll();

      if(m_scroll.x<-500 && m_scroll.x>-600) {
        SetCameraMovableArea(vector2(50, 200), vector2(-50, -100));
      }

      vector4 base;
      gobj_ptr p;
      if(m_phase==0) {
        ++m_phase;

        new Act1(vector4());

        base = vector4(600, 0, 0);
        putStaticGround(base, box(vector4(0, 50, 45), vector4(120, 500, -45))); // 入り口 
        putStaticGround(base, box(vector4(0,-50, 45), vector4(120,-500, -45)));
        new Linkage1(
          putSmallGear(base+vector4(-40, 80, 0)),
          putStaticGround(base, box(vector4(5, 50, 35), vector4(115, -50, -35))) );

        {
          gobj_ptr hatch[4];
          base+=vector4(120, 0, 0);
        p=putStaticGround(base, box(vector4(   0, 250,35), vector4( 300, 500,-35))); // 天井 
          hatch[0]=putLargeHatch(p,vector4( 60, 250,  0), vector4(0,-1,0), new Hatch_GenFighterT1(175, false, 750, 120));
          hatch[1]=putLargeHatch(p,vector4(170, 250,  0), vector4(0,-1,0), new Hatch_GenFighterT1(175, false, 740, 120));

        p=putStaticGround(base, box(vector4(   0,-250,35), vector4( 440,-500,-35))); // 床 
          hatch[2]=putLargeHatch(p,vector4( 60,-250,  0), vector4(0,1,0), new Hatch_GenFighterT1(175, true, 750, 120));
          hatch[3]=putLargeHatch(p,vector4(170,-250,  0), vector4(0,1,0), new Hatch_GenFighterT1(175, true, 740, 120));
          new Act2(base, hatch);
        }

        putLargeGear(base+vector4(700,   0, 0));

        putStaticGround(base, box(vector4( 300,  60,35), vector4( 440, 500,-35))); // 中央上突起 
      p=putStaticGround(base, box(vector4( 300, -60,35), vector4( 440,-500,-35))); // 中央下突起 
        new Act3(base+vector4(490, -240, 0),
          putSmallHatch(p,vector4(440,-220, 0), vector4(1,0,0), new Hatch_Laser(1200)), false, false);

      p=putStaticGround(base, box(vector4( 440,-280,35), vector4(1000,-500,-35))); // 床 
        new Act3(base+vector4(950, -240, 0),
          putSmallHatch(p,vector4(920,-280, 0), vector4(0,1,0), new Hatch_Laser(1250)), true, false);

        putStaticGround(base, box(vector4( 900, 350,35), vector4(1200, 500,-35))); // 右上壁 
      p=putStaticGround(base, box(vector4(1105, 250,15), vector4(1195, 350,-15))); // 蓋 
        new DestroyLinkage(putSmallHatch(p,vector4(1105, 300, 0), vector4(-1,0,0), 0), p);
      p=putStaticGround(base, box(vector4(1000, 250,35), vector4(1200,-500,-35))); // 右下壁 
        new Act3(base+vector4(950,  240, 0),
          putSmallHatch(p,vector4(1000, 220, 0), vector4(-1,0,0), new Hatch_Laser(1300)), true, true);

      p=putStaticGround(base, box(vector4( 440, 280,25), vector4( 900, 500,-25))); // 天井 
        new Act3(base+vector4(490,  240, 0),
          putSmallHatch(p,vector4(480, 280, 0), vector4(0,-1,0), new Hatch_Laser(1350)), false, true);
      }
      else if(m_phase==1 && m_scroll.x<=-1420) {
        ++m_phase;
        m_scroll.x+=1420;
        base = m_scroll+vector4(500, 0, 0);

        SendKillMessage(0, this);
      }
      else if(m_phase==2 && m_scroll.x<=-1000) {
        SendKillMessage(0, this);
      }
    }
  };

  // レーザーギア地帯 
  class Scene2 : public LeftScrollScene
  {
  typedef LeftScrollScene Super;
  public:

    // ザブラッシュ 
    class Act1 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      int m_frame;
      int m_wait;

    public:
      Act1(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_wait;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_wait;
      }

    public:
      Act1(const vector4& pos, int wait) : m_frame(0), m_wait(wait)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;

        int keyframe[] = {
            60, 100, 140, 180, 220, 260,
           800, 840, 880, 920, 960,1000,
          1500,1500,1500,1500,1500,1500,
        };
        for(int i=0; i<18; ++i) {
          if(f!=keyframe[i]) { continue; }
          vector4 pos[] = {
            vector4(  50,-330, 0),
            vector4( 110,-330, 0),
            vector4( 170,-330, 0),
            vector4( 230,-330, 0),
            vector4( 280,-330, 0),
            vector4( 340,-330, 0),

            vector4(  50, 330, 0),
            vector4( 110, 330, 0),
            vector4( 170, 330, 0),
            vector4( 230, 330, 0),
            vector4( 290, 330, 0),
            vector4( 350, 330, 0),

            vector4( 350, 100, 0),
            vector4( 350, 170, 0),
            vector4( 350, 240, 0),
            vector4( 350,-100, 0),
            vector4( 350,-170, 0),
            vector4( 350,-240, 0),
          };
          PutZab(getPosition()+pos[i], true);
          if(i==17) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_wait>0) {
          --m_wait;
        }
        else {
          action();
        }

        if(getPosition().x<-600.0f) {
          SendKillMessage(0, this);
        }
      }
    };

    // 壁スライド 
    class Act2 : public Actor
    {
    typedef Actor Super;
    private:
      Layer *m_layer;
      StaticGround *m_g;
      SmallHatch *m_sh;
      LargeHatch *m_lh;
      int m_frame;
      int m_wait;
      bool m_iv;

    public:
      Act2(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_layer);
        DeserializeLinkage(s, m_g);
        DeserializeLinkage(s, m_sh);
        DeserializeLinkage(s, m_lh);
        s >> m_frame >> m_wait >> m_iv;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_layer);
        SerializeLinkage(s, m_g);
        SerializeLinkage(s, m_sh);
        SerializeLinkage(s, m_lh);
        s << m_frame << m_wait << m_iv;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_layer);
        ReconstructLinkage(m_g);
        ReconstructLinkage(m_sh);
        ReconstructLinkage(m_lh);
      }

    public:
      Act2(StaticGround *g, SmallHatch *sh, LargeHatch *lh, int wait, bool iv) :
          m_frame(0), m_g(g), m_sh(sh), m_lh(lh), m_wait(wait), m_iv(iv)
      {
        m_layer = new Layer();
        m_g->setParent(m_layer);
      }

      void action()
      {
        int f = ++m_frame;
        float pq = Cos180I(1.0f/120*(f-1));
        float q = Cos180I(1.0f/120*f);
        m_layer->setPosition(m_layer->getRelativePosition()+vector4(0, 200.0f*(pq-q)*(m_iv?1:-1), 0));

        if(f==1) {
          if(m_sh) {
            m_sh->setControler(new Hatch_Bolt2(240));
          }
          if(m_lh) {
            m_lh->setControler(new Hatch_GenMissileShell(0, true));
          }
        }
        if(f==120) {
          SendKillMessage(0, this);
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_g);
        SweepDeadObject(m_sh);
        SweepDeadObject(m_lh);

        if(!m_layer) {
          SendKillMessage(0, this);
        }
        else if(m_wait>0) {
          --m_wait;
        }
        else {
          action();
        }
      }
    };

    // レーザーギア-上シャッター リンケージ 
    class Linkage1 : public Linkage
    {
    typedef Linkage Super;
    private:
      LaserGear *m_gear;
      StaticGround *m_ground;

    public:
      Linkage1(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_gear);
        DeserializeLinkage(s, m_ground);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_gear);
        SerializeLinkage(s, m_ground);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_gear);
        ReconstructLinkage(m_ground);
      }

    public:
      Linkage1(LaserGear *gear, StaticGround *g) :
        Super(gear, g), m_gear(gear), m_ground(g)
      {
        m_gear->setMinRotate(-100.0f*5.0f);
        m_gear->setMaxRotate(0.0f);
        m_gear->setDecrementOnly(true);
      }

      void updateLinkage()
      {
        if(m_ground) {
          box b = m_ground->getBox();
          vector4 bl = b.getBottomLeft();
          bl.y =-100-m_gear->getRotate()*0.2f;
          b.setBottomLeft(bl);
          m_ground->setBox(b);
        }
      }
    };

    // レーザーギア-下シャッター リンケージ 
    class Linkage2 : public Linkage
    {
    typedef Linkage Super;
    private:
      LaserGear *m_gear;
      StaticGround *m_ground;

    public:
      Linkage2(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_gear);
        DeserializeLinkage(s, m_ground);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_gear);
        SerializeLinkage(s, m_ground);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_gear);
        ReconstructLinkage(m_ground);
      }

    public:
      Linkage2(LaserGear *gear, StaticGround *g) :
        Super(gear, g), m_gear(gear), m_ground(g)
      {
        m_gear->setMinRotate(-100.0f*5.0f);
        m_gear->setMaxRotate(0.0f);
        m_gear->setDecrementOnly(true);
      }

      void updateLinkage()
      {
        if(m_ground) {
          box b = m_ground->getBox();
          vector4 ur = b.getUpperRight();
          ur.y = 100+m_gear->getRotate()*0.2f;
          b.setUpperRight(ur);
          m_ground->setBox(b);
        }
      }
    };

  private:
    vector4 m_scroll;
    int m_phase;

  public:
    Scene2(Deserializer& s) : Super(s)
    {
      s >> m_scroll >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_scroll << m_phase;
    }

  public:
    Scene2() : m_phase(0)
    {
      SetGlobalScroll(vector4(-0.5f, 0, 0));
      SetCameraMovableArea(vector2(50, 100), vector2(-50, -100));
    }

    void progress(int f)
    {
      const int end_frame = 60*60;
      m_scroll+=GetGlobalScroll();

      gobj_ptr p = 0;
      vector4 base;
      if(m_phase==0) {
        ++m_phase;
        base = m_scroll+vector4(500, 0, 0);

        putStaticGround(base, box(vector4(  0, 500,15), vector4(200, 400,-15))); // 詰め物 
        putStaticGround(base, box(vector4(  0,-100,15), vector4(200,-600,-15))); //
        {
          VElevator *l = new VElevator();
          l->setPosition(base+vector4(100, 300, 0));
          l->setBound(box(vector4(3000, 1000, 1000), vector4(-800, -1000, -1000)));
          SmallGear *g = l->getGear();
          g->setDecrementOnly(true);
        }

        base+=vector4(200, 0, 0); // x=700 

        putStaticGround(base, box(vector4(  0, 500,35), vector4(150, 300,-35))); // 左上壁 
      p=putStaticGround(base, box(vector4( 60, 300,30), vector4(150, 100,-30))); //
        putSmallHatch(p ,vector4( 60, 265,0), vector4(-1,0,0), new Hatch_Bolt(1000+ 30));
        putSmallHatch(p ,vector4( 60, 200,0), vector4(-1,0,0), new Hatch_Bolt(1000+ 60));
        putSmallHatch(p ,vector4( 60, 135,0), vector4(-1,0,0), new Hatch_Bolt(1000+ 90));
        putStaticGround(base, box(vector4(  0, 100,35), vector4(150,  50,-35))); //

      p=putStaticGround(base, box(vector4(  5,  50,30), vector4( 95, -50,-30))); // 蓋 
        new DestroyLinkage(putLaserTurret(p, vector4(5, 0, 0), vector4(-1,0,0), 600), p);
        { // 左下スライド壁 
          StaticGround *s[2];
          putStaticGround(base, box(vector4(  0, -50,35), vector4(150,-100,-35)));
          s[0] = putStaticGround(base, box(vector4(  0,-100,30), vector4( 40,-300,-30)));
          s[1] = putStaticGround(base, box(vector4( 40,-100,25), vector4(145,-300,-25)));
          s[1]->setGroup(s[0]->getGroup());
          putStaticGround(base, box(vector4(  0,-300,35), vector4(150,-500,-35)));
          new Act2(s[1],
            putSmallHatch(s[0], vector4( 40,-140, 0), vector4(1,0,0), 0),
            putLargeHatch(s[0], vector4( 40,-240, 0), vector4(1,0,0), 0), 1400, true);
        }

        base+=vector4(150, 0, 0); // x=850 

        new Act1(base+vector4(  0,0,0), 1200); // ザブ発生器 
        putStaticGround(base, box(vector4(100, 150,20), vector4(150, 250,-20))); //
      p=putStaticGround(base, box(vector4(100,  50,20), vector4(225,   0,-20))); // 中央ブロック 
        putSmallHatch(p, vector4(140,  0, 0), vector4(0,-1,0), new Hatch_Bolt(1200));
      p=putStaticGround(base, box(vector4(175,   0,20), vector4(300, -50,-20))); //
        putSmallHatch(p, vector4(260,  0, 0), vector4(0, 1,0), new Hatch_Bolt(1600));
        putStaticGround(base, box(vector4(250,-150,20), vector4(300,-250,-20))); //

        new Linkage2(
          putLaserGear(base+vector4(150,-190, 0), 1700),
          putStaticGround(base+vector4(405,-50,0), box(vector4(0,0,25), vector4(40,0,-25))) ); // 下ギア&シャッター 
        new Linkage1(
          putLaserGear(base+vector4(250, 190, 0), 2200),
          putStaticGround(base+vector4(455, 50,0), box(vector4(0,0,25), vector4(40,0,-25))) ); // 上ギア&シャッター 

        base+=vector4(400, 0, 0); // x=1250 

        { // 右上スライド壁 
          StaticGround *s[2];
          putStaticGround(base, box(vector4(  0, 500,35), vector4(150, 300,-35)));
          s[0] = putStaticGround(base, box(vector4(110, 300,30), vector4(150, 100,-30)));
          s[1] = putStaticGround(base, box(vector4(  5, 300,25), vector4(110, 100,-25)));
          s[1]->setGroup(s[0]->getGroup());
          putStaticGround(base, box(vector4(  0, 100,35), vector4(150,  50,-35)));
          new Act2(s[1],
            putSmallHatch(s[0], vector4(110,140, 0), vector4(-1,0,0), 0),
            putLargeHatch(s[0], vector4(110,240, 0), vector4(-1,0,0), 0), 2100, false);
        }
        putStaticGround(base, box(vector4(  0, -50,35), vector4(150,-500,-35)));

        putStaticGround(base, box(vector4(150, 500,15), vector4(350, 100,-15))); // 詰め物 
        putStaticGround(base, box(vector4(150,-400,15), vector4(350,-500,-15))); //
        {
          VElevator *l = new VElevator();
          l->setPosition(base+vector4(250, 0, 0));
          l->setBound(box(vector4(3000, 1000, 1000), vector4(-800, -1000, -1000)));
          SmallGear *g = l->getGear();
          g->setDecrementOnly(true);
        }

        base+=vector4(350, 0, 0); // x=1600 

        putStaticGround(base, box(vector4( 0, 500,35), vector4(200,   0,-35))); // 右壁 
      p=putStaticGround(base, box(vector4(60,   0,30), vector4(200,-200,-30))); //
        putSmallHatch(p ,vector4(60, -35,0), vector4(-1,0,0), new Hatch_Bolt(2000+ 30));
        putSmallHatch(p ,vector4(60,-100,0), vector4(-1,0,0), new Hatch_Bolt(2000+ 60));
        putSmallHatch(p ,vector4(60,-165,0), vector4(-1,0,0), new Hatch_Bolt(2000+ 90));
        putStaticGround(base, box(vector4( 0,-200,35), vector4(200,-250,-35))); //
      p=putStaticGround(base, box(vector4( 5,-250,15), vector4( 95,-350,-15))); //
        new DestroyLinkage(putLaserTurret(p, vector4(5,-300, 0), vector4(-1,0,0), 3000), p);
        putStaticGround(base, box(vector4( 0,-350,35), vector4(200,-600,-35))); //
      }
      else if(m_phase==1 && m_scroll.x<-1300) {
        ++m_phase;

        SendKillMessage(0, this);
      }
    }
  };


  // 上昇エレベーター地帯 
  class Scene3 : public LeftScrollScene
  {
  typedef LeftScrollScene Super;
  public:

    virtual PillerBlock* putPillerBlock(const vector4& pos, const box& b, float life=100)
    {
      PillerBlock *e = new PillerBlock();
      e->setPosition(pos);
      e->setBox(b);
      e->setLife(life);
      e->setBound(box(vector4(2000,3000,1000), vector4(-800,-1000,-1000)));
      return e;
    }

    virtual DynamicGround* putDynamicGround(const vector4& pos, const box& b)
    {
      DynamicGround *e = new DynamicGround();
      e->setPosition(pos);
      e->setBox(b);
      e->setBound(box(vector4(2000,3000,1000), vector4(-800,-1000,-1000)));
      return e;
    }

    // 入り口用ザブ発生器 
    class GenZab1 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_trigger;
      int m_frame;

    public:
      GenZab1(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_trigger);
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_trigger);
        s << m_frame;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_trigger);
      }

    public:
      GenZab1(const vector4& pos, gobj_ptr trigger) :
        m_trigger(trigger), m_frame(0)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;
        int keyframe[] = {
          300, 420, 540,
        };
        for(int i=0; i<3; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }
          vector4 pos[] = {
            vector4(0, 150, 0),
            vector4(0,  50, 0),
            vector4(0, -50, 0),
            vector4(0,-150, 0),
          };
          for(int j=0; j<4; ++j) {
            PutZab(getPosition()+pos[j], true);
          }
          if(i==2) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_trigger);

        if(!m_trigger) {
          action();
        }
        if(getPosition().y<-150) {
          SendKillMessage(0, this);
        }
      }
    };

    // 4連大ハッチ地帯用ザブ発生器 
    class GenZab2 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_trigger;
      int m_frame;

    public:
      GenZab2(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_trigger);
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_trigger);
        s << m_frame;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_trigger);
      }

    public:
      GenZab2(const vector4& pos, gobj_ptr trigger) :
        m_trigger(trigger), m_frame(0)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;
        int keyframe[] = {
          60, 100, 140, 180,
        };
        for(int i=0; i<4; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }
          vector4 pos[] = {
            vector4(210, 0, 0),
            vector4(140, 0, 0),
            vector4( 70, 0, 0),
            vector4(  0, 0, 0),
          };
          PutZab(getPosition()+pos[i]+vector4(0, 50, 0), true);
          PutZab(getPosition()+pos[i]+vector4(0,-50, 0), true);
          if(i==3) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_trigger);

        if(!m_trigger) {
          action();
        }
        if(getPosition().y<-250) {
          SendKillMessage(0, this);
        }
      }
    };

    // 3連大ハッチ地帯用ザブ発生器 
    class GenZab3 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_trigger;
      int m_frame;

    public:
      GenZab3(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_trigger);
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_trigger);
        s << m_frame;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_trigger);
      }

    public:
      GenZab3(const vector4& pos, gobj_ptr trigger) :
        m_trigger(trigger), m_frame(0)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;
        int keyframe[] = {
          60, 100, 140, 180,
        };
        for(int i=0; i<4; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }
          vector4 pos[] = {
            vector4(-210, 0, 0),
            vector4(-140, 0, 0),
            vector4( -70, 0, 0),
            vector4(  -0, 0, 0),
          };
          PutZab(getPosition()+pos[i]+vector4(0, 70,0), true);
          PutZab(getPosition()+pos[i]+vector4(0,  0,0), true);
          PutZab(getPosition()+pos[i]+vector4(0,-70,0), true);
          if(i==3) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_trigger);

        if(!m_trigger) {
          action();
        }
        if(getPosition().y<-200) {
          SendKillMessage(0, this);
        }
      }
    };

    // ラストの蓋地帯ザブ発生器 
    class GenZab4 : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      gobj_ptr m_trigger;
      int m_frame;

    public:
      GenZab4(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_trigger);
        s >> m_frame;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_trigger);
        s << m_frame;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_trigger);
      }

    public:
      GenZab4(const vector4& pos, gobj_ptr trigger) :
        m_trigger(trigger), m_frame(0)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;
        int keyframe[] = {
          60, 90, 120, 150, 190,
        };
        for(int i=0; i<4; ++i) {
          if(f!=keyframe[i]) {
            continue;
          }

          vector4 pos[] = {
            vector4( 30, 0, 0),
            vector4( 90, 0, 0),
            vector4(150, 0, 0),
            vector4(210, 0, 0),
            vector4(270, 0, 0),
          };
          for(int j=0; j<5; ++j) {
            PutZab(getPosition()+pos[i]+vector4(0, 60*j-120, 0), true);
          }
          if(i==4) {
            SendKillMessage(0, this);
          }
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_trigger);

        if(!m_trigger) {
          action();
        }
        if(getPosition().y<-300) {
          SendKillMessage(0, this);
        }
      }
    };

    // ハッチ破壊をトリガーに壁破壊 
    class Act3 : public Actor
    {
    typedef Actor Super;
    private:
      LargeHatch *m_lh[2];
      SmallHatch *m_sh[3];
      StaticGround *m_sground;

    public:
      Act3(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_lh);
        DeserializeLinkage(s, m_sh);
        DeserializeLinkage(s, m_sground);
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_lh);
        SerializeLinkage(s, m_sh);
        SerializeLinkage(s, m_sground);
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_lh);
        ReconstructLinkage(m_sh);
        ReconstructLinkage(m_sground);
      }

    public:
      Act3(LargeHatch *lh[2], SmallHatch *sh[3], StaticGround *s)
      {
        for(int i=0; i<2; ++i) { m_lh[i]=lh[i]; }
        for(int i=0; i<3; ++i) { m_sh[i]=sh[i]; }
        m_sground = s;
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_lh);
        SweepDeadObject(m_sh);
        SweepDeadObject(m_sground);

        if(!m_sground) {
          SendKillMessage(0, this);
        }
        else if(!AliveAny(m_sh)) {
          for(int i=0; i<2; ++i) {
            if(!m_lh[i]) { continue; }
            m_lh[i]->setControler(new Hatch_GenMissileShell(0, true));
          }
          SendDestroyMessage(0, m_sground, 1);
          SendKillMessage(0, this);
        }
      }
    };

    class GenEgg : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      Egg *m_egg[2];
      int m_frame;
      int m_wait;

    public:
      GenEgg(Deserializer& s) : Super(s)
      {
        DeserializeLinkage(s, m_egg);
        s >> m_frame >> m_wait;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        SerializeLinkage(s, m_egg);
        s << m_frame << m_wait;
      }

      void reconstructLinkage()
      {
        Super::reconstructLinkage();
        ReconstructLinkage(m_egg);
      }

    public:
      GenEgg(const vector4& pos, int wait) : m_frame(0), m_wait(wait)
      {
        ZeroClear(m_egg);
        setPosition(pos);
      }

      Egg* putMissileEgg(const vector4& pos)
      {
        Egg *e = new Egg(new Egg_Missile(true));
        e->setPosition(pos);
        e->setDirection(vector4(-1,0,0));
        return e;
      }

      void action()
      {
        int f = ++m_frame;
        if(f>0 && !m_egg[0]) {
          m_egg[0] = putMissileEgg(getPosition()+vector4(300,  0,0));
        }
        if(f>40 && !m_egg[1]) {
          m_egg[1] = putMissileEgg(getPosition()+vector4(300,150,0));
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        SweepDeadObject(m_egg);

        if(m_wait>0) {
          --m_wait;
        }
        if(m_wait==0) {
          action();
        }
        if(getPosition().y<-250) {
          SendKillMessage(0, this);
        }
      }
    };

    class GenHeavyFighter : public ScrolledActor
    {
    typedef ScrolledActor Super;
    private:
      int m_frame;
      int m_wait;

    public:
      GenHeavyFighter(Deserializer& s) : Super(s)
      {
        s >> m_frame >> m_wait;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_frame << m_wait;
      }

    public:
      GenHeavyFighter(const vector4& pos, int wait) : m_frame(0), m_wait(wait)
      {
        setPosition(pos);
      }

      void action()
      {
        int f = ++m_frame;
        if(f%270==1) {
          HeavyFighter *e = new HeavyFighter(new HeavyFighter_Turns1());
          e->setPosition(getPosition()+vector4(-300, 0, 0));
        }
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);
        if(m_wait>0) {
          --m_wait;
        }
        if(m_wait==0) {
          action();
        }
        if(getPosition().y<-400) {
          SendKillMessage(0, this);
        }
      }
    };

  private:
    vector4 m_scroll;
    int m_phase;

  public:
    Scene3(Deserializer& s) : Super(s)
    {
      s >> m_scroll >> m_phase;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_scroll << m_phase;
    }

  public:
    Scene3() : m_phase(0)
    {
      SetGlobalScroll(vector4(-0.5f, 0, 0));
      SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
    }

    void progress(int f)
    {
      const int end_frame = 60*60;
      m_scroll+=GetGlobalScroll();

      if(m_phase==1 && m_scroll.x<-1045) {
        vector4 gs = GetGlobalScroll();
        if(fabsf(gs.x)>0.01f) {
          gs.x*=0.975f;
        }
        else if(gs.y>-0.4f) {
          gs.x = 0;
          gs.y = gs.y+(-0.4f-gs.y)*0.01f;
        }
        SetGlobalScroll(gs);
      }

      gobj_ptr p = 0;
      vector4 base;
      if(m_phase==0) {
        ++m_phase;
        base = m_scroll+vector4(500, 0, 0);

        putStaticGround(base, box(vector4(  0,-350,35), vector4(150,-500,-35)));
      p=putStaticGround(base, box(vector4(  0,  50,25), vector4(150, -50,-25)));
        new DestroyLinkage(
          putLargeHatch(p,vector4(75,-50, 0), vector4(0,-1,0), new Hatch_GenFighterT1(200, true, 300, 60)), p);
        putDynamicGround(base,box(vector4(  1, 195,35), vector4(149, 50,-35)));
      p=putDynamicGround(base,box(vector4(  1, 305,30), vector4( 40,195,-30)));
        new DestroyLinkage(
          putLargeHatch(p,vector4(40,250, 0), vector4(1,0,0), new Hatch_GenFighterT1(300, true, 900, 60)), p);
        putDynamicGround(base,box(vector4(  1, 305,35), vector4(149,420,-35)));

      p=putStaticGround(base, box(vector4(150, 200,35), vector4(250,-100,-35))); // 左側入り口 
        putLargeHatch(p,vector4(250,  10, 0), vector4(1,0,0), new Hatch_GenFighterT1(200, true,1100, 60));
      p=putStaticGround(base, box(vector4(150,-200,35), vector4(250,-500,-35))); //
        putLargeHatch(p,vector4(250,-310, 0), vector4(1,0,0), new Hatch_GenFighterT1(200,false,1100, 60));

        base+=vector4(250, 0, 0);
        {
          VHElevator *e = new VHElevator();
          e->setPosition(base+vector4(100,-150, 0));
          e->setBound(box(vector4(3000,1000, 1000), vector4(-1000, -10000, -1000)));
        }
      p=putStaticGround(base, box(vector4(  0,-400,35), vector4(800,-500,-35))); // 床 
        putLaserTurret(p, vector4(250,-400, 0), vector4(0,1,0), 1450);
        putLaserTurret(p, vector4(350,-400, 0), vector4(0,1,0), 1500);

      p=putStaticGround(base, box(vector4(420,-330,30), vector4(650,-400,-25))); // 右側段差&ハッチ 
        putLargeHatch(p,vector4(480,-330, 0), vector4(0, 1,0), new Hatch_GenMissileShell(1500, true));
        putLargeHatch(p,vector4(590,-330, 0), vector4(0, 1,0), new Hatch_GenFighterT1(180, false,1500, 130));
      p=putStaticGround(base, box(vector4(420, 130,30), vector4(650, 200,-25)));
        putLargeHatch(p,vector4(480, 130, 0), vector4(0,-1,0), new Hatch_GenMissileShell(1500, true));
      p=putLargeHatch(p,vector4(590, 130, 0), vector4(0,-1,0), new Hatch_GenFighterT1(180,  true,1565, 130));
        new GenZab1(base+vector4(100, 0, 0), p);
        putStaticGround(base, box(vector4(420,-400,-25), vector4(650, 200,-35))); // ストッパー 
        putStaticGround(base, box(vector4(  0, -50,-25), vector4(200, 200,-35)));
        putStaticGround(base, box(vector4(  0,-250,-25), vector4(420,-400,-35)));
        putStaticGround(base, box(vector4(650, 250, 35), vector4(800,-400,-35))); // 右壁 

        putStaticGround(base, box(vector4(150, 200,40), vector4(-100, 300,-40))); // 天井 
        putStaticGround(base, box(vector4(420, 200,40), vector4( 800, 300,-40))); //
      p=putStaticGround(base, box(vector4(150, 205,15), vector4( 420, 295,-15))); // 天井 蓋 
        new DestroyLinkage(putLaserTurret(p, vector4(300,205, 0), vector4(0,-1,0), 1550), p);

        putStaticGround(base, box(vector4(150, 230,-25), vector4( 420, 550,-35))); // ストッパー 
        putStaticGround(base, box(vector4(630, 300,-26), vector4( 800, 820,-35))); //


        new GenEgg(base+vector4(730,500,0), 3500);

      p=putStaticGround(base, box(vector4(-200, 550,35), vector4( 430, 600,-35))); // 左側壁 
        putLargeHatch(p,vector4( 40, 560, 0), vector4(0,-1, 0), new Hatch_GenMissileShell(2600+  0, true));
        putLargeHatch(p,vector4(150, 560, 0), vector4(0,-1, 0), new Hatch_GenMissileShell(2600+100, true));
        putLargeHatch(p,vector4(260, 560, 0), vector4(0,-1, 0), new Hatch_GenMissileShell(2600+200, true));
      p=putLargeHatch(p,vector4(370, 560, 0), vector4(0,-1, 0), new Hatch_GenMissileShell(2600+300, true));
        new GenZab2(base+vector4(-50,425,0), p);
        {
          StaticGround *s[2];
          LargeHatch *lh[2];
          SmallHatch *sh[3];
          s[0] =  putStaticGround(base, box(vector4(-200, 600,30), vector4( 250, 820,-30)));
          s[1] =  putStaticGround(base, box(vector4( 250, 600,26), vector4( 370, 820,-15)));
          s[1]->setGroup(s[0]->getGroup());
          lh[0] = putLargeHatch(s[0],vector4(250, 655, 0), vector4(1,0,0), 0);
          lh[1] = putLargeHatch(s[0],vector4(250, 765, 0), vector4(1,0,0), 0);
          sh[0] = putSmallHatch(s[1] ,vector4( 370, 640,0), vector4(1,0,0), new Hatch_Bolt(4000+ 0));
          sh[1] = putSmallHatch(s[1] ,vector4( 370, 710,0), vector4(1,0,0), new Hatch_Bolt(4000+30));
          sh[2] = putSmallHatch(s[1] ,vector4( 370, 780,0), vector4(1,0,0), new Hatch_Bolt(4000+60));
          new Act3(lh, sh, s[1]);
        }
        putStaticGround(base, box(vector4(-200, 820,35), vector4( 350, 870,-35))); //

        {
          VElevator *l = new VElevator();
          l->setPosition(base+vector4(530, 400, 0));
          l->setBound(box(vector4(3000, 1000, 1000), vector4(-800, -1000, -1000)));
          SmallGear *g = l->getGear();
          g->setPosition(vector4(0, 35, 0));
          g->setIncrementOnly(true);
        }

        putStaticGround(base, box(vector4( 630,820,35), vector4( 800, 870,-35)));
        putStaticGround(base, box(vector4( 700,870,35), vector4( 800,1120,-35)));
      p=putStaticGround(base, box(vector4( 350,825,15), vector4( 630, 865,-15)));
        new DestroyLinkage(putLaserTurret(p, vector4(530,825,0), vector4(0,-1,0), 3900), p);
      }
      else if(m_phase==1 && m_scroll.y<-500) {
        ++m_phase;
        base = m_scroll+vector4(750, 870, 0);

        {
          VElevator *l = new VElevator();
          l->setPosition(base+vector4(250, 100, 0));
          l->setBound(box(vector4(3000, 1000, 1000), vector4(-800, -1000, -1000)));
          SmallGear *g = l->getGear();
          g->setPosition(vector4(0, 35, 0));
          g->setIncrementOnly(true);
        }
        putStaticGround(base, box(vector4(   0,  0,-25), vector4( 150,400,-35))); // ストッパー 
        putStaticGround(base, box(vector4( 350,-20,-25), vector4( 800,250,-35))); //
        new GenHeavyFighter(base+vector4(0, 150, 0), 1100); // HeavyFighter発生器 

      p=putStaticGround(base, box(vector4( 350,250,35), vector4( 800,400,-35))); //
        { // 下向き大ハッチ*3 
          gobj_ptr h;
          putLargeHatch(p,vector4(410, 260, 0), vector4(0,-1,0), new Hatch_GenMissileShell(500+  0, true));
          putLargeHatch(p,vector4(520, 260, 0), vector4(0,-1,0), new Hatch_GenMissileShell(500+100, true));
        h=putLargeHatch(p,vector4(630, 260, 0), vector4(0,-1,0), new Hatch_GenMissileShell(500+200, true));
          new GenZab3(base+vector4(660,125,0), h);
        }
      p=putStaticGround(base, box(vector4( 355,400,15), vector4( 395,700,-15)));
        new DestroyLinkage(putLaserTurret(p, vector4(355,550,0), vector4(-1,0,0), 1400), p);
        new GenZab4(base+vector4(400,550,0), p);
        putStaticGround( base, box(vector4( 700,400,30), vector4( 800,700,-30))); // 右壁 

        putStaticGround(base, box(vector4(   0,400,-25), vector4( 150,700,-35))); // ストッパー 
        putStaticGround(base, box(vector4( 350,400,-25), vector4( 500,700,-35))); //

        base+=vector4(0,700, 0); // y=1520 

      p=putStaticGround(base, box(vector4( 350,  0,35), vector4( 550, 50,-35))); // 天井 
        putStaticGround(base, box(vector4( 650,  0,35), vector4( 800, 50,-35)));

        putStaticGround( base, box(vector4( 650, 50,30), vector4( 800, 300,-30))); // 右壁 
      p=putStaticGround( base, box(vector4( 155, 50,15), vector4( 400, 150,-15))); // つっかえ 
        new DestroyLinkage(putLaserTurret(p, vector4(400,100,0), vector4(1,0,0), 9900), p);
        putPillerBlock(  base, box(vector4( 500, 50,30), vector4( 550, 150,-30)), 40);
        putDynamicGround(base, box(vector4( 351,151,30), vector4( 400, 300,-30))); //
      p=putDynamicGround(base, box(vector4( 401,151,30), vector4( 649, 300,-30))); //
        putLargeHatch(p,vector4(470,300,  0), vector4(0,1, 0), new Hatch_GenMissileShell(3200+  0, true));
        putLargeHatch(p,vector4(580,300,  0), vector4(0,1, 0), new Hatch_GenMissileShell(3200+150, true));

        putStaticGround(base, box(vector4(-200,  0,35), vector4( 150, 50,-35))); // 左壁 
      p=putStaticGround(base, box(vector4( 150,200,15), vector4( 350,350,-15))); // 蓋 
        new DestroyLinkage(putLaserTurret(p, vector4(250,200,0), vector4(0,-1,0), 9900), p);
      p=putStaticGround(base, box(vector4(-200, 50,30), vector4(  90,250,-30))); //
        putSmallHatch(p,vector4( 90, 90,0), vector4(1,0,0), new Hatch_Bolt(3100+ 0));
        putSmallHatch(p,vector4( 90,150,0), vector4(1,0,0), new Hatch_Bolt(3100+ 0));
        putSmallHatch(p,vector4( 90,210,0), vector4(1,0,0), new Hatch_Bolt(3100+ 0));
        putStaticGround(base, box(vector4(-200,250,30), vector4( 150,350,-30))); //
        putStaticGround(base, box(vector4(-200,350,35), vector4( 350,400,-35))); //
      }
      else if(m_phase==2 && m_scroll.y<-1800) {
        SendKillMessage(0, this);
      }
    }
  };

  class Scene4 : public Scene
  {
  typedef Scene Super;
  private:
    gobj_ptr m_ship[2];
    int m_wait[2];
    int m_count[2];
    vector4 m_scroll;

  public:
    Scene4(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_ship);
      s >> m_wait >> m_count >> m_scroll;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_ship);
      s << m_wait << m_count << m_scroll;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_ship);
    }

  public:
    Scene4()
    {
      ZeroClear(m_ship);
      ZeroClear(m_wait);
      ZeroClear(m_count);

      SetGlobalScroll(vector4(0, -0.4f, 0));
    }

    void putShips()
    {
      int f = getFrame();
      if(f%200==0) {
        putMediumBlock(vector4(brand()*350.0f, 450.0f+brand()*30.0f, 0.0f));
      }

      for(int i=0; i<2; ++i) {
        if(m_ship[i] && m_ship[i]->isDead()) {
          m_ship[i] = 0;
          m_wait[i] = 150;
        }
        if(m_wait[i]>0) {
          --m_wait[i];
        }
        else if(!m_ship[i]) {
          vector4 pos[] = {
            vector4( 610,300, 0),
            vector4(-610,300, 0),
          };
          bool ih = i==0;
          if((i==0 && m_count[i]%2==0) || (i==1 && m_count[i]%2==1)) {
            LargeCarrier *e = new LargeCarrier(new LargeCarrier_GenMissileShell(ih));
            e->setPosition(pos[i]);
            m_ship[i] = e;
          }
          else {
            Turtle *e = new Turtle(new Turtle_Sliding(ih));
            e->setPosition(pos[i]);
            m_ship[i] = e;
          }
          ++m_count[i];
        }
      }
    }

    void progress(int f)
    {
      m_scroll+=GetGlobalScroll();
      if(m_scroll.y>-800.0f) {
        putShips();
      }
      SweepDeadObject(m_ship);
      if(m_scroll.y<-1100.0f) {
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
      GetMusic("stage4.ogg")->play();
      SetGlobalAccel(vector4(0, -0.02f, 0));
      SetGlobalScroll(vector4(-0.6f, 0, 0));
      SetCameraMovableArea(vector2(0,0), vector2(0,0));
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

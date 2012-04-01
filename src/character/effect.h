#ifndef effect_h
#define effect_h

namespace exception {




  class Flash : public Particle
  {
  typedef Particle Super;
  public:
    typedef TCache<Flash> Factory;
    friend class TCache<Flash>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<Flash>
    {
    typedef ParticleSet<Flash> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("flare.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);

        tex->assign();
        Super::drawSprite();
        tex->disassign();

        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    float m_size;

    Flash() : m_size(0.0f)
    {
      Drawer::instance()->insert(this);
    }

    Flash(Deserializer& s) : Super(s)
    {
      s >> m_size;
    }

  public:
    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_size;
    }

    void setSize(float v)
    {
      m_size = v;
      setScale(vector4(m_size));
    }

    void update()
    {
      m_size-=2.0f;
      setScale(vector4(m_size));
      if(m_size<=0.0f) {
        kill();
      }
    }
  };


  class RingBase : public Particle
  {
  typedef Particle Super;
  private:
    float m_scale;
    float m_scale_speed;
    float m_opa;
    float m_fadeout_speed;

  public:
    RingBase() :
      m_scale(10.0f), m_scale_speed(2.0f), m_opa(1.5f), m_fadeout_speed(0.01f)
    {
    }

    RingBase(Deserializer& s) : Super(s)
    {
      s >> m_scale >> m_scale_speed >> m_opa >> m_fadeout_speed;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_scale << m_scale_speed << m_opa << m_fadeout_speed;
    }

    void setScale(float v)
    {
      m_scale = v;
      Super::setScale(vector4(m_scale));
    }
    void setScaleSpeed(float v)  { m_scale_speed=v; }
    void setOpacity(float v)     { m_opa=v; }
    void setFadeoutSpeed(float v){ m_fadeout_speed=v; }

    void updateSprite(Sprite& sp)
    {
      Super::updateSprite(sp);
      sp.setColor(vector4(1.0f, 1.0f, 1.0f, std::min<float>(1.0f, m_opa)));
    }

    void update()
    {
      m_scale+=m_scale_speed;
      m_opa-=m_fadeout_speed;
      setScale(m_scale);
      if(m_scale<=0.0f || m_opa<=0.0f) {
        kill();
      }
    }
  };

  class RedRing : public RingBase
  {
  typedef RingBase Super;
  public:
    typedef TCache<RedRing> Factory;
    friend class TCache<RedRing>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<RedRing>
    {
    typedef ParticleSet<RedRing> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("ring_r.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        tex->assign();
        Super::drawSprite();
        tex->disassign();
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    RedRing()
    {
      Drawer::instance()->insert(this);
    }

    RedRing(Deserializer& s) : Super(s)
    {}

  public:
    void initialize(const vector4& pos, float scale, float scale_speed=2.0f, float fade_speed=0.02f)
    {
      setScale(scale);
      setScaleSpeed(scale_speed);
      setFadeoutSpeed(fade_speed);
      setPosition(pos);
    }
  };

  class BlueRing : public RingBase
  {
  typedef RingBase Super;
  public:
    typedef TCache<BlueRing> Factory;
    friend class TCache<BlueRing>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<BlueRing>
    {
    typedef ParticleSet<BlueRing> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("ring_b.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        tex->assign();
        Super::drawSprite();
        tex->disassign();
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    BlueRing()
    {
      Drawer::instance()->insert(this);
    }

    BlueRing(Deserializer& s) : Super(s)
    {}

  public:
    void initialize(const vector4& pos, float scale, float scale_speed=2.0f, float fade_speed=0.02f)
    {
      setScale(scale);
      setScaleSpeed(scale_speed);
      setFadeoutSpeed(fade_speed);
      setPosition(pos);
    }
  };

  void PutSmallRedRing(const vector4& pos)  { RedRing::Factory::create()->initialize(pos, 0.0f, 3.5f, 0.06f); }
  void PutMediumRedRing(const vector4& pos) { RedRing::Factory::create()->initialize(pos, 0.0f, 5.0f, 0.04f); }
  void PutBigRedRing(const vector4& pos)    { RedRing::Factory::create()->initialize(pos, 0.0f, 8.0f, 0.02f); }

  void PutSmallBlueRing(const vector4& pos)  { BlueRing::Factory::create()->initialize(pos, 0.0f, 3.5f, 0.06f); }
  void PutMediumBlueRing(const vector4& pos) { BlueRing::Factory::create()->initialize(pos, 0.0f, 5.0f, 0.04f); }
  void PutBigBlueRing(const vector4& pos)    { BlueRing::Factory::create()->initialize(pos, 0.0f, 8.0f, 0.02f); }


  class Explode : public Particle
  {
  typedef Particle Super;
  public:
    typedef TCache<Explode> Factory;
    friend class TCache<Explode>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<Explode>
    {
    typedef ParticleSet<Explode> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("explode2.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        tex->assign();
        Super::drawSprite();
        tex->disassign();
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    size_t m_past;
    vector4 m_vel;

    Explode() : m_past(0)
    {
      Drawer::instance()->insert(this);

      setRotate(vector4(GenRand()*360.0f, 0, 0));
      setScale(vector4(50.0f));
    }

    Explode(Deserializer& s) : Super(s)
    {
      s >> m_past >> m_vel;
    }

  public:
    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_past << m_vel;
    }

    void setStrength(float str)
    {
      m_vel = vector4((GenRand()-0.5f)*str, (GenRand()-0.5f)*str, (GenRand()-0.5f))*str;
    }

    void updateSprite(Sprite& sp)
    {
      Super::updateSprite(sp);

      vector2 cell(32.0f/512.0f, 32.0f/256.0f);
      int x = (m_past-1)%16;
      int y = (m_past-1)/16;
      sp.setTexcoord(cell*vector2(x+1,y+1), cell*vector2(x,y));
    }

    void update()
    {
      setPosition(getPosition()+m_vel+GetGlobalScroll());

      ++m_past;
      if(m_past>16*6) {
        kill();
      }
    }
  };


  class CubeExplode : public Particle
  {
  typedef Particle Super;
  public:
    typedef TCache<CubeExplode> Factory;
    friend class TCache<CubeExplode>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<CubeExplode>
    {
    typedef ParticleSet<CubeExplode> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("explode1.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        tex->assign();
        Super::drawSprite();
        tex->disassign();
        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    size_t m_past;
    vector4 m_vel;

    CubeExplode() : m_past(0)
    {
      Drawer::instance()->insert(this);

      setRotate(vector4(GenRand()*360.0f, 0, 0));
      setScale(vector4(35.0f, 35.0f, 35.0f));

      float str = 1.5f;
      m_vel = vector4((GenRand()-0.5f)*str, (GenRand()-0.5f)*str, (GenRand()-0.5f))*str;
    }

    CubeExplode(Deserializer& s) : Super(s)
    {
      s >> m_past >> m_vel;
    }

  public:
    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_past << m_vel;
    }

    void accel(const vector4& v)
    {
      m_vel+=v;
    }

    void updateSprite(Sprite& sp)
    {
      Super::updateSprite(sp);

      vector2 cell(32.0f/256.0f, 32.0f/256.0f);
      int x = (m_past-1)%8;
      int y = (m_past-1)/8;
      sp.setTexcoord(cell*vector2(x+1,y+1), cell*vector2(x,y));
    }

    void update()
    {
      setPosition(getPosition()+m_vel+GetGlobalScroll());

      ++m_past;
      if(m_past>8*8) {
        kill();
      }
    }
  };


  class BlueParticle : public Particle
  {
  typedef Particle Super;
  public:
    typedef TCache<BlueParticle> Factory;
    friend class TCache<BlueParticle>;
    void release()
    {
      Factory::insertNotUsed(this);
    }

    class Drawer : public ParticleSet<BlueParticle>
    {
    typedef ParticleSet<BlueParticle> Super;
    public:
      Drawer() {}
      Drawer(Deserializer& s) : Super(s) {}
      float getDrawPriority() { return 5.0f; }
      void drawSprite()
      {
        texture_ptr tex = GetTexture("burnerb.png");
        glDisable(GL_LIGHTING);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);

        tex->assign();
        Super::drawSprite();
        tex->disassign();

        glDisable(GL_TEXTURE_2D);
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LIGHTING);
      }
    };

  private:
    int m_frame;
    float m_fade;
    float m_opa;
    float m_size;
    vector4 m_vel;

    BlueParticle() : m_frame(0), m_fade(2.0f/10.0f), m_opa(2.0f), m_size(7.5f)
    {
      Drawer::instance()->insert(this);
      setScale(vector4(m_size));
    }

    BlueParticle(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_fade >> m_opa >> m_size >> m_vel;
    }

  public:
    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_fade << m_opa << m_size << m_vel;
    }

    void setLifeTime(float v) { m_fade=2.0f/float(v); }
    void setVel(const vector4& v) { m_vel=v; }

    void updateSprite(Sprite& sp)
    {
      Super::updateSprite(sp);
      sp.setColor(vector4(1.0f, 1.0f, 1.0f, std::min<float>(1.0f, m_opa)));
    }

    void update()
    {
      ++m_frame;
      setPosition(getPosition()+m_vel+GetGlobalScroll());
      m_size-=(7.5f/30.0f);
      setScale(vector4(m_size));
      m_opa-=m_fade;
      if(m_opa<=0.0f || m_size<=0.0f) {
        kill();
      }
    }
  };


  void PutFlash(const vector4& pos, float size)
  {
    Flash *p = Flash::Factory::create();
    p->setPosition(pos);
    p->setSize(size);
  }


  void PutSmallExplode(const box& box, const matrix44& mat, int num, float strength)
  {
    for(int i=0; i<num; ++i) {
      vector4 pos = box.getBottomLeft()+vector4(
        box.getWidth()*GenRand(), box.getHeight()*GenRand(), box.getLength()*GenRand());
      Explode *p = Explode::Factory::create();
      p->setPosition(mat*pos);
      p->setStrength(strength);
    }
  }

  void PutSmallExplode(const vector4& pos, int num, float strength)
  {
    for(int i=0; i<num; ++i) {
      Explode *p = Explode::Factory::create();
      p->setPosition(pos);
      p->setStrength(strength);
    }
  }

  void PutCubeExplode(const vector4& pos)
  {
    CubeExplode *p = CubeExplode::Factory::create();
    p->setPosition(pos);
  }





  namespace impact {
    extern std::map<gid, float> g_opacity;

    float GetAttenuation()
    {
      float a = 0.0f;
      for(std::map<gid, float>::iterator i=g_opacity.begin(); i!=g_opacity.end(); ++i) {
        a+=powf(i->second, 3.0f);
      }
      return a;
    }

    float GetMaxOpacity()
    {
      float a = 0.0f;
      for(std::map<gid, float>::iterator i=g_opacity.begin(); i!=g_opacity.end(); ++i) {
        a = std::max<float>(i->second, a);
      }
      return a;
    }
  }

  class DirectionalImpact : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    static const int lifetime = 60;

    ist::Light m_light;
    size_t m_frame;
    vector2 m_center;
    vector4 m_center4;
    vector4 m_dir4;
    float m_opa;

  public:
    DirectionalImpact(Deserializer& s) : Super(s)
    {
      s >> m_light >> m_frame >> m_center >> m_center4 >> m_dir4 >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_light << m_frame << m_center << m_center4 << m_dir4 << m_opa;
    }

  public:
    DirectionalImpact(const vector4& center, const vector4& dir) : m_frame(0), m_dir4(dir), m_opa(1.0f)
    {
      Register(this);

      m_center4 = center;
      m_light.setPosition(center);
      m_light.setAmbient(vector4());
      m_light.setSpecular(vector4());

      m_opa-=impact::GetAttenuation();
      impact::g_opacity[getID()] = m_opa;
    }

    ~DirectionalImpact()
    {
      impact::g_opacity.erase(getID());
    }

    bool isDead() { return m_frame > lifetime; }
    float getDrawPriority() { return 19.9f; }

    float getLeft() { return std::max<float>(0.0f, 1.0f-(float(m_frame)/float(lifetime))); }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      ++m_frame;
      m_light.setDiffuse(vector4(0.0f, 0.1f, 0.8f)*getLeft());
      if(GetConfig()->exlight) {
        m_light.enable();
      }

      float opa = getLeft()*m_opa;
      impact::g_opacity[getID()] = opa;
    }

    void draw()
    {
      if(GetConfig()->exlight && !m_light.isEnabled()) {
        m_light.enable();
      }
      else if(!GetConfig()->exlight && m_light.isEnabled()) {
        m_light.disable();
      }

      if(GetConfig()->noblur) {
        return;
      }

      static vector2 s_vertex[32*24];
      static vector2 s_texcoord[32*24];
      static uchar s_color[32*24*4];
      static int s_index[31*23*4];

      vector2 dir = vector2(m_dir4.x, -m_dir4.y).normal();
      vector2 c = GetProjectedPosition(m_center4);
      float str = 10.0f;
      float opa = getLeft()*m_opa;

      float boundU = 1.0f;
      float boundV = 1.0f;
      if(!GetConfig()->npttexture) {
        const IFrameBuffer& rFrameBuffer = GetGame()->getBackFrameBuffer();
        GLsizei widthBackBuffer = rFrameBuffer.getWidth();
        GLsizei heightBackBuffer = rFrameBuffer.getHeight();
        GLsizei widthScreen = rFrameBuffer.getScreenWidth();
        GLsizei heightScreen = rFrameBuffer.getScreenHeight();
        boundU = float(widthScreen) / float(widthBackBuffer);
        boundV = float(heightScreen) / float(heightBackBuffer);
      }

      for(int j=0; j<24; ++j) {
        for(int i=0; i<32; ++i) {
          vector2 pos = vector2((640.0f/31)*i, (480.0f/23)*j);
          vector2 rel = pos-c;
          vector2 urel = rel.normal();
          float dot = dir.dot(urel);
          float range = 700.0f*(std::max<float>(0.0f, dir.dot(urel)));
          float q = std::max<float>(0.0f, 1.0f-(rel.norm()/range));
          float s = str*q;

          uchar *col = &s_color[32*4*j + 4*i];
          col[0] = col[1] = col[2] = 255;
          col[3] = uchar(std::min<float>(1.0f, s*3.0f)*255.0f*opa);

          s_vertex[32*j+i] = pos+((urel+dir).normal()*s);
          s_texcoord[32*j+i] = vector2((boundU/31)*i, boundV-(boundV/23)*j);
        }
      }
      for(int j=0; j<23; ++j) {
        for(int i=0; i<31; ++i) {
          int *index = &s_index[(31*4*j)+(4*i)];
          index[0] = 32*j + i;
          index[1] = 32*(j+1) + i;
          index[2] = 32*(j+1) + (i+1);
          index[3] = 32*j + (i+1);
        }
      }

      IFrameBuffer& fb = GetGame()->getFrontFrameBuffer();
      fb.assign();
      glEnable(GL_TEXTURE_2D);
      {
        ScreenMatrix sm;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glBegin(GL_QUADS);
        for(int i=0; i<31*23*4; ++i) {
          int index = s_index[i];
          glColor4ubv(&s_color[index*4]);
          glTexCoord2fv(s_texcoord[index].v);
          glVertex2fv(s_vertex[index].v);
        }
        glEnd();
        /*
        // テスト用ワイヤフレーム 
        for(int i=0; i<31*23; ++i) {
          glBegin(GL_LINE_LOOP);
          for(int j=0; j<4; ++j) {
            int *index = &s_index[i*4];
            glTexCoord2fv(s_texcoord[index[j]].v);
            glVertex2fv(s_vertex[index[j]].v);
          }
          glEnd();
        }
        */
        glColor4ub(255, 255, 255, 255);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
      fb.disassign();
      glDisable(GL_TEXTURE_2D);
    }
  };


  class ImpactBase : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  protected:
    ist::Light m_light;
    int m_lifetime;
    size_t m_frame;
    vector2 m_center;
    vector4 m_center4;
    float m_opa;

  public:
    ImpactBase(Deserializer& s) : Super(s)
    {
      s >> m_light >> m_lifetime >> m_frame >> m_center >> m_center4;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_light << m_lifetime << m_frame << m_center << m_center4;
    }

  public:
    ImpactBase(const vector4& center) : m_lifetime(0), m_frame(0), m_opa(0.0f)
    {
      Register(this);

      m_center4 = center;
      m_light.setPosition(center);
      m_light.setAmbient(vector4());
      m_light.setSpecular(vector4());
    }

    ~ImpactBase()
    {
      impact::g_opacity.erase(getID());
    }

    float getDrawPriority() { return 20.0f; }

    virtual void setLifetime(int v) { m_lifetime=v; }
    virtual void setOpacity(float v)
    {
      m_opa = v;
      impact::g_opacity[getID()] = getLeft()*m_opa;
    }

    virtual float getLeft() { return std::max<float>(0.0f, 1.0f-(float(m_frame)/float(m_lifetime))); }


    void draw()
    {
      if(GetConfig()->exlight && !m_light.isEnabled()) {
        m_light.enable();
      }
      else if(!GetConfig()->exlight && m_light.isEnabled()) {
        m_light.disable();
      }

      if(GetConfig()->noblur) {
        return;
      }

      m_center = GetProjectedPosition(m_center4);
      float str = 10.0f;
      vector2 ur = (vector2(640, 480)-m_center)/vector2(640, 480) * str;
      vector2 bl = vector2(str, str)-ur;
      float opa = getLeft()*m_opa;

      IFrameBuffer& fb = GetGame()->getFrontFrameBuffer();
      fb.assign();
      glEnable(GL_TEXTURE_2D);
      glColor4f(1.0f, 0.92f, 0.86f, opa);

      vector2 point0(640.0f, 480.0f), point1(0.0f, 0.0f);
      if(!GetConfig()->npttexture) {
        GLsizei widthBackBuffer = fb.getWidth();
        GLsizei heightBackBuffer = fb.getHeight();
        GLsizei widthScreen = fb.getScreenWidth();
        GLsizei heightScreen = fb.getScreenHeight();
        point0.x = 640.0f * widthBackBuffer / widthScreen;
        point1.y = - 480.0f * (heightBackBuffer - heightScreen) / heightScreen;
      }
      DrawRect(point0+ur, point1-bl);

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
      fb.disassign();
      glDisable(GL_TEXTURE_2D);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      float opa = getLeft()*m_opa;
      ++m_frame;
      m_light.setDiffuse(vector4(1.0f, 0.1f, -0.1f)*(getLeft()*impact::GetMaxOpacity()));
      if(GetConfig()->exlight) {
        m_light.enable();
      }
      impact::g_opacity[getID()] = opa;

      if(m_frame > m_lifetime) {
        SendDestroyMessage(0, this);
      }
    }
  };


  class SmallImpact : public ImpactBase
  {
  typedef ImpactBase Super;
  public:
    SmallImpact(const vector4& center) : ImpactBase(center)
    {
      setOpacity((1.0f-impact::GetAttenuation())*0.9f);
      setLifetime(60);
      GetSound("explosion3.wav")->play(2);
      Blow(getTSM(), center, 200, 1);

      PutSmallRedRing(center);
    }

    SmallImpact(Deserializer& s) : Super(s) {}
  };

  class MediumImpact : public ImpactBase
  {
  typedef ImpactBase Super;
  public:
    MediumImpact(const vector4& center) : ImpactBase(center)
    {
      setOpacity(1.0f-impact::GetAttenuation());
      setLifetime(100);
      GetSound("explosion4.wav")->play(2);
      Blow(getTSM(), center, 250, 2);

      PutMediumRedRing(center);
    }

    MediumImpact(Deserializer& s) : Super(s) {}
  };

  class BigImpact : public ImpactBase
  {
  typedef ImpactBase Super;
  public:
    BigImpact(const vector4& center) : ImpactBase(center)
    {
      setOpacity(1.0f-impact::GetAttenuation());
      setLifetime(180);
      GetSound("explosion5.wav")->play(3);
      Blow(getTSM(), center, 350, 3);

      PutMediumRedRing(center);
    }

    BigImpact(Deserializer& s) : Super(s) {}
  };

  class BossDestroyImpact : public ImpactBase
  {
  typedef ImpactBase Super;
  public:
    BossDestroyImpact(const vector4& center) : ImpactBase(center)
    {
      setOpacity(0.99f);
      setLifetime(360);
      GetSound("explosion6.wav")->play(4);

      PutBigRedRing(center);
    }

    BossDestroyImpact(Deserializer& s) : Super(s) {}
  };

  class Shine : public Inherit3(HaveParent, HavePosition, IEffect)
  {
  typedef Inherit3(HaveParent, HavePosition, IEffect) Super;
  private:
    vector4 m_dir;
    float m_length;
    float m_opa;

  public:
    Shine(Deserializer& s) : Super(s)
    {
      s >> m_dir >> m_length >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_dir << m_length << m_opa;
    }

  public:
    Shine(gobj_ptr p) : m_length(0), m_opa(0)
    {
      Register(this);
      setParent(p);
      m_dir = vector4(GenRand()*2.0f-1.0f, GenRand()*2.0f-1.0f, 0).normal();
      m_dir.z = GenRand()*1.2f-0.6f;
      m_dir.normalize();
    }

    float getDrawPriority() { return 6.0f; }

    void draw()
    {
      glDepthMask(GL_FALSE);
      glDisable(GL_LIGHTING);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glBegin(GL_TRIANGLES);
      glColor4f(1,1,1,m_opa);
      glVertex3fv(getPosition().v);
      glColor4f(1,1,1,0);
      glVertex3fv((getPosition()+m_dir*m_length+matrix44().rotateZ(-90)*m_dir*(m_length*0.05)).v);
      glColor4f(1,1,1,0);
      glVertex3fv((getPosition()+m_dir*m_length+matrix44().rotateZ(90)*m_dir*(m_length*0.05)).v);
      glEnd();
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1,1,1,1);
      glEnable(GL_LIGHTING);
      glDepthMask(GL_TRUE);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      m_opa+=0.005f;
      if(m_length<400.0f) {
        m_length+=3.0f;
      }
    }
  };

  class DamageFlash : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    gobj_ptr m_owner;
    float m_opa;

  public:
    DamageFlash(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_owner);
      s >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_owner);
      s << m_opa;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_owner);
    }

  public:
    DamageFlash(gobj_ptr p) : m_owner(p), m_opa(0.0f)
    {
      Register(this);
    }

    float getDrawPriority() { return 20.0f; }

    float getOpacity() { return m_opa; }
    void setOpacity(float v) { m_opa=v; }

    void draw()
    {
      if(m_opa<=0.0f) {
        return;
      }

      texture_ptr tex = GetTexture("damage.png");
      vector2 center = GetProjectedPosition(m_owner->getPosition());

      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glEnable(GL_TEXTURE_2D);
      glColor4f(1,1,1,m_opa);
      tex->assign();
      DrawRect(center+vector2(640, 480), center-vector2(640, 480));
      tex->disassign();
      glColor4f(1,1,1,1);
      glDisable(GL_TEXTURE_2D);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      if(!m_owner || m_owner->isDead()) {
        SendKillMessage(0, this);
      }
      else if(m_opa>0.0f) {
        m_opa-=0.04f;
      }
    }
  };

  class Bloom : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    static const int s_buffer_count = 3;
    po_ptr m_po;
    fbo_ptr m_fbo[s_buffer_count];
    fbo_ptr m_fbo_b[s_buffer_count];
    fbo_ptr m_flatten;
    size_t m_width;
    size_t m_height;

  public:
    Bloom(size_t width=240, size_t height=180) : m_width(width), m_height(height)
    {
      Register(this);
    }

    Bloom(Deserializer& s) : Super(s)
    {
      s >> m_width >> m_height;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_width << m_height;
    }

    float getDrawPriority() { return 10.0f; }

    void swap(fbo_ptr& l, fbo_ptr& r)
    {
      fbo_ptr t = l;
      l = r;
      r = t;
    }

    void draw()
    {
      IConfig& conf = *GetConfig();
      if(!conf.shader || conf.bloom<=0.001f) {
        return;
      }

      if(!m_po) {
        m_po = new ist::ProgramObject();
        m_po->attach(GetFragmentShader("bloom.fsh"));
        m_po->link();

        m_flatten = new ist::FrameBufferObject(m_width, m_height);
        int div[] = {1,2,4,8};
        for(int i=0; i<s_buffer_count; ++i) {
          m_fbo[i] = new ist::FrameBufferObject(m_width/div[i], m_height/div[i]);
          m_fbo_b[i] = new ist::FrameBufferObject(m_width/div[i], m_height/div[i]);
        }
      }

      glDisable(GL_LIGHTING);
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      for(int j=0; j<s_buffer_count; ++j) {
        m_po->enable();
        m_po->setUniform1f("width", float(m_fbo[j]->getWidth()));
        m_po->setUniform1f("height", float(m_fbo[j]->getHeight()));
        GetGame()->getBackFrameBuffer().assign();
        for(int i=0; i<3; ++i) { // bloom.fshは3パス必要 
          if(i!=0) {
            swap(m_fbo[j], m_fbo_b[j]);
            m_fbo_b[j]->assign();
          }
          m_fbo[j]->enable();
          glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
          glClear(GL_COLOR_BUFFER_BIT);
          m_po->setUniform1i("pass", i+1);
          DrawRect(vector2(640.0f, 480.0f), vector2(0.0f, 0.0f));
          m_fbo[j]->disable();
          m_fbo_b[j]->disassign();
        }
        m_po->disable();
      }

      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      m_flatten->enable();
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      for(int i=0; i<s_buffer_count; ++i) {
        glColor4f(1,1,1, (i==0 ? 1.0f : 0.7f)*conf.bloom);
        m_fbo[i]->assign();
        DrawRect(vector2(640.0f, 480.0f), vector2(0.0f, 0.0f));
        m_fbo[i]->disassign();
        glColor4f(1,1,1,1);
      }
      m_flatten->disable();

      m_flatten->assign();
      vector2 point0(640.0f, 480.0f), point1(0.0f, 0.0f);
      if(!conf.npttexture) {
        const IFrameBuffer& rFrameBuffer = GetGame()->getBackFrameBuffer();
        GLsizei widthBackBuffer = rFrameBuffer.getWidth();
        GLsizei heightBackBuffer = rFrameBuffer.getHeight();
        GLsizei widthScreen = rFrameBuffer.getScreenWidth();
        GLsizei heightScreen = rFrameBuffer.getScreenHeight();
        point0.x = 640.0f * widthBackBuffer / widthScreen;
        point1.y = - 480.0f * (heightBackBuffer - heightScreen) / heightScreen;
      }
      DrawRect(point0, point1);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // 確認用 
    //  DrawRect(vector2(float(m_flatten->getWidth()), float(m_flatten->getHeight())), vector2(0.0f, 0.0f));

      m_flatten->disassign();

      glDisable(GL_TEXTURE_2D);
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glEnable(GL_LIGHTING);
    }

    void onDestroy(DestroyMessage& m)
    {
      // KillMessage送らない 
    }
  };




  void PutDirectionalImpact(const vector4& pos, const vector4& dir) { new DirectionalImpact(pos, dir); }

  void PutTinyImpact(IThreadSpecificMethod& tsm, const vector4& pos)
  {
    Blow(tsm, pos, 200, 0.7f);
  }
  void PutSmallImpact(const vector4& pos)  { new SmallImpact(pos); }
  void PutMediumImpact(const vector4& pos) { new MediumImpact(pos); }
  void PutBigImpact(const vector4& pos)    { new BigImpact(pos); }
  void PutBossDestroyImpact(const vector4& pos)    { new BossDestroyImpact(pos); }
  void PutBloom() { new Bloom(); }



  class GameOver : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    wstring m_message;
    int m_frame;
    float m_width;
    int m_mcount;

  public:
    GameOver() :m_frame(0), m_width(0), m_mcount(0)
    {
      Register(this);
      m_message = L"game over";
    }

    GameOver(Deserializer& s) : Super(s)
    {
      s >> m_message >> m_frame >> m_width >> m_mcount;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_message << m_frame << m_width << m_mcount;
    }

    float getDrawPriority() { return 10.0f; }

    void draw()
    {
      glColor4fv(vector4(0,0,0,0.8f).v);
      DrawRect(vector2(640, 240+m_width), vector2(0, 240-m_width));
      glColor4fv(vector4(1,1,1,1).v);

      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glColor4f(1,1,1,0.7f);

      if(sgui::Font *font = GetFont()) {
        float length = font->getAdvance(m_message.c_str(), m_message.size());
        wstring tmp = wstring(m_message.c_str(), m_mcount);
        font->draw(tmp, sgui::Rect(sgui::Point(320.0f-(length/2.0f), 240), sgui::Size(640,480)));
      }

      glColor4f(1,1,1,1);
      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      ++m_frame;
      if(m_frame<=50) {
        m_width = Cos180I(1.0f/50.0f*m_frame)*30;
      }

      if(m_frame>60) {
        m_mcount = std::min<int>(m_mcount+1, m_message.size());
      }

      if(m_frame==450) {
        SendKillMessage(0, this);
      }
    }
  };

  gobj_ptr CreateGameOver()
  {
    return new GameOver();
  }


  class StageResult : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    wstring m_message;
    int m_frame;
    float m_width;
    int m_mcount;

  public:
    StageResult(int bosstime, int hit, float score) :m_frame(0), m_width(0), m_mcount(0)
    {
      Register(this);

      int bossbonus = bosstime*GetBossBonusRate();
      AddScore(bossbonus);

      char buf[512];
      sprintf(buf,
        "stage result\n\n"
        "boss destroy bonus\n%d\n\n"
        "score\n%.0f\n(max %d hit)\n",
        bossbonus, score, hit);
      m_message = sgui::_L(buf);
    }

    StageResult(Deserializer& s) : Super(s)
    {
      s >> m_message >> m_frame >> m_width >> m_mcount;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_message << m_frame << m_width << m_mcount;
    }

    float getDrawPriority() { return 10.0f; }

    void draw()
    {
      glColor4fv(vector4(0,0,0,0.6f).v);
      DrawRect(vector2(400, 240+m_width), vector2(240, 240-m_width));
      glColor4fv(vector4(1,1,1,1).v);

      float opa = 0.7f;
      if(m_frame>370) {
        opa-=0.02f*(m_frame-370);
        if(opa<=0.0f) {
          return;
        }
      }


      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glColor4f(1,1,1,opa);

      if(sgui::Font *font = GetFont()) {
        wstring tmp;
        int prev = 0;
        float h = 130;
        for(int i=0; i<m_mcount; ++i) {
          if(m_message[i]==L'\n') {
            tmp = wstring(m_message.c_str()+prev, i-prev);
            prev = i;
            h+=20.0f;
            float length = font->getAdvance(tmp.c_str(), tmp.size());
            font->draw(tmp, sgui::Rect(sgui::Point(320.0f-(length/2.0f), h), sgui::Size(640, 480)));
          }
          else if(i==m_mcount-1) {
            int i2 = i;
            while(m_message[i2]!=L'\n' && m_message[i2]!=0) {++i2;}
            wstring tmp2 = wstring(m_message.c_str()+prev, i2-prev);
            tmp = wstring(m_message.c_str()+prev, i-prev);
            prev = i;
            h+=20.0f;
            float length = font->getAdvance(tmp2.c_str(), tmp2.size());
            font->draw(tmp, sgui::Rect(sgui::Point(320.0f-(length/2.0f), h), sgui::Size(640, 480)));
          }
        }
      }
      glColor4f(1,1,1,1);

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      ++m_frame;
      if(m_frame<=50) {
        m_width = Cos180I(1.0f/50.0f*m_frame)*100;
      }
      else if(m_frame>400) {
        m_width = Cos180(1.0f/50.0f*(m_frame-400))*100;
      }

      if(m_frame>60) {
        m_mcount = std::min<int>(m_mcount+1, m_message.size());
      }

      if(m_frame==450) {
        SendKillMessage(0, this);
      }
    }
  };

  gobj_ptr CreateStageResult(int bosstime, int hit, float score)
  {
    return new StageResult(bosstime, hit, score);
  }

  class Warning : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  private:
    int m_frame;
    float m_width;
    wstring m_message[3];
    int m_mcount[3];

  public:
    Warning(const wstring& targetname) :
      m_frame(0), m_width(0)
    {
      Register(this);
      GetSound("warning.wav")->play(7);
      m_message[0] = L"w a r n i n g";
      m_message[1] = L"encounting an error protection";
      m_message[2] = targetname;

      for(int i=0; i<3; ++i) {
        m_mcount[i] = 0;;
      }
    }

    Warning(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_width;
      for(size_t i=0; i<3; ++i) { s >> m_message[i]; }
      for(size_t i=0; i<3; ++i) { s >> m_mcount[i]; }
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_width;
      for(size_t i=0; i<3; ++i) { s << m_message[i]; }
      for(size_t i=0; i<3; ++i) { s << m_mcount[i]; }
    }

    float getDrawPriority() { return 10.0f; }

    void draw()
    {
      glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
      DrawRect(vector2(640, 200+m_width), vector2(0, 200-m_width));
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      if(GLEW_EXT_blend_subtract) {
        glColor3f(0.8f, 0.8f, 0.6f);
        glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        DrawRect(vector2(640, 200+m_width), vector2(0, 200-m_width));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationEXT(GL_FUNC_ADD_EXT);
        glColor3f(1,1,1);
      }

      float opa = 0.0f;
      if(m_frame>25) {
        opa = std::min<float>(0.6f, 0.03f*(m_frame-30));
      }
      if(m_frame>140) {
        opa-=0.02f*(m_frame-140);
      }
      if(opa<=0.0f) {
        return;
      }

      ScreenMatrix sm;
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);

      glColor4f(1,1,1,opa*1.3f);
      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);
      texture_ptr tex = GetTexture("warning.png");
      tex->assign();
      sgui::DrawRect(sgui::Rect(sgui::Point(220, 155), sgui::Size(200, 50)));
      tex->disassign();
      glDisable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);

      glColor4f(1,1,1,opa);

      if(sgui::Font *font = GetFont()) {
        wstring tmp = wstring(m_message[1].c_str(), m_mcount[1]);
        float length = font->getAdvance(m_message[1].c_str(), m_message[1].size());
        DrawText(tmp, sgui::Point(320.0f-(length/2.0f), 205));
      }
      if(sgui::Font *font = GetFont()) {
        wstring tmp = wstring(m_message[2].c_str(), m_mcount[2]);
        float length = font->getAdvance(m_message[2].c_str(), m_message[2].size());
        DrawText(tmp, sgui::Point(320.0f-(length/2.0f), 225));
      }
      glColor4f(1,1,1,1);

      glEnable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      ++m_frame;
      if(m_frame<=40) {
        m_width = Cos180I(1.0f/40.0f*m_frame)*60;
      }
      else if(m_frame>160) {
        m_width = Cos180(1.0f/40.0f*(m_frame-160))*60;
      }

      if(m_frame>25) {
        m_mcount[0] = std::min<int>(m_mcount[0]+1, m_message[0].size());
      }
      if(m_frame>40) {
        m_mcount[1] = std::min<int>(m_mcount[1]+1, m_message[1].size());
      }
      if(m_frame>80) {
        m_mcount[2] = std::min<int>(m_mcount[2]+1, m_message[2].size());
      }

      if(m_frame==200) {
        SendKillMessage(0, this);
      }
    }
  };
}
#endif

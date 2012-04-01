#ifndef stage1_h
#define stage1_h

namespace exception {
namespace stage1 {


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
      Block() : m_vel(-5.0f, 0.0f, 0.0f)
      {
        setBox(box(vector4(150, 5, 50)));
      }

      void setVel(const vector4& v) { m_vel=v; }
      const vector4& getVel() const { return m_vel; }

      void update()
      {
        vector4 pos = getPosition();
        pos+=m_vel;
        if(pos.x < -1200.0f) {
          pos.x+=2400.0f;
        }
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
      m_cam.setPosition(vector4(-500.0f, 450.0f, 400.0f));
      m_cam.setTarget(vector4(0.0f, 0.0f, -500.0f));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setDiffuse(vector4(0.5f, 0.5f, 0.7f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<150; ++i) {
        float d = (2400.0f/150.0f);
        Block *block = new Block();
        float y = (GetRand()-0.5f)*2.0f;
        y = y>=0.0f ? 1.0f-(y*y) : -1.0f+(y*y);
        y = y*800.0f-300.0f;
        block->setPosition(vector4(d*i-1200.0f, y, GetRand()*600.0f-800.0f));
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

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }

      drawFadeEffect();

      glClear(GL_DEPTH_BUFFER_BIT);
    }

    void draw_gl20()
    {
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      m_cam.setPosition(vector4(-500.0f, 450.0f, 400.0f) + GetCamera().getTarget());
      m_cam.setTarget(vector4(0.0f, 0.0f, -500.0f) + GetCamera().getTarget());

      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }

      ++m_frame;
    }
  };




  class LongBlock : public Inherit2(HaveSpinAttrib, Enemy)
  {
  typedef Inherit2(HaveSpinAttrib, Enemy) Super;
  public:
    LongBlock(Deserializer& s) : Super(s) {}
    LongBlock()
    {
      setLife(150.0f);
      setBox(box(vector4(30.0f, 100.0f, 30.0f)));
      setAxis(vector4(0.0f, 0.0f, 1.0f));
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      setRotate(getRotate()+0.7f);
      setPosition(getRelativePosition()+vector4(-0.8f, 0.0f, 0.0f));
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }
  };




  class Scene : public SceneBase
  {
  typedef SceneBase Super;
  protected:
    static float brand()
    {
      return float(s_brand->genReal()-0.5)*2.0f;
    }

    static Background* getBackground()
    {
      return s_bg;
    }

    virtual Fighter* putFighter(const vector4& pos)
    {
      Fighter *e = new Fighter(new Fighter_Straight(1.6f));
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setDirection(vector4(-1,0,0));
      return e;
    }

    void putFighter3(const vector4& pos)
    {
      vector4 rpos[3] = {
        vector4(0, 0, 0),
        vector4(100, -50, 0),
        vector4(100, 50, 0),
      };
      for(int i=0; i<3; ++i) {
        Fighter *e = new Fighter(new Fighter_Straight(1.6f));
        e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
        e->setPosition(pos+rpos[i]);
        e->setDirection(vector4(-1,0,0));
      }
    }

    virtual Shell* putBlasterShell(const vector4& pos)
    {
      Shell *e = new Shell(new Shell_Blaster());
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setDirection(vector4(-1,0,0));
      return e;
    }

    virtual LongBlock* putLongBlock(const vector4& pos)
    {
      LongBlock *e = new LongBlock();
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setRotate(brand()*180.0f);
      return e;
    }

    virtual MediumBlock* putMediumBlock(const vector4& pos)
    {
      MediumBlock *e = new MediumBlock();
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setVel(vector4(brand(), brand(), 0)-vector4(0.5f, 0, 0));
      e->setAccel(vector4(-0.02f, 0, 0));
      e->setAxis(vector4(brand(), brand(), brand()).normalize());
      return e;
    }

    virtual LargeBlock* putLargeBlock(const vector4& pos)
    {
      LargeBlock *e = new LargeBlock();
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setPosition(pos);
      e->setVel(vector4(-1.0f, 0.0f, 0)-vector4(0.5f, 0, 0));
      e->setAccel(vector4(-0.02f, 0, 0));
      e->setAxis(vector4(brand(), brand(), brand()).normalize());
      return e;
    }

    virtual LargeCarrier* putLargeCarrier(const vector4& pos)
    {
      LargeCarrier *e = new LargeCarrier(new LargeCarrier_GenFighter());
      e->setBound(box(vector4(1500,1000,1000), vector4(-600,-1000,-1000)));
      e->setDirection(vector4(-1,0,0));
      e->setPosition(pos);
      return e;
    }

  private:
    static Background *s_bg;
    static random_ptr s_brand;

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




  class Scene1 : public Scene
  {
  typedef Scene Super;
  public:
    Scene1(Deserializer& s) : Super(s) {}
    Scene1() {}
    void progress(int f)
    {
      const int phase1 = 120;
      const int phase2 = phase1 + 540;
      const int phase3 = phase2 + 540;
      const int phase4 = phase3 + 660;
      const int phase5 = phase4 + 660;
      const int phase_end = phase5 + 600;

      if(f%180==0 && f<phase5) {
        putMediumBlock(vector4(550.0f+brand()*100.0f, brand()*250.0f, 0.0f));
      }

      if(f==phase1) {
        for(int i=0; i<10; ++i) {
          putFighter(vector4(400+100*i, 150, 0));
        }
      }
      else if(f==phase2) {
        for(int i=0; i<10; ++i) {
          putFighter(vector4(400+100*i, -120, 0));
        }
      }
      else if(f==phase3) {
        putLargeBlock(vector4(400, -120, 0));
        putMediumBlock(vector4(480, -200, 0));
        putMediumBlock(vector4(480, -170, 0));
        putBlasterShell(vector4(650, -230, 0));
        putBlasterShell(vector4(650, -140, 0));
        for(int i=0; i<4; ++i) {
          putFighter3(vector4(400+250*i, 200-120*i, 0));
        }
      }
      else if(f==phase4) {
        putLargeBlock(vector4(400, 100, 0));
        putMediumBlock(vector4(480, 200, 0));
        putMediumBlock(vector4(480, 170, 0));
        putBlasterShell(vector4(650, 230, 0));
        putBlasterShell(vector4(650, 140, 0));
        for(int i=0; i<4; ++i) {
          putFighter3(vector4(400+250*i, -200+120*i, 0));
        }
      }
      else if(f==phase5) {
        for(int i=0; i<13; ++i) {
          float a = float(180/12*i-90)*ist::radian;
          vector4 pos(1100-cosf(a)*700, sinf(a)*225.0f, 0);
          putFighter(pos);
        }
        for(int i=0; i<12; ++i) {
          putMediumBlock(vector4(600, 0, 0)+vector4(50*i, brand()*(50+5*i), 0));
        }
      }
      else if(f==phase5+300) {
        putBlasterShell(vector4(550,  100, 0));
        putBlasterShell(vector4(550,    0, 0));
        putBlasterShell(vector4(550, -100, 0));
      }

      if(f==phase_end) {
        SendKillMessage(0, this);
      }
    }
  };



  class Scene2 : public Scene
  {
  typedef Scene Super;
  private:
    int m_lb_count;

  public:
    Scene2(Deserializer& s) : Super(s)
    {
      s >> m_lb_count;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_lb_count;
    }

  public:
    Scene2() : m_lb_count(0) {}
    void progress(int f)
    {
      const int phase1 = 0;
      const int phase2 = phase1 + 270;
      const int phase3 = phase2 + 270;
      const int phase4 = phase3 + 270;
      const int phase5 = phase4 + 270;
      const int phase6 = phase5 + 270;
      const int phase7 = phase6 + 270;
      if(f%270==0 && f < 2400) {
        ++m_lb_count;
        vector4 pos(470+brand()*20, brand()*300, 0);
        putLongBlock(pos);
        if(m_lb_count==3) {
          putBlasterShell(vector4(700, pos.y, 0.0f));
        }
        else if(m_lb_count==5) {
          putBlasterShell(vector4(700, pos.y+50, 0));
          putBlasterShell(vector4(700, pos.y-50, 0));
        }
        else if(m_lb_count==7) {
          for(int i=0; i<4; ++i) {
            putBlasterShell(vector4(700, 200-i*100, 0));
          }
        }
      }

      if(f%70==0 && f < 2400) {
        putMediumBlock(vector4(600+brand()*100, brand()*300, 0));
      }

      if(f==2700) {
        SendKillMessage(0, this);
      }
    }
  };

  class Scene3 : public Scene
  {
  typedef Scene Super;
  private:
    LargeCarrier *m_ship[8];
    int m_quick;

  public:
    Scene3(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_ship);
      s >> m_quick;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_ship);
      s << m_quick;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_ship);
    }

  public:
    Scene3() : m_quick(0)
    {
      ZeroClear(m_ship);
    }

    void progress(int f)
    {
      SweepDeadObject(m_ship);
      for(int i=0; i<8; ++i) {
        KillIfOutOfBox(m_ship[i], box(vector4(1500), vector4(-600, -1500, -1500)));
      }

      if(f==0) {
        SetCameraMovableArea(vector2(0,100), vector2(-0,-100));
      }

      if(f<=1200) {
        if(f==0) {
          m_ship[0] = putLargeCarrier(vector4(700, -50, 0));
        }
        else if(f==400) {
          m_ship[1] = putLargeCarrier(vector4(800, 260, 0));
        }
        else if(f==800) {
          m_ship[2] = putLargeCarrier(vector4(900, -170, 0));
        }
      }
      else if((m_quick&3)!=3) {
        if(!(m_quick&1) && ((!m_ship[0] && !m_ship[1]) || f==2400)) {
          m_ship[3] = putLargeCarrier(vector4(800, 160, 0));
          m_quick|=1;
        }
        if(!(m_quick&2) && ((!m_ship[1] && !m_ship[2]) || f==3500)) {
          m_ship[4] = putLargeCarrier(vector4(600, -160, 0));
          m_quick|=2;
        }
      }
      else if((m_quick&4)!=4) {
        if(!m_ship[3] && !m_ship[4]) {
          if(f<=3000) {
            m_ship[5] = putLargeCarrier(vector4(1100,-250, 0));
          }
          if(f<=3300) {
            m_ship[6] = putLargeCarrier(vector4( 800,   0, 0));
          }
          if(f<=3600) {
            m_ship[7] = putLargeCarrier(vector4( 500, 250, 0));
          }
          m_quick|=4;
        }
      }


      if(f%180==0 && f<3600) {
        putMediumBlock(vector4(1000+brand()*100, brand()*300, 0));
      }

      if(f==4800) {
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
    SceneBoss() : m_boss(0) {}
    void progress(int f)
    {
      if(f==0) {
        DestroyAllEnemy(getTSM());
        IMusic::FadeOut(2500);
        new Warning(L"\"constructor\"");
      }
      else if(f==200) {
        IMusic::WaitFadeOut();
        GetMusic("boss.ogg")->play();
        m_boss = CreateBoss1();
        SetBossMode(true);
        SetCameraMovableArea(vector2(50,100), vector2(-50,-100));
      }
      if(m_boss && m_boss->isDead()) {
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
      GetMusic("stage1.ogg")->play();
      SetCameraMovableArea(vector2(0,50), vector2(0,-50));
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
      else if(scene==4) { return new SceneBoss(); }
      else if(scene==5) { return new SceneEnd(); }
      return 0;
    }
  };


} // stage1
} // exception
#endif

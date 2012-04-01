#ifndef enemy_bossex_h
#define enemy_bossex_h

namespace exception {
namespace stageex {



  class Boss : public Optional
  {
  typedef Optional Super;
  public:

    class ICore : public Inherit3(HaveSpinAttrib2, HaveInvincibleMode, Enemy)
    {
    public:
      virtual float getOpacity()=0;
    };

    class CoreParts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    private:
      ICore *m_core;

    public:
      CoreParts(ICore *core) : m_core(core)
      {
        setParent(core);
        setLife(9999);
        enableCollision(false);
      }

      float getDeltaDamage()
      {
        if(m_core) {
          return m_core->getDeltaDamage();
        }
        return 0.0f;
      }

      void drawLifeGauge() {}

      void draw()
      {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,1,m_core->getOpacity()).v);
        glMaterialfv(GL_FRONT, GL_EMISSION, m_core->getEmission().v);
        Super::draw();
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.0f).v);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f,0.8f,0.8f,1.0f).v);
      }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        if(m_core->getOpacity()>0.7f) {
          enableCollision(true);
        }
      }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(IsFraction(p)) {
          SendDestroyMessage(0, p);
        }
      }

      int getFractionCount() { return 0; }
    };

    class Core : public ICore
    {
    typedef ICore Super;
    private:
      float m_opa;
      CoreParts *m_parts[12];

    public:
      Core() : m_opa(0.0f)
      {
        setLife(getMaxLife());
        setBox(box(vector4(40)));
        enableCollision(false);

        box b[12] = {
          box(vector4( 45, 70, 70), vector4(-45, 45, 45)),
          box(vector4( 45, 70,-70), vector4(-45, 45,-45)),
          box(vector4( 45,-70,-70), vector4(-45,-45,-45)),
          box(vector4( 45,-70, 70), vector4(-45,-45, 45)),

          box(vector4( 70, 45, 70), vector4( 45,-45, 45)),
          box(vector4( 70, 45,-70), vector4( 45,-45,-45)),
          box(vector4(-70, 45,-70), vector4(-45,-45,-45)),
          box(vector4(-70, 45, 70), vector4(-45,-45, 45)),

          box(vector4( 70, 70, 45), vector4( 45, 45,-45)),
          box(vector4( 70,-70, 45), vector4( 45,-45,-45)),
          box(vector4(-70,-70, 45), vector4(-45,-45,-45)),
          box(vector4(-70, 70, 45), vector4(-45, 45,-45)),
        };
        for(int i=0; i<12; ++i) {
          CoreParts *e = new CoreParts(this);
          e->setBox(b[i]);
          m_parts[i] = e;
        }
      }

      float getOpacity() { return m_opa; }

      static float getMaxLife()
      {
        float l[] = {4200, 4400, 5000, 5200, 5400};
        return l[GetLevel()];
      }

      float getDrawPriority() { return 1.0f; }
      void drawLifeGauge() {}

      void draw()
      {
        if(!isDamaged()) {
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.3f,0.3f,1.0f,m_opa).v);
          Super::draw();
          glMaterialfv(GL_FRONT, GL_DIFFUSE,  vector4(0.8f,0.8f,0.8f).v);
        }
        else {
          Super::draw();
        }
      }

      void updateEmission()
      {
        vector4 emission = getEmission();
        if(isInvincible()) {
          emission+=(vector4(1.0f, 1.0f, 1.0f)-emission)*0.03f;
        }
        else {
          emission+=(vector4(0.4f, 0.4f, 1.0f)-emission)*0.03f;
        }
        setEmission(emission);
      }

      void updateEmission(bool v)
      {}

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        m_opa+=(0.8f-m_opa)*0.005f;
        if(m_opa>0.7f) {
          enableCollision(true);
        }

        if(getLife()<=0) {
          float r = getRotateSpeed();
          r+=(0.0f-r)*0.005f;
          setRotateSpeed(r);
        }
        else {
          float r = getRotateSpeed();
          r+=(0.5f-r)*0.003f;
          setRotateSpeed(r);
        }
      }

      int getFractionCount() { return 0; }

      void onCollide(CollideMessage& m)
      {
        gobj_ptr p = m.getFrom();
        if(IsFraction(p)) {
          SendDestroyMessage(0, p);
        }
      }

      void onDestroy(DestroyMessage& m)
      {
        Super::onDestroy(m);
        PutBossDestroyImpact(getPosition());
      }

      void onLifeZero(gobj_ptr from)
      {
        PutSmallExplode(getBox(), getMatrix(), getExplodeCount()/2);
        PutBigImpact(getCenter());
      }
    };



  private:
    Layer *m_root_layer;
    Core *m_core;
    vector4 m_scroll;
    int m_frame;
    int m_time;

    void (Boss::*m_action)();

  public:
    Boss() : m_frame(0), m_time(60*240)
    {
      m_root_layer = new Layer();
      m_root_layer->setPosition(vector4(0,-140, 0));

      m_core = new Core();
      m_core->setParent(m_root_layer);
      m_core->setInvincible(false);

      m_action = &Boss::appear;
    }

    float getDrawPriority() { return 2.0f; }

    void draw()
    {
      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        char buf[16];
        sprintf(buf, "%d", m_time/60+(m_time%60!=0 ? 1 : 0));
        DrawText(sgui::_L(buf), sgui::Point(5,20));
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
      }

      vector2 center(25, 22);
      glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
      DrawRect(center, center+vector2(550, 5));

      static float4 s_col = float4(1,1,1,0.5f);
      s_col+=(float4(1,1,1,0.5f)-s_col)*0.05f;
      if(m_core->isDamaged())    { s_col = float4(1,0,0,0.9f); }
      if(m_core->isInvincible()) { s_col = float4(0.1f,0.1f,1.0f,0.9f); }
      glColor4fv(s_col.v);
      DrawRect(center+vector2(1,1), center+vector2(m_core->getLife()/m_core->getMaxLife()*550.0f, 5)-vector2(1,1));

      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }




    void appear()
    {
      int f = m_frame;

      if(f>180 && f<=360) {
        int af = f-180;
        float pq = Cos180I(1.0f/180*(af-1));
        float q = Cos180I(1.0f/180*af);
        m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0, 340, 0)*(q-pq));
      }
      if(f==360) {
        m_frame = 0;
        m_action = &Boss::action1;
        SetCameraMovableArea(vector2(50, 50), vector2(-50, -50));
      }
    }

    void action1()
    {
    }


    void clearEnemy()
    {
      gobj_iter& i = GetAllObjects();
      while(i.has_next()) {
        gobj_ptr p = i.iterate();
        if(enemy_ptr e = ToEnemy(p)) {
          if(e!=m_core && !dynamic_cast<CoreParts*>(e)) {
            SendDestroyMessage(0, e, 1);
          }
        }
        else if(ground_ptr g = ToGround(p)) {
          SendDestroyMessage(0, g, 1);
        }
      }
    }

    void destroy()
    {
      int f = m_frame;
      if(f==1) {
        InvincibleAllPlayers(600);
        clearEnemy();
        SetBossTime(m_time/60+(m_time%60!=0 ? 1 : 0));
        IMusic::FadeOut(6000);
        SetCameraMovableArea(vector2(0, 0), vector2(0,-0));
      }

      float pq = Cos180I(1.0f/480*(f-1));
      float q = Cos180I(1.0f/480*f);
      m_root_layer->setPosition(m_root_layer->getRelativePosition()+vector4(0,-340, 0)*(q-pq));
      m_core->setEmission(vector4(1.2f/480*f, 0.5f/480*f, 0));
      if(f%30==10) {
        Shine *s = new Shine(m_core);
      }

      if(f==480) {
        SendDestroyMessage(0, m_core);
        SendKillMessage(0, this);
      }
    }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;

      if(m_core->getLife()>0 && !m_core->isInvincible()) {
        --m_time;
        if(m_time==0) {
          SendDestroyMessage(0, this);
        }
      }
      if(m_core->getLife()<=0.0f && m_action!=&Boss::destroy) {
        m_frame = 0;
        m_action = &Boss::destroy;
      }

      (this->*m_action)();
    }

    void onDestroy(DestroyMessage& m)
    {
      // デフォルトの挙動握り潰し 
      if(m_core) {
        m_core->setLife(0);
      }
    }
  };

}
}
#endif

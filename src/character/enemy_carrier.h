#ifndef enemy_warship_h
#define enemy_warship_h

namespace exception {


  class LargeCarrier : public Inherit3(HaveInvincibleMode, HaveDirection, Enemy)
  {
  typedef Inherit3(HaveInvincibleMode, HaveDirection, Enemy) Super;
  private:
    static const int s_hatch_count = 6;
    static const int s_block_count = 5;

    LargeHatch *m_parts[s_hatch_count];
    BoxModel *m_blocks[s_block_count];
    int m_frame;

  public:
    LargeCarrier(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_blocks);
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_blocks);
      s << m_frame;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_blocks);
    }

  public:
    LargeCarrier(controler_ptr c) : m_frame(0)
    {
      setControler(c);
      setBox(box(vector4(40, 50, 30), vector4(-40, -50, -30))+vector4(205, 0, 0));
      setLife(70.0f);
      setEnergy(100.0f);
      setBound(box(vector4(1500)));

      {
        const vector4 pos[] = {
          vector4( 105, 5, 0),
          vector4(   0, 5, 0),
          vector4(-105, 5, 0),
          vector4( 105,-5, 0),
          vector4(   0,-5, 0),
          vector4(-105,-5, 0),
        };
        for(int i=0; i<s_hatch_count; ++i) {
          LargeHatch *p = new LargeHatch();
          p->setParent(this);
          p->setPosition(pos[i]);
          p->setDirection(i<3 ? vector4(0,1,0) : vector4(0,-1,0));
          m_parts[i] = p;
        }
      }

      {
        box b[] = {
          box(vector4( 205, 40, -15), vector4(-155, -40, -30)),
          box(vector4(-180, 80, -10), vector4(-140, -80, -40)),
          box(vector4( 170,120, -10), vector4( 130,-120, -30)),
          box(vector4(  70,120, -10), vector4(  30,-120, -30)),
          box(vector4( -70,120, -10), vector4( -30,-120, -30)),
        };
        for(int i=0; i<s_block_count; ++i) {
          BoxModel *p = new BoxModel(this);
          p->setBox(b[i]);
          m_blocks[i] = p;
        }
      }
      setGroup(createGroupID());
    }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
    }


    int getPartsCount() { return s_hatch_count; }
    LargeHatch* getParts(int i) { return m_parts[i]; }

    void drawModel()
    {
      Super::drawModel();

      if(isInvincible()) { // 子パーツは無敵状態でも発光させない 
        glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);
      }
      for(int i=0; i<s_block_count; ++i) {
        m_blocks[i]->draw();
      }
    }

    void draw()
    {
      Super::draw();

      const vector4 bpos[2] = {
        vector4(-195,  60, -25),
        vector4(-195, -60, -25)
      };
      for(int i=0; i<2; ++i) {
        DrawSprite("burner.png",
          getMatrix()*bpos[i],
          vector4(60.0f+::sinf(43.0f*m_frame*ist::radian)*5.0f));
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      ++m_frame;
      SweepDeadObject(m_parts);
      if(isInvincible() && !AliveAny(m_parts)) {
        setInvincible(false);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      for(int i=0; i<s_block_count; ++i) {
        PutSmallExplode(m_blocks[i]->getBox(), getMatrix()*m_blocks[i]->getMatrix(), 20);
      }
      if(m.getStat()==0) {
        PutBigImpact(getCenter());
      }
    }
  };


  class LargeCarrier_Controler : public TControler<LargeCarrier>
  {
  typedef TControler<LargeCarrier> Super;
  public:
    LargeCarrier_Controler(Deserializer& s) : Super(s) {}
    LargeCarrier_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getDirection, const vector4&);

    Getter(getPartsCount, int);
    Getter2(getParts, LargeHatch*, int);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setDirection, const vector4&);
  };



  // 1面ラスト用 
  class LargeCarrier_GenFighter : public LargeCarrier_Controler
  {
  typedef LargeCarrier_Controler Super;
  private:
    vector4 m_initial_pos;
    vector4 m_target_pos;
    int m_frame;

  public:
    LargeCarrier_GenFighter(Deserializer& s) : Super(s)
    {
      s >> m_initial_pos >> m_target_pos >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_initial_pos << m_target_pos << m_frame;
    }

  public:
    LargeCarrier_GenFighter() : m_frame(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      const int wait[] = {
        0, 50, 100,
        0, 50, 100,
      };
      for(int i=0; i<getPartsCount(); ++i) {
        Hatch_GenRushFighter *c = new Hatch_GenRushFighter();
        c->setWait(wait[i]);
        getParts(i)->setControler(c);
      }

      m_initial_pos = getRelativePosition();
      m_target_pos = m_initial_pos+getDirection()*500.0f;
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f < 300) {
        setPosition(m_initial_pos+(m_target_pos-m_initial_pos)*Sin90(1.0f/300.0f*m_frame));
      }
      else {
        vector4 pos = getRelativePosition();
        pos+=vector4(-0.30f, 0, 0);
        setPosition(pos);
      }
    }
  };

  // 4面ラスト用 
  class LargeCarrier_GenMissileShell : public LargeCarrier_Controler
  {
  typedef LargeCarrier_Controler Super;
  private:
    vector4 m_move;
    int m_frame;
    bool m_ih;

  public:
    LargeCarrier_GenMissileShell(Deserializer& s) : Super(s)
    {
      s >> m_move >> m_frame >> m_ih;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_move << m_frame << m_ih;
    }

  public:
    LargeCarrier_GenMissileShell(bool ih) : m_frame(0), m_ih(ih)
    {}

    void onConstruct(ConstructMessage& m)
    {
      const int wait[] = {
        0, 80, 160,
        0, 80, 160,
      };
      for(int i=0; i<getPartsCount(); ++i) {
        Hatch_GenMissileShell *c = new Hatch_GenMissileShell();
        c->setWait(wait[i]);
        getParts(i)->setControler(c);
      }

      setDirection(vector4(0,1,0));
      m_move = vector4(450*(m_ih?-1:1), 0, 0);
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f<=300) {
        vector4 pos = getRelativePosition();
        float pq = Sin90(1.0f/300*(f-1));
        float q = Sin90(1.0f/300*f);
        setPosition(pos+m_move*(q-pq));
      }
      setPosition(getRelativePosition()+getParentIMatrix()*(GetGlobalScroll()*0.5f));
    }
  };
}
#endif

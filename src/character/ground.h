#ifndef ground_h
#define ground_h

namespace exception {

  typedef Inherit8(HaveDirection, HaveBoxBound, HaveParent, HaveBoxCollision, Box, HaveControler, HavePosition, IGround) GroundBase;
  class BasicGround : public GroundBase
  {
  typedef GroundBase Super;
  public:
    BasicGround()
    {
      Register(this);
      setBound(box(vector4(2000)));
    }

    BasicGround(Deserializer& s) : Super(s)
    {}

    float getLife() { return 1000.0f; }

    virtual void setMaterial()
    {
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.0f, 0.0f, 0.3f).v);
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.5f).v);
    }

    void draw()
    {
      setMaterial();
      Super::draw();
      glMaterialfv(GL_FRONT, GL_DIFFUSE, vector4(0.8f).v);
      glMaterialfv(GL_FRONT, GL_AMBIENT, vector4(0.2f).v);
    }

    void update()
    {
      Super::update();
      updateCollisionMatrix();
    }

    void checkBound()
    {
      if(!getBound().isInner(getCenter())) {
        SendKillMessage(0, this);
      }
    }

    virtual int getFractionCount()
    {
      return int(getVolume()/powf(45.0f, 3.0f)*GetFractionRate());
    }

    virtual int getExplodeCount()
    {
      return int(getVolume()/25000.0f);
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        SplinkleCube(getBox(), getMatrix(), getFractionCount());
      }
      PutSmallExplode(getBox(), getMatrix(), getExplodeCount());
    }
  };

  class ChildGround : public BasicGround
  {
  typedef BasicGround Super;
  public:
    ChildGround() {}
    ChildGround(Deserializer& s) : Super(s) {}

    void checkBound()
    {
      if(!GetParentSolid(this)) {
        Super::checkBound();
      }
    }

    void setParent(gobj_ptr p)
    {
      Super::setParent(p);
      if(solid_ptr s=ToSolid(p)) {
        setGroup(s->getGroup());
      }
    }
  };


  class Ground : public BasicGround
  {
  typedef BasicGround Super;
  public:
    Ground()
    {
      setParent(GetGlobals());
    }

    Ground(Deserializer& s) : Super(s)
    {}

    void setParent(gobj_ptr p)
    {
      Super::setParent(GetParentLayer(p));
    }

    virtual void scroll()
    {
      vector4 pos = getRelativePosition();
      pos+=getParentIMatrix()*GetGlobalScroll();
      setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      scroll();
    }
  };


  class MoveGround : public Inherit2(HaveVelocity, Ground)
  {
  typedef Inherit2(HaveVelocity, Ground) Super;
  public:
    MoveGround() {}
    MoveGround(Deserializer& s) : Super(s) {}

    void scroll()
    {
      Super::scroll();
      setPosition(getRelativePosition()+getVel());
    }
  };

}
#endif

#include "stdafx.h"
#include "creater.h"
#include "enemy_util.h"
#include "draw.h"
#include "enemy.h"
#include "effect.h"
#include "bullet.h"
#include "ground.h"
#include "player.h"

#include "enemy_fraction.h"
#include "enemy_missile.h"
#include "enemy_bolt.h"
#include "enemy_zab.h"
#include "enemy_block.h"
#include "enemy_fighter.h"
#include "enemy_egg.h"
#include "enemy_shell.h"
#include "enemy_turret.h"
#include "enemy_hatch.h"
#include "enemy_carrier.h"
#include "enemy_gear.h"
#include "enemy_heavyfighter.h"
#include "enemy_armorship.h"
#include "enemy_weakiterator.h"
#include "enemy_defenser.h"

#include "stage_common.h"
#include "stage_title.h"
#include "stage1.h"
#include "stage3.h"
#ifndef EXCEPTION_TRIAL
  #include "stage2.h"
  #include "stage4.h"
  #include "stage5.h"
  #include "stagex.h"
#endif

#include "boss1.h"
#include "boss3.h"
#ifndef EXCEPTION_TRIAL
  #include "boss2.h"
  #include "boss4.h"
  #include "boss5.h"
  #include "bossex.h"
#endif

namespace exception {


  class Globals : public LayerBase
  {
  typedef LayerBase Super;
  private:
    static Globals *s_inst;
    vector4 m_accel;
    vector4 m_scroll;
    matrix44 m_mat;
    matrix44 m_imat;
    box m_bound_box;
    rect m_bound_rect;

  public:
    static Globals* instance()
    {
      if(!s_inst) {
        s_inst = new Globals();
      }
      return s_inst;
    }

    Globals()
    {
      chain(); // ŸŽè‚ÉŠJ•ú‚³‚ê‚È‚¢‚æ‚¤‚É 
      resetGlobals();
    }

    Globals(Deserializer& s) : Super(s)
    {
      s_inst = this;
      s >> m_accel >> m_scroll >> m_mat >> m_imat >> m_bound_box >> m_bound_rect;
    }

    ~Globals()
    {
      s_inst = 0;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_accel << m_scroll << m_mat << m_imat << m_bound_box << m_bound_rect;
    }

    const vector4& getPosition()
    {
      static vector4 pos;
      pos = getGlobalMatrix()*vector4();
      return pos;
    }

    void resetGlobals()
    {
      m_accel = vector4(-0.01f, 0, 0, 0);
      m_scroll = vector4(-0.5f, 0, 0, 0);
      m_mat = matrix44();
      m_imat = matrix44();
      m_bound_box = box(vector4(1500));
      m_bound_rect = rect(vector2(300));
    }

    void setGlobalScroll(const vector4& v) { m_scroll=v; m_scroll.w=0; }
    void setGlobalAccel(const vector4& v)  { m_accel=v; m_accel.w=0; }
    void setGlobalMatrix(const matrix44& v){ m_mat=v; m_imat=matrix44(v).invert(); }
    void setGlobalBoundBox(const box& v)   { m_bound_box=v; }
    void setGlobalBoundRect(const rect& v) { m_bound_rect=v; }

    const vector4& getGlobalScroll()  { return m_scroll; }
    const vector4& getGlobalAccel()   { return m_accel; }
    const matrix44& getGlobalMatrix() { return m_mat; }
    const matrix44& getGlobalIMatrix(){ return m_imat; }
    const box& getGlobalBoundBox()    { return m_bound_box; }
    const rect& getGlobalBoundRect()  { return m_bound_rect; }

    void onDestroy(DestroyMessage& m)
    {
      // DestroyMessageˆ¬‚è‚Â‚Ô‚µ 
    }
  };
  Globals* Globals::s_inst = 0;


  gobj_ptr GetGlobals() { return Globals::instance(); }

  void ResetGlobals() { Globals::instance()->resetGlobals(); }
  void SetGlobalScroll(const vector4& v) { Globals::instance()->setGlobalScroll(v); }
  void SetGlobalAccel(const vector4& v)  { Globals::instance()->setGlobalAccel(v); }
  void SetGlobalMatrix(const matrix44& v){ Globals::instance()->setGlobalMatrix(v); }
  void SetGlobalBoundBox(const box& v)   { Globals::instance()->setGlobalBoundBox(v); }
  void SetGlobalBoundRect(const rect& v) { Globals::instance()->setGlobalBoundRect(v); }

  const vector4& GetGlobalScroll()  { return Globals::instance()->getGlobalScroll(); }
  const vector4& GetGlobalAccel()   { return Globals::instance()->getGlobalAccel(); }
  const matrix44& GetGlobalMatrix() { return Globals::instance()->getGlobalMatrix(); }
  const matrix44& GetGlobalIMatrix(){ return Globals::instance()->getGlobalIMatrix(); }
  const box& GetGlobalBoundBox()    { return Globals::instance()->getGlobalBoundBox(); }
  const rect& GetGlobalBoundRect()  { return Globals::instance()->getGlobalBoundRect(); }



  BlueBlur* BlueBlur::s_inst;


  namespace impact {
    std::map<gid, float> g_opacity;
  }




  player_ptr CreatePlayer(int cid, const string& name, IInput *input)
  {
    BlueBlur::instance();
    return new Player(cid, name, new Player_Controler(input));
  }


  fraction_ptr CreateFraction()
  {
    return Fraction::Factory::create();
  }
}



namespace exception {

  obj_ptr CreateTitleBackground()   { return new title::Background(); }

  gobj_ptr CreateStage1() { return new stage1::Stage(); }
  gobj_ptr CreateStage3() { return new stage3::Stage(); }

  gobj_ptr CreateBoss1() { return new stage1::Boss(); }
  gobj_ptr CreateBoss3() { return new stage3::Boss(); }


#ifndef EXCEPTION_TRIAL
  gobj_ptr CreateStage2() { return new stage2::Stage(); }
  gobj_ptr CreateStage4() { return new stage4::Stage(); }
  gobj_ptr CreateStage5() { return new stage5::Stage(); }
  gobj_ptr CreateStageEx(){ return new stagex::Stage(); }

  gobj_ptr CreateBoss2() { return new stage2::Boss(); }
  gobj_ptr CreateBoss4() { return new stage4::Boss(); }
  gobj_ptr CreateBoss5() { return new stage5::Boss(); }
  gobj_ptr CreateBossEx(){ return new stageex::Boss(); }
#endif




namespace impl {
  template<class T>
  inline bool deserialize_object(const std::string& name, Deserializer& s, gobj_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = new T(s);
      return true;
    }
    return false;
  }

  template<class T>
  inline bool deserialize_object_cached(const std::string& name, Deserializer& s, gobj_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = T::Factory::create(s);
      return true;
    }
    return false;
  }

  template<class T>
  inline bool deserialize_controler(const std::string& name, Deserializer& s, controler_ptr& r)
  {
    if(name==typeid(T).name()) {
      r = new T(s);
      return true;
    }
    return false;
  }

  inline bool throw_exception(const string& message) {
    throw Error(message);
    return false;
  }
}

  void SerializeObject(Serializer& s, const gobj_ptr p)
  {
    s << string(typeid(*p).name());
    p->serialize(s);
  }

  gobj_ptr DeserializeObject(Deserializer& s)
  {
    gobj_ptr r = 0;
    string name;
    s >> name;

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("%s ", name.c_str());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    impl::deserialize_object_cached<Fraction>(name, s, r) ||

    impl::deserialize_object<Player>(name, s, r) ||

    impl::deserialize_object<Globals>(name, s, r)       ||
    impl::deserialize_object<LayerBase>(name, s, r)     ||
    impl::deserialize_object<ChildLayer>(name, s, r)    ||
    impl::deserialize_object<Layer>(name, s, r)         ||
    impl::deserialize_object<RotLayer>(name, s, r)      ||
    impl::deserialize_object<ChildRotLayer>(name, s, r) ||
    impl::deserialize_object<ScrolledLayer>(name, s, r) ||

    impl::deserialize_object<BoxModel>(name, s, r) ||

    impl::deserialize_object<MediumBlock>(name, s, r)    ||
    impl::deserialize_object<LargeBlock>(name, s, r)     ||
    impl::deserialize_object<CrashGround>(name, s, r)    ||
    impl::deserialize_object<CrashGround>(name, s, r)    ||
    impl::deserialize_object<BigCrashGround>(name, s, r) ||
    impl::deserialize_object<DynamicBlock>(name, s, r)   ||
    impl::deserialize_object<PillerBlock>(name, s, r)    ||
    impl::deserialize_object<StaticGround>(name, s, r)   ||
    impl::deserialize_object<DynamicGround>(name, s, r)  ||
    impl::deserialize_object<CoverGround>(name, s, r)    ||

    impl::deserialize_object<Fighter>(name, s, r)        ||
    impl::deserialize_object<Fighter::Parts>(name, s, r) ||

    impl::deserialize_object<Shell>(name, s, r)             ||
    impl::deserialize_object<Shell::Parts>(name, s, r)      ||
    impl::deserialize_object<Shell::DummyParts>(name, s, r) ||

    impl::deserialize_object<LargeCarrier>(name, s, r) ||

    impl::deserialize_object<LargeHatch>(name, s, r) ||
    impl::deserialize_object<SmallHatch>(name, s, r) ||

    impl::deserialize_object<HeavyFighter>(name, s, r)        ||
    impl::deserialize_object<HeavyFighter::Parts>(name, s, r) ||
    impl::deserialize_object<HeavyFighter::Arm>(name, s, r)   ||

    impl::deserialize_object<ArmorShip::Core>(name, s, r)  ||
    impl::deserialize_object<ArmorShip::Parts>(name, s, r) ||
    impl::deserialize_object<ArmorShip::Fort>(name, s, r)  ||
    impl::deserialize_object<Turtle>(name, s, r)           ||

    impl::deserialize_object<Bolt>(name, s, r) ||

    impl::deserialize_object<Egg>(name, s, r)        ||
    impl::deserialize_object<Egg::Parts>(name, s, r) ||

    impl::deserialize_object<Zab>(name, s, r) ||
    impl::deserialize_object<Zab::Parts>(name, s, r) ||

    impl::deserialize_object<LaserTurret>(name, s, r)        ||
    impl::deserialize_object<LaserTurret::Parts>(name, s, r) ||

    impl::deserialize_object<FloatingTurret>(name, s, r)            ||
    impl::deserialize_object<FloatingTurret::HeadParts>(name, s, r) ||

    impl::deserialize_object<WeakIterator>(name, s, r)        ||
    impl::deserialize_object<WeakIterator::Guard>(name, s, r) ||
    impl::deserialize_object<WeakIterator_Defense::PararelLaser>(name, s, r) ||
    impl::deserialize_object<WeakIterator_Sliding::PararelLaser>(name, s, r) ||

    impl::deserialize_object<GravityMine>(name, s, r)    ||
    impl::deserialize_object<BurstMine>(name, s, r)      ||
    impl::deserialize_object<MiniBurstMine>(name, s, r)  ||
    impl::deserialize_object<GravityMissile>(name, s, r) ||
    impl::deserialize_object<BurstMissile>(name, s, r)   ||

    impl::deserialize_object<Gravity>(name, s, r)   ||
    impl::deserialize_object<MineBurst>(name, s, r) ||

    impl::deserialize_object<ChildGround>(name, s, r) ||
    impl::deserialize_object<Ground>(name, s, r)      ||
    impl::deserialize_object<MoveGround>(name, s, r)  ||

    impl::deserialize_object<GearParts>(name, s, r) ||
    impl::deserialize_object<SmallGear>(name, s, r) ||
    impl::deserialize_object<LargeGear>(name, s, r) ||
    impl::deserialize_object<LaserAmp>(name, s, r)  ||
    impl::deserialize_object<LaserGear>(name, s, r) ||

    impl::deserialize_object<BlueBlur>(name, s, r)  ||
    impl::deserialize_object<Ray>(name, s, r)       ||
    impl::deserialize_object<GLaser>(name, s, r)    ||
    impl::deserialize_object<Laser>(name, s, r)     ||
    impl::deserialize_object<LaserBit>(name, s, r)  ||
    impl::deserialize_object<RedRay>(name, s, r)    ||
    impl::deserialize_object<RedRayBit>(name, s, r) ||
    impl::deserialize_object<Blaster>(name, s, r)   ||

    impl::deserialize_object<Fraction::Drawer>(name, s, r)     ||
    impl::deserialize_object<LockOnMarker::Drawer>(name, s, r) ||
    impl::deserialize_object<Flash::Drawer>(name, s, r)        ||
    impl::deserialize_object<RedRing::Drawer>(name, s, r)      ||
    impl::deserialize_object<BlueRing::Drawer>(name, s, r)     ||
    impl::deserialize_object<Explode::Drawer>(name, s, r)      ||
    impl::deserialize_object<CubeExplode::Drawer>(name, s, r)  ||
    impl::deserialize_object<BlueParticle::Drawer>(name, s, r) ||

    impl::deserialize_object<DirectionalImpact>(name, s, r) ||
    impl::deserialize_object<SmallImpact>(name, s, r)       ||
    impl::deserialize_object<MediumImpact>(name, s, r)      ||
    impl::deserialize_object<BigImpact>(name, s, r)         ||
    impl::deserialize_object<BossDestroyImpact>(name, s, r) ||
    impl::deserialize_object<Shine>(name, s, r)       ||
    impl::deserialize_object<DamageFlash>(name, s, r) ||
    impl::deserialize_object<Bloom>(name, s, r)       ||
    impl::deserialize_object<GameOver>(name, s, r)    ||
    impl::deserialize_object<StageResult>(name, s, r) ||
    impl::deserialize_object<Warning>(name, s, r)     ||

    impl::deserialize_object<stage1::LongBlock>(name, s, r)  ||
    impl::deserialize_object<stage1::Background>(name, s, r) ||
    impl::deserialize_object<stage1::Scene1>(name, s, r)     ||
    impl::deserialize_object<stage1::Scene2>(name, s, r)     ||
    impl::deserialize_object<stage1::Scene3>(name, s, r)     ||
    impl::deserialize_object<stage1::SceneBoss>(name, s, r)  ||
    impl::deserialize_object<stage1::SceneEnd>(name, s, r)   ||
    impl::deserialize_object<stage1::Stage>(name, s, r)      ||
    impl::deserialize_object<stage1::Boss>(name, s, r)                     ||
    impl::deserialize_object<stage1::Boss::Generator>(name, s, r)          ||
    impl::deserialize_object<stage1::Boss::GeneratorCollision>(name, s, r) ||
    impl::deserialize_object<stage1::Boss::Wall>(name, s, r)               ||
    impl::deserialize_object<stage1::Boss::WallGuard>(name, s, r)          ||
    impl::deserialize_object<stage1::Boss::Blade>(name, s, r)              ||
    impl::deserialize_object<stage1::Boss::BladeGenerator>(name, s, r)     ||
    impl::deserialize_object<stage1::Boss::Arm>(name, s, r)                ||
    impl::deserialize_object<stage1::Boss::ArmLayer>(name, s, r)           ||
    impl::deserialize_object<stage1::Boss::Core>(name, s, r)               ||
    impl::deserialize_object<stage1::Boss::BoundLaser>(name, s, r)         ||

    impl::deserialize_object<stage3::Background>(name, s, r) ||
    impl::deserialize_object<stage3::Scene1>(name, s, r)     ||
    impl::deserialize_object<stage3::Scene2>(name, s, r)     ||
    impl::deserialize_object<stage3::Scene3>(name, s, r)     ||
    impl::deserialize_object<stage3::Scene4>(name, s, r)     ||
    impl::deserialize_object<stage3::SceneBoss>(name, s, r)  ||
    impl::deserialize_object<stage3::SceneEnd>(name, s, r)   ||
    impl::deserialize_object<stage3::Stage>(name, s, r)      ||
    impl::deserialize_object<stage3::CellBlock>(name, s, r)            ||
    impl::deserialize_object<stage3::CellGround>(name, s, r)           ||
    impl::deserialize_object<stage3::Boss>(name, s, r)                 ||
    impl::deserialize_object<stage3::Boss::Guard>(name, s, r)          ||
    impl::deserialize_object<stage3::Boss::GuardLayer>(name, s, r)     ||
    impl::deserialize_object<stage3::Boss::GuardLayer2>(name, s, r)    ||
    impl::deserialize_object<stage3::Boss::Core>(name, s, r)           ||
    impl::deserialize_object<stage3::Boss::PararelLaser1>(name, s, r)  ||
    impl::deserialize_object<stage3::Boss::PararelLaser2>(name, s, r)  ||
    impl::deserialize_object<stage3::Boss::PararelLaser3>(name, s, r)  ||
    impl::deserialize_object<stage3::Boss::PararelLaser4>(name, s, r)  ||
    impl::deserialize_object<stage3::Boss::PararelLaser11>(name, s, r) ||
    impl::deserialize_object<stage3::Boss::PararelLaser12>(name, s, r) ||
    impl::deserialize_object<stage3::Boss::PararelLaser13>(name, s, r) ||

#ifndef EXCEPTION_TRIAL
    impl::deserialize_object<stage2::Background>(name, s, r)        ||
    impl::deserialize_object<stage2::LightGroundDrawer>(name, s, r) ||
    impl::deserialize_object<stage2::LGround>(name, s, r)           ||
    impl::deserialize_object<stage2::MiniFallGround>(name, s, r)    ||
    impl::deserialize_object<stage2::MiniFallBlock>(name, s, r)     ||
    impl::deserialize_object<stage2::FallGround>(name, s, r)        ||
    impl::deserialize_object<stage2::FallBlock>(name, s, r)         ||
    impl::deserialize_object<stage2::Scene>(name, s, r)             ||
    impl::deserialize_object<stage2::Scroller>(name, s, r)          ||
    impl::deserialize_object<stage2::Scene1>(name, s, r)            ||
    impl::deserialize_object<stage2::Scene2>(name, s, r)            ||
    impl::deserialize_object<stage2::Scene2::Action1>(name, s, r)   ||
    impl::deserialize_object<stage2::Scene2::Action2>(name, s, r)   ||
    impl::deserialize_object<stage2::SceneBoss>(name, s, r)         ||
    impl::deserialize_object<stage2::SceneEnd>(name, s, r)          ||
    impl::deserialize_object<stage2::Stage>(name, s, r)             ||
    impl::deserialize_object<stage2::VBoundBlock>(name, s, r)     ||
    impl::deserialize_object<stage2::Burst>(name, s, r)           ||
    impl::deserialize_object<stage2::Burst2>(name, s, r)          ||
    impl::deserialize_object<stage2::BoundBurstMine>(name, s, r)  ||
    impl::deserialize_object<stage2::Boss>(name, s, r)            ||
    impl::deserialize_object<stage2::Boss::CoreParts>(name, s, r) ||
    impl::deserialize_object<stage2::Boss::Core>(name, s, r)      ||

    impl::deserialize_object<stage4::Elevator>(name, s, r)        ||
    impl::deserialize_object<stage4::Elevator::Parts>(name, s, r) ||
    impl::deserialize_object<stage4::VElevator>(name, s, r)       ||
    impl::deserialize_object<stage4::VHElevator>(name, s, r)      ||
    impl::deserialize_object<stage4::DestroyLinkage>(name, s, r)  ||
    impl::deserialize_object<stage4::Background>(name, s, r)              ||
    impl::deserialize_object<stage4::Scene>(name, s, r)                   ||
    impl::deserialize_object<stage4::SceneBoss>(name, s, r)               ||
    impl::deserialize_object<stage4::SceneEnd>(name, s, r)                ||
    impl::deserialize_object<stage4::Scene1>(name, s, r)                  ||
    impl::deserialize_object<stage4::Scene1::Act1>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene1::Act2>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene1::Act3>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene1::Linkage1>(name, s, r)        ||
    impl::deserialize_object<stage4::Scene2>(name, s, r)                  ||
    impl::deserialize_object<stage4::Scene2::Act1>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene2::Act2>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene2::Linkage1>(name, s, r)        ||
    impl::deserialize_object<stage4::Scene2::Linkage2>(name, s, r)        ||
    impl::deserialize_object<stage4::Scene3>(name, s, r)                  ||
    impl::deserialize_object<stage4::Scene3::GenZab1>(name, s, r)         ||
    impl::deserialize_object<stage4::Scene3::GenZab2>(name, s, r)         ||
    impl::deserialize_object<stage4::Scene3::GenZab3>(name, s, r)         ||
    impl::deserialize_object<stage4::Scene3::GenZab4>(name, s, r)         ||
    impl::deserialize_object<stage4::Scene3::Act3>(name, s, r)            ||
    impl::deserialize_object<stage4::Scene3::GenEgg>(name, s, r)          ||
    impl::deserialize_object<stage4::Scene3::GenHeavyFighter>(name, s, r) ||
    impl::deserialize_object<stage4::Scene4>(name, s, r)                  ||
    impl::deserialize_object<stage4::Stage>(name, s, r)                   ||
    impl::deserialize_object<stage4::ArmBase::Parts>(name, s, r)            ||
    impl::deserialize_object<stage4::ArmBase::Core>(name, s, r)             ||
    impl::deserialize_object<stage4::ArmBase::ChildCore>(name, s, r)        ||
    impl::deserialize_object<stage4::ArmBase::Shutter>(name, s, r)          ||
    impl::deserialize_object<stage4::ArmBase::ShutterGenerator>(name, s, r) ||
    impl::deserialize_object<stage4::Arm1>(name, s, r)                      ||
    impl::deserialize_object<stage4::Arm2>(name, s, r)                      ||
    impl::deserialize_object<stage4::Arm2::ChildArm>(name, s, r)            ||
    impl::deserialize_object<stage4::Boss>(name, s, r)                      ||
    impl::deserialize_object<stage4::Boss::Parts>(name, s, r)               ||
    impl::deserialize_object<stage4::Boss::Core>(name, s, r)                ||

    impl::deserialize_object<stage5::Background>(name, s, r) ||
    impl::deserialize_object<stage5::Scene1>(name, s, r)     ||
    impl::deserialize_object<stage5::SceneBoss>(name, s, r)  ||
    impl::deserialize_object<stage5::SceneEnd>(name, s, r)   ||
    impl::deserialize_object<stage5::Stage>(name, s, r)      ||
    impl::deserialize_object<stage5::Pendulum>(name, s, r)        ||
    impl::deserialize_object<stage5::LaserBar>(name, s, r)        ||
    impl::deserialize_object<stage5::Boss>(name, s, r)            ||
    impl::deserialize_object<stage5::Boss::Core>(name, s, r)      ||
    impl::deserialize_object<stage5::Boss::CoreParts>(name, s, r) ||
#endif

    impl::throw_exception("DeserializeObject() : "+name);

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("(%d)\n", r->getID());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    return r;
  }

  void SerializeControler(Serializer& s, const controler_ptr p)
  {
    if(p) {
      s << string(typeid(*p).name());
      p->serialize(s);
    }
    else {
      s << string("null");
    }
  }

  controler_ptr DeserializeControler(Deserializer& s)
  {
    controler_ptr r = 0;
    string name;
    s >> name;

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    printf("& %s ", name.c_str());
    fflush(stdout);
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

    if(name=="null") { return 0; }

    impl::deserialize_controler<Player_Controler>(name, s, r) ||

    impl::deserialize_controler<Fighter_Rush>(name, s, r)           ||
    impl::deserialize_controler<Fighter_Straight>(name, s, r)       ||
    impl::deserialize_controler<Fighter_Straight2>(name, s, r)      ||
    impl::deserialize_controler<stage1::Fighter_Cross>(name, s, r)  ||
    impl::deserialize_controler<stage1::Fighter_Turn>(name, s, r)   ||
    impl::deserialize_controler<stage2::Fighter_Turn>(name, s, r)   ||
    impl::deserialize_controler<stage3::Fighter_Turns1>(name, s, r) ||
    impl::deserialize_controler<stage4::Fighter_Turns1>(name, s, r) ||

    impl::deserialize_controler<Shell_Blaster>(name, s, r)        ||
    impl::deserialize_controler<Shell_BurstMissile>(name, s, r)   ||
    impl::deserialize_controler<Shell_GravityMissile>(name, s, r) ||

    impl::deserialize_controler<LargeCarrier_GenFighter>(name, s, r)      ||
    impl::deserialize_controler<LargeCarrier_GenMissileShell>(name, s, r) ||

    impl::deserialize_controler<Hatch_GenRushFighter>(name, s, r)       ||
    impl::deserialize_controler<Hatch_GenMissileShell>(name, s, r)      ||
    impl::deserialize_controler<Hatch_Bolt>(name, s, r)                 ||
    impl::deserialize_controler<Hatch_Bolt2>(name, s, r)                ||
    impl::deserialize_controler<Hatch_MiniMine>(name, s, r)             ||
    impl::deserialize_controler<Hatch_Laser>(name, s, r)                ||
    impl::deserialize_controler<stage3::Hatch_GenFighterT1>(name, s, r) ||
    impl::deserialize_controler<stage4::Hatch_GenFighterT1>(name, s, r) ||

    impl::deserialize_controler<HeavyFighter_Missiles>(name, s, r)         ||
    impl::deserialize_controler<HeavyFighter_Missiles2>(name, s, r)        ||
    impl::deserialize_controler<HeavyFighter_PutMines>(name, s, r)         ||
    impl::deserialize_controler<stage2::HeavyFighter_Run>(name, s, r)      ||
    impl::deserialize_controler<stage3::HeavyFighter_Straight>(name, s, r) ||
    impl::deserialize_controler<stage3::HeavyFighter_Turns1>(name, s, r)   ||
    impl::deserialize_controler<stage3::HeavyFighter_Turns2>(name, s, r)   ||
    impl::deserialize_controler<stage3::HeavyFighter_Turns3>(name, s, r)   ||
    impl::deserialize_controler<stage3::HeavyFighter_Turns4>(name, s, r)   ||
    impl::deserialize_controler<stage4::HeavyFighter_Laser>(name, s, r)    ||
    impl::deserialize_controler<stage4::HeavyFighter_Turns1>(name, s, r)   ||

    impl::deserialize_controler<Turtle_Wait>(name, s, r)    ||
    impl::deserialize_controler<Turtle_Sliding>(name, s, r) ||

    impl::deserialize_controler<Bolt_Straight>(name, s, r) ||
    impl::deserialize_controler<Bolt_Rush>(name, s, r)     ||

    impl::deserialize_controler<Egg_Mine>(name, s, r)    ||
    impl::deserialize_controler<Egg_Missile>(name, s, r) ||
    impl::deserialize_controler<Egg_Laser>(name, s, r)   ||

    impl::deserialize_controler<Zab_Rush>(name, s, r)     ||
    impl::deserialize_controler<Zab_Straight>(name, s, r) ||

    impl::deserialize_controler<LaserTurret_Normal>(name, s, r) ||

    impl::deserialize_controler<FloatingTurret_Fall>(name, s, r) ||
    impl::deserialize_controler<FloatingTurret_Wait>(name, s, r) ||

    impl::deserialize_controler<WeakIterator_Defense>(name, s, r)          ||
    impl::deserialize_controler<WeakIterator_Defense::Guard_C>(name, s, r) ||
    impl::deserialize_controler<WeakIterator_Sliding>(name, s, r)          ||
    impl::deserialize_controler<WeakIterator_Sliding2>(name, s, r)         ||

    impl::throw_exception(string("DeserializeControler() : ")+name);

    return r;
  }
}

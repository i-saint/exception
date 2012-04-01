#ifndef interface_h
#define interface_h

#pragma warning(disable : 4996)
#pragma warning(disable : 4018) // 符号有り/無し数値比較 
#pragma warning(disable : 4244) // float←→int数値変換 
#pragma warning(disable : 4267) // 符号有り/無し数値変換 

#include "version.h"

#ifdef EXCEPTION_CHECK_LEAK
  #include <stdlib.h>
  #include <crtdbg.h>
#endif // EXCEPTION_CHECK_LEAK 

#if defined(EXCEPTION_ENABLE_NETRANKING) || defined(EXCEPTION_ENABLE_NETUPDATE) || defined(EXCEPTION_ENABLE_NETPLAY)
  #include <ist/ist_net.h>
#endif

#define NO_SDL_GLEXT

#include <string>
#include <deque>
#include <queue>
#include <set>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/any.hpp>
#include <boost/regex.hpp>

#include <ist/iterator.h>
#include <ist/ist_i3d.h>
#include <ist/shader_object.h>
#include <ist/frame_buffer_object.h>
#include <ist/random.h>
#include <ist/spline.h>
#include <ist/scheduler.h>
#include <ist/bstream.h>
#include <ctime>
#include <zlib.h>
#include <GL/glew.h>
#include <sgui/sgui.h>

#define Inherit2(C2, C1)  C2< C1 >
#define Inherit3(C3, C2, C1)  C3< Inherit2(C2, C1) >
#define Inherit4(C4, C3, C2, C1)  C4< Inherit3(C3, C2, C1) >
#define Inherit5(C5, C4, C3, C2, C1)  C5< Inherit4(C4, C3, C2, C1) >
#define Inherit6(C6, C5, C4, C3, C2, C1)  C6< Inherit5(C5, C4, C3, C2, C1) >
#define Inherit7(C7, C6, C5, C4, C3, C2, C1)  C7< Inherit6(C6, C5, C4, C3, C2, C1) >
#define Inherit8(C8, C7, C6, C5, C4, C3, C2, C1)  C8< Inherit7(C7, C6, C5, C4, C3, C2, C1) >
#define Inherit9(C9, C8, C7, C6, C5, C4, C3, C2, C1)  C9< Inherit8(C8, C7, C6, C5, C4, C3, C2, C1) >

namespace exception {

  using boost::any;
  using boost::any_cast;
  using boost::static_pointer_cast;
  using boost::shared_ptr;
  using boost::intrusive_ptr;

  using std::string;
  using std::wstring;
  using ist::vector2;
  using ist::vector3;
  using ist::vector4;
  using ist::matrix22;
  using ist::matrix33;
  using ist::matrix44;
  using ist::float4;

  typedef unsigned char uchar;
  typedef unsigned short ushort;
  typedef unsigned int uint;
  typedef unsigned long ulong;
  typedef size_t gid;

  typedef ist::ICollision collision;
  typedef ist::CollisionDetector cdetector;
  typedef ist::BoxCollision box_collision;
  typedef ist::PointCollision point_collision;
  typedef ist::Spline2D spline;
  typedef ist::Rect rect;
  typedef ist::Box box;
  typedef ist::Sphere sphere;

  typedef intrusive_ptr<ist::ProgramObject> po_ptr;
  typedef intrusive_ptr<ist::ShaderObject> so_ptr;
  typedef intrusive_ptr<ist::FrameBufferObject> fbo_ptr;

  typedef intrusive_ptr<ist::Random> random_ptr;
  typedef intrusive_ptr<ist::Texture> texture_ptr;
  typedef intrusive_ptr<ist::PolygonModel> pmodel_ptr;
  typedef intrusive_ptr<ist::MorphModel> amodel_ptr;
  typedef intrusive_ptr<ist::CachedMorphModel> cmodel_ptr;

#if defined(EXCEPTION_ENABLE_NETRANKING) || defined(EXCEPTION_ENABLE_NETUPDATE) || defined(EXCEPTION_ENABLE_NETPLAY)
  typedef shared_ptr<ist::HTTPRequestAsync> httpasync_ptr;
#endif


  class IMusic;
  class ISound;
  class IVertexBufferObject;

  typedef intrusive_ptr<IMusic> music_ptr;
  typedef intrusive_ptr<ISound> sound_ptr;
  typedef intrusive_ptr<IVertexBufferObject> vbo_ptr;


  class Message;
  class Object;
  class GameObject;
  class Solid;

  class IEnemy;
  class IFraction;
  class IPlayer;
  class IBullet;
  class IGround;
  class IEffect;
  class ILayer;

  class IDrawer;
  class IControler;

  typedef Message* message_ptr;

  typedef Object* obj_ptr;
  typedef GameObject* gobj_ptr;
  typedef Solid* solid_ptr;

  typedef IEnemy* enemy_ptr;
  typedef IFraction* fraction_ptr;
  typedef IPlayer* player_ptr;
  typedef IBullet* bullet_ptr;
  typedef IGround* ground_ptr;
  typedef IEffect* effect_ptr;
  typedef ILayer* layer_ptr;

  typedef intrusive_ptr<IDrawer> draw_ptr;
  typedef intrusive_ptr<IControler> controler_ptr;

  typedef std::vector<gobj_ptr> gobj_vector;
  typedef std::vector<solid_ptr> solid_vector;
  typedef ist::IIterator<gobj_ptr> gobj_iter;
  typedef ist::ContainerIterator<gobj_vector> gvector_iter;
  typedef std::vector<message_ptr> message_queue;
  typedef std::queue<gid> linkage_info;


  class Error : public std::runtime_error
  {
  typedef std::runtime_error Super;
  public:
    Error(const string& message) : Super(message) {}
  };

#ifdef WIN32
  class Win32Exception : public std::runtime_error
  {
  typedef std::runtime_error Super;
  private:
    DWORD m_code;

  public:
    Win32Exception(struct _EXCEPTION_POINTERS* ep) : Super(createErrorMessage(ep)) , m_code(ep->ExceptionRecord->ExceptionCode)
    {}

    DWORD code() const { return m_code; }

    static std::string createErrorMessage(struct _EXCEPTION_POINTERS* ep)
    {
      std::string r;
      switch(ep->ExceptionRecord->ExceptionCode) {
      case STATUS_ACCESS_VIOLATION: r = "STATUS_ACCESS_VIOLATION"; break;
      case STATUS_DATATYPE_MISALIGNMENT: r = "STATUS_DATATYPE_MISALIGNMENT"; break;
      case STATUS_BREAKPOINT: r = "STATUS_BREAKPOINT"; break;
      case STATUS_SINGLE_STEP: r = "STATUS_SINGLE_STEP"; break;
      case STATUS_ARRAY_BOUNDS_EXCEEDED: r = "STATUS_ARRAY_BOUNDS_EXCEEDED"; break;
      case STATUS_FLOAT_DENORMAL_OPERAND: r = "STATUS_FLOAT_DENORMAL_OPERAND"; break;
      case STATUS_FLOAT_DIVIDE_BY_ZERO: r = "STATUS_FLOAT_DIVIDE_BY_ZERO"; break;
      case STATUS_FLOAT_INEXACT_RESULT: r = "STATUS_FLOAT_INEXACT_RESULT"; break;
      case STATUS_FLOAT_INVALID_OPERATION: r = "STATUS_FLOAT_INVALID_OPERATION"; break;
      case STATUS_FLOAT_OVERFLOW: r = "STATUS_FLOAT_OVERFLOW"; break;
      case STATUS_FLOAT_STACK_CHECK: r = "STATUS_FLOAT_STACK_CHECK"; break;
      case STATUS_FLOAT_UNDERFLOW: r = "STATUS_FLOAT_UNDERFLOW"; break;
      case STATUS_INTEGER_DIVIDE_BY_ZERO: r = "STATUS_INTEGER_DIVIDE_BY_ZERO"; break;
      case STATUS_INTEGER_OVERFLOW: r = "STATUS_INTEGER_OVERFLOW"; break;
      case STATUS_PRIVILEGED_INSTRUCTION: r = "STATUS_PRIVILEGED_INSTRUCTION"; break;
      case STATUS_IN_PAGE_ERROR: r = "STATUS_IN_PAGE_ERROR"; break;
      case STATUS_ILLEGAL_INSTRUCTION: r = "STATUS_ILLEGAL_INSTRUCTION"; break;
      case STATUS_NONCONTINUABLE_EXCEPTION: r = "STATUS_NONCONTINUABLE_EXCEPTION"; break;
      case STATUS_STACK_OVERFLOW: r = "STATUS_STACK_OVERFLOW"; break;
      case STATUS_INVALID_DISPOSITION: r = "STATUS_INVALID_DISPOSITION"; break;
      case STATUS_GUARD_PAGE_VIOLATION: r = "STATUS_GUARD_PAGE_VIOLATION"; break;
      case STATUS_INVALID_HANDLE: r = "STATUS_INVALID_HANDLE"; break;
    //  case STATUS_POSSIBLE_DEADLOCK: r = "STATUS_POSSIBLE_DEADLOCK"; break;
      default: r = "unknown error"; break;
      }
      r+="\n";

      char buf[128];
      PEXCEPTION_RECORD rec = ep->ExceptionRecord;
      sprintf(buf, "code:%X flag:%X addr:%p params:%d\n",
        rec->ExceptionCode, rec->ExceptionFlags, rec->ExceptionAddress, rec->NumberParameters);
      r+=buf;
      for (DWORD i=0; i<rec->NumberParameters; ++i) {
        sprintf(buf, "param[%d]:%X\n", i, rec->ExceptionInformation[i]);
        r+=buf;
      }
      return r;
    }
  };
#endif

  class Serializer : public ist::bbuffer
  {
  public:
    bool read(void *p, size_t s)
    {
      throw Error("Serializer::read()");
      return false;
    }
  };

  class Deserializer : public ist::bbuffer
  {
  public:
    void write(const void *p, size_t s)
    {
      throw Error("Deserializer::write()");
    }
  };

  void SerializeMessage(Serializer& s, const message_ptr p);
  void SerializeObject(Serializer& s, const gobj_ptr p);
  void SerializeControler(Serializer& s, const controler_ptr p);
  message_ptr   DeserializeMessage(Deserializer& s);
  gobj_ptr      DeserializeObject(Deserializer& s);
  controler_ptr DeserializeControler(Deserializer& s);




  // 各種設定項目 
  class IConfig
  {
  public:
    string scorename;
    ushort port;
    size_t width;
    size_t height;
    size_t threads;

    bool fullscreen;
    bool vsync;
    bool simplebg;
    bool shader;
    bool vertex_buffer;
    bool noblur;
    bool show_fps;
    bool show_obj;
    bool update;

    bool fps_30;
    bool exlight;
    bool npttexture;
    float bloom;

    bool sound;
    bool bgm_mute;
    bool se_mute;
    int bgm_volume;
    int se_volume;

    int key[10];
    int pad[7];
    int controller;
    int daxis1;
    int daxis2;
    int threshold1;
    int threshold2;
    bool hat;

    string last_server;
    ushort last_port;

    virtual ~IConfig() {}
    virtual void save()=0;
    virtual void load()=0;
    virtual boost::mutex& getMutex()=0;
  };

  // モード別ステージクリア情報 
  class IClearFlag
  {
  public:
    int light;
    int normal;
    int heavy;
    int excess;
    int future;

    virtual ~IClearFlag() {}
    virtual int* getModeFlag(int mode)=0;
    virtual void save()=0;
    virtual void load()=0;
  };

  IConfig* GetConfig();
  IClearFlag* GetClearFlag();


  linkage_info& GetLinkageInfo();

  // 全ての基底クラス 
  class Object
  {
  private:
    static size_t s_idgen;
    static size_t s_count; // 現存するオブジェクトの数 
    static gid createID();

    gid m_id;

  public:
    static size_t getCount();

    Object();
    Object(Deserializer& s);
    virtual ~Object();
    virtual void reconstructLinkage();
    virtual void serialize(Serializer& s) const;
    virtual void release();

    gid getID() const;

    virtual void update()=0;
    virtual void draw()=0;

    // nameに対応するメソッドをvalueを引数に呼び出す。あまり多用はしたくない 
    virtual bool call(const string& name, const any& value);

    // デバッグ用 
    virtual string p();
  };


  // ダミー。templateの頭にするとか用 
  class EmptyClass
  {
  public:
    virtual ~EmptyClass() {}
  };

  template<class T>
  class Dummy : public T
  {
  public:
    Dummy() {}
    Dummy(Deserializer& s) {}
    virtual void serialize(Serializer& s) const {}
    virtual void update() {}
    virtual void draw() {}
    virtual const matrix44& getMatrix() { static matrix44 dummy; return dummy; }
    virtual gobj_ptr getParent() { return 0; }
    virtual bool call(const string& name, const any& value) { return false; }
    virtual string p() { return ""; }
  };






  // intrusive_ptr 用 
  class RefCounter
  {
  private:
    size_t m_ref;

  public:
    RefCounter();
    virtual ~RefCounter();
    void addRef();
    void release();
    size_t getRefCount() const;
  };

  inline void intrusive_ptr_add_ref(RefCounter *p) { p->addRef(); }
  inline void intrusive_ptr_release(RefCounter *p) { p->release(); }


  // イベントとかメッセージとか呼ばれるアレ一族 
  class Message
  {
  friend class ThreadSpecificMethod;
  private:
    bool m_shared;

  protected:
    gobj_ptr m_from;
    gobj_ptr m_to;

  public:
    enum { // 型一覧 
      UNKNOWN,
      CONSTRUCT,
      UPDATE,
      KILL,
      DAMAGE,
      DESTROY,
      COLLIDE,
      ACCEL,
      CALL,
    };
    // 上記の型を返す
    virtual int getType() const { return Message::UNKNOWN; }

    Message() : m_shared(false), m_from(0), m_to(0) {}
    virtual ~Message() {}
    virtual void reconstructLinkage();
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    gobj_ptr getFrom() const { return m_from; }
    gobj_ptr getTo() const   { return m_to; }
    void share()       { m_shared=true; }
    void unshare()     { m_shared=false; }
    bool isShared()    { return m_shared; }
  };


  // オブジェクト作成直後一回だけ呼ばれる 
  class ConstructMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  public:
    virtual int getType() const { return Message::CONSTRUCT; }
  };

  // アップデートしたいとき発行(=ほぼ毎フレーム大量に発行される) 
  class UpdateMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  public:
    virtual int getType() const { return Message::UPDATE; }
  };

  // 破壊された後等、完全に不要になったらこれを送って消えてもらう 
  class KillMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  public:
    virtual int getType() const { return Message::KILL; }
  };

  // 破壊されたとき発行 
  class DestroyMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  private:
    int m_stat;

  public:
    DestroyMessage() : m_stat() {}
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    virtual int getType() const { return Message::DESTROY; }
    int getStat() const { return m_stat; }
  };

  // ダメージ食らったとき発行 
  class DamageMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  private:
    gobj_ptr m_source;
    float m_damage;

  public:
    DamageMessage() : m_damage(0.0f), m_source(0) {}
    virtual void reconstructLinkage();
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    virtual int getType() const { return Message::DAMAGE; }
    float getDamage() const { return m_damage; }
    gobj_ptr getSource() const { return m_source; }
  };

  // 主に衝突したとき発行 
  class CollideMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  private:
    vector4 m_pos;
    vector4 m_normal;
    float m_distance;
    float m_force;

  public:
    CollideMessage() : m_distance(0.0f), m_force(0.0f) {}
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    virtual int getType() const { return Message::COLLIDE; }
    const vector4& getPosition() { return m_pos; }
    const vector4& getNormal() const { return m_normal; }
    float getDistance() const { return m_distance; }
    float getForce() const { return m_force; }
  };

  // 加速させるとき発行 
  class AccelMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  private:
    vector4 m_accel;

  public:
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    virtual int getType() const { return Message::ACCEL; }
    const vector4& getAccel() const { return m_accel; }
  };

  // 任意のメソッドをCallしたいとき発行 
  class CallMessage : public Message
  {
  typedef Message Super;
  friend class ThreadSpecificMethod;
  private:
    string m_name;
    any m_value;

  public:
    virtual void deserialize(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    virtual int getType() const { return Message::CALL; }
    const string& getName() const { return m_name; }
    const any&  getValue() const { return m_value; }
  };




  class IThreadSpecificMethod : public RefCounter
  {
  public:
    virtual gobj_iter& getObjects(const box& box)=0;
    virtual gobj_iter& getObjects(const sphere& sphere)=0;
    virtual gobj_iter& getAllObjects()=0;

    virtual void sendConstructMessage(gobj_ptr from, gobj_ptr to)=0;
    virtual void sendUpdateMessage(gobj_ptr from, gobj_ptr to)=0;
    virtual void sendDamageMessage(gobj_ptr from, gobj_ptr to, float d, gobj_ptr source=0)=0;
    virtual void sendDestroyMessage(gobj_ptr from, gobj_ptr to, int stat=0)=0;
    virtual void sendKillMessage(gobj_ptr from, gobj_ptr to)=0;
    virtual void sendCollideMessage(gobj_ptr from, gobj_ptr to, const vector4& p, const vector4& n, float d)=0;
    virtual void sendAccelMessage(gobj_ptr from, gobj_ptr to, const vector4& a)=0;
    virtual void sendCallMessage(gobj_ptr from, gobj_ptr to, const string& name, const any& value)=0;
    virtual message_ptr deserializeMessage(Deserializer& s)=0;
  };

  template<class T>
  class HaveThreadSpecificMethod : public T
  {
  typedef T Super;
  private:
    IThreadSpecificMethod *m_tsm;

  public:
    HaveThreadSpecificMethod() : m_tsm(0)
    {}

    HaveThreadSpecificMethod(Deserializer& s) : Super(s)
    {}

    void setTSM(IThreadSpecificMethod& tsm) { m_tsm = &tsm; }
    IThreadSpecificMethod& getTSM() { return *m_tsm; }

    gobj_iter& GetObjects(const box& box)
    {
      return m_tsm->getObjects(box);
    }

    gobj_iter& GetObjects(const sphere& sphere)
    {
      return m_tsm->getObjects(sphere);
    }

    gobj_iter& GetAllObjects()
    {
      return m_tsm->getAllObjects();
    }


    void SendConstructMessage(gobj_ptr from, gobj_ptr to)
    {
      m_tsm->sendConstructMessage(from, to);
    }

    void SendUpdateMessage(gobj_ptr from, gobj_ptr to)
    {
      m_tsm->sendUpdateMessage(from, to);
    }

    void SendDamageMessage(gobj_ptr from, gobj_ptr to, float d, gobj_ptr source=0)
    {
      m_tsm->sendDamageMessage(from, to, d, source);
    }

    void SendDestroyMessage(gobj_ptr from, gobj_ptr to, int stat=0)
    {
      m_tsm->sendDestroyMessage(from, to, stat);
    }

    void SendKillMessage(gobj_ptr from, gobj_ptr to)
    {
      m_tsm->sendKillMessage(from, to);
    }

    void SendCollideMessage(gobj_ptr from, gobj_ptr to, const vector4& p, const vector4& n, float d)
    {
      m_tsm->sendCollideMessage(from, to, p, n, d);
    }

    void SendAccelMessage(gobj_ptr from, gobj_ptr to, const vector4& a)
    {
      m_tsm->sendAccelMessage(from, to, a);
    }

    void SendCallMessage(gobj_ptr from, gobj_ptr to, const string& name, const any& value)
    {
      m_tsm->sendCallMessage(from, to, name, value);
    }
  };


  template<class T>
  class HaveMessageQueue : public T
  {
  typedef T Super;
  private:
    message_queue m_events;

  public:
    HaveMessageQueue()
    {
      m_events.reserve(8);
    }

    HaveMessageQueue(Deserializer& s) : Super(s)
    {
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_events.push_back(DeserializeMessage(s));
      }
    }

    ~HaveMessageQueue()
    {
      clearMessage();
    }

    virtual void reconstructLinkage()
    {
      Super::reconstructLinkage();
      for(size_t i=0; i<m_events.size(); ++i) {
        m_events[i]->reconstructLinkage();
      }
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_events.size();
      for(size_t i=0; i<m_events.size(); ++i) {
        SerializeMessage(s, m_events[i]);
      }
    }

    message_queue& getMessageQueue()
    {
      return m_events;
    }

    void clearMessage()
    {
      for(size_t i=0; i<m_events.size(); ++i) {
        m_events[i]->unshare();
      }
      m_events.clear();
    }

    void pushMessage(message_ptr m)
    {
      m->share();
      m_events.push_back(m);
    }

    virtual void processMessageQueue()
    {
      bool destroy = false;
      for(size_t i=0; i<m_events.size(); ++i) {
        Message& m = *m_events[i];
        if(m.getType()==Message::DESTROY) { // 複数回onDestroy呼ばれるのを防止 
          if(destroy) {
            m.unshare();
            continue;
          }
          else {
            destroy = true;
          }
        }
        dispatch(m);
        m.unshare();
      }
      m_events.clear();
    }

    virtual void dispatch(Message& m)
    {
      // 速度優先static_cast 
      int t = m.getType();
      switch(t) {
        case Message::CONSTRUCT: onConstruct(static_cast<ConstructMessage&>(m)); break;
        case Message::UPDATE:    onUpdate(static_cast<UpdateMessage&>(m));       break;
        case Message::DAMAGE:    onDamage(static_cast<DamageMessage&>(m));       break;
        case Message::DESTROY:   onDestroy(static_cast<DestroyMessage&>(m));     break;
        case Message::KILL:      onKill(static_cast<KillMessage&>(m));           break;
        case Message::COLLIDE:   onCollide(static_cast<CollideMessage&>(m));     break;
        case Message::ACCEL:     onAccel(static_cast<AccelMessage&>(m));         break;
        case Message::CALL:      onCall(static_cast<CallMessage&>(m));           break;
        default: throw Error("HaveMessageQueue::dispatch() : unknown type"); break;
      }
    }

    virtual void onConstruct(ConstructMessage& m){}
    virtual void onUpdate(UpdateMessage& m)      {}
    virtual void onKill(KillMessage& m)          {}
    virtual void onDestroy(DestroyMessage& m)    {}
    virtual void onDamage(DamageMessage& m)      {}
    virtual void onCollide(CollideMessage& m)    {}
    virtual void onAccel(AccelMessage& m)        {}
    virtual void onCall(CallMessage& m)          {}
  };





  // ゲーム内空間オブジェクト 
  class GameObject : public Inherit3(HaveMessageQueue, HaveThreadSpecificMethod, Object)
  {
  typedef Inherit3(HaveMessageQueue, HaveThreadSpecificMethod, Object) Super;
  private:
    bool m_killed;
    bool m_destroyed;

  public:
    GameObject(Deserializer& s);
    GameObject();
    virtual void serialize(Serializer& s) const;
    virtual void update();
    virtual void asyncupdate();
    virtual void draw();

    virtual bool isDead();
    bool isKilled();
    bool isDestroyed();

    virtual float getDrawPriority();
    virtual gobj_ptr getParent();
    virtual const vector4& getPosition()=0;
    virtual const matrix44& getMatrix()=0;

    virtual void onKill(KillMessage& m);
    virtual void onDestroy(DestroyMessage& m);
    virtual void onCall(CallMessage& m);

    // 型一覧。dynamic_castは遅いので、これらから型判別してstatic_castする 
    enum {
      UNKNOWN = 0,
      PLAYER  = 1<<0,
      ENEMY   = 1<<1,
      GROUND  = 1<<2,
      BULLET  = 1<<3,
      EFFECT  = 1<<4,
      LAYER   = 1<<5,

      RECIEVER= 1<<9,
      SOLID   = 1<<10,
      FRACTION= 1<<11,
    };
    virtual int getType() const { return UNKNOWN; }
  };


  class Reciever : public GameObject
  {
  typedef GameObject Super;
  public:
    Reciever() {}
    Reciever(Deserializer& s) : Super(s) {}
  };

  class Solid : public GameObject
  {
  typedef GameObject Super;
  private:
    static gid s_groupgen;
    gid m_group;

  public:
    static gid createGroupID();

    Solid(Deserializer& s);
    virtual void serialize(Serializer& s) const;

    Solid();
    virtual const collision& getCollision()=0;
    virtual float getVolume()=0;
    virtual vector4 getCenter() { return getPosition(); }
    virtual gid getGroup() const { return m_group; }
    virtual void setGroup(gid v) { m_group=v; }
    virtual int getType() const { return SOLID; }
    virtual bool call(const string& name, const any& value);
    virtual string p();
  };


  // 味方機 
  class IPlayer : public Solid
  {
  typedef Solid Super;
  public:
    IPlayer(Deserializer& s) : Super(s) {}
    IPlayer() {}
    virtual const vector4& getDirection()=0;
    virtual int getCatapultLevel()=0;
    virtual float getEnergy() const=0;
    virtual void setPosition(const vector4& v)=0;
    virtual void setEnergy(float)=0;
    virtual void setInvincible(int frame)=0;
    virtual float getLife()=0;
    virtual int getType() const { return PLAYER | SOLID; }
  };


  // 敵。破壊可能物。
  class IEnemy : public Solid
  {
  typedef Solid Super;
  public:
    IEnemy() {}
    IEnemy(Deserializer& s) : Super(s) {}
    virtual float getLife()=0;
    virtual int getType() const { return ENEMY | SOLID; }
  };

  class IFraction : public IEnemy
  {
  typedef IEnemy Super;
  public:
    IFraction(Deserializer& s) : Super(s) {}
    IFraction() {}
    virtual const vector4& getVel()=0;
    virtual const vector4& getAccel()=0;
    virtual void setVel(const vector4& v)=0;
    virtual void setAccel(const vector4& v)=0;
    virtual void setPosition(const vector4& v)=0;
    virtual int getType() const { return ENEMY | SOLID | FRACTION; }
  };


  // 地形。破壊不可能障害物 
  class IGround : public Solid
  {
  typedef Solid Super;
  public:
    IGround(Deserializer& s) : Super(s) {}
    IGround() {}
    virtual int getType() const { return GROUND | SOLID; }
  };


  // 弾 
  class IBullet : public GameObject
  {
  typedef GameObject Super;
  public:
    IBullet(Deserializer& s) : Super(s) {}
    IBullet() {}
    virtual gobj_ptr getOwner()=0;
    virtual int getType() const { return BULLET; }
  };


  // エフェクト類。ゲーム進行に影響を及ぼさないもの全般。 
  class IEffect : public GameObject
  {
  typedef GameObject Super;
  public:
    IEffect(Deserializer& s) : Super(s) {}
    IEffect() {}
    virtual int getType() const { return EFFECT; }
  };


  // グループ化オブジェクト 
  class ILayer : public GameObject
  {
  typedef GameObject Super;
  public:
    ILayer(Deserializer& s) : Super(s) {}
    ILayer() {}
    virtual void chain()=0;
    virtual void unchain()=0;
    virtual const matrix44& getIMatrix()=0;
    virtual int getType() const { return LAYER; }
  };




  // モデル描画とか用 
  class IDrawer : public RefCounter
  {
  public:
    virtual void update()=0;
    virtual void draw()=0;
    virtual float getPriority() const { return 0.0f; }
  };


  // 自機とか敵とかの行動制御 
  class IControler : public RefCounter
  {
  public:
    IControler(Deserializer& s) {}
    IControler() {}
    virtual void serialize(Serializer& s) const {}
    virtual void reconstructLinkage() {}

    virtual void setObject(gobj_ptr obj)=0;
    virtual void update() {}
    virtual void onConstruct(ConstructMessage& m)  {}
    virtual void onUpdate(UpdateMessage& m)  {}
    virtual void onKill(KillMessage& m)      {}
    virtual void onDestroy(DestroyMessage& m){}
    virtual void onDamage(DamageMessage& m)  {}
    virtual void onCollide(CollideMessage& m){}
    virtual void onAccel(AccelMessage& m)    {}
    virtual void onCall(CallMessage& m)      {}

    virtual bool call(const string& name, const any& value) { return false; }
  };



  class IMusic : public RefCounter
  {
  public:
    static IMusic* getCurrent();
    static void Halt();
    static void Pause();
    static void Resume();
    static bool FadeOut(int msec);
    static void WaitFadeOut();
    static void Serialize(Serializer& s);
    static void Deserialize(Deserializer& s);

    virtual const string& getFileName() const=0;
    virtual void setPosition(int msec)=0;
    virtual int getPosition() const=0;
    virtual bool play(int loop=-1)=0;
    virtual void halt()=0;
    virtual void pause()=0;
    virtual void resume()=0;
    virtual bool fadeOut(int msec)=0;
    virtual void waitFadeOut()=0;
  };

  class ISound : public RefCounter
  {
  public:
    static void halt(int channel);
    static bool isPlaying(int channel);
    virtual bool play(int channel, int loop=0)=0;
  };

  class IVertexBufferObject : public RefCounter
  {
  public:
    virtual void assign()=0;
    virtual void disassign()=0;
    virtual void* lock()=0;
    virtual void unlock()=0;
    virtual void draw()=0;
  };


  class IFrameBuffer : public RefCounter
  {
  public:
    virtual void assign()=0;
    virtual void disassign()=0;
    virtual GLsizei getWidth() const=0;
    virtual GLsizei getHeight() const=0;
    virtual GLsizei getScreenWidth() const=0;
    virtual GLsizei getScreenHeight() const=0;
  };



  class IResource : public RefCounter
  {
  public:
    virtual texture_ptr getTexture(const string& name)=0;
    virtual vbo_ptr getVBO(const string& name)=0;
    virtual so_ptr getVertexShader(const string& name)=0;
    virtual so_ptr getFragmentShader(const string& name)=0;
    virtual music_ptr getMusic(const string& name)=0;
    virtual sound_ptr getSound(const string& name)=0;
  };

  class IInput : public RefCounter
  {
  public:
    virtual void update()=0;

    virtual bool up() const=0;
    virtual bool down() const=0;
    virtual bool left() const=0;
    virtual bool right() const=0;
    virtual bool button(int b) const=0;
    virtual bool buttonPressed(int b) const=0;
    virtual bool buttonReleased(int b) const=0;

    virtual void serialize(Serializer& s) const=0;
  };


  // オブジェクトをキャッシュするクラスの基底となるもの。単なるログ取り用 
  class CacheInfo
  {
  private:
    typedef std::vector<CacheInfo*> info_cont;
    static info_cont s_info;
  public:
    static size_t getCacheInfoCount() { return s_info.size(); }
    static CacheInfo& getCacheInfo(size_t i) { return *s_info[i]; }

    CacheInfo() { s_info.push_back(this); }
    virtual ~CacheInfo() {}
    virtual string p()=0;
  };


  enum {
    LIGHT,
    NORMAL,
    HEAVY,
    EXCESS,
    FUTURE,
  };

  enum {
    STAGE_ALL,
    STAGE_1,
    STAGE_2,
    STAGE_3,
    STAGE_4,
    STAGE_5,
    STAGE_EX,
  };

  class IGame : public RefCounter
  {
  public:
    virtual void serialize(Serializer& s) const=0;
    virtual void join(size_t cid)=0;
    virtual void leave(size_t cid)=0;

    virtual void update()=0;
    virtual void draw()=0;
    virtual void exit()=0;

    virtual void setPause(bool f)=0;
    virtual bool isPaused()=0;

    virtual IThreadSpecificMethod& getTSM()=0;
    virtual void onThreadCountChange()=0;

    virtual void insertObject(gobj_ptr p)=0;
    virtual gobj_ptr getObject(gid)=0;
    virtual void getObjects(gobj_vector& store, const box& box)=0;
    virtual void getObjects(gobj_vector& store, const sphere& sphere)=0;
    virtual gobj_vector& getAllObjects()=0;

    virtual size_t getPlayerCount()=0;
    virtual player_ptr getPlayer(size_t i=0)=0;
    virtual player_ptr getPlayerByID(size_t cid)=0;
    virtual vector4 getPlayerPosition(size_t i=0)=0;
    virtual void newPlayer(int i=0)=0;

    virtual IFrameBuffer& getFrontFrameBuffer()=0;
    virtual IFrameBuffer& getBackFrameBuffer()=0;
    virtual ist::Light& getLight()=0;
    virtual ist::PerspectiveCamera& getCamera()=0;
    virtual const matrix44& getAimCameraMatrix()=0;
    virtual void setCameraMovableArea(const vector2& ur, const vector2& bl)=0;

    virtual size_t getPast()=0;
    virtual float getRand()=0;

    virtual int getLevel()=0;
    virtual bool isBossMode()=0;

    virtual void setBossMode(bool)=0;
    virtual void setBossTime(int)=0;
    virtual float getScore()=0;
    virtual int getHitCount()=0;
    virtual void addScore(float)=0;
    virtual void addHitCount(int)=0;

    virtual void showStageResult()=0;
    virtual void goNextStage()=0;

    virtual size_t getSessionID()=0;
    virtual IInput& getInput(size_t sid)=0;
    virtual void write()=0;

    // 以下デバッグ用 
    virtual bool isSynchronized() const=0; // 同期更新モードか否か 
    virtual void step()=0; // ポーズ中でも1フレームだけ進めて再度ポーズする 
    virtual string p()=0; //  
    virtual string pDetail()=0; //  
  };


  IGame* GetGame();
  IResource* GetResource();
} // exception 


// windows.h対策 
#ifdef SendMessage
  #undef SendMessage
#endif
#ifdef GetObject
  #undef GetObject
#endif


#define Register(o)           exception::GetGame()->insertObject(o)
#define GetObjectByID(id)     exception::GetGame()->getObject(id)

#define GetAimCameraMatrix()  exception::GetGame()->getAimCameraMatrix()
#define GetPast()             exception::GetGame()->getPast()
#define GetRand()             exception::GetGame()->getRand()
#define GetCamera()           exception::GetGame()->getCamera()
#define GetPlayerCount()      exception::GetGame()->getPlayerCount()
#define GetPlayer(i)          exception::GetGame()->getPlayer(i)
#define GetPlayerPosition(i)  exception::GetGame()->getPlayerPosition(i)
#define GetMainTSM()          exception::GetGame()->getTSM()

#define GetLevel()            exception::GetGame()->getLevel()
#define IsPaused()            exception::GetGame()->isPaused()
#define IsBossMode()          exception::GetGame()->isBossMode()
#define GetScore()            exception::GetGame()->getScore()
#define GetHitCount()         exception::GetGame()->getHitCount()

#define SetBossMode(v)        exception::GetGame()->setBossMode(v)
#define SetBossTime(v)        exception::GetGame()->setBossTime(v)
#define AddScore(v)           exception::GetGame()->addScore(v)
#define AddHitCount(v)        exception::GetGame()->addHitCount(v)
#define GetScore()            exception::GetGame()->getScore()
#define GetHitCount()         exception::GetGame()->getHitCount()

#define GoNextStage()         exception::GetGame()->goNextStage()
#define ShowStageResult()     exception::GetGame()->showStageResult()

#define SetCameraMovableArea(v1, v2) exception::GetGame()->setCameraMovableArea(v1, v2)

#define GetTexture(name)        exception::GetResource()->getTexture(name)
#define GetVBO(name)            exception::GetResource()->getVBO(name)
#define GetVertexShader(name)   exception::GetResource()->getVertexShader(name)
#define GetFragmentShader(name) exception::GetResource()->getFragmentShader(name)
#define GetMusic(name)          exception::GetResource()->getMusic(name)
#define GetSound(name)          exception::GetResource()->getSound(name)

#endif

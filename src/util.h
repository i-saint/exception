#ifndef util_h
#define util_h
#include "interface.h"

#ifdef DrawText
  #undef DrawText
#endif

namespace exception {

  inline sgui::Font* GetFont()
  {
    return sgui::App::instance()->getDefaultFont();
  }

  inline void DrawText(const wstring& str, const sgui::Point& pos)
  {
    if(sgui::Font *font = GetFont()) {
      font->draw(str, sgui::Rect(pos, sgui::Size(640, 480)));
    }
  }

  inline void DrawText(const wstring& str, const vector2& pos)
  {
    DrawText(str, sgui::Point(pos.x, pos.y));
  }


#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  void ErrorIfSyncPhase();
  void ErrorIfAsyncPhase();
#else
  #define ErrorIfSyncPhase()
  #define ErrorIfAsyncPhase()
#endif

  void PushLinkage(Deserializer& s);
  void PushLinkage(gid id);
  gid PopLinkageID();
  gobj_ptr PopLinkage();
  gid ToID(gobj_ptr p);


  template<class T>
  struct UnPtr
  {
    typedef T type;
  };

  template<class T>
  struct UnPtr<T*>
  {
    typedef T type;
  };

  template<class T>
  T* CheckedPopLinkage()
  {
    gid id = PopLinkageID();
    if(id==0) {
      return 0;
    }
    else if(gobj_ptr p = GetObjectByID(id)) {
      if(T *c = dynamic_cast<T*>(p)) {
        return c;
      }
      else {
        char buf[256];
        sprintf(buf, "%s / %s (%d)", typeid(*p).name(), typeid(T).name(), id);
        throw Error(string("ReconstructLinkage() : type not matched\n") + buf);
      }
    }
    else {
      char buf[256];
      sprintf(buf, "%s (%d)", typeid(T).name(), id);
      throw Error(string("ReconstructLinkage() : object not found\n") + buf);
    }
    return 0;
  }



  template<class Ptr>
  void SerializeLinkage(Serializer& s, const Ptr& v)
  {
    s << ToID(v);
  }

  template<class Ptr>
  void DeserializeLinkage(Deserializer& s, Ptr& v)
  {
    PushLinkage(s);
  }

  template<class Ptr>
  void ReconstructLinkage(Ptr& v)
  {
    v = CheckedPopLinkage< UnPtr<Ptr>::type >();
  }


  template<class Ptr, size_t N>
  void SerializeLinkage(Serializer& s, const Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      s << ToID(v[i]);
    }
  }

  template<class Ptr, size_t N>
  void DeserializeLinkage(Deserializer& s, Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      PushLinkage(s);
    }
  }

  template<class Ptr, size_t N>
  void ReconstructLinkage(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      v[i] = CheckedPopLinkage< UnPtr<Ptr>::type >();
    }
  }


  template<class T>
  void DeserializeLinkageContainer(Deserializer& s, T& v)
  {
    size_t size;
    s >> size;
    v.resize(size);
    for(size_t i=0; i<size; ++i) {
      PushLinkage(s);
    }
  }

  template<class T>
  void ReconstructLinkageContainer(T& v)
  {
    for(typename T::iterator i=v.begin(); i!=v.end(); ++i) {
      *i = CheckedPopLinkage<UnPtr<T::value_type>::type>();
    }
  }

  template<class T>
  void SerializeLinkageContainer(Serializer& s, const T& v)
  {
    s << v.size();
    for(typename T::const_iterator i=v.begin(); i!=v.end(); ++i) {
      s << ToID(*i);
    }
  }



  template<class Ptr>
  inline void ZeroClear(Ptr &v)
  {
    v = 0;
  }

  template<class Ptr, size_t N>
  inline void ZeroClear(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      v[i] = 0;
    }
  }


  template<class Ptr>
  inline void SweepDeadObject(Ptr &v)
  {
    if(v && v->isDead()) {
      v = 0;
    }
  }

  template<class Ptr, size_t N>
  inline void SweepDeadObject(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      if(v[i] && v[i]->isDead()) {
        v[i] = 0;
      }
    }
  }



  bool IsSolid(gobj_ptr p);
  bool IsEnemy(gobj_ptr p);
  bool IsFraction(gobj_ptr p);
  bool IsPlayer(gobj_ptr p);
  bool IsGround(gobj_ptr p);
  bool IsBullet(gobj_ptr p);
  bool IsEffect(gobj_ptr p);
  bool IsLayer(gobj_ptr p);

  solid_ptr  ToSolid(gobj_ptr p);
  enemy_ptr  ToEnemy(gobj_ptr p);
  fraction_ptr ToFraction(gobj_ptr p);
  player_ptr ToPlayer(gobj_ptr p);
  ground_ptr ToGround(gobj_ptr p);
  bullet_ptr ToBullet(gobj_ptr p);
  effect_ptr ToEffect(gobj_ptr p);
  layer_ptr  ToLayer(gobj_ptr p);

  float GenRand();
  size_t GetChecksum(const string& path);
  void CreateBoxVBO(vbo_ptr vbo, const box& box);
  vector4 GetUnprojectedPosition(const vector2& screen);
  vector2 GetProjectedPosition(const vector4& pos);

  bool IsInnerScreen(const vector4& pos, const vector2& expand=vector2());
  void DrawRect(const vector2& ur, const vector2& bl, const vector2& tur=vector2(1,1), const vector2& tbl=vector2(0,0));
  const float* CreateBoxVBO(const box& box); // 24*6‚Ìfloat”z—ñ‚ð•Ô‚· 

  void PlaySound(ISound *s, int ch);

  class ScreenMatrix
  {
  private:
    ist::ModelviewMatrixSaver m_ms;
    ist::ProjectionMatrixSaver m_ps;

  public:
    ScreenMatrix(int width=640, int height=480)
    {
      ist::OrthographicCamera cam;
      cam.setPosition(vector4(0.0f, 0.0f, 500.0f));
      cam.setScreen(0,width, height,0);
      cam.setZNear(-1.0f);
      cam.setZFar(1000.0f);
      cam.look();
    }
  };

  class ViewportSaver
  {
  private:
    int m_viewport[4];
    ist::ModelviewMatrixSaver m_ms;
    ist::ProjectionMatrixSaver m_ps;

  public:
    ViewportSaver()
    {
      glGetIntegerv(GL_VIEWPORT, m_viewport);
    }

    ~ViewportSaver()
    {
      glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
    }
  };


  class StopWatch
  {
  private:
    size_t m_start;
    size_t m_end;

  public:
    StopWatch() : m_start(0), m_end(0) {}

    void run(size_t limit)
    {
      m_start = GetPast();
      m_end = m_start+limit;
    }

    void stop() { m_end=GetPast(); }
    size_t getPast() { return GetPast()-m_start; }
    size_t getLeft() { return m_end-GetPast(); }
    bool isRunning() { return GetPast()<m_end; }
    bool isFinished(){ return GetPast()>=m_end; }

    void serialize(ist::bostream& s) const { s << m_start << m_end; }
    void deserialize(ist::bistream& s)     { s >> m_start >> m_end; }
  };




  struct is_dead
  {
    bool operator()(gobj_ptr p) const
    {
      return p->isDead();
    }
  };

  struct greater_draw_priority
  {
    bool operator()(gobj_ptr l, gobj_ptr r) const
    {
      float pl = l->getDrawPriority();
      float pr = r->getDrawPriority();
      return pl==pr ? l->getID()<r->getID() : pl<pr;
    }
  };

  struct less_id
  {
    bool operator()(gobj_ptr l, gobj_ptr r) const
    {
      return l->getID() < r->getID();
    }
  };

  struct equal_id
  {
    bool operator()(gobj_ptr l, gobj_ptr r) const
    {
      return l->getID()==r->getID();
    }
  };

}

inline ist::bostream& operator<<(ist::bostream& b, const exception::StopWatch& v) { v.serialize(b); return b; }
inline ist::bistream& operator>>(ist::bistream& b, exception::StopWatch& v)       { v.deserialize(b); return b; }

namespace impl {
  template<class T>
  inline bool serialize_any(ist::bostream& b, const boost::any& v)
  {
    if(v.type()==typeid(T)) {
      b << boost::any_cast<const T&>(v);
      return true;
    }
    return false;
  }

  template<class T>
  inline bool deserialize_any(ist::bistream& b, const std::string& name, boost::any& v)
  {
    if(name==typeid(T).name()) {
      T tmp;
      b >> tmp;
      v = tmp;
      return true;
    }
    return false;
  }

  inline bool throw_exception(const std::string& message)
  {
    throw exception::Error(message);
    return false;
  }
}

inline ist::bostream& operator<<(ist::bostream& b, const boost::any& v)
{
  b << std::string(v.type().name());

  impl::serialize_any<char>(b,v)     || impl::serialize_any<unsigned char>(b, v)  ||
  impl::serialize_any<short>(b, v)   || impl::serialize_any<unsigned short>(b, v) ||
  impl::serialize_any<int>(b, v)     || impl::serialize_any<unsigned int>(b, v)   ||
  impl::serialize_any<float>(b, v)   ||
  impl::serialize_any<double>(b, v)  ||
  impl::serialize_any<bool>(b, v)    ||
  impl::serialize_any<wchar_t>(b, v) ||
  impl::serialize_any<std::string>(b, v)  ||
  impl::serialize_any<std::wstring>(b, v) ||

  impl::serialize_any<ist::vector2>(b, v)  ||
  impl::serialize_any<ist::vector3>(b, v)  ||
  impl::serialize_any<ist::vector4>(b, v)  ||
  impl::serialize_any<ist::matrix22>(b, v) ||
  impl::serialize_any<ist::matrix33>(b, v) ||
  impl::serialize_any<ist::matrix44>(b, v) ||

  impl::throw_exception(std::string("serialize_any : ")+v.type().name());

  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, boost::any& v)
{
  std::string name;
  b >> name;

  impl::deserialize_any<char>(b, name, v)    || impl::deserialize_any<unsigned char>(b, name, v)  ||
  impl::deserialize_any<short>(b, name, v)   || impl::deserialize_any<unsigned short>(b, name, v) ||
  impl::deserialize_any<int>(b, name, v)     || impl::deserialize_any<unsigned int>(b, name, v)   ||
  impl::deserialize_any<float>(b, name, v)   ||
  impl::deserialize_any<double>(b, name, v)  ||
  impl::deserialize_any<bool>(b, name, v)    ||
  impl::deserialize_any<wchar_t>(b, name, v) ||
  impl::deserialize_any<std::string>(b, name, v)  ||
  impl::deserialize_any<std::wstring>(b, name, v) ||

  impl::deserialize_any<ist::vector2>(b, name, v)  ||
  impl::deserialize_any<ist::vector3>(b, name, v)  ||
  impl::deserialize_any<ist::vector4>(b, name, v)  ||
  impl::deserialize_any<ist::matrix22>(b, name, v) ||
  impl::deserialize_any<ist::matrix33>(b, name, v) ||
  impl::deserialize_any<ist::matrix44>(b, name, v) ||

  impl::throw_exception(std::string("deserialize_any : ")+name);

  return b;
}

#endif


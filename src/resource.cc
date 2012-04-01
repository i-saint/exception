#include "stdafx.h"
#include <SDL/SDL_mixer.h>
#include "ist/ist_sys.h"

namespace exception {


  namespace {
    IMusic *g_current_music = 0;
  }

  void IMusic::Halt()
  {
    if(g_current_music) {
      g_current_music->halt();
    }
  }

  void IMusic::Pause()
  {
    if(g_current_music) {
      g_current_music->pause();
    }
  }

  void IMusic::Resume()
  {
    if(g_current_music) {
      g_current_music->resume();
    }
  }

  void IMusic::WaitFadeOut()
  {
    if(g_current_music) {
      g_current_music->waitFadeOut();
    }
  }

  bool IMusic::FadeOut(int msec)
  {
    if(g_current_music) {
      return g_current_music->fadeOut(msec);
    }
    return false;
  }


  class Music : public IMusic
  {
  private:
    Mix_Music *m_music;
    string m_filename;
    int m_position;
    int m_start_time;

  public:
    Music() : m_music(0), m_position(0), m_start_time(0)
    {}

    Music(const string& filename) : m_music(0), m_position(0), m_start_time(0)
    {
      load(filename);
    }

    ~Music()
    {
      release();
    }

    void release()
    {
      if(m_music) {
        halt();
        Mix_FreeMusic(m_music);
        m_music = 0;
      }
    }

    void halt()
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return;
      }
      g_current_music = 0;
      Mix_HaltMusic();
    }

    void pause()
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return;
      }
      if(!Mix_FadingMusic()) {
        m_position += sgui::GetTicks()-m_start_time;
        Mix_PauseMusic();
      }
    }

    void resume()
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return;
      }
      if(Mix_PausedMusic()) {
        m_start_time = sgui::GetTicks();
        Mix_ResumeMusic();
      }
    }

    void setPosition(int msec)
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return;
      }
      m_position = msec;
      Mix_SetMusicPosition(double(msec)/1000.0);
    }

    int getPosition() const
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return 0;
      }
      int r = m_position;
      if(Mix_PlayingMusic()) {
        r+=(sgui::GetTicks()-m_start_time);
      }
      return r;
    }

    void waitFadeOut()
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return;
      }
      if(Mix_FadingMusic()==MIX_FADING_OUT && Mix_PlayingMusic()) {
        halt();
      }
    }

    bool fadeOut(int msec)
    {
      if(!GetConfig()->sound || g_current_music!=this) {
        return false;
      }
      if(Mix_FadingMusic()==MIX_FADING_IN) {
        halt();
        return true;
      }
      else {
        return Mix_FadeOutMusic(msec)==1;
      }
    }

    const string& getFileName() const { return m_filename; }

    bool load(const string& filename)
    {
      m_filename = filename;
      if(!GetConfig()->sound) {
        return false;
      }
      release();
      m_music = Mix_LoadMUS(filename.c_str());
      return !!m_music;
    }

    bool play(int loop)
    {
      if(!GetConfig()->sound) {
        return false;
      }
      if(m_music && Mix_PlayMusic(m_music, loop)==0) {
        m_position = 0;
        m_start_time = sgui::GetTicks();
        g_current_music = this;
        return true;
      }
      return false;
    }

    bool operator!()
    {
      return !m_music;
    }
  };


  void ISound::halt(int channel)
  {
    if(GetConfig()->sound) {
      if(isPlaying(channel)) {
        Mix_HaltChannel(channel);
      }
    }
  }

  bool ISound::isPlaying(int channel)
  {
    if(GetConfig()->sound) {
      return GetConfig()->sound && Mix_Playing(channel)!=0;
    }
    else {
      return false;
    }
  }

  class Sound : public ISound
  {
  private:
    Mix_Chunk *m_chunk;

  public:
    Sound() : m_chunk(0) {}
    Sound(const string& filename) : m_chunk(0) { load(filename); }

    ~Sound()
    {
      release();
    }

    void release()
    {
      if(m_chunk) {
        Mix_FreeChunk(m_chunk);
        m_chunk = 0;
      }
    }

    bool load(const string& filename)
    {
      if(GetConfig()->sound) {
        release();
        m_chunk = Mix_LoadWAV(filename.c_str());
      }
      return !!m_chunk;
    }

    bool play(int channel, int loop)
    {
      if(GetConfig()->sound) {
        if(channel!=-1 && isPlaying(channel)) {
          halt(channel);
        }
        if(m_chunk) {
          Mix_PlayChannel(channel, m_chunk, loop);
          return true;
        }
      }
      return false;
    }

    bool operator!()
    {
      return !m_chunk;
    }
  };




  class VertexBufferObject : public IVertexBufferObject
  {
  private:
    const VertexBufferObject& operator=(const VertexBufferObject&); // コピー禁止 
    VertexBufferObject(const VertexBufferObject&);

    GLuint m_vbo;
    size_t m_vsize;
    size_t m_size;
    int m_primitive;
    int m_format;
    int m_access;

    std::vector<char> m_vertex;

  public:
    VertexBufferObject(size_t vsize, size_t size, int format, int primitive, int access=GL_STATIC_DRAW_ARB) :
        m_vsize(vsize), m_size(size), m_vbo(0), m_primitive(primitive), m_format(format), m_access(access)
    {
      if(!GetConfig()->vertex_buffer) {
        m_access = 0;
      }

      if     (m_format==GL_V2F) {}
      else if(m_format==GL_V3F) {}
      else if(m_format==GL_T2F_V3F) {}
      else if(m_format==GL_N3F_V3F) {}
      else if(m_format==GL_T2F_N3F_V3F) {}
      else {
        throw Error("VertexBufferObject::VertexBufferObject()");
      }

      if(m_access==0) {
        m_vertex.resize(m_size*m_vsize);
      }
      else {
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_size*m_vsize, 0, m_access);
        CheckGLError();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    ~VertexBufferObject()
    {
      if(m_access!=0) {
        glDeleteBuffers(1, &m_vbo); // 存在しないvboをglDeleteBuffersしても何も起きない 
        m_vbo = 0;
      }
    }

    void assign()
    {
      if(m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
      }

      void *data = 0;
      if(!m_vbo) {
        data = &m_vertex[0];
      }
      glInterleavedArrays(m_format, m_vsize, data);
    }

    void disassign()
    {
      if(m_vbo) {
        glBindBufferARB(GL_ARRAY_BUFFER, 0);
      }
    }

    void* lock()
    {
      if(m_vbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      }
      else {
        return &m_vertex[0];
      }
    }

    void unlock()
    {
      if(m_vbo) {
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }


    void draw()
    {
      glDrawArrays(m_primitive, 0, m_size);
    }
  };



  class Archive
  {
  private:
    std::fstream *m_in;
    ist::biostream *m_bio;
    ist::IGZExtracter *m_ext;

    Archive(const Archive&);
    Archive& operator=(const Archive&);

  public:
    Archive(const string& path)
    {
      m_in = new std::fstream(path.c_str(), std::ios::in | std::ios::binary);
      m_bio = new ist::biostream(*m_in);
      m_ext = new ist::IGZExtracter(*m_bio);
    }

    ~Archive()
    {
      delete m_ext;
      delete m_bio;
      delete m_in;
    }

    bool extract(const string& path)
    {
      return m_ext->extractToFile(path, "resource/");
    }
  };


  class Resource : public IResource
  {
  private:
    typedef std::map<string, texture_ptr> texture_cont;
    typedef std::map<string, vbo_ptr>     vbo_cont;
    typedef std::map<string, fbo_ptr>     fbo_cont;
    typedef std::map<string, po_ptr>      po_cont;
    typedef std::map<string, sound_ptr>   sound_cont;
    texture_cont m_texture;
    vbo_cont m_vbo;
    fbo_cont m_fbo;
    po_cont m_po;
    sound_cont m_sound;
    music_ptr m_music;

    Archive *m_arch;

  public:
    Resource()
    {
      m_arch = new Archive("resource/resource.igz");

      loadTexture("title.png");
      loadTexture("damage.png");
      loadTexture("warning.png");
      loadTexture("lockon.png");
      loadTexture("gl.png");
      loadTexture("ray.png");
      loadTexture("rray.png");
      loadTexture("laser.png");
      loadTexture("gravity.png");
      loadTexture("flare.png");
      loadTexture("ring_r.png");
      loadTexture("ring_b.png");
      loadTexture("burner.png");
      loadTexture("burnerb.png");
    //  loadTexture("explode0.png");
      loadTexture("explode1.png");
      loadTexture("explode2.png");

      loadSound("explosion1.wav");
      loadSound("explosion2.wav");
      loadSound("explosion3.wav");
      loadSound("explosion4.wav");
      loadSound("explosion5.wav");
      loadSound("explosion6.wav");
      loadSound("lock.wav");
      loadSound("warning.wav");
      loadSound("charge1.wav");
      loadSound("charge2.wav");
      loadSound("charge3.wav");


      {
        vector3 vertex[4] = {
          vector3(0.0f, 1.0f, 1.0f),
          vector3(0.0f,-1.0f, 1.0f),
          vector3(0.0f,-1.0f,-1.0f),
          vector3(0.0f, 1.0f,-1.0f),
        };
        vector2 tex[4] = {
          vector2(0.0f, 0.0f),
          vector2(0.0f, 1.0f),
          vector2(1.0f, 1.0f),
          vector2(1.0f, 0.0f),
        };
        float tmp[5*4];
        VertexBufferObject *vbo;

        vbo= new VertexBufferObject(sizeof(float)*5, 4, GL_T2F_V3F, GL_QUADS);
        for(int i=0; i<4; ++i) {
          float *dst = tmp+(5*i);
          std::copy(tex[i].v,    tex[i].v+2,    dst+0);
          std::copy(vertex[i].v, vertex[i].v+3, dst+2);
        }

        float *data = (float*)vbo->lock();
        std::copy(tmp, tmp+5*4, data);
        vbo->unlock();
        m_vbo["plane"] = vbo;
      }
      {
        vbo_ptr vbo = new VertexBufferObject(sizeof(float)*6, 24, GL_N3F_V3F, GL_QUADS);
        float *data = (float*)vbo->lock();
        const float *cube = CreateBoxVBO(box(vector4(7.0f, 7.0f, 7.0f), vector4(-7.0f, -7.0f, -7.0f)));
        std::copy(cube, cube+6*24, data);
        vbo->unlock();
        m_vbo["cube"] = vbo;
      }
      {
        vbo_ptr vbo = new VertexBufferObject(32, 24, GL_T2F_N3F_V3F, GL_QUADS, 0);
        m_vbo["generic"] = vbo;
      }
    }

    ~Resource()
    {
      delete m_arch;
    }

    texture_ptr loadTexture(const string& name)
    {
      texture_ptr& r = m_texture[name];
      if(!r) {
        string path = string("resource/")+name;
        if(ist::IsFile(path)) {
          r = new ist::Texture(path.c_str());
        }
        else if(m_arch->extract(name)) {
          r = new ist::Texture(path.c_str());
          ist::Remove(path);
        }
        else {
          throw Error(name+": texture not found");
        }
      }
      return r;
    }

    texture_ptr getTexture(const string& name)
    {
      texture_cont::iterator p = m_texture.find(name);
      if(p==m_texture.end()) {
        throw Error(name+": texture not found");
      }
      return p->second;
    }

    music_ptr getMusic(const string& name)
    {
      m_music = 0;
      m_music = new Music(string("resource/")+name);
      return m_music;
    }


    sound_ptr loadSound(const string& name)
    {
      sound_ptr& r = m_sound[name];
      if(!r) {
        string path = string("resource/")+name;
        if(ist::IsFile(path)) {
          r = new Sound(path);
        }
        else if(m_arch->extract(name)) {
          r = new Sound(path);
          ist::Remove(path);
        }
        else {
          throw Error(name+": sound not found");
        }
      }
      return r;
    }

    sound_ptr getSound(const string& name)
    {
      sound_cont::iterator p = m_sound.find(name);
      if(p==m_sound.end()) {
        throw Error(name+": sound not found");
      }
      return p->second;
    }

    vbo_ptr getVBO(const string& name)
    {
      return m_vbo[name];
    }

    so_ptr getVertexShader(const string& name)
    {
      so_ptr r;
      string path = string("resource/")+name;
      if(ist::IsFile(path)) {
        r = new ist::VertexShader(path.c_str());
      }
      else if(m_arch->extract(name)) {
        r = new ist::VertexShader(path.c_str());
        ist::Remove(path);
      }
      else {
        throw Error(name+": shader not found");
      }
      return r;
    }

    so_ptr getFragmentShader(const string& name)
    {
      so_ptr r;
      string path = string("resource/")+name;
      if(ist::IsFile(path)) {
        r = new ist::FragmentShader(path.c_str());
      }
      else if(m_arch->extract(name)) {
        r = new ist::FragmentShader(path.c_str());
        ist::Remove(path);
      }
      else {
        throw Error(name+": shader not found");
      }
      return r;
    }

    void clear()
    {
      m_texture.clear();
      m_vbo.clear();
      m_sound.clear();
    }
  };

  IResource* CreateResource()
  {
    return new Resource();
  }




  void IMusic::Serialize(Serializer& s)
  {
    if(g_current_music) {
      s << g_current_music->getFileName() << g_current_music->getPosition();
    }
    else {
      s << string("null") << int(0);
    }
  }

  void IMusic::Deserialize(Deserializer& s)
  {
    string filename;
    int pos;
    s >> filename >> pos;
    if(filename!="null") {
      filename = boost::regex_replace(filename, boost::regex("resource/"), "");
      if(music_ptr m = GetMusic(filename)) {
        m->play();
        m->setPosition(pos);
      }
    }
  }

} // exception 

#ifndef game_h
#define game_h

namespace exception {


  class Config : public IConfig
  {
  private:
    boost::mutex m_mutex;

  public:
    Config()
    {
      exlight = true;
      npttexture = true;
      bloom =1.0f;

      scorename = "nullpo";
      width = 640;
      height = 480;

      fullscreen = false;
      vsync = true;
      simplebg = false;
      shader = false;
      vertex_buffer = false;
      noblur = false;
      fps_30 = false;

      show_fps = true;
      show_obj = true;
      update = true;
#ifdef WIN32
      SYSTEM_INFO info;
      GetSystemInfo(&info);
      threads = std::max<size_t>(1, info.dwNumberOfProcessors);
#else
      threads = 2;
#endif
      port = 10040;

      sound = false;
      bgm_mute = false;
      se_mute = false;
      bgm_volume = 96;
      se_volume = 96;

      key[0]=sgui::KEY_Z;
      key[1]=sgui::KEY_X;
      key[2]=sgui::KEY_V;
      key[3]=sgui::KEY_C;
      key[4]=sgui::KEY_A;
      key[5]=sgui::KEY_S;
      key[6]=sgui::KEY_UP;
      key[7]=sgui::KEY_DOWN;
      key[8]=sgui::KEY_RIGHT;
      key[9]=sgui::KEY_LEFT;

      pad[0]=0;
      pad[1]=1;
      pad[2]=2;
      pad[3]=3;
      pad[4]=4;
      pad[5]=5;
      pad[6]=10;

      controller = 0;
      daxis1 = 5;
      daxis2 = 5;
      threshold1 = 15000;
      threshold2 = 15000;
      hat = false;

      last_server = "192.168.0.1";
      last_port = 10040;
    }
      
    void save()
    {
      FILE *f = fopen("config", "wb");
      if(!f) {
        return;
      }

      fprintf(f, "resolution=%dx%d\n", width, height);
      fprintf(f, "fullscreen=%d\n", fullscreen);
      fprintf(f, "vsync=%d\n", vsync);
      fprintf(f, "exlight=%d\n", exlight);
      fprintf(f, "shader=%d\n", shader);
      fprintf(f, "vertex_buffer=%d\n", vertex_buffer);
      fprintf(f, "simplebg=%d\n", simplebg);
      fprintf(f, "noblur=%d\n", noblur);
      fprintf(f, "30fps=%d\n", fps_30);
      fprintf(f, "bloom=%f\n", bloom);

      fprintf(f, "scorename=%s\n", scorename.c_str());
      fprintf(f, "show_fps=%d\n", show_fps);
      fprintf(f, "show_obj=%d\n", show_obj);
      fprintf(f, "update=%d\n", update);
      fprintf(f, "thread=%d\n", threads);
      fprintf(f, "port=%d\n", port);

      fprintf(f, "bgm=%d\n", !bgm_mute);
      fprintf(f, "se=%d\n", !se_mute);
      fprintf(f, "bgm_volume=%d\n", bgm_volume);
      fprintf(f, "se_volume=%d\n", se_volume);

      fprintf(f, "key0=%d\n", key[0]);
      fprintf(f, "key1=%d\n", key[1]);
      fprintf(f, "key2=%d\n", key[2]);
      fprintf(f, "key3=%d\n", key[3]);
      fprintf(f, "key4=%d\n", key[4]);
      fprintf(f, "key5=%d\n", key[5]);
      fprintf(f, "key6=%d\n", key[6]);
      fprintf(f, "key7=%d\n", key[7]);
      fprintf(f, "key8=%d\n", key[8]);
      fprintf(f, "key9=%d\n", key[9]);
      fprintf(f, "pad0=%d\n", pad[0]);
      fprintf(f, "pad1=%d\n", pad[1]);
      fprintf(f, "pad2=%d\n", pad[2]);
      fprintf(f, "pad3=%d\n", pad[3]);
      fprintf(f, "pad4=%d\n", pad[4]);
      fprintf(f, "pad5=%d\n", pad[5]);
      fprintf(f, "pad6=%d\n", pad[6]);
      fprintf(f, "controller=%d\n", controller);
      fprintf(f, "daxis1=%d\n", daxis1);
      fprintf(f, "daxis2=%d\n", daxis2);
      fprintf(f, "threshold1=%d\n", threshold1);
      fprintf(f, "threshold2=%d\n", threshold2);
      fprintf(f, "hat=%d\n", hat);

      fprintf(f, "last_server=%s\n", last_server.c_str());
      fprintf(f, "last_port=%d\n", last_port);
      fclose(f);
    }

    void load()
    {
      FILE *in = fopen("config", "rb");
      if(!in) {
        return;
      }

      char l[256];
      char buf[128];
      int i, j;
      float f;
      while(fgets(l, 256, in)) {
        if(sscanf(l, "resolution=%dx%d", &i, &j)==2) { width=i; height=j; }
        if(sscanf(l, "fullscreen=%d", &i))   { fullscreen = i!=0; }
        if(sscanf(l, "vsync=%d", &i))        { vsync = i!=0; }
        if(sscanf(l, "exlight=%d", &i))      { exlight = i!=0; }
        if(sscanf(l, "shader=%d", &i))       { shader = i!=0; }
        if(sscanf(l, "vertex_buffer=%d", &i)){ vertex_buffer = i!=0; }
        if(sscanf(l, "simplebg=%d", &i))     { simplebg = i!=0; }
        if(sscanf(l, "noblur=%d", &i))       { noblur = i!=0; }
        if(sscanf(l, "30fps=%d", &i))        { fps_30 = i!=0; }
        if(sscanf(l, "bloom=%f", &f))        { bloom = f; }

        if(sscanf(l, "scorename=%[^\n]", buf)) { scorename = buf; }
        if(sscanf(l, "show_fps=%d", &i))     { show_fps = i!=0; }
        if(sscanf(l, "show_obj=%d", &i))     { show_obj = i!=0; }
        if(sscanf(l, "update=%d", &i))       { update = i!=0; }
        if(sscanf(l, "thread=%d", &i))       { threads = std::max<int>(1, std::min<int>(i, 8)); }
        if(sscanf(l, "port=%d", &i))         { port = i; }

        if(sscanf(l, "bgm=%d", &i))          { bgm_mute = i==0; }
        if(sscanf(l, "se=%d", &i))           { se_mute = i==0; }
        if(sscanf(l, "bgm_volume=%d", &i))   { bgm_volume = i; }
        if(sscanf(l, "se_volume=%d", &i))    { se_volume = i; }

        if(sscanf(l, "key0=%d", &i)) { key[0] = i; }
        if(sscanf(l, "key1=%d", &i)) { key[1] = i; }
        if(sscanf(l, "key2=%d", &i)) { key[2] = i; }
        if(sscanf(l, "key3=%d", &i)) { key[3] = i; }
        if(sscanf(l, "key4=%d", &i)) { key[4] = i; }
        if(sscanf(l, "key5=%d", &i)) { key[5] = i; }
        if(sscanf(l, "key6=%d", &i)) { key[6] = i; }
        if(sscanf(l, "key7=%d", &i)) { key[7] = i; }
        if(sscanf(l, "key8=%d", &i)) { key[8] = i; }
        if(sscanf(l, "key9=%d", &i)) { key[9] = i; }

        if(sscanf(l, "pad0=%d", &i)) { pad[0] = i; }
        if(sscanf(l, "pad1=%d", &i)) { pad[1] = i; }
        if(sscanf(l, "pad2=%d", &i)) { pad[2] = i; }
        if(sscanf(l, "pad3=%d", &i)) { pad[3] = i; }
        if(sscanf(l, "pad4=%d", &i)) { pad[4] = i; }
        if(sscanf(l, "pad5=%d", &i)) { pad[5] = i; }
        if(sscanf(l, "pad6=%d", &i)) { pad[6] = i; }

        if(sscanf(l, "controller=%d", &i)) { controller = i; }
        if(sscanf(l, "daxis1=%d", &i))     { daxis1 = i; }
        if(sscanf(l, "daxis2=%d", &i))     { daxis2 = i; }
        if(sscanf(l, "threshold1=%d", &i)) { threshold1 = i; }
        if(sscanf(l, "threshold2=%d", &i)) { threshold2 = i; }
        if(sscanf(l, "hat=%d", &i))        { hat = i!=0; }

        if(sscanf(l, "last_server=%[^\n]", buf)) { last_server = buf; }
        if(sscanf(l, "last_port=%d", &i))        { last_port = i; }
      }
      checkScorename();

      fclose(in);
    }

    boost::mutex& getMutex() { return m_mutex; }

    void checkScorename()
    {
      if(scorename.size()>12) {
        scorename.resize(12);
      }
      for(size_t i=0; i<scorename.size(); ++i) {
        if(scorename[i]==',') {
          scorename[i] = '.';
        }
      }
    }
  };


  struct ClearFlag : public IClearFlag
  {
  public:
    ClearFlag()
    {
      light = 0;
      normal = 0;
      heavy = 0;
      excess = 0;
      future = 0;

      load();
    }

    void save()
    {
      if(gzFile f = gzopen("flag", "wb")) {
        gzwrite(f, &light, sizeof(int));
        gzwrite(f, &normal, sizeof(int));
        gzwrite(f, &heavy,  sizeof(int));
        gzwrite(f, &excess, sizeof(int));
        gzwrite(f, &future, sizeof(int));
        gzclose(f);
      }
    }

    void load()
    {
      if(gzFile f = gzopen("flag", "rb")) {
        gzread(f, &light, sizeof(int));
        gzread(f, &normal, sizeof(int));
        gzread(f, &heavy,  sizeof(int));
        gzread(f, &excess, sizeof(int));
        gzread(f, &future, sizeof(int));
        gzclose(f);
      }
    }

    int* getModeFlag(int mode)
    {
      int *f = 0;
      if     (mode==LIGHT) { f=&light; }
      else if(mode==NORMAL){ f=&normal; }
      else if(mode==HEAVY) { f=&heavy; }
      else if(mode==EXCESS){ f=&excess; }
      else if(mode==FUTURE){ f=&future; }
      return f;
    }
  };

  struct GameOption
  {
  public:
    string record;
    int mode;
    int stage;
    int scene;
    int seed;
    int delay;

    GameOption() : mode(NORMAL), stage(0), scene(1), seed(::time(0)), delay(5)
    {}

    void serialize(ist::bostream& s) const
    {
      s << record << mode << stage << scene << seed << delay;
    }

    void deserialize(ist::bistream& s)
    {
      s >> record >> mode >> stage >> scene >> seed >> delay;
    }
  };

  struct PadState
  {
    char button[23];
    int move_x;
    int move_y;
    int dir_x;
    int dir_y;

    PadState() : move_x(0), move_y(0), dir_x(0), dir_y(0)
    {
      for(int i=0; i<23; ++i) {
        button[i] = 0;
      }
    }
  };
  PadState GetPadState();
  ushort GetMouseInput();
  ushort GetKeyboardInput();
  ushort GetJoystickInput();

  sgui::Window* GetTitleWindow();
  sgui::Window* GetGameWindow();

#ifdef EXCEPTION_ENABLE_PROFILE
  void AddUpdateTime(float v);
  void AddDrawTime(float v);
  void AddThreadTime(boost::thread::id tid, float v);
#endif // EXCEPTION_ENABLE_PROFILE 

  void Pause();
  void Resume();
  void Ending();
  void FadeToTitle();
  void FadeToGame(const GameOption& opt);
  void CreateContinuePanel();
  IGame* CreateGame(const GameOption& opt);
  IGame* CreateGame(Deserializer& s);
  void PushChatText(const string& t);

  void SaveState(const string& filename);
  void LoadState(const string& filename);
  void LoadState(Deserializer& s);

  SDL_Joystick* GetJoystick();
  const vector2& GetMousePosition();

} // namespace exception 


inline ist::bostream& operator<<(ist::bostream& b, const exception::GameOption& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, exception::GameOption& v)
{
  v.deserialize(b);
  return b;
}




#endif

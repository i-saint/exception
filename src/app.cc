#include "stdafx.h"
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
  #include <eh.h>
#endif // EXCEPTION_ENABLE_DATA_RESCUE 

#include <ist/random.h>
#include <ist/ist_sys.h>
#include <ist/wchar.h>
#include <sgui/sgui.h>
#include <SDL/SDL_mixer.h>

#include "input.h"
#include "game.h"
#include "network.h"
#include "character/creater.h"

namespace exception {


  void CreateTitlePanel(sgui::Window *w);
  void CreateGamePanel(IGame *game);
  IResource* CreateResource();



  class Selecter;
  class FadeoutWindow;

  class DebugDialog;
  class ObjBrowserDialog;
  class ProfileDialog;

  class ChatDialog;
  class ConfigDialog;
  class StartDialog;
  class RecordDialog;
  class StateDialog;

  class TitlePanel;
  class GamePanel;
  class PausePanel;
  class ContinuePanel;


  namespace {
    Selecter *g_selecter = 0;
    FadeoutWindow *g_fadeout_window = 0;

    DebugDialog *g_debug_dialog = 0;
    ObjBrowserDialog *g_objbrowser_dialog = 0;
    ProfileDialog *g_profile_dialog = 0;

    ChatDialog *g_chat_dialog = 0;
    ConfigDialog *g_config_dialog = 0;
    StartDialog *g_start_dialog = 0;
    RecordDialog *g_record_dialog = 0;
    StateDialog *g_state_dialog = 0;

    TitlePanel *g_title_panel = 0;
    GamePanel *g_game_panel = 0;
    PausePanel *g_pause_panel = 0;
    ContinuePanel *g_continue_panel = 0;

#if defined(EXCEPTION_ENABLE_NETRANKING) || defined(EXCEPTION_ENABLE_NETUPDATE) || defined(EXCEPTION_ENABLE_NETPLAY)
    boost::asio::io_service g_io_service;
#endif
  }


  enum {
    BU_EXIT = 100,
    BU_START,
    BU_RANKING,
    BU_CONFIG,

    DL_UPDATE,

    DL_START,
    BU_START_LOCAL,
    BU_START_SERVER,
    BU_START_CLIENT,
    ED_START_ADDRESS,
    ED_START_PORT,
    BU_START_CONNECT,
    LI_START_SERVERLIST,
    BU_START_CONNECTLIST,
    BU_START_LIGHT,
    BU_START_NORMAL,
    BU_START_HEAVY,
    BU_START_EXCESS,
    BU_START_FUTURE,
    BU_START_STAGE_ALL,
    BU_START_STAGE_1,
    BU_START_STAGE_2,
    BU_START_STAGE_3,
    BU_START_STAGE_4,
    BU_START_STAGE_5,
    BU_START_STAGE_EX,
    CB_START_SCENE,
    BU_START_START,

    DL_RECORD,
    LI_RECORD,
    BU_RECORD,
    BU_RECORD_OPEN,
    BU_RECORD_UPLOAD,
    BU_RECORD_DELETE,
    BU_RECORD_DELETE_CONFIRM,

    DL_STATE,
    LI_STATE,
    BU_STATE,
    BU_STATE_SAVE,
    BU_STATE_LOAD,
    BU_STATE_DELETE,
    BU_STATE_DELETE_CONFIRM,

    LI_RANKING,
    BU_RANKING_OPEN,
    BU_RANKING_LIGHT,
    BU_RANKING_NORMAL,
    BU_RANKING_HEAVY,
    BU_RANKING_EXCESS,
    BU_RANKING_FUTURE,
    BU_RANKING_TEST,
    BU_RANKING_STAGE_ALL,
    BU_RANKING_STAGE_1,
    BU_RANKING_STAGE_2,
    BU_RANKING_STAGE_3,
    BU_RANKING_STAGE_4,
    BU_RANKING_STAGE_5,
    BU_RANKING_STAGE_EX,


    DL_CONFIG,
    LI_RESOLUTION,
    BU_FULLSCREEN,
    BU_VSYNC,
    BU_EXLIGHT,
    BU_SHADER,
    BU_VERTEX_BUFFER,
    BU_SIMPLEBG,
    BU_FPS_30,
    BU_NOBLUR,
    BU_BLOOM_INC,
    BU_BLOOM_DEC,
    BU_THREAD_INC,
    BU_THREAD_DEC,
    ED_SCORENAME,
    BU_AUTOUPDATE,
    BU_SHOW_FPS,
    BU_SHOW_OBJ,
    BU_BGM,
    BU_BGM_VOLUME_UP,
    BU_BGM_VOLUME_DOWN,
    BU_SE,
    BU_SE_VOLUME_UP,
    BU_SE_VOLUME_DOWN,

    BU_PAUSE_RESUME,
    BU_PAUSE_CONFIG,
    BU_PAUSE_EXIT,
    BU_PAUSE_EXIT_CONFIRM,

    LI_DEBUG,
    BU_DEBUG_P,
    BU_DEBUG_DESTROY,
  };



  void PrintScreen(size_t width, size_t height)
  {
    std::vector<ist::bRGBA> buf(width*height);
    glReadBuffer(GL_FRONT);
    glReadPixels(0,0, width,height, GL_RGBA, GL_UNSIGNED_BYTE, &buf[0][0] );
    glReadBuffer(GL_BACK);

    ist::Bitmap bmp;
    bmp.resize(width, height);
    for(int i=0; i<height; ++i) {
      for(int j=0; j<width; ++j) {
        bmp[i][j] = buf[width*height - (i+1)*width + j];
        bmp[i][j].a = 255;
      }
    }
    char filename[128];
    sprintf(filename, "%d%d.png", ::time(0), sgui::GetTicks());
    bmp.save(filename);
  }


  PadState GetPadState()
  {
    PadState r;
    IConfig *conf = GetConfig();
    SDL_Joystick *joy = GetJoystick();
    if(!joy) {
      return r;
    }

    for(int i=0; i<16; ++i) {
      r.button[i] = SDL_JoystickGetButton(joy, i);
    }
    for(int i=0; i<3; ++i) {
      r.button[16+i*2+0] = SDL_JoystickGetAxis(joy, i+2) > conf->threshold1;
      r.button[16+i*2+1] = SDL_JoystickGetAxis(joy, i+2) <-conf->threshold1;
    }

    r.move_x = SDL_JoystickGetAxis(joy, 0);
    r.move_y = SDL_JoystickGetAxis(joy, 1);
    if(conf->daxis1<5) { r.dir_x = SDL_JoystickGetAxis(joy, conf->daxis1); }
    if(conf->daxis2<5) { r.dir_y = SDL_JoystickGetAxis(joy, conf->daxis2); }

    if(conf->hat) {
      Uint8 hat = SDL_JoystickGetHat(joy, 0);
      if     (hat==SDL_HAT_UP)       { r.move_y =-32767; }
      else if(hat==SDL_HAT_RIGHT)    { r.move_x = 32767; }
      else if(hat==SDL_HAT_DOWN)     { r.move_y = 32767; }
      else if(hat==SDL_HAT_LEFT)     { r.move_x =-32767; }
      else if(hat==SDL_HAT_RIGHTUP)  { r.move_x = 32767; r.move_y =-32767; }
      else if(hat==SDL_HAT_RIGHTDOWN){ r.move_x = 32767; r.move_y = 32767; }
      else if(hat==SDL_HAT_LEFTUP)   { r.move_x =-32767; r.move_y =-32767; }
      else if(hat==SDL_HAT_LEFTDOWN) { r.move_x =-32767; r.move_y = 32767; }
    }

    return r;
  }


  // Radeon様特別調査隊 
  bool IsValidDriver()
  {
    string card((char*)glGetString(GL_RENDERER));
    string version((char*)glGetString(GL_VERSION));
    boost::smatch m;
    if(boost::regex_search(card, m, boost::regex("Radeon"))) {
      if(boost::regex_search(version, m, boost::regex("(\\d+)\\.(\\d+)\\.(\\d+)"))) {
        int major = atoi(m.str(1).c_str());
        int minor = atoi(m.str(2).c_str());
        int patch = atoi(m.str(3).c_str());
        // 2.0.6645以上なら合格 
        if(major>2 || (major==2 && minor>0) || (major==2 && minor==0 && patch>=6645)) {
          return true;
        }
        else {
          return false;
        }
      }
    }
    return true;
  }

  bool IsVBOAvailable()
  {
    return GLEW_VERSION_1_5!=0;
  }

  bool IsNPTTextureAvailable()
  {
    return IsValidDriver() &&
           GLEW_ARB_texture_non_power_of_two;
  }

  bool IsGLSLAvailable()
  {
    return IsValidDriver() &&
           GLEW_ARB_shading_language_100 &&
           GLEW_ARB_shader_objects &&
           GLEW_ARB_vertex_shader &&
           GLEW_ARB_fragment_shader &&
           GLEW_EXT_framebuffer_object;
  }






  class ObjCounter : public sgui::Window
  {
  public:
    ObjCounter(sgui::Window *parent) :
      sgui::Window(parent)
    {
      setPosition(sgui::Point(580,20));
      setSize(sgui::Size(90,16));
      setDrawPriority(1.2f);
    }

    void draw()
    {
      if(GetConfig()->show_obj) {
        wchar_t buf[32];
        swprintf(buf, 32, L"obj: %d", exception::Object::getCount());
        drawText(buf, sgui::Rect(getSize()));
      }
    }
  };

  class FPSCounter : public sgui::Window
  {
  private:
    size_t m_pre_sec;
    size_t m_cur_fps;
    size_t m_fps;
    sgui::Label *m_label;

  public:
    FPSCounter(sgui::Window *parent) :
      sgui::Window(parent),
      m_pre_sec(::time(0)),
      m_cur_fps(0),
      m_fps(0),
      m_label(0)
    {
      setPosition(sgui::Point(580,5));
      setSize(sgui::Size(90,16));
      setDrawPriority(1.2f);
    }

    void draw()
    {
      ++m_cur_fps;
      size_t sec = size_t(::time(0));
      if(m_pre_sec!=sec) {
        m_fps = m_cur_fps;
        m_pre_sec = sec;
        m_cur_fps = 0;
      }

      if(GetConfig()->show_fps) {
        wchar_t buf[32];
        swprintf(buf, 32, L"fps: %d", getFPS());
        drawText(buf, sgui::Rect(getSize()));
      }
    }

    size_t getFPS() const { return m_fps; }
  };






#ifdef EXCEPTION_ENABLE_PROFILE

  class DebugDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::Button *m_bu_detail;
    sgui::Button *m_bu_pause;
    sgui::Button *m_bu_step;
    sgui::Label *m_message;

  public:
    DebugDialog(sgui::Window *parent) :
      Super(parent, sgui::Point(30,30), sgui::Size(360,250), L"debug")
    {
      if(g_debug_dialog) {
        throw Error("DebugDialog::DebugDialog()");
      }
      g_debug_dialog = this;

      m_bu_detail = new sgui::Button(this, sgui::Point(5,30), sgui::Size(45, 20), L"detail");
      m_bu_pause = new sgui::Button(this, sgui::Point(105,30), sgui::Size(45, 20), L"pause");
      m_bu_step = new sgui::Button(this, sgui::Point(155,30), sgui::Size(45, 20), L"step");
      m_message = new sgui::Label(this, sgui::Point(5,50), sgui::Size(350, 195), L"");
    }

    ~DebugDialog()
    {
      g_debug_dialog = 0;
    }

    void update()
    {
      Super::update();
      m_message->setText(_L(GetGame()->p()));
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_bu_detail) {
          printf(GetGame()->pDetail().c_str());
          fflush(stdout);
          return true;
        }
        else if(evt.getSrc()==m_bu_pause) {
          GetGame()->setPause(!IsPaused());
          return true;
        }
        else if(evt.getSrc()==m_bu_step) {
          GetGame()->step();
          return true;
        }
      }
      return sgui::Dialog::handleEvent(evt);
    }
  };


  class GObjItem : public sgui::ListItem
  {
  public:
    gid id;

    GObjItem(sgui::wstring l, gid _id) : sgui::ListItem(l), id(_id) {}
  };

  class ObjBrowserDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::List *m_list;
    sgui::Button *m_bu_update;

  public:
    ObjBrowserDialog(sgui::Window *parent) :
      Super(parent, sgui::Point(30,30), sgui::Size(360,400), L"object inspector")
    {
      if(g_objbrowser_dialog) {
        throw Error("ObjBrowserDialog::ObjBrowserDialog()");
      }
      g_objbrowser_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);

      m_bu_update = new sgui::Button(this, sgui::Point(5,30), sgui::Size(50, 20), L"update");
      new sgui::Button(this, sgui::Point(55,30), sgui::Size(50, 20), L"p", BU_DEBUG_P);
      new sgui::Button(this, sgui::Point(110,30), sgui::Size(50, 20), L"destroy", BU_DEBUG_DESTROY);
      m_list = new sgui::List(this, sgui::Point(5,55), sgui::Size(350, 300));
    }

    ~ObjBrowserDialog()
    {
      g_objbrowser_dialog = 0;
    }

    void updateList()
    {
      gobj_vector gv;
      gobj_iter& iter = GetMainTSM().getAllObjects();
      while(iter.has_next()) {
        gobj_ptr p = iter.iterate();
        if(!IsFraction(p)) {
          gv.push_back(p);
        }
      }
    //  std::sort(gv.begin(), gv.end(), greater_draw_priority());

      m_list->clearItem();
      for(size_t i=0; i<gv.size(); ++i) {
        m_list->addItem(new GObjItem(_L(typeid(*gv[i]).name()), gv[i]->getID()));
      }
    }

    void showInfo(gid id)
    {
      if(gobj_ptr p = GetObjectByID(id)) {
        new sgui::MessageDialog(L"object", _L(p->p()), getParent(), sgui::Point(50, 50));
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(evt.getSrc()==m_bu_update) {
          updateList();
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_P) {
          if(GObjItem *gi = dynamic_cast<GObjItem*>(m_list->getSelectedItem())) {
            showInfo(gi->id);
          }
          return true;
        }
        else if(evt.getSrc()->getID()==BU_DEBUG_DESTROY) {
          if(GObjItem *gi = dynamic_cast<GObjItem*>(m_list->getSelectedItem())) {
            if(gobj_ptr obj = GetObjectByID(gi->id)) {
              GetMainTSM().sendDestroyMessage(0, obj);
            }
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(GObjItem *gi = dynamic_cast<GObjItem*>(e.getItem())) {
          showInfo(gi->id);
          return true;
        }
      }
      return sgui::Dialog::handleEvent(evt);
    }
  };


  class Graph : public sgui::Window
  {
  private:
    boost::mutex m_mutex;
    size_t m_max_data;
    sgui::Color m_line_color;
    sgui::Range m_range;
    std::deque<float> m_dat;

  public:
    const sgui::Color& getLineColor() const { return m_line_color; }
    const sgui::Range& getRange() const { return m_range; }
    size_t getMaxData() const { return m_max_data; }
    void setLineColor(const sgui::Color& v) { m_line_color=v; }
    void setRange(const sgui::Range& v) { m_range=v; }
    void setMaxData(size_t v) { m_max_data=v; }

    void clearData() { m_dat.clear(); }
    void addData(float v)
    {
      boost::mutex::scoped_lock l(m_mutex);
      m_dat.push_front(v);
      if(m_dat.size()>m_max_data) {
        m_dat.pop_back();
      }
    }

    Graph(sgui::Window *w, const sgui::Point& pos, const sgui::Size& size) :
      sgui::Window(w, pos, size),
      m_max_data(100)
    {}

    void draw()
    {
      {
        ist::ModelviewMatrixSaver m_ms;
        ist::ProjectionMatrixSaver m_ps;

        ist::OrthographicCamera cam;
        cam.setPosition(vector4(0.0f, 0.0f, 500.0f));
        cam.setScreen(getSize().getWidth(),0, m_range.getMin(),m_range.getMax());
        cam.setZNear(-1.0f);
        cam.setZFar(1000.0f);
        cam.look();

        float half = (m_range.getMax()-m_range.getMin())/2.0f;
        glColor4fv(sgui::Color(1,1,1,0.2f).v);
        glBegin(GL_LINE_STRIP);
        glVertex2f(getSize().getWidth(), half);
        glVertex2f(0, half);
        glEnd();

        glColor4fv(getLineColor().v);
        glBegin(GL_LINE_STRIP);
        {
          boost::mutex::scoped_lock l(m_mutex);
          for(size_t i=0; i<m_dat.size(); ++i) {
            glVertex2f(float(i), m_dat[i]);
          }
        }
        glEnd();
      }
      glColor4fv(getBorderColor().v);
      sgui::DrawRectEdge(sgui::Rect(getSize()));
      glColor4f(1,1,1,1);
    }
  };

  class ProfileDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    typedef std::map<boost::thread::id, Graph*> graph_cont;
    Graph *m_update;
    Graph *m_draw;
    graph_cont m_async;

  public:
    ProfileDialog(sgui::Window *w) : 
      sgui::Dialog(w, sgui::Point(5,5), sgui::Size(500,400), L"profile"),
      m_update(0), m_draw(0)
    {
      if(g_profile_dialog) {
        throw Error("ProfileDialog::ProfileDialog()");
      }
      g_profile_dialog = this;

      setBackgroundColor(sgui::Color(0,0,0,0.6f));

      new sgui::Label(this, sgui::Point(5, 30), sgui::Size(55, 30), L"同期\n更新");
      m_update = new Graph(this, sgui::Point(45, 30), sgui::Size(400, 40));
      m_update->setRange(sgui::Range(0.0f, 20.0f));
      m_update->setLineColor(sgui::Color(0.4f, 0.4f, 1.0f));
      m_update->setMaxData(400);

      new sgui::Label(this, sgui::Point(5, 75), sgui::Size(55, 30), L"描画");
      m_draw = new Graph(this, sgui::Point(45, 75), sgui::Size(400, 40));
      m_draw->setRange(sgui::Range(0.0f, 20.0f));
      m_draw->setLineColor(sgui::Color(0.0f, 1.0f, 0.0f));
      m_draw->setMaxData(400);

      new sgui::Label(this, sgui::Point(5, 120), sgui::Size(55, 30), L"非同期\n更新");
      onThreadCountChange();

      toTopLevel();
    }

    ~ProfileDialog()
    {
      g_profile_dialog = 0;
    }

    void onThreadCountChange()
    {
      for(graph_cont::iterator i=m_async.begin(); i!=m_async.end(); ++i) {
        i->second->destroy();
      }
      m_async.clear();

      sgui::Size s(450, 120);
      ist::Scheduler *schedulr = ist::Scheduler::instance();
      for(size_t i=0; i<schedulr->getThreadCount()+1; ++i) {
        Graph *t = new Graph(this, sgui::Point(45, 75+45*(i+1)), sgui::Size(400, 40));
        t->setRange(sgui::Range(0.0f, 20.0f));
        t->setLineColor(sgui::Color(1.0f, 0.0f, 0.0f));
        t->setMaxData(400);
        if(i<schedulr->getThreadCount()) {
          m_async[schedulr->getThreadID(i)] = t;
        }
        else {
          m_async[boost::this_thread::get_id()] = t;
        }
        s+=sgui::Size(0, 45);
      }
      setSize(s);
    }

    void addUpdateTime(float v) { m_update->addData(v); }
    void addDrawTime(float v) { m_draw->addData(v); }
    void addThreadTime(boost::thread::id tid, float v) { m_async[tid]->addData(v); }
 };

  void AddUpdateTime(float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addUpdateTime(float(v)*0.5f);
    }
  }

  void AddDrawTime(float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addDrawTime(float(v)*0.5f);
    }
  }

  void AddThreadTime(boost::thread::id tid, float v)
  {
    if(g_profile_dialog) {
      g_profile_dialog->addThreadTime(tid, float(v)*0.5f);
    }
  }

#endif // EXCEPTION_ENABLE_PROFILE 




  void CreateChatDialog();

  class View : public sgui::View
  {
  private:
    FPSCounter *m_fps;
    ObjCounter *m_obj;

  public:
    View(const string& title, const sgui::Size& size, const sgui::Size& gsize=sgui::Size(), bool full=false) :
      sgui::View(title, size, gsize, full)
    {
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_EDIT_ENTER);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_COMBO);
      listenEvent(sgui::EVT_DD_RECIEVE);
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));

      m_fps = new FPSCounter(this);
      m_obj = new ObjCounter(this);
    }

    size_t getFPS() { return m_fps->getFPS(); }

    bool handleEvent(const sgui::Event& evt)
    {
      if(evt.getType()==sgui::EVT_KEYUP) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_PRINT) {
          sgui::Size size = getWindowSize();
          PrintScreen(size_t(size.getWidth()), size_t(size.getHeight()));
          return true;
        }
      }
      return sgui::View::handleEvent(evt);
    }
  };







  struct less_x_coord
  {
    bool operator()(sgui::Window *l, sgui::Window *r) {
      return l->getGlobalPosition().getX() < r->getGlobalPosition().getX();
    }
  };

  struct less_y_coord
  {
    bool operator()(sgui::Window *l, sgui::Window *r) {
      return l->getGlobalPosition().getY() < r->getGlobalPosition().getY();
    }
  };


  class Selecter : public sgui::Window
  {
  private:
    static Selecter *s_inst;
    typedef std::vector<sgui::Window*> cont;
    cont m_targets;
    cont m_targets_x;
    cont m_targets_y;
    sgui::Window *m_panel;
    sgui::Window *m_focus;
    int m_past;
    bool m_refresh;

  public:
    Selecter(sgui::Window *parent) :
      sgui::Window(parent), m_panel(0), m_focus(0), m_past(0), m_refresh(true)
    {
      g_selecter = this;

      listenEvent(sgui::EVT_CONSTRUCT);
      listenEvent(sgui::EVT_APP_DESTROY_WINDOW);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_KEYDOWN);
      listenEvent(sgui::EVT_JOY_AXIS);
      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_JOY_BUTTONDOWN);
      setDrawPriority(1.5f);

      setPosition(sgui::Point(260, 300));
      setSize(sgui::Size(120, 20));
    }

    ~Selecter()
    {
      g_selecter = 0;
    }

    void setRefreshFlag(bool v) { m_refresh=v; }
    void setFocus(sgui::Window *v) { m_focus=v; }
    sgui::Window* getFocus() { return m_focus; }

    void updateTargetsR(sgui::Window *w, sgui::Window *exclude)
    {
      if(!w || !w->isVisible() || w==exclude) {
        return;
      }
      sgui::Window::window_cont& wc = w->getChildren();
      for(sgui::Window::window_cont::iterator p=wc.begin(); p!=wc.end(); ++p) {
        sgui::Window *c = *p;
        if(c==exclude) {
          continue;
        }

        if(dynamic_cast<sgui::Button*>(c)) {
          m_targets.push_back(c);
        }
        else if(sgui::List *l=dynamic_cast<sgui::List*>(c)) {
          for(size_t i=0; i<l->getItemCount(); ++i) {
            m_targets.push_back(l->getItem(i));
          }
        }
        else if(sgui::Panel *p=dynamic_cast<sgui::Panel*>(c)) {
          if(p!=(sgui::Window*)g_chat_dialog) {
            m_panel = c;
            m_targets.clear();
            updateTargetsR(c, exclude);
          }
        }
        else if(typeid(*c)==typeid(sgui::Window&)) {
          updateTargetsR(c, exclude);
        }
      }
    }

    void updateTargets(sgui::Window *w, sgui::Window *exclude=0)
    {
      m_targets.clear();
      updateTargetsR(w, exclude);
      m_targets_x = m_targets;
      m_targets_y = m_targets;
      std::stable_sort(m_targets_x.begin(), m_targets_x.end(), less_x_coord());
      std::stable_sort(m_targets_y.begin(), m_targets_y.end(), less_y_coord());
      if(!m_focus) {
        m_focus = m_targets.empty() ? 0 : m_targets.front();
      }
    }


    sgui::Point GetCenter(sgui::Window *w)
    {
      if(!w) {
        return sgui::Point();
      }
      sgui::Size size = w->getSize();
      return w->getGlobalPosition()+sgui::Point(size.getWidth()/2.0f, size.getHeight()/2.0f);
    }

    void up()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_y.front()==m_focus) { m_focus=m_targets_y.back(); return; }

      int i = 0;
      for(; i<m_targets_y.size(); ++i) { if(m_targets_y[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i-1; i>=0; --i) {
        sgui::Point tpos = GetCenter(m_targets_y[i]);
        if(cpos.getY()>tpos.getY()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_y[i];
          }
        }
      }
      m_focus = n ? n : m_targets_y.back();
    }

    void down()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_y.back()==m_focus) { m_focus=m_targets_y.front(); return; }

      int i = 0;
      for(; i<m_targets_y.size(); ++i) { if(m_targets_y[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i+1; i<m_targets_y.size(); ++i) {
        sgui::Point tpos = GetCenter(m_targets_y[i]);
        if(cpos.getY()<tpos.getY()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_y[i];
          }
        }
      }
      m_focus = n ? n : m_targets_y.front();
    }

    void left()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_x.front()==m_focus) { m_focus=m_targets_x.back(); return; }

      int i = 0;
      for(; i<m_targets_x.size(); ++i) { if(m_targets_x[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i-1; i>=0; --i) {
        sgui::Point tpos = GetCenter(m_targets_x[i]);
        if(cpos.getX()>tpos.getX()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_x[i];
          }
        }
      }
      m_focus = n ? n : m_targets_x.back();
    }

    void right()
    {
      if(m_targets.empty()) { return; }
      if(m_targets_x.back()==m_focus) { m_focus=m_targets_x.front(); return; }

      int i = 0;
      for(; i<m_targets_x.size(); ++i) { if(m_targets_x[i]==m_focus) break;}

      float dist = 0;
      sgui::Window *n = 0;
      sgui::Point cpos = GetCenter(m_focus);
      for(i=i+1; i<m_targets_x.size(); ++i) {
        sgui::Point tpos = GetCenter(m_targets_x[i]);
        if(cpos.getX()<tpos.getX()) {
          float d = fabsf(cpos.getX()-tpos.getX()) + fabsf(cpos.getY()-tpos.getY());
          if(!n || d < dist) {
            dist = d;
            n = m_targets_x[i];
          }
        }
      }
      m_focus = n ? n : m_targets_x.front();
    }


    enum {
      SELECTER_UP,
      SELECTER_DOWN
    };

    void action(int t)
    {
      if(!m_focus) {
        return;
      }

      sgui::EventType type = t==SELECTER_UP ? sgui::EVT_MOUSE_BUTTONUP : sgui::EVT_MOUSE_BUTTONDOWN;
      sgui::Point cpos = sgui::View::instance()->toWindowCoord(GetCenter(m_focus));

      queueEvent(new sgui::MouseEvent(
        type, this, sgui::App::instance()->getMouseFocus(),
        cpos, sgui::Point(), sgui::MOUSE_LEFT));
    }

    void update()
    {
      sgui::Window::update();

      ++m_past;
      if(m_refresh) {
        updateTargets(m_panel ? m_panel : getParent());
        m_refresh = false;
      }
      if(m_focus) {
        sgui::Point pos = getPosition();
        pos+=(m_focus->getGlobalPosition()-pos)*0.4f;
        setPosition(pos);

        sgui::Size size = getSize();
        size+=(m_focus->getSize()-size)*0.4f;
        setSize(size);
      }


      static ushort s_input;
      ushort input = GetKeyboardInput() | GetJoystickInput();
      sgui::Window *prev_focus = m_focus;
      if     (!(s_input&(1<<0)) && (input&(1<<0))) { up(); }
      else if(!(s_input&(1<<1)) && (input&(1<<1))) { down(); }
      if     (!(s_input&(1<<3)) && (input&(1<<3))) { right(); }
      else if(!(s_input&(1<<2)) && (input&(1<<2))) { left(); }
      if     (!(s_input&(1<<4)) && (input&(1<<4))) { action(SELECTER_DOWN); }
      else if( (s_input&(1<<4)) &&!(input&(1<<4))) { action(SELECTER_UP); }
      s_input = input;

      if(m_focus!=prev_focus) {
        if(sgui::ListItem* li = dynamic_cast<sgui::ListItem*>(m_focus)) {
          sgui::List *ls = dynamic_cast<sgui::List*>(li->getParent());
          ls->setScrollPosition(sgui::Point(0, (ls->getScrollPosition()-li->getPosition()).getY()));
        }
        sgui::Point cpos = sgui::View::instance()->toWindowCoord(GetCenter(m_focus));
        queueEvent(new sgui::MouseEvent(sgui::EVT_MOUSE_MOVE, this, m_focus, cpos, sgui::Point(), sgui::MOUSE_NONE));
      }
    }

    void draw()
    {
      if(!m_focus) {
        return;
      }
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glColor4fv(float4(0.4f, 0.4f, 0.8f, (sinf(float(m_past)*ist::radian*5.0f)+1.0f)*0.3f+0.4f).v);
      sgui::DrawRect(sgui::Rect(getSize()));
      glColor4fv(float4(1.0f, 1.0f, 1.0f, 1.0f).v);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


    bool handleEvent(const sgui::Event& evt)
    {
      const IConfig& conf = *GetConfig();
      if(evt.getType()==sgui::EVT_CONSTRUCT) {
        if(dynamic_cast<sgui::Panel*>(evt.getSrc())) {
          m_focus = 0;
        }
        m_refresh = true;
        return true;
      }
      else if(evt.getType()==sgui::EVT_APP_DESTROY_WINDOW) {
        if(evt.getDst() && evt.getDst()->isInclude(m_focus)) {
          m_focus = 0;
        }
        while(evt.getDst() && evt.getDst()->isInclude(m_panel)) {
          m_panel = m_panel->getParent();
        }
        updateTargets(m_panel ? m_panel : getParent(), evt.getDst());
        return true;
      }

      return sgui::Window::handleEvent(evt);
    }
  };



#ifdef EXCEPTION_ENABLE_NETUPDATE
  namespace updater {
    int g_patch_version = EXCEPTION_VERSION;
  }

  class UpdaterThread : public Thread
  {
  public:
    UpdaterThread()
    {}

    void checkUpdate()
    {
      bool newer = false;
      ist::HTTPRequest req(g_io_service);
      string path;
#ifdef EXCEPTION_TRIAL
      path = "/exception/t/update/";
#else
      path = "/exception/r/update/";
#endif

#ifdef EXCEPTION_DEBUG
      path = "/exception/d/update/";
#endif
      if(req.get("i-saint.skr.jp", path)) {
        std::istream in(&req.getBuf());
        string l;
        while(std::getline(in, l)) {
          int version;
          char file[32];
          if(sscanf(l.c_str(), "%d, %s", &version, file)==2 && version>EXCEPTION_VERSION) {
            updater::g_patch_version = version;
          }
        }
      }
    }

    void exec()
    {
      try {
        checkUpdate();
      }
      catch(...) {
      }
    }
  };

  void ExecUpdater()
  {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ::CreateProcess(NULL, "updater.exe", NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS, NULL,NULL, &si, &pi);
    sgui::App::instance()->exit();
  }
#endif // EXCEPTION_ENABLE_NETUPDATE 


  class App : public sgui::App
  {
  typedef sgui::App Super;
  public:

    class AbnormalFrameRate : public std::exception {};
    class ChangeFrameRate : public std::exception {};

#ifdef EXCEPTION_ENABLE_DATA_RESCUE
    static void se_handler(unsigned int code, struct _EXCEPTION_POINTERS* ep)
    {
      throw Win32Exception(ep);
    }
#endif

  private:
    boost::intrusive_ptr<IResource> m_res;
    boost::shared_ptr<ist::Scheduler> m_scheduler;
    SDL_Joystick *m_joy;
    Config *m_conf;
    ClearFlag *m_flag;
    vector2 m_mousepos;

#ifdef EXCEPTION_ENABLE_NETUPDATE
    UpdaterThread *m_updater;
#endif // EXCEPTION_ENABLE_NETUPDATE 

  public:
    App(int argc, char **argv) : sgui::App(argc, argv), m_joy(0), m_conf(0), m_flag(0)
    {
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
      _set_se_translator(&App::se_handler);
#endif // EXCEPTION_ENABLE_DATA_RESCUE 

      m_conf = new Config();
      m_conf->load();

      m_flag = new ClearFlag();
      m_flag->load();

      updateScheduler();


#ifdef EXCEPTION_ENABLE_NETUPDATE
      m_updater = 0;
      if(m_conf->update) {
        m_updater = new UpdaterThread();
        m_updater->run();
      }
#endif // EXCEPTION_ENABLE_NETUPDATE 

      if(SDL_Init(SDL_INIT_AUDIO)<0) {
        throw Error(SDL_GetError());
      }

      if(Mix_OpenAudio(44100, AUDIO_S16, 2, 4096)<0) {
      }
      else {
        IConfig& c = *GetConfig();
        c.sound = true;
        Mix_VolumeMusic(c.bgm_mute ? 0 : c.bgm_volume);
        Mix_Volume(-1, c.se_mute ? 0 : c.se_volume);
      }

      m_joy = SDL_JoystickOpen(m_conf->controller);

      setDefaultFont("resource/VL-Gothic-Regular.ttf");
      SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, int(m_conf->vsync && !m_conf->fps_30));
      setView(new View("exception", sgui::Size(float(m_conf->width), float(m_conf->height)), sgui::Size(640, 480), m_conf->fullscreen));

      glewInit();
      printSystemInfo();

      if(!IsGLSLAvailable() || SDL_GetVideoInfo()->vfmt->BitsPerPixel!=32) {
        getConfig()->shader = false;
      }
      if(!IsNPTTextureAvailable()) {
        getConfig()->npttexture = false;
      }
      if(!IsVBOAvailable()) {
        getConfig()->vertex_buffer = false;
      }

      m_res = CreateResource();

      setDefaultBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.8f));
      new Selecter(sgui::View::instance());
      CreateTitlePanel(sgui::View::instance());
    }

    ~App()
    {
#ifdef EXCEPTION_ENABLE_NETUPDATE
      delete m_updater;
#endif // EXCEPTION_ENABLE_NETUPDATE 

      View::instance()->releaseChildren();
      m_res = 0;

      g_iclient.reset();
      g_iserver.reset();

      save();
      delete m_conf;
      delete m_flag;

      m_scheduler.reset();

      if(SDL_JoystickOpened(0)) {
        SDL_JoystickClose(m_joy);
      }
      Mix_CloseAudio();
    }

    void save()
    {
      if(m_conf) { m_conf->save(); }
      if(m_flag) { m_flag->save(); }
      if(IGame *game = GetGame()) { game->write(); }
    }

    void printSystemInfo()
    {
      ist::GLInfo glinfo;
      glinfo.print(std::cout);

      printf("joystick\n");
      int joy = SDL_NumJoysticks();
      for(int i=0; i<joy; ++i) {
        printf("Joystick %d: %s\n", i, SDL_JoystickName(i));
      }
      printf("\n");
    }

    void updateScheduler()
    {
      if(m_scheduler) {
        m_scheduler->waitForAll();
        m_scheduler.reset();
      }
      m_scheduler.reset(new ist::Scheduler(m_conf->threads));
      if(IGame *g = GetGame()) {
        g->onThreadCountChange();
      }
#ifdef EXCEPTION_ENABLE_PROFILE
      if(g_profile_dialog) {
        g_profile_dialog->onThreadCountChange();
      }
#endif // EXCEPTION_ENABLE_PROFILE 
    }


    Config* getConfig() { return m_conf; }
    ClearFlag* getClearFlag() { return m_flag; }
    SDL_Joystick* getJoystick() { return m_joy; }

    void update()
    {
      if(g_iclient && !GetGame()) {
        g_iclient->sync();
      }
      Super::update();
    }

    void loopVSYNC()
    {
      while(!m_end_flag) {
        update();
        draw();

        View *v = static_cast<View*>(sgui::View::instance());
        if(m_conf->vsync && v->getFPS()>100) {
          throw AbnormalFrameRate();
        }
        else if(m_conf->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }

    void loop30FPS()
    {
      Uint32 frame = 0;
      Uint32 interval = 100;
      Uint32 pre_frame = sgui::GetTicks()*3;
      while(!m_end_flag) {
        bool wait = false;
        while(sgui::GetTicks()*3-pre_frame < interval) { wait=true; /* busy loop */ }
        if(wait) {
          pre_frame+=interval;
        }
        else {
          pre_frame = sgui::GetTicks()*3; // 処理落ちしている場合 
        }

        update();
        update();
        draw();

        if(!m_conf->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }

    void loop60FPS()
    {
      Uint32 interval = 50;
      Uint32 pre_frame = sgui::GetTicks()*3;
      while(!m_end_flag) {
        bool wait = false;
        while(sgui::GetTicks()*3-pre_frame < interval) { wait=true; /* busy loop */ }
        if(wait) {
          pre_frame+=interval;
        }
        else {
          pre_frame = sgui::GetTicks()*3; // 処理落ちしている場合 
        }

        update();
        draw();

        if(m_conf->fps_30) {
          throw ChangeFrameRate();
        }
      }
    }


    void loop()
    {
      for(;;) {
        try {
          if(m_conf->vsync && !m_conf->fps_30) {
            loopVSYNC();
          }
          else if(m_conf->fps_30) {
            loop30FPS();
          }
          else {
            loop60FPS();
          }
          break;
        }
        catch(const AbnormalFrameRate&) {
          m_conf->vsync = false;
        }
        catch(const ChangeFrameRate&) {
        }
#ifdef EXCEPTION_ENABLE_DATA_RESCUE
        catch(const Win32Exception& e) {
          printf("win32 exception: %s", e.what());
          save();
          throw;
        }
#endif // EXCEPTION_ENABLE_DATA_RESCUE 
      }
    }

    bool handleEvent(const sgui::Event& e)
    {
      if(e.getType()==sgui::EVT_MOUSE_MOVE) {
        const sgui::MouseEvent& m = dynamic_cast<const sgui::MouseEvent&>(e);
        sgui::Point p = m.getPosition();
        m_mousepos = vector2(p.getX(), p.getY());
        return true;
      }
      return Super::handleEvent(e);
    }

    const vector2& getMousePosition() { return m_mousepos; }
    IResource* getResource() { return m_res.get(); }
  };

  App* GetApp() { return static_cast<App*>(sgui::App::instance()); }
  const vector2& GetMousePosition() { return GetApp()->getMousePosition(); }
  IResource* GetResource() { return GetApp()->getResource(); }
  SDL_Joystick* GetJoystick() { return GetApp()->getJoystick(); }

  Config* GetConfigRW() { return GetApp()->getConfig(); }
  IConfig* GetConfig() { return GetApp()->getConfig(); }
  IClearFlag* GetClearFlag() { return GetApp()->getClearFlag(); }

  sgui::App* CreateApp(int argc, char *argv[]) { return new App(argc, argv); }






  class ChatDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  public:
    struct Text
    {
      wstring text;

      Text(const wstring& t) : text(t) {}
    };

    typedef std::list<Text> text_cont;

  private:
    text_cont m_log;
    sgui::Edit *m_textbox;

  public:
    ChatDialog() : Super(sgui::View::instance(), sgui::Point(0, 60), sgui::Size(240, 360), L"chat")
    {
      if(g_chat_dialog) {
        throw Error("ChatDialog::ChatDialog()");
      }
      g_chat_dialog = this;

      sgui::Color bg(0, 0, 0, 0.1f);
      sgui::Color border(1, 1, 1, 0.5f);

      listenEvent(sgui::EVT_EDIT_ENTER);
      setBackgroundColor(bg);
      setBorderColor(border);

      m_textbox = new sgui::Edit(this, sgui::Point(95, 340), sgui::Size(140, 16));
      m_textbox->setBackgroundColor(bg);
      m_textbox->setBorderColor(border);

      m_close_button->setBackgroundColor(bg);
      m_close_button->setBorderColor(border);

      new sgui::Label(this, sgui::Point(5, 340), sgui::Size(90, 16), _L(GetConfig()->scorename+":"));
    }

    ~ChatDialog()
    {
      g_chat_dialog = 0;
    }

    void focus()
    {
      m_textbox->focus();
    }

    void pushText(const string& mes)
    {
      m_log.push_front(Text(sgui::_L(mes)));
      if(m_log.size()>14) {
        m_log.pop_back();
      }
    }

    void draw()
    {
      Super::draw();

      char buf[64];
      sgui::Point base;
      base = sgui::Point(5, 30);
      for(size_t i=0; i<GetSessionCount(); ++i) {
        session_ptr s = GetSession(i);
        sprintf(buf, "%s: ping %d", s->getName().c_str(), s->getPing());
        DrawText(_L(buf), base);
        base.setY(base.getY()+16.0f);
      }

      base = sgui::Point(5, 320);
      for(text_cont::iterator p=m_log.begin(); p!=m_log.end(); ++p) {
        DrawText(p->text, base);
        base.setY(base.getY()-16.0f);
      }
    }

    bool handleEvent(const sgui::Event &evt)
    {
      if(evt.getType()==sgui::EVT_EDIT_ENTER && evt.getSrc()==m_textbox) {
        const sgui::EditEvent& e = dynamic_cast<const sgui::EditEvent&>(evt);
        if(!m_textbox->getText().empty()) {
          SendNMessage(NMessage::Text(0, _S(m_textbox->getText())));
          m_textbox->setText(L"");
        }
        return true;
      }
      return Super::handleEvent(evt);
    }
  };

  void CreateChatDialog()
  {
    if(!g_chat_dialog) {
      new ChatDialog();
      if(g_title_panel) {
        g_chat_dialog->setParent((sgui::Window*)g_title_panel);
      }
      else if(g_game_panel) {
        g_chat_dialog->setParent((sgui::Window*)g_game_panel);
      }
    }
  }

  void PushChatText(const string& t)
  {
    CreateChatDialog();
    g_chat_dialog->pushText(t);
  }




  void show_error(const std::string& mes)
  {
    fprintf(stderr, mes.c_str());
    new sgui::MessageDialog(_L("exception caught"), _L(mes), sgui::View::instance(), sgui::Point(120, 90), sgui::Size(400, 300));
  }




  class ConfigDialog : public sgui::Dialog
  {
  private:
    sgui::Label *m_bloom;
    sgui::Label *m_thread;
    sgui::Label *m_bgm_vol;
    sgui::Label *m_se_vol;

  public:
    ConfigDialog(sgui::Window *parent) :
      sgui::Dialog(parent, sgui::Point(30,30), sgui::Size(360,270), L"config", DL_CONFIG)
    {
      if(g_config_dialog) {
        throw Error("ConfigDialog::ConfigDialog()");
      }
      g_config_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_BUTTON_DOWN);
      listenEvent(sgui::EVT_LIST_SELECT);
      listenEvent(sgui::EVT_EDIT);

      sgui::Label *label;
      sgui::ToggleButton *tb;
      sgui::Button *b;

      sgui::Point base(5, 30);

      label = new sgui::Label(this, base, sgui::Size(90, 16), L"画面解像度");
      new sgui::ToolTip(label, L"画面解像度を指定します。\nゲーム再起動後反映されます。");
      base+=sgui::Point(0, 20);
      sgui::List *list = new sgui::List(this, base, sgui::Size(90,100), LI_RESOLUTION);
      sgui::Size res[6] = {
        sgui::Size(640,480),
        sgui::Size(800,600),
        sgui::Size(1024,768),
        sgui::Size(1280,800),
        sgui::Size(1280,960),
        sgui::Size(1280,1024),
      };

      Config& conf = *GetConfigRW();
      char buf[32];
      for(size_t i=0; i<6; ++i) {
        size_t w = size_t(res[i].getWidth());
        size_t h = size_t(res[i].getHeight());
        sprintf(buf, "%dx%d", w, h);
        list->addItem(new sgui::ListItem(_L(buf)));
        if(w==conf.width && h==conf.height) {
          list->setSelection(i);
        }
      }

      base+=sgui::Point(0, 110);
      tb = new sgui::ToggleButton(this, base, sgui::Size(90, 20), L"フルスクリーン", BU_FULLSCREEN);
      new sgui::ToolTip(tb, L"指定解像度、32bitカラー、\nリフレッシュレート60Hzのフルスクリーンモードでの起動を試みます。\nゲーム再起動後反映されます。");
      if(conf.fullscreen) { tb->setButtonState(sgui::Button::DOWN); }



      base = sgui::Point(110, 30);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"VSync", BU_VSYNC);
      new sgui::ToolTip(tb, L"画面の更新間隔をモニタのリフレッシュレートに合わせ、\n滑らかに画面を更新します。\nリフレッシュレートが60Hzの場合や\nフルスクリーンモードの場合に効果的です。\nゲーム再起動後反映されます。");
      if(conf.vsync) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"頂点バッファ", BU_VERTEX_BUFFER);
      new sgui::ToolTip(tb, L"対応していれば頂点バッファを使用します。\n描画の若干の高速化が期待できます。\nゲーム再起動後反映されます。");
      if(conf.vertex_buffer) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"追加光源", BU_EXLIGHT);
      new sgui::ToolTip(tb, L"敵破壊時等に光源を設置し、周囲を明るくします。\nオフにすると大分見た目が寂しくなってしまいますが、\n若干処理負荷が軽減します。");
      if(conf.exlight) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"ブラー無効化", BU_NOBLUR);
      new sgui::ToolTip(tb, L"でかい敵を破壊したときなどに発生する、\n画面が歪む感じのエフェクト類を無効にします。\n若干処理負荷が軽減されますが、見た目が寂しくなります。");
      if(conf.noblur) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"30FPS", BU_FPS_30);
      new sgui::ToolTip(tb, L"描画の頻度を半分にします。\nどうしても動作が重い場合有効にしてみてください。\nまた、これを有効にしている場合、\nVSyncの設定を無視してタイマーで待つようになります。");
      if(conf.fps_30) { tb->setButtonState(sgui::Button::DOWN); }


      base+=sgui::Point(0, 30);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"シェーダ", BU_SHADER);
      new sgui::ToolTip(tb, L"対応していれば各種シェーダを使用します。\n発光エフェクトや動的テクスチャを使ったエフェクト等\nが有効になり、見た目が派手になりますが、\nその分処理負荷も大きくなります。");
      if(conf.shader) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(120, 20), L"背景簡略化", BU_SIMPLEBG);
      new sgui::ToolTip(tb, L"背景へのシェーダの使用を無効化します。\nシェーダ有効時の背景は処理負荷が高いので、\n動作が遅い場合まずはこれを変えてみるといいでしょう。\nシェーダを使わない設定の場合変化はありません。");
      if(conf.simplebg) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      new sgui::Label(this, base, sgui::Size(60, 20), L"ブルーム");
      sprintf(buf, "%.1f", conf.bloom);
      b = new sgui::Button(this, base+sgui::Point(55, 0), sgui::Size(20, 20), L"<", BU_BLOOM_DEC);
      new sgui::ToolTip(b, L"シェーダ有効時の、明るい部分をぼかして\n眩しい感じにするエフェクトの強弱を設定します。\nシェーダを使わない設定の場合変化はありません。");
      m_bloom = new sgui::Label(this, base+sgui::Point(80, 0), sgui::Size(20, 20), _L(buf));
      b = new sgui::Button(this, base+sgui::Point(100, 0), sgui::Size(20, 20), L">", BU_BLOOM_INC);
      new sgui::ToolTip(b, L"シェーダ有効時の、明るい部分をぼかして\n眩しい感じにするエフェクトの強弱を設定します。\nシェーダを使わない設定の場合変化はありません。");

      base+=sgui::Point(0, 30);
      new sgui::Label(this, base, sgui::Size(60, 20), L"並列処理");
      sprintf(buf, "%d", conf.threads);
      b = new sgui::Button(this, base+sgui::Point(55, 0), sgui::Size(20, 20), L"<", BU_THREAD_DEC);
      new sgui::ToolTip(b, L"内部処理を何スレッドに分けるかを指定します。\nデフォルト最適になるようにしていますが、\n動作が異常に重いとき等は調整してみると変化があるかもしれません。\n大抵はCPUのコア数と同数の時最も効果があります。");
      m_thread = new sgui::Label(this, base+sgui::Point(80, 0), sgui::Size(20, 20), _L(buf));
      b = new sgui::Button(this, base+sgui::Point(100, 0), sgui::Size(20, 20), L">", BU_THREAD_INC);
      new sgui::ToolTip(b, L"内部処理を何スレッドに分けるかを指定します。\nデフォルト最適になるようにしていますが、\n動作が異常に重いとき等は調整してみると変化があるかもしれません。\n大抵はCPUのコア数と同数の時最も効果があります。");



      base = sgui::Point(250, 30);
      new sgui::Label(this, base, sgui::Size(100, 16), L"スコアネーム:");
      base+=sgui::Point(0, 16);
      sgui::Edit *ed = new sgui::Edit(this, base, sgui::Size(100, 16), _L(conf.scorename), ED_SCORENAME);
      ed->setBackgroundColor(sgui::Color(1,1,1,0.2f));

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"自動アップデート", BU_AUTOUPDATE);
      new sgui::ToolTip(tb, L"ゲーム起動時にWeb経由で最新パッチの有無を調べます。");
      if(conf.update) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"FPS表示", BU_SHOW_FPS);
      new sgui::ToolTip(tb, L"フレームレートを表示します。");
      if(conf.show_fps) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(100, 20), L"OBJ数表示", BU_SHOW_OBJ);
      new sgui::ToolTip(tb, L"ゲーム内のオブジェクトの数を表示します。");
      if(conf.show_obj) { tb->setButtonState(sgui::Button::DOWN); }

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(30, 20), L"BGM", BU_BGM);
      if(!conf.bgm_mute) { tb->setButtonState(sgui::Button::DOWN); }
      b = new sgui::Button(this, base+sgui::Point(35, 0), sgui::Size(20, 20), L"<", BU_BGM_VOLUME_DOWN);
      b = new sgui::Button(this, base+sgui::Point(80, 0), sgui::Size(20, 20), L">", BU_BGM_VOLUME_UP);
      sprintf(buf, "%d", conf.bgm_volume);
      m_bgm_vol = new sgui::Label(this, base+sgui::Point(60, 0), sgui::Size(30, 20), _L(buf));

      base+=sgui::Point(0, 25);
      tb = new sgui::ToggleButton(this, base, sgui::Size(30, 20), L"SE", BU_SE);
      if(!conf.se_mute) { tb->setButtonState(sgui::Button::DOWN); }
      b = new sgui::Button(this, base+sgui::Point(35, 0), sgui::Size(20, 20), L"<", BU_SE_VOLUME_DOWN);
      b = new sgui::Button(this, base+sgui::Point(80, 0), sgui::Size(20, 20), L">", BU_SE_VOLUME_UP);
      sprintf(buf, "%d", conf.se_volume);
      m_se_vol = new sgui::Label(this, base+sgui::Point(60, 0), sgui::Size(30, 20), _L(buf));

      toTopLevel();
    }

    ~ConfigDialog()
    {
      g_config_dialog = 0;
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;
      Config& conf = *GetConfigRW();

      char buf[32];
      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_FULLSCREEN)        { conf.fullscreen=false; return true; }
        else if(id==BU_VSYNC)        { conf.vsync=false; return true; }
        else if(id==BU_EXLIGHT)      { conf.exlight=false; return true; }
        else if(id==BU_SHADER)       { conf.shader=false; return true; }
        else if(id==BU_VERTEX_BUFFER){ conf.vertex_buffer=false; return true; }
        else if(id==BU_SIMPLEBG)     { conf.simplebg=false; return true; }
        else if(id==BU_FPS_30)        { conf.fps_30=false; return true; }
        else if(id==BU_NOBLUR)       { conf.noblur = false; return true; }
        else if(id==BU_AUTOUPDATE)   { conf.update=false; return true; }
        else if(id==BU_SHOW_FPS)     { conf.show_fps=false; return true; }
        else if(id==BU_SHOW_OBJ)     { conf.show_obj=false; return true; }
        else if(id==BU_SE) {
          conf.se_mute = true;
          Mix_Volume(-1, 0);
          return true;
        }
        else if(id==BU_BGM) {
          conf.bgm_mute = true;
          Mix_VolumeMusic(0);
          return true;
        }
        else if(id==BU_BGM_VOLUME_UP) {
          conf.bgm_volume = std::max<int>(0, std::min<int>(128, conf.bgm_volume+16));
          sprintf(buf, "%d", conf.bgm_volume);
          m_bgm_vol->setText(_L(buf));
          if(conf.sound && !conf.bgm_mute) {
            Mix_VolumeMusic(conf.bgm_volume);
          }
        }
        else if(id==BU_BGM_VOLUME_DOWN) {
          conf.bgm_volume = std::max<int>(0, std::min<int>(128, conf.bgm_volume-16));
          sprintf(buf, "%d", conf.bgm_volume);
          m_bgm_vol->setText(_L(buf));
          if(conf.sound && !conf.bgm_mute) {
            Mix_VolumeMusic(conf.bgm_volume);
          }
        }
        else if(id==BU_SE_VOLUME_UP) {
          conf.se_volume = std::max<int>(0, std::min<int>(128, conf.se_volume+16));
          sprintf(buf, "%d", conf.se_volume);
          m_se_vol->setText(_L(buf));
          if(conf.sound && !conf.se_mute) {
            Mix_Volume(-1, conf.se_volume);
          }
        }
        else if(id==BU_SE_VOLUME_DOWN) {
          conf.se_volume = std::max<int>(0, std::min<int>(128, conf.se_volume-16));
          sprintf(buf, "%d", conf.se_volume);
          m_se_vol->setText(_L(buf));
          if(conf.sound && !conf.se_mute) {
            Mix_Volume(-1, conf.se_volume);
          }
        }
        else if(id==BU_THREAD_DEC) {
          if(conf.threads > 1) {
            conf.threads--;
            sprintf(buf, "%d", conf.threads);
            m_thread->setText(_L(buf));
            dynamic_cast<App&>(*App::instance()).updateScheduler();
          }
          return true;
        }
        else if(id==BU_THREAD_INC) {
          if(conf.threads < 8) {
            conf.threads++;
            char buf[16];
            sprintf(buf, "%d", conf.threads);
            m_thread->setText(_L(buf));
            dynamic_cast<App&>(*App::instance()).updateScheduler();
          }
          return true;
        }
        else if(id==BU_BLOOM_DEC) {
          conf.bloom = std::max<float>(0.0f, conf.bloom-0.1f);
          sprintf(buf, "%.1f", conf.bloom);
          m_bloom->setText(_L(buf));
          return true;
        }
        else if(id==BU_BLOOM_INC) {
          conf.bloom = std::min<float>(1.0f, conf.bloom+0.1f);
          sprintf(buf, "%.1f", conf.bloom);
          m_bloom->setText(_L(buf));
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(id==BU_FULLSCREEN)     { conf.fullscreen=true; return true; }
        else if(id==BU_VSYNC)     { conf.vsync=true; return true; }
        else if(id==BU_FPS_30)     { conf.fps_30=true; return true; }
        else if(id==BU_SIMPLEBG)  { conf.simplebg=true; return true; }
        else if(id==BU_EXLIGHT)   { conf.exlight=true; return true; }
        else if(id==BU_NOBLUR)    { conf.noblur=true; return true; }
        else if(id==BU_AUTOUPDATE){ conf.update=true; return true; }
        else if(id==BU_SHOW_FPS)  { conf.show_fps=true; return true; }
        else if(id==BU_SHOW_OBJ)  { conf.show_obj=true; return true; }
        else if(id==BU_VERTEX_BUFFER) {
          sgui::ToggleButton *tb = static_cast<sgui::ToggleButton*>(evt.getSrc());
          if(!IsVBOAvailable()) {
            new sgui::MessageDialog(
              L"error",
              L"未対応の環境のようです。\nこの機能を使用するにはOpenGL1.5に対応している必要があります。",
              this, sgui::Point(50,50), sgui::Size(300, 150));
            tb->setButtonState(sgui::Button::UP);
          }
          else {
            conf.vertex_buffer = true;
          }
          return true;
        }
        else if(id==BU_SHADER) {
          sgui::ToggleButton *tb = static_cast<sgui::ToggleButton*>(evt.getSrc());
          if(!IsGLSLAvailable()) {
            new sgui::MessageDialog(
              L"error",
              L"未対応の環境のようです。\nこの機能を使用するにはOpenGL2.0およびGL_EXT_framebuffer_object拡張に対応している必要があります。",
              this, sgui::Point(50,50), sgui::Size(300, 150));
            tb->setButtonState(sgui::Button::UP);
          }
          else if(SDL_GetVideoInfo()->vfmt->BitsPerPixel!=32) {
            new sgui::MessageDialog(
              L"warning",
              L"画面の色が32bitではありません。\n32bitカラーでない状態でシェーダを使う場合、極端に動作が遅くなる可能性があります。\n32bitカラーにするか、フルスクリーンモードにすることで対処可能です。",
              this, sgui::Point(50,50), sgui::Size(300, 150));
            conf.shader = true;
          }
          else {
            conf.shader = true;
          }
          return true;
        }
        else if(id==BU_SE) {
          conf.se_mute = false;
          Mix_Volume(-1, conf.se_volume);
          return true;
        }
        else if(id==BU_BGM) {
          conf.bgm_mute = false;
          Mix_VolumeMusic(conf.bgm_volume);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_SELECT) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_RESOLUTION) {
          int w,h;
          string text = _S(e.getItem()->getText());
          if(sscanf(text.c_str(), "%dx%d", &w, &h)==2) {
            conf.width = w;
            conf.height = h;
            return true;
          }
        }
      }
      else if(evt.getType()==sgui::EVT_EDIT) {
        const sgui::EditEvent& e = dynamic_cast<const sgui::EditEvent&>(evt);
        if(id==ED_SCORENAME) {
          boost::mutex::scoped_lock lock(conf.getMutex());

          conf.scorename = _S(e.getString());
          conf.checkScorename();
          e.getSrc()->setText(_L(conf.scorename));
        }
      }
      return sgui::Dialog::handleEvent(evt);
    }
  };


  class FadeoutWindow : public sgui::Window
  {
  typedef sgui::Window Super;
  private:
    int m_fadetime;

  public:
    FadeoutWindow(sgui::Window *parent) :
      Super(parent, sgui::Point(), parent->getSize()), m_fadetime(60)
    {
      if(g_fadeout_window) {
        throw Error("FadeWindow::FadeWindow()");
      }
      g_fadeout_window = this;

      setBackgroundColor(sgui::Color(0,0,0,0));
      setDrawPriority(1.5f);

      IMusic::Resume();
    }

    ~FadeoutWindow()
    {
      g_fadeout_window = 0;
    }

    void setFadeTime(int frame) { m_fadetime=frame; }

    virtual void action()=0;

    void update()
    {
      Super::update();
      sgui::Color c = getBackgroundColor();
      c.a+=(1.0f/m_fadetime);
      setBackgroundColor(c);

      if(c.a>1.0f) {
        action();
      }
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);
    }
  };


  class EndingWindow : public sgui::Window
  {
  typedef sgui::Window Super;
  private:
    int m_fadetime;
    int m_frame;
    int m_mcount[3];
    wstring m_message[3];

  public:
    EndingWindow(sgui::Window *parent) :
      Super(parent, sgui::Point(), parent->getSize()), m_fadetime(180), m_frame(0)
    {
      setBackgroundColor(sgui::Color(0,0,0,0));
      setDrawPriority(1.5f);

      for(int i=0; i<3; ++i) { m_mcount[i]=0; }
      m_message[0] = L"an unhandled exception occurred.";
      m_message[1] = L"the program will be terminated.";
      m_message[2] = L"the end.";
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(-1,-1), getSize()+sgui::Size(2,2)));
      glColor4f(1,1,1,1);

      sgui::Point pos[3] = {
        sgui::Point(20, 20),
        sgui::Point(20, 40),
        sgui::Point(20, 80),
      };
      for(int i=0; i<3; ++i) {
        wstring tmp = wstring(m_message[i].c_str(), m_mcount[i]);
        drawText(tmp, sgui::Rect(pos[i], getSize()));
      }
    }

    void action()
    {
      ++m_frame;
      int start[] = {30, 100, 200};
      for(int i=0; i<3; ++i) {
        if(m_frame>start[i]) {
          m_mcount[i] = std::min<int>(m_mcount[i]+1, m_message[i].size());
        }
      }
      if(m_frame==320) {
        SendNMessage(NMessage::End());
      }
    }

    void update()
    {
      Super::update();
      sgui::Color c = getBackgroundColor();
      c.a+=(1.0f/m_fadetime);
      setBackgroundColor(c);

      if(c.a>1.0f) {
        action();
      }
    }
  };


  class FadeToGameWindow : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  private:
    GameOption m_opt;

  public:
    FadeToGameWindow(sgui::Window *parent, const GameOption& opt) : Super(parent), m_opt(opt)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void read()
    {
      ist::gzbstream s(m_opt.record, "rb");
      if(!s) {
        throw Error("リプレイデータを開けませんでした。");
      }

      int version = 0;
      ulong seed = 0;
      size_t fps;
      size_t score;

      s >> version;
      if(version!=EXCEPTION_REPLAY_VERSION && version!=105) {
        throw Error("不正な形式のリプレイデータです。");
      }

      s >> m_opt.mode >> m_opt.stage >> m_opt.scene >> score >> m_opt.seed >> fps;
      g_iclient.reset(new InputClientReplay(s, version));
    }

    void action()
    {
      try {
        IMusic::WaitFadeOut();

        if(!m_opt.record.empty()) {
          read();
          if(m_opt.record=="record/ranking.tmp") {
            remove(m_opt.record.c_str());
          }
        }
        else if(!g_iclient) {
          g_iclient.reset(new InputClientLocal());
        }
        IGame *game = CreateGame(m_opt);
        CreateGamePanel(game);

        getParent()->destroy();
      }
      catch(const Error& e) {
        destroy();
        show_error(e.what());
      }
    }
  };

  class FadeToTitleWindow : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  public:
    FadeToTitleWindow(sgui::Window *parent) : Super(parent)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void action()
    {
      IMusic::WaitFadeOut();
      getParent()->destroy();
      CreateTitlePanel(sgui::View::instance());
    }
  };

  class FadeToExitPanel : public FadeoutWindow
  {
  typedef FadeoutWindow Super;
  public:
    FadeToExitPanel(sgui::Window *parent) : Super(parent)
    {
      setFadeTime(60);
      IMusic::FadeOut(900);
    }

    void action()
    {
      IMusic::WaitFadeOut();
      sgui::App::instance()->exit();
    }
  };


  void Ending()
  {
    new EndingWindow(GetGameWindow());
  }

  void FadeToGame(const GameOption& opt)
  {
    if(!g_fadeout_window && !g_game_panel) {
      new FadeToGameWindow(GetTitleWindow(), opt);
    }
  }

  void FadeToTitle()
  {
    if(!g_fadeout_window && !g_title_panel) {
      new FadeToTitleWindow(GetGameWindow());
    }
  }

  void FadeToExit()
  {
    if(!g_fadeout_window) {
      new FadeToExitPanel(GetTitleWindow());
    }
  }





  class RecordItem : public sgui::ListItem
  {
  typedef sgui::ListItem Super;
  public:
    string file;
    string name;
    int mode;
    int stage;
    int scene;
    int fps;
    int players;
    int playtime;
    int score;

    RecordItem() :
      Super(L""), mode(0), stage(0), scene(0), fps(0), players(0), playtime(0), score(0)
    {}

    void setToolTip()
    {
      const string modes[] = {"light", "normal", "heavy", "excess", "future"};
      const string stages[] = {"all", "1", "2", "3", "4", "5", "ex"};
      char buf[256];
#ifdef EXCEPTION_DEBUG
      sprintf(buf, "name: %s\nmode: %s\nstage: %s\nscene: %d\nscore: %d\ntime: %dm%02ds\navg fps:%d",
        name.c_str(), modes[mode].c_str(), stages[stage].c_str(), scene, score, int(playtime/3600), int(playtime%3600/60), fps);
#else
      sprintf(buf, "name: %s\nmode: %s\nstage: %s\nscore: %d\ntime: %dm%02ds\navg fps:%d",
        name.c_str(), modes[mode].c_str(), stages[stage].c_str(), score, int(playtime/3600), int(playtime%3600/60), fps);
#endif
      new sgui::ToolTip(this, _L(buf));
    }

    void genText()
    {
      char buf[256];
      sprintf(buf, "%s: %d", name.c_str(), score);
      setText(_L(buf));
    }

    void genDetailedText()
    {
      const string modes[] = {"light", "normal", "heavy", "excess", "future"};
      const string stages[] = {"all", "1", "2", "3", "4", "5", "ex"};

      char buf[256];
      sprintf(buf, "%s %s %s %d", name.c_str(), modes[mode].c_str(), stages[stage].c_str(), score);
      setText(_L(buf));
    }
  };

  class RecordDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::List *m_list;

#ifdef EXCEPTION_ENABLE_NETRANKING
    sgui::List *m_ranking_list;
    static const int s_num_stagebutton = 7;
    sgui::ToggleButton *m_modeb[5];
    sgui::ToggleButton *m_stageb[s_num_stagebutton];
    int m_mode;
    int m_stage;
    httpasync_ptr m_http;
#endif

  public:
    RecordDialog(sgui::Window *parent) :
      Super(parent, sgui::Point(15,15), sgui::Size(530,325), L"record", DL_RECORD)
#ifdef EXCEPTION_ENABLE_NETRANKING
      , m_mode(-1), m_stage(-1)
#endif // EXCEPTION_ENABLE_NETRANKING 
    {
      if(g_record_dialog) {
        throw Error("RecordDialog::RecordDialog()");
      }
      g_record_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_CONFIRMDIALOG);

      sgui::Window *w;
      w = new sgui::Window(this, sgui::Point(5, 30), sgui::Size(300,290));

      new sgui::Label(w, sgui::Point(0,  0), sgui::Size(100,60), L"local data");
      new sgui::Button(w, sgui::Point(0, 20), sgui::Size(60,20), L"play", BU_RECORD_OPEN);
#ifdef EXCEPTION_ENABLE_NETRANKING
      new sgui::Button(w, sgui::Point(65, 20), sgui::Size(60,20), L"upload", BU_RECORD_UPLOAD);
#endif // EXCEPTION_ENABLE_NETRANKING 
      new sgui::Button(w, sgui::Point(130, 20), sgui::Size(60,20), L"delete", BU_RECORD_DELETE);
      m_list = new sgui::List(w, sgui::Point(0, 45), sgui::Size(200,245), LI_RECORD);
      loadLocal();

#ifdef EXCEPTION_ENABLE_NETRANKING
      for(int i=0; i<5; ++i) {
        m_modeb[i] = 0;
      }

      w = new sgui::Window(this, sgui::Point(225, 30), sgui::Size(300,290));
      sgui::Point pos(0, 0);
      new sgui::Label(w, pos, sgui::Size(100,60), L"net raking");

      pos+=sgui::Point(0, 20);
      new sgui::Button(w, pos, sgui::Size(90,20), L"play", BU_RANKING_OPEN);

      pos+=sgui::Point(0, 25);
      m_ranking_list = new sgui::List(w, pos, sgui::Size(150,245), LI_RANKING);
      m_ranking_list->addItem(new sgui::ListItem(L"modeとstageを選択"));

      pos = sgui::Point(155, 40);
      new sgui::Label(w, pos, sgui::Size(100, 20), L"mode");
      pos+=sgui::Point(0, 20);
      m_modeb[0] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"light", BU_RANKING_LIGHT);
      pos+=sgui::Point(0, 25);
      m_modeb[1] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"normal", BU_RANKING_NORMAL);
      pos+=sgui::Point(0, 25);
      m_modeb[2] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"heavy", BU_RANKING_HEAVY);
      pos+=sgui::Point(0, 25);
      m_modeb[3] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"excess", BU_RANKING_EXCESS);
      pos+=sgui::Point(0, 35);
      m_modeb[4] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"future", BU_RANKING_FUTURE);


      for(int i=0; i<s_num_stagebutton; ++i) {
        m_stageb[i] = 0;
      }

      pos = sgui::Point(230, 40);
      new sgui::Label(w, pos, sgui::Size(100, 20), L"stage");
      pos+=sgui::Point(0, 20);
      m_stageb[0] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"all stage", BU_RANKING_STAGE_ALL);
      pos+=sgui::Point(0, 25);
      m_stageb[1] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage 1", BU_RANKING_STAGE_1);

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      m_stageb[2] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage 2", BU_RANKING_STAGE_2);
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 25);
      m_stageb[3] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage 3", BU_RANKING_STAGE_3);

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      m_stageb[4] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage 4", BU_RANKING_STAGE_4);
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      m_stageb[5] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage 5", BU_RANKING_STAGE_5);
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 35);
#ifndef EXCEPTION_TRIAL
#ifdef EXCEPTION_DEBUG
      m_stageb[6] = new sgui::ToggleButton(w, pos, sgui::Size(70,20), L"stage ex", BU_RANKING_STAGE_EX);
#endif // EXCEPTION_DEBUG 
#endif // EXCEPTION_TRIAL 

#endif // EXCEPTION_ENABLE_NETRANKING 

      toTopLevel();
    }

    ~RecordDialog()
    {
      g_record_dialog = 0;
    }


#ifdef EXCEPTION_ENABLE_NETRANKING
    void update()
    {
      Super::update();
      updateRanking();
    }
#endif // EXCEPTION_ENABLE_NETRANKING


    void loadLocal()
    {
      ist::Dir dir("record");
      for(size_t i=0; i<dir.size(); ++i) {
        const string filename = dir[i];
        ist::gzbstream f(string("record/")+filename, "rb");
        if(!f) {
          continue;
        }
        int version = 0;
        int mode = 0;
        int stage = 0;
        int scene = 0;
        int score = 0;
        int seed = 0;
        int fps = 0;
        size_t players = 0;
        string name;
        size_t playtime = 0;

        f >> version;
        if(version!=EXCEPTION_REPLAY_VERSION && version!=105) { // バージョンが合ってるもののみリスト 
          continue;
        }
        f >> mode >> stage >> scene >> score >> seed >> fps >> players >> name >> playtime;

        {
          RecordItem * ri = new RecordItem();
          ri->file = filename;
          ri->name = name;
          ri->mode = mode;
          ri->stage = stage;
          ri->scene = scene;
          ri->fps = fps;
          ri->score = score;
          ri->players = players;
          ri->playtime = playtime;
          m_list->addItem(ri);
          ri->genDetailedText();
          ri->setToolTip();
        }
      }
    }

    void start(const string& record)
    {
      GameOption opt;
      opt.record = string("record/")+record;
      FadeToGame(opt);
    }

#ifdef EXCEPTION_ENABLE_NETRANKING
    void updateRanking()
    {
      if(!m_http || !m_http->isComplete()) {
        return;
      }

      if(m_http->getStatus()==200) {
        m_ranking_list->clearItem();
        string l;
        std::istream in(&m_http->getBuf());
        while(std::getline(in, l)) {
          char name[32];
          char file[32];
          int scene;
          int score;
          int fps;
          int playtime;
          if(sscanf(l.c_str(), "%d, %d, %[^,], %d, %d, %s", &scene, &score, name, &fps, &playtime, file)==6) {
            RecordItem * ri = new RecordItem();
            ri->mode = m_mode;
            ri->stage = m_stage;
            ri->scene = scene;
            ri->score = score;
            ri->name = name;
            ri->fps = fps;
            ri->playtime = playtime;
            ri->file = file;
            m_ranking_list->addItem(ri);
            ri->genText();
            ri->setToolTip();
          }
        }
        if(m_ranking_list->getItemCount()==0) {
          m_ranking_list->addItem(new sgui::ListItem(L"no data"));
        }
      }
      else {
        new sgui::MessageDialog(
          _L("network error"),
          _L(sgui::Format("return code: %d", m_http->getStatus())),
          sgui::View::instance(), sgui::Point(120, 90), sgui::Size(300, 200));
      }
      m_http.reset();
    }

    void loadRanking()
    {
      if(m_mode==-1 || m_stage==-1) {
        return;
      }

      m_ranking_list->clearItem();
      m_ranking_list->addItem(new sgui::ListItem(L"connecting..."));

      char buf[256];
      string path;
#ifdef EXCEPTION_DEBUG
      path = "/exception/d/record/";
#elif defined(EXCEPTION_TRIAL)
      path = "/exception/t/record/";
#else
      path = "/exception/r/record/";
#endif
      sprintf(buf, "%s?cmd=list&version=%d&mode=%d&stage=%d", path.c_str(), EXCEPTION_REPLAY_VERSION, m_mode, m_stage);
      m_http.reset(new ist::HTTPRequestAsync(g_io_service));
      m_http->get("i-saint.skr.jp", buf);
    }

    bool upload(const string& filename)
    {
      try {
        ist::HTTPRequest req(g_io_service);
        string path;
#ifdef EXCEPTION_DEBUG
        path = "/exception/d/record/";
#elif defined(EXCEPTION_TRIAL)
        path = "/exception/t/record/";
#else
        path = "/exception/r/record/";
#endif
        if(req.postFile("i-saint.skr.jp", path, "data", filename, string("record/")+filename)) {
          std::vector<char> buf;
          buf.resize(req.size());
          req.read(&buf[0], req.size());
          buf.push_back(0);
          new sgui::MessageDialog(L"upload", _L(&buf[0]), this, sgui::Point(50, 50));
          return true;
        }
      }
      catch(std::exception& e) {
        new sgui::MessageDialog(L"network error", _L(e.what()), this, sgui::Point(50, 50));
      }
      return false;
    }

    bool download(const string& filename)
    {
      try {
        char buf[256];
        ist::HTTPRequest req(g_io_service);
        string path;
#ifdef EXCEPTION_DEBUG
        path = "/exception/d/record/";
#elif defined(EXCEPTION_TRIAL)
        path = "/exception/t/record/";
#else
        path = "/exception/r/record/";
#endif
        sprintf(buf, "%s?cmd=get&version=%d&mode=%d&stage=%d&file=%s", path.c_str(), EXCEPTION_REPLAY_VERSION, m_mode, m_stage, filename.c_str());
        if(req.get("i-saint.skr.jp", buf)) {
          std::ofstream of((string("record/")+"ranking.tmp").c_str(), std::ios::binary);
          while(!req.eof()) {
            char c = req.getchar();
            of.write(&c, 1);
          }
          return true;
        }
      }
      catch(std::exception& e) {
        new sgui::MessageDialog(L"network error", _L(e.what()), this, sgui::Point(50, 50));
      }
      return false;
    }
#endif // EXCEPTION_ENABLE_NETRANKING 

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(id>=BU_RANKING_LIGHT && id<=BU_RANKING_FUTURE) {
#ifdef EXCEPTION_ENABLE_NETRANKING
          m_mode = id - BU_RANKING_LIGHT;
          for(int i=0; i<5; ++i) {
            if(m_modeb[i] && m_modeb[i]!=evt.getSrc()) {
              m_modeb[i]->setButtonState(sgui::Button::UP);
            }
          }
          loadRanking();
          return true;
#endif // EXCEPTION_ENABLE_NETRANKING 
        }
        else if(id>=BU_RANKING_STAGE_ALL && id<=BU_RANKING_STAGE_EX) {
#ifdef EXCEPTION_ENABLE_NETRANKING
          m_stage = id - BU_RANKING_STAGE_ALL;
          for(int i=0; i<s_num_stagebutton; ++i) {
            if(m_stageb[i] && m_stageb[i]!=evt.getSrc()) {
              m_stageb[i]->setButtonState(sgui::Button::UP);
            }
          }
          loadRanking();
          return true;
#endif // EXCEPTION_ENABLE_NETRANKING 
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_RECORD_OPEN) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(m_list->getSelectedItem())) {
            start(ri->file);
          }
          return true;
        }
        else if(id==BU_RECORD_UPLOAD) {
#ifdef EXCEPTION_ENABLE_NETRANKING
          if(RecordItem *ri = dynamic_cast<RecordItem*>(m_list->getSelectedItem())) {
            upload(ri->file);
          }
          return true;
#endif // EXCEPTION_ENABLE_NETRANKING 
        }
        else if(id==BU_RECORD_DELETE) {
          if(m_list->getSelectionCount()>0) {
            new sgui::ConfirmDialog(L"delete", L"選択されたデータを消去します。\nよろしいですか？",
              BU_RECORD_DELETE_CONFIRM, this, sgui::Point(20, 50));
          }
          return true;
        }
        else if(id==BU_RANKING_OPEN) {
#ifdef EXCEPTION_ENABLE_NETRANKING
          if(RecordItem *item = dynamic_cast<RecordItem*>(m_ranking_list->getSelectedItem())) {
            if(download(item->file)) {
              start("ranking.tmp");
            }
          }
          return true;
#endif // EXCEPTION_ENABLE_NETRANKING 
        }
#ifdef EXCEPTION_ENABLE_NETRANKING
        else if(m_mode==id-BU_RANKING_LIGHT || m_stage==id-BU_RANKING_STAGE_ALL) {
          dynamic_cast<sgui::ToggleButton*>(evt.getSrc())->setButtonState(sgui::Button::DOWN);
          return true;
        }
#endif // EXCEPTION_ENABLE_NETRANKING 
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_RECORD_DELETE_CONFIRM && e.isOK()) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(m_list->getSelectedItem())) {
            remove((string("record/")+ri->file).c_str());
            m_list->removeItem(ri);
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_RECORD) {
          if(RecordItem *ri = dynamic_cast<RecordItem*>(e.getItem())) {
            start(ri->file);
          }
          return true;
        }
        else if(id==LI_RANKING) {
#ifdef EXCEPTION_ENABLE_NETRANKING
          if(RecordItem *ri = dynamic_cast<RecordItem*>(e.getItem())) {
            if(download(ri->file)) {
              start("ranking.tmp");
            }
          }
          return true;
#endif // EXCEPTION_ENABLE_NETRANKING 
        }
      }

      return Super::handleEvent(evt);
    }
  };


#ifdef EXCEPTION_ENABLE_STATE_SAVE
  class StateDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:
    sgui::List *m_list;

  public:
    StateDialog() :
      Super(sgui::View::instance(), sgui::Point(15,15), sgui::Size(160,285), L"state", DL_STATE)
      {
      if(g_state_dialog) {
        throw Error("StateDialog::StateDialog()");
      }
      g_state_dialog = this;

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_CONFIRMDIALOG);

      sgui::Window *w;
      w = new sgui::Window(this, sgui::Point(5, 30), sgui::Size(150,290));

      new sgui::Button(w, sgui::Point(0, 0), sgui::Size(45,20), L"save", BU_STATE_SAVE);
      new sgui::Button(w, sgui::Point(50, 0), sgui::Size(45,20), L"load", BU_STATE_LOAD);
      new sgui::Button(w, sgui::Point(100, 0), sgui::Size(45,20), L"delete", BU_STATE_DELETE);
      m_list = new sgui::List(w, sgui::Point(0, 25), sgui::Size(150,225), LI_STATE);
      listLocal();

      toTopLevel();
    }

    ~StateDialog()
    {
      g_state_dialog = 0;
    }


    void listLocal()
    {
      m_list->clearItem();

      ist::Dir dir(".");
      for(size_t i=0; i<dir.size(); ++i) {
        const string& path = dir[i];
        boost::smatch m;
        if(boost::regex_search(path, m, boost::regex("\\.dump$"))) {
          m_list->addItem(new sgui::ListItem(sgui::_L(path)));
        }
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_STATE_SAVE) {
          if(IGame *game = GetGame()) {
            char filename[128];
            sprintf(filename, "%d%d.dump", ::time(0), sgui::GetTicks());
            SaveState(filename);
            listLocal();
          }
          return true;
        }
        else if(id==BU_STATE_LOAD) {
          LoadState(sgui::_S(m_list->getSelectedItem()->getText()));
          return true;
        }
        else if(id==BU_STATE_DELETE) {
          if(m_list->getSelectionCount()>0) {
            new sgui::ConfirmDialog(L"delete", L"選択されたデータを消去します。\nよろしいですか？",
              BU_STATE_DELETE_CONFIRM, this, sgui::Point(20, 50));
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_STATE_DELETE_CONFIRM && e.isOK()) {
          sgui::ListItem *li = m_list->getSelectedItem();
          remove(sgui::_S(li->getText()).c_str());
          m_list->removeItem(li);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==LI_STATE) {
          LoadState(sgui::_S(e.getItem()->getText()));
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };
#endif // EXCEPTION_ENABLE_STATE_SAVE 


  class ServerListItem : public sgui::ListItem
  {
  typedef sgui::ListItem Super;
  public:
    string ip;
    int port;
    string name;

    ServerListItem(const string& _ip, int _port, const string& _name) :
      Super(L""), ip(_ip), port(_port), name(_name)
    {
      setText(sgui::_L(_name));
    }
  };

  class StartDialog : public sgui::Dialog
  {
  typedef sgui::Dialog Super;
  private:

#ifdef EXCEPTION_ENABLE_NETPLAY
    class ConnectThread : public Thread
    {
    private:
      typedef shared_ptr<InputClientIP> client_ptr;
      client_ptr m_client;
      string m_server;
      ushort m_port;
      string m_error;
      bool m_connected;

    public:
      ConnectThread(const client_ptr& c, const string& server, ushort port) :
        m_client(c), m_server(server), m_port(port), m_connected(false)
      {}

      bool isConnected() { return m_connected; }
      const string& getError() { return m_error; }

      void exec()
      {
        try {
          m_client->connect(m_server, m_port);
          m_connected = true;
        }
        catch(std::exception& e) {
          m_error = e.what();
        }
      }

      void runClient()
      {
        m_client->run();
      }
    };
    typedef shared_ptr<ConnectThread> connect_thread_ptr;

    connect_thread_ptr m_connect_thread;
    httpasync_ptr m_http_serverlist;
#endif

    static const int s_num_stagebutton = 7;

    sgui::Window *m_lpanel;
    sgui::Window *m_addresspanel;
    sgui::Window *m_delaypanel;
    sgui::Window *m_stagepanel;
    sgui::ToggleButton *m_netb[3];
    sgui::ToggleButton *m_modeb[5];
    sgui::ToggleButton *m_stageb[s_num_stagebutton];
    sgui::Button *m_startbutton;

    sgui::Edit *m_serveraddress;
    sgui::Edit *m_serverport;
    sgui::Edit *m_serverdelay;
    sgui::Combo *m_serverlist;
    sgui::Label *m_connecting;

    GameOption m_opt;

  public:
    StartDialog(sgui::Window *parent) :
      Super(parent, sgui::Point(140,100), sgui::Size(360,230), L"start", DL_START)
    {
      if(g_start_dialog) {
        throw Error("StartDialog::StartDialog()");
      }
      g_start_dialog = this;

      listenEvent(sgui::EVT_CONSTRUCT);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_COMBO);

      m_lpanel = new sgui::Window(this, sgui::Point(0, 30), sgui::Size(60, 200));
      m_stagepanel = new sgui::Window(this, sgui::Point(60, 30), sgui::Size(300, 200));
      m_addresspanel = new sgui::Window(this, sgui::Point(60, 30), sgui::Size(300, 200));
      m_addresspanel->setVisible(false);
      m_delaypanel = new sgui::Window(this, sgui::Point(260, 30), sgui::Size(100, 200));
      m_delaypanel->setVisible(false);

      // ネットワーク選択 
      sgui::Point pos(5, 0);

#ifdef EXCEPTION_ENABLE_NETPLAY
      new sgui::Label(m_lpanel, pos, sgui::Size(100, 20), L"network");
      pos+=sgui::Point(0, 20);
      m_netb[0] = new sgui::ToggleButton(m_lpanel, pos, sgui::Size(50,20), L"local", BU_START_LOCAL);
      pos+=sgui::Point(0, 35);
      m_netb[1] = new sgui::ToggleButton(m_lpanel, pos, sgui::Size(50,20), L"server", BU_START_SERVER);
      pos+=sgui::Point(0, 25);
      m_netb[2] = new sgui::ToggleButton(m_lpanel, pos, sgui::Size(50,20), L"client", BU_START_CLIENT);
      pos+=sgui::Point(0, 25);
      if(g_iserver) {
        m_netb[1]->setButtonState(sgui::Button::DOWN);
      }
      else if(g_iclient && typeid(*g_iclient)==typeid(InputClientIP)) {
        m_netb[2]->setButtonState(sgui::Button::DOWN);
      }
      else {
        m_netb[0]->setButtonState(sgui::Button::DOWN);
      }

      new sgui::Label(m_delaypanel, sgui::Point(0, 0), sgui::Size(100, 16), L"delay");
      m_serverdelay = new sgui::Edit(m_delaypanel, sgui::Point(0, 20), sgui::Size(50, 16), L"5");
#endif

      // サーバーのアドレス入力欄 
      new sgui::Label(m_addresspanel, sgui::Point(  5,50), sgui::Size(120, 20), L"address");
      new sgui::Label(m_addresspanel, sgui::Point(130,50), sgui::Size( 60, 20), L"port");
      m_serveraddress = new sgui::Edit(m_addresspanel,  sgui::Point(  5,70), sgui::Size(120, 16), sgui::_L(GetConfig()->last_server), ED_START_ADDRESS);
      m_serverport = new sgui::Edit(m_addresspanel,  sgui::Point(130,70), sgui::Size( 60, 16), boost::lexical_cast<wstring>(GetConfig()->last_port), ED_START_PORT);
      new sgui::Button(m_addresspanel,sgui::Point(200,70), sgui::Size(60, 16), L"connect", BU_START_CONNECT);
      m_connecting = new sgui::Label(m_addresspanel, sgui::Point(200,90), sgui::Size(100, 20), L"");

      new sgui::Label(m_addresspanel, sgui::Point(  5, 0), sgui::Size(120, 20), L"server list");
      m_serverlist = new sgui::Combo(m_addresspanel, sgui::Point(  5,20), sgui::Size(120, 16), LI_START_SERVERLIST);
      new sgui::Button(m_addresspanel,sgui::Point(130,20), sgui::Size(60, 16), L"connect", BU_START_CONNECTLIST);


      // 難度/ステージ選択 
      pos = sgui::Point(5, 0);
      new sgui::Label(m_stagepanel, pos, sgui::Size(100, 20), L"mode");

      for(int i=0; i<5; ++i) {
        m_modeb[i] = 0;
      }

      pos+=sgui::Point(0, 20);

      m_modeb[0] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"light", BU_START_LIGHT);
      new sgui::ToolTip(m_modeb[0], L"破片倍率1.0\n少々画面が寂しいモードです。");
      pos+=sgui::Point(0, 25);

      m_modeb[1] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"normal", BU_START_NORMAL);
      new sgui::ToolTip(m_modeb[1], L"破片倍率1.8\nそこそこ画面が賑やかなモードです。");
      m_modeb[1]->setButtonState(sgui::Button::DOWN);
      pos+=sgui::Point(0, 25);

      m_modeb[2] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"heavy", BU_START_HEAVY);
      new sgui::ToolTip(m_modeb[2], L"破片倍率3.2\n適度に画面が賑やかなモードです。");
      pos+=sgui::Point(0, 25);

      int clear_bit;
#ifdef EXCEPTION_TRIAL
      clear_bit = 5;
#else // EXCEPTION_TRIAL 
      clear_bit = 31;
#endif // EXCEPTION_TRIAL 
      IClearFlag& cf = *GetClearFlag();
      if((cf.heavy&clear_bit)==clear_bit) {
        m_modeb[3] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"excess", BU_START_EXCESS);
        new sgui::ToolTip(m_modeb[3], L"破片倍率4.8\n必要以上に画面が賑やかなモードです。");
      }
      pos+=sgui::Point(0, 35);

      if((cf.excess&clear_bit)==clear_bit) {
        m_modeb[4] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"future", BU_START_FUTURE);
        new sgui::ToolTip(m_modeb[4], L"破片倍率9.0\n2年後くらいの最新マシンで快適に遊べると思います。");
      }

      for(int i=0; i<s_num_stagebutton; ++i) {
        m_stageb[i] = 0;
      }

      pos = sgui::Point(100, 0);
      new sgui::Label(m_stagepanel, pos, sgui::Size(100, 20), L"stage");
      pos+=sgui::Point(0, 20);
      m_stageb[0] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"all stage", BU_START_STAGE_ALL);
      updateStageButtons();

      m_startbutton = new sgui::Button(m_stagepanel, sgui::Point(200,170), sgui::Size(90,20), L"start", BU_START_START);

#ifdef EXCEPTION_ENABLE_SCENE_EDIT
      pos = sgui::Point(200, 50);
      new sgui::Label(m_stagepanel, pos, sgui::Size(100, 20), L"scene");
      pos+=sgui::Point(0, 20);

      sgui::Combo *combo = new sgui::Combo(m_stagepanel, pos, sgui::Size(90,16), CB_START_SCENE);
      for(int i=1; i<10; ++i) {
        char buf[8];
        sprintf(buf, "%d", i);
        combo->addItem(new sgui::ListItem(_L(buf)));
      }
      combo->getList()->setSelection(size_t(0));
#endif // EXCEPTION_ENABLE_SCENE_EDIT 

      toTopLevel();
    }

    ~StartDialog()
    {
      g_start_dialog = 0;
    }

    void onCloseButton()
    {
      Super::onCloseButton();
      g_iserver.reset();
      g_iclient.reset();
    }

#ifdef EXCEPTION_ENABLE_NETPLAY
    void update()
    {
      Super::update();
      updateClientConnection();
      updateServerList();
    }
#endif // EXCEPTION_ENABLE_NETPLAY


    void updateStageButtons()
    {
      for(int i=1; i<s_num_stagebutton; ++i) {
        if(m_stageb[i]) {
          break;
        }
        if(i==s_num_stagebutton-1) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
      }

      sgui::Point pos = sgui::Point(100, 45);

      IClearFlag& cf = *GetClearFlag();
      int f = *cf.getModeFlag(m_opt.mode);
      if((f&1)==1 && !m_stageb[1]) {
        m_stageb[1] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage 1", BU_START_STAGE_1);
      }
      else if((f&1)!=1 && m_stageb[1]) {
        if(m_stageb[1]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[1]->destroy();
        m_stageb[1] = 0;
      }

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      if((f&2)==2 && !m_stageb[2]) {
        m_stageb[2] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage 2", BU_START_STAGE_2);
      }
      else if((f&2)!=2 && m_stageb[2]) {
        if(m_stageb[2]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[2]->destroy();
        m_stageb[2] = 0;
      }
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 25);
      if((f&4)==4 && !m_stageb[3]) {
        m_stageb[3] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage 3", BU_START_STAGE_3);
      }
      else if((f&4)!=4 && m_stageb[3]) {
        if(m_stageb[3]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[3]->destroy();
        m_stageb[3] = 0;
      }

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      if((f&8)==8 && !m_stageb[4]) {
        m_stageb[4] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage 4", BU_START_STAGE_4);
      }
      else if((f&8)!=8 && m_stageb[4]) {
        if(m_stageb[4]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[4]->destroy();
        m_stageb[4] = 0;
      }
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 25);
#ifndef EXCEPTION_TRIAL
      if((f&16)==16 && !m_stageb[5]) {
        m_stageb[5] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage 5", BU_START_STAGE_5);
      }
      else if((f&16)!=16 && m_stageb[5]) {
        if(m_stageb[5]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[5]->destroy();
        m_stageb[5] = 0;
      }
#endif // EXCEPTION_TRIAL 

      pos+=sgui::Point(0, 25);
#if !defined(EXCEPTION_TRIAL) && defined(EXCEPTION_DEBUG)
      if((f&31)==31 && !m_stageb[6]) {
        m_stageb[6] = new sgui::ToggleButton(m_stagepanel, pos, sgui::Size(90,20), L"stage ex", BU_START_STAGE_EX);
      }
      else if((f&31)!=31 && m_stageb[6]) {
        if(m_stageb[6]->getButtonState()==sgui::Button::DOWN) {
          m_stageb[0]->setButtonState(sgui::Button::DOWN);
          m_opt.stage = 0;
        }
        m_stageb[6]->destroy();
        m_stageb[6] = 0;
      }
#endif // EXCEPTION_TRIAL && EXCEPTION_DEBUG 
    }


#ifdef EXCEPTION_ENABLE_NETPLAY
    void startServer()
    {
      g_iserver.reset(new InputServer(g_io_service));
      g_iserver->run();

      connect("127.0.0.1", GetConfig()->port);
      CreateChatDialog();
      toTopLevel();
    }

    void updateClientConnection()
    {
      if(m_connect_thread) {
        if(m_connect_thread->isConnected()) {
          m_connect_thread->runClient();
          m_connect_thread.reset();
        }
        else if(!m_connect_thread->getError().empty()) {
          new sgui::MessageDialog(
            _L("network error"),
            _L(m_connect_thread->getError()),
            sgui::View::instance(), sgui::Point(120, 90), sgui::Size(300, 200));
          m_connect_thread.reset();
        }

        if(!m_connect_thread) {
          m_connecting->setText(L"");
        }
      }
    }

    void connect(const string& server, int port)
    {
      m_connecting->setText(L"connectiong...");
      g_iclient.reset();

      shared_ptr<InputClientIP> client(new InputClientIP());
      m_connect_thread.reset(new ConnectThread(client, server, port));
      m_connect_thread->run();
      g_iclient = client;
    }

    void updateServerList()
    {
      if(!m_http_serverlist || !m_http_serverlist->isComplete()) {
        return;
      }
      m_serverlist->clearItem();

      if(m_http_serverlist->getStatus()==200) {
        std::istream in(&m_http_serverlist->getBuf());
        string l;
        char ip[32];
        int port;
        char name[32];
        while(std::getline(in, l)) {
          if(sscanf(l.c_str(), "'%31[^']', %d, '%31[^']'", ip, &port, name)==3) {
            m_serverlist->addItem(new ServerListItem(ip, port, name));
          }
        }
      }
      else {
        new sgui::MessageDialog(
          _L("network error"),
          _L(sgui::Format("return code: %d", m_http_serverlist->getStatus())),
          sgui::View::instance(), sgui::Point(120, 90), sgui::Size(300, 200));
      }
      m_http_serverlist.reset();
    }

    void loadServerList()
    {
      if(m_http_serverlist) {
        return;
      }
      m_serverlist->clearItem();
      m_serverlist->addItem(new sgui::ListItem(L"connecting..."));

      m_http_serverlist.reset(new ist::HTTPRequestAsync(g_io_service));
#ifdef EXCEPTION_DEBUG
      m_http_serverlist->get("i-saint.skr.jp", "/exception/d/server/");
#elif defined(EXCEPTION_TRIAL)
      m_http_serverlist->get("i-saint.skr.jp", "/exception/t/server/");
#else
      m_http_serverlist->get("i-saint.skr.jp", "/exception/r/server/");
#endif
    }

    void resetClientAndServer()
    {
      g_iserver.reset();
      g_iclient.reset();
      m_connect_thread.reset();
    }

#endif // EXCEPTION_ENABLE_NETPLAY 

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_DOWN) {
        if(id>=BU_START_LOCAL && id<=BU_START_CLIENT) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          if(id==BU_START_SERVER) {
            resetClientAndServer();
            startServer();
          }
          else if(id==BU_START_CLIENT) {
            resetClientAndServer();
            loadServerList();
          }
          else if(id==BU_START_LOCAL) {
            resetClientAndServer();
            g_iclient.reset(new InputClientLocal());
          }

          if(id==BU_START_LOCAL) {
            m_stagepanel->setVisible(true);
            m_delaypanel->setVisible(false);
            m_addresspanel->setVisible(false);
          }
          else if(id==BU_START_SERVER) {
            m_stagepanel->setVisible(true);
            m_delaypanel->setVisible(true);
            m_addresspanel->setVisible(false);
          }
          else if(id==BU_START_CLIENT) {
            m_stagepanel->setVisible(false);
            m_delaypanel->setVisible(false);
            m_addresspanel->setVisible(true);
          }
          g_selecter->setRefreshFlag(true);
          if(sgui::Window *w=g_selecter->getFocus()) {
            if(!w->isVisible()) {
              g_selecter->setFocus(0);
            }
          }

          for(int i=0; i<3; ++i) {
            if(m_netb[i] && m_netb[i]!=evt.getSrc()) {
              m_netb[i]->setButtonState(sgui::Button::UP);
            }
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id>=BU_START_LIGHT && id<=BU_START_FUTURE) {
          m_opt.mode = id - BU_START_LIGHT;
          updateStageButtons();
          for(int i=0; i<5; ++i) {
            if(m_modeb[i] && m_modeb[i]!=evt.getSrc()) {
              m_modeb[i]->setButtonState(sgui::Button::UP);
            }
          }
          return true;
        }
        else if(id>=BU_START_STAGE_ALL && id<=BU_START_STAGE_EX) {
          m_opt.stage = id - BU_START_STAGE_ALL;
          for(int i=0; i<s_num_stagebutton; ++i) {
            if(m_stageb[i] && m_stageb[i]!=evt.getSrc()) {
              m_stageb[i]->setButtonState(sgui::Button::UP);
            }
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if((id>=BU_START_LOCAL && id<=BU_START_CLIENT) ||
           (id>=BU_START_LIGHT && id<=BU_START_FUTURE) ||
           (id>=BU_START_STAGE_ALL && id<=BU_START_STAGE_EX) ) {
          if(sgui::ToggleButton *tb = dynamic_cast<sgui::ToggleButton*>(evt.getSrc())) {
            tb->setButtonState(sgui::Button::DOWN);
          }
          return true;
        }
        else if(id==BU_START_START) {
          if(IsServerMode()) {
#ifdef EXCEPTION_ENABLE_NETPLAY
            bool ok = true;
            try {
              m_opt.delay = boost::lexical_cast<int>(m_serverdelay->getText());
              if(m_opt.delay<1 || m_opt.delay>20) {
                throw Error("delayには1～20の数値を入力してください");
              }
            }
            catch(const Error& e) {
              new sgui::MessageDialog(_L("error"), _L(e.what()), sgui::View::instance(), sgui::Point(120, 90), sgui::Size(300, 200));
              ok = false;
            }
            if(ok) {
              SendNMessage(NMessage::Start(m_opt.mode, m_opt.stage, m_opt.seed, m_opt.delay));
            }
#endif // EXCEPTION_ENABLE_NETPLAY
          }
          else {
            FadeToGame(m_opt);
          }
          return true;
        }
        else if(id==BU_START_CONNECT) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          try {
            string server = _S(m_serveraddress->getText());
            int port = boost::lexical_cast<int>(m_serverport->getText());
            GetConfig()->last_server = server;
            GetConfig()->last_port = port;
            connect(server, port);
          }
          catch(const boost::bad_lexical_cast&) {
            new sgui::MessageDialog(_L("error"), L"portには数値を入力してください", sgui::View::instance(), sgui::Point(120, 90), sgui::Size(300, 200));
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
        else if(id==BU_START_CONNECTLIST) {
#ifdef EXCEPTION_ENABLE_NETPLAY
          if(ServerListItem *li=dynamic_cast<ServerListItem*>(m_serverlist->getSelection())) {
            connect(li->ip, li->port);
          }
          return true;
#endif // EXCEPTION_ENABLE_NETPLAY
        }
      }
      else if(evt.getType()==sgui::EVT_COMBO) {
        const sgui::ListEvent& e = dynamic_cast<const sgui::ListEvent&>(evt);
        if(id==CB_START_SCENE) {
          int scene = 0;
          if(sscanf(_S(e.getItem()->getText()).c_str(), "%d", &scene)==1) {
            m_opt.scene = scene;
          }
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONSTRUCT) {
        if(evt.getSrc()==m_startbutton && g_selecter) {
          g_selecter->setFocus(m_startbutton);
        }
      }

      return sgui::Dialog::handleEvent(evt);
    }
  };










  class PausePanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    sgui::StopWatch m_sw;
    sgui::StopWatch m_sw_resume;

  public:
    PausePanel(sgui::Window *parent) :
      Super(parent, sgui::Point(0,0), sgui::Size(640,480), L"pause")
    {
      if(g_pause_panel) {
        throw Error("PauseDialog::PauseDialog()");
      }
      g_pause_panel = this;

      IMusic::Pause();

      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_CONFIRMDIALOG);
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));
      setBorderColor(getBackgroundColor());
      m_sw.run(50);

      new sgui::Button(this, sgui::Point(270, 320), sgui::Size(100,20), L"resume", BU_PAUSE_RESUME);
      new sgui::Button(this, sgui::Point(270, 370), sgui::Size(100,20), L"config", BU_PAUSE_CONFIG);
      new sgui::Button(this, sgui::Point(270, 420), sgui::Size(100,20), L"exit", BU_PAUSE_EXIT);
      GetGame()->setPause(true);
    }

    ~PausePanel()
    {
      g_pause_panel = 0;

      IMusic::Resume();
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);
    }

    void update()
    {
      Super::update();

      float opa = 0.4f;
      if(m_sw.isRunning()) {
        opa = std::min<float>(opa, float(m_sw.getPast())/50.0f*0.4f);
      }
      if(m_sw_resume.isActive()) {
        opa = std::min<float>(opa, float(m_sw_resume.getLeft())/50.0f*0.4f);
      }
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, opa));
      setBorderColor(getBackgroundColor());

      if(m_sw_resume.isFinished()) {
        if(IGame *g = GetGame()) {
          g->setPause(false);
        }
        destroy();
      }
    }

    void resume()
    {
      m_sw_resume.run(50);
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_PAUSE_RESUME) {
          SendNMessage(NMessage::Resume());
          return true;
        }
        else if(id==BU_PAUSE_CONFIG) {
          if(!g_config_dialog) {
            new ConfigDialog(this);
          }
          return true;
        }
        else if(id==BU_PAUSE_EXIT) {
          new sgui::ConfirmDialog(L"exit", L"タイトル画面に戻ります。\nよろしいですか？", BU_PAUSE_EXIT_CONFIRM, this);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_PAUSE_EXIT_CONFIRM && e.isOK()) {
          SendNMessage(NMessage::End());
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_KEYUP) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          SendNMessage(NMessage::Resume());
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_JOY_BUTTONUP) {
        const sgui::JoyEvent& e = dynamic_cast<const sgui::JoyEvent&>(evt);
        if(e.getButton()==9) {
          SendNMessage(NMessage::Resume());
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };


  class ContinuePanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    sgui::StopWatch m_sw;
    sgui::StopWatch m_sw_resume;

  public:
    ContinuePanel(sgui::Window *parent) :
      Super(parent, sgui::Point(0,0), sgui::Size(640,480), L"pause")
    {
      if(g_continue_panel) {
        throw Error("ContinuePanel::ContinuePanel()");
      }
      g_continue_panel = this;

      IMusic::Pause();

      listenEvent(sgui::EVT_JOY_BUTTONUP);
      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_CONFIRMDIALOG);
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, 0.0f));
      setBorderColor(getBackgroundColor());
      m_sw.run(120);

      new sgui::Button(this, sgui::Point(270, 320), sgui::Size(100,20), L"continue", BU_PAUSE_RESUME);
      new sgui::Button(this, sgui::Point(270, 370), sgui::Size(100,20), L"config", BU_PAUSE_CONFIG);
      new sgui::Button(this, sgui::Point(270, 420), sgui::Size(100,20), L"exit", BU_PAUSE_EXIT);
      GetGame()->setPause(true);
    }

    ~ContinuePanel()
    {
      IMusic::Resume();
      g_continue_panel = 0;
    }

    void draw()
    {
      glColor4fv(getBackgroundColor().v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);
    }

    void update()
    {
      Super::update();

      float opa = 0.4f;
      if(m_sw.isRunning()) {
        opa = std::min<float>(opa, float(m_sw.getPast())/120.0f*0.4f);
      }
      if(m_sw_resume.isActive()) {
        opa = std::min<float>(opa, float(m_sw_resume.getLeft())/50.0f*0.4f);
      }
      setBackgroundColor(sgui::Color(0.0f, 0.0f, 0.0f, opa));
      setBorderColor(getBackgroundColor());

      if(m_sw_resume.isFinished()) {
        if(IGame *g = GetGame()) {
          g->setPause(false);
          g->newPlayer();
        }
        destroy();
      }
    }

    void resume()
    {
      m_sw_resume.run(50);
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_PAUSE_RESUME) {
          resume();
          return true;
        }
        else if(id==BU_PAUSE_CONFIG) {
          if(!g_config_dialog) {
            new ConfigDialog(this);
          }
          return true;
        }
        else if(id==BU_PAUSE_EXIT) {
          new sgui::ConfirmDialog(L"exit", L"タイトル画面に戻ります。\nよろしいですか？", BU_PAUSE_EXIT_CONFIRM, this);
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
        if(id==BU_PAUSE_EXIT_CONFIRM && e.isOK()) {
          FadeToTitle();
          return true;
        }
      }

      return Super::handleEvent(evt);
    }
  };



  class GamePanel : public sgui::Panel
  {
  typedef sgui::Panel Super;
  private:
    IGame *m_game;

  public:
    GamePanel(IGame *game) :
      Super(sgui::View::instance(), sgui::Point(), sgui::View::instance()->getSize(), _L(""), 0, 0)
    {
      if(g_game_panel) {
        throw Error("GamePanel::GamePanel()");
      }
      g_game_panel = this;

      if(g_chat_dialog) {
        g_chat_dialog->toTopLevel();
      }
#ifdef EXCEPTION_ENABLE_STATE_SAVE
      if(g_state_dialog) {
        g_state_dialog->toTopLevel();
      }
#endif // EXCEPTION_ENABLE_STATE_SAVE 
      listenEvent(sgui::EVT_APP_LOSINGKEYFOCUS);
      listenEvent(sgui::EVT_APP_ICONIZE);
      listenEvent(sgui::EVT_MOUSE_BUTTONDOWN);
      listenEvent(sgui::EVT_MOUSE_BUTTONUP);
      listenEvent(sgui::EVT_MOUSE_MOVE);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_JOY_BUTTONUP);

      m_game = game;
    }

    ~GamePanel()
    {
      releaseGame();
      g_game_panel = 0;
    }

    void destroy()
    {
      releaseGame();
      if(g_chat_dialog) {
        g_chat_dialog->destroy();
      }
      g_game_panel = 0;

      Super::destroy();
    }

    bool saveState(const string& path)
    {
      if(m_game) {
        Serializer s;
        m_game->serialize(s);
        s.save(path);
        return true;
      }
      return false;
    }

    bool loadState(const string& path)
    {
      Deserializer s;
      if(s.load(path)) {
        if(m_game) {
          delete m_game;
          m_game = 0;
        }
        m_game = CreateGame(s);
        return true;
      }
      return false;
    }

    void loadState(Deserializer& s)
    {
      m_game = CreateGame(s);
    }

    void releaseGame()
    {
      if(m_game) {
        m_game->exit();
        m_game->write();
        delete m_game;
        m_game = 0;
      }
      g_iserver.reset();
      g_iclient.reset();
    }

    void update()
    {
      Super::update();
      if(m_game) {
        ViewportSaver vs;
        fitViewport();
        m_game->update();
      }
    }

    void draw()
    {
      if(m_game) {
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_game->draw();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
      }
    }

    bool handleEvent(const sgui::Event& evt)
    {
      if(  evt.getType()==sgui::EVT_APP_LOSINGKEYFOCUS
        || evt.getType()==sgui::EVT_APP_ICONIZE) {
        SendNMessage(NMessage::Pause());
        return true;
      }
      else if(evt.getType()==sgui::EVT_MOUSE_MOVE) {
      //  GetGame()->moveCameraByMouseDrag(dynamic_cast<const sgui::MouseEvent&>(evt));
        return true;
      }
      else if(evt.getType()==sgui::EVT_KEYUP) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          SendNMessage(NMessage::Pause());
          return true;
        }
#ifdef EXCEPTION_ENABLE_STATE_SAVE
        else if(e.getKey()==sgui::KEY_F6) {
          char filename[128];
          sprintf(filename, "%d%d.dump", ::time(0), sgui::GetTicks());
          saveState(filename);
          if(g_state_dialog) {
            g_state_dialog->listLocal();
          }
        }
        else if(e.getKey()==sgui::KEY_F7) {
          if(!g_state_dialog) {
            new StateDialog();
          }
          else {
            g_state_dialog->destroy();
          }
          return true;
        }
#endif // EXCEPTION_ENABLE_STATE_SAVE 
#ifdef EXCEPTION_ENABLE_PROFILE
        else if(e.getKey()==sgui::KEY_F8) {
          if(!g_debug_dialog) {
            new DebugDialog(this);
          }
          else {
            g_debug_dialog->destroy();
          }
          return true;
        }
        else if(e.getKey()==sgui::KEY_F9) {
          if(!g_objbrowser_dialog) {
            new ObjBrowserDialog(this);
          }
          else {
            g_objbrowser_dialog->destroy();
          }
          return true;
        }
        else if(e.getKey()==sgui::KEY_F10) {
          if(!g_config_dialog) {
            new ConfigDialog(this);
          }
          else {
            g_config_dialog->destroy();
          }
          return true;
        }
        else if(e.getKey()==sgui::KEY_F11) {
          if(!g_profile_dialog) {
            new ProfileDialog(this);
          }
          else {
            g_profile_dialog->destroy();
          }
          return true;
        }
#endif // EXCEPTION_ENABLE_PROFILE 
        else if(e.getKey()==sgui::KEY_RETURN) {
          CreateChatDialog();
          g_chat_dialog->focus();
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_JOY_BUTTONUP) {
        const sgui::JoyEvent& e = dynamic_cast<const sgui::JoyEvent&>(evt);
        if(e.getButton()==GetConfig()->pad[6]) {
          if(!g_pause_panel) {
            new PausePanel(this);
          }
          return true;
        }
      }
      return Super::handleEvent(evt);
    }
  };




  class TitlePanel : public sgui::Panel
  {
  typedef sgui::Window Super;
  private:
    obj_ptr m_background;

  public:
    TitlePanel(sgui::Window *parent) :
      sgui::Panel(parent, sgui::Point(), parent->getSize()), m_background(0)
    {
      if(g_title_panel) {
        throw Error("TitlePanel::TitlePanel()");
      }
      g_title_panel = this;

#ifdef EXCEPTION_ENABLE_STATE_SAVE
      if(g_state_dialog) {
        g_state_dialog->toTopLevel();
      }
#endif // EXCEPTION_ENABLE_STATE_SAVE 

      m_background = CreateTitleBackground();
      GetMusic("title.ogg")->play();

      listenEvent(sgui::EVT_BUTTON_UP);
      listenEvent(sgui::EVT_CLOSEDIALOG);
      listenEvent(sgui::EVT_LIST_DOUBLECLICK);
      listenEvent(sgui::EVT_KEYUP);
      listenEvent(sgui::EVT_CONFIRMDIALOG);
      setBorderColor(sgui::Color(0,0,0,0));
      
      sgui::Point base = sgui::Point(260, 300);
      new sgui::Button(this, base, sgui::Size(120, 20), L"start", BU_START);
      base+=sgui::Point(0, 40);
      new sgui::Button(this, base, sgui::Size(120, 20), L"record", BU_RECORD);
      base+=sgui::Point(0, 40);
      new sgui::Button(this, base, sgui::Size(120, 20), L"config", BU_CONFIG);
      base+=sgui::Point(0, 40);
      new sgui::Button(this, base, sgui::Size(120, 20), L"exit", BU_EXIT);

      char buf[128];
#ifdef EXCEPTION_TRIAL
      sprintf(buf, "version t%.2f (c)2007 primitive", float(EXCEPTION_VERSION)/100.0f+0.0001);
#else // EXCEPTION_TRIAL 
      sprintf(buf, "version %.2f (c)2007 primitive", float(EXCEPTION_VERSION)/100.0f+0.0001);
#endif // EXCEPTION_TRIAL 
      new sgui::Label(this, sgui::Point(450, 465), sgui::Size(260, 20), _L(buf));

      update();
    }

    ~TitlePanel()
    {
      m_background->release();
      g_title_panel = 0;
    }

    void destroy()
    {
      g_title_panel = 0;

      Super::destroy();
    }

    void update()
    {
      Super::update();
      m_background->update();

#ifdef EXCEPTION_ENABLE_NETUPDATE
      if(updater::g_patch_version > EXCEPTION_VERSION) {
        char buf[256];
        sprintf(buf, "version %.2f へのパッチが見つかりました。\nアップデートを行いますか？", float(updater::g_patch_version)/100.0f+0.001);
        new sgui::ConfirmDialog(L"update", _L(buf), DL_UPDATE, this);
        updater::g_patch_version = EXCEPTION_VERSION;
      }
#endif // EXCEPTION_ENABLE_NETUPDATE 
    }

    void draw()
    {
#ifdef EXCEPTION_ENABLE_PROFILE
      Uint32 t = sgui::GetTicks();
#endif // EXCEPTION_ENABLE_PROFILE 
      {
        ViewportSaver vs;
        fitViewport();

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_background->draw();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
      }
      glColor4fv(vector4(0,0,0, 0.2f).v);
      sgui::DrawRect(sgui::Rect(sgui::Point(0,-1), getSize()+sgui::Size(1,2)));
      glColor4f(1,1,1,1);

      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);
      texture_ptr tex = GetTexture("title.png");
      tex->assign();
      sgui::DrawRect(sgui::Rect(sgui::Point(125, 80), sgui::Size(400, 120)));
      tex->disassign();
      glDisable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);

#ifdef EXCEPTION_ENABLE_PROFILE
      AddDrawTime(float(sgui::GetTicks()-t));
#endif // EXCEPTION_ENABLE_PROFILE 
    }

    bool handleEvent(const sgui::Event& evt)
    {
      size_t id = evt.getSrc() ? evt.getSrc()->getID() : 0;

      if(evt.getType()==sgui::EVT_BUTTON_UP) {
        if(id==BU_START) {
          if(!g_start_dialog) {
            new StartDialog(this);
          }
          return true;
        }
        else if(id==BU_CONFIG) {
          if(!g_config_dialog) {
            new ConfigDialog(this);
          }
          return true;
        }
        else if(id==BU_RECORD) {
          if(!g_record_dialog) {
            new RecordDialog(this);
          }
          return true;
        }
        else if(id==BU_EXIT) {
          FadeToExit();
          return true;
        }
      }
      else if(evt.getType()==sgui::EVT_CONFIRMDIALOG) {
        const sgui::DialogEvent& e = dynamic_cast<const sgui::DialogEvent&>(evt);
#ifdef EXCEPTION_ENABLE_NETUPDATE
        if(id==DL_UPDATE && e.isOK()) {
          ExecUpdater();
          return true;
        }
#endif // EXCEPTION_ENABLE_NETUPDATE 
      }
      else if(evt.getType()==sgui::EVT_KEYUP) {
        const sgui::KeyboardEvent& e = dynamic_cast<const sgui::KeyboardEvent&>(evt);
        if(e.getKey()==sgui::KEY_ESCAPE) {
          FadeToExit();
          return true;
        }
        else if(e.getKey()==sgui::KEY_F10) {
          if(!g_config_dialog) {
            new ConfigDialog(this);
          }
          else {
            g_config_dialog->destroy();
          }
          return true;
        }
        else if(e.getKey()==sgui::KEY_F7) {
#ifdef EXCEPTION_ENABLE_STATE_SAVE
          if(!g_state_dialog) {
            new StateDialog();
          }
          else {
            g_state_dialog->destroy();
          }
          return true;
#endif // EXCEPTION_ENABLE_STATE_SAVE 
        }
        else if(e.getKey()==sgui::KEY_F11) {
#ifdef EXCEPTION_ENABLE_PROFILE
          if(!g_profile_dialog) {
            new ProfileDialog(this);
          }
          else {
            g_profile_dialog->destroy();
          }
          return true;
#endif // EXCEPTION_ENABLE_PROFILE 
        }
      }

      return sgui::Panel::handleEvent(evt);
    }
  };




  void CreateTitlePanel(sgui::Window *parent)
  {
    if(!g_title_panel) {
      new TitlePanel(parent);
      if(g_chat_dialog) {
        g_chat_dialog->setParent(g_title_panel);
      }
    }
  }

  void CreateGamePanel(IGame *game)
  {
    if(!g_game_panel) {
      new GamePanel(game);
      if(g_chat_dialog) {
        g_chat_dialog->setParent(g_game_panel);
      }
    }
  }

  void SaveState(const string& path)
  {
    if(g_game_panel) {
      g_game_panel->saveState(path);
    }
  }

  void LoadState(const string& path)
  {
    if(g_title_panel) {
      g_title_panel->destroy();
    }
    if(!g_game_panel) {
      new GamePanel(0);
    }
    g_game_panel->loadState(path);
  }

  void LoadState(Deserializer& s)
  {
    if(g_title_panel) {
      g_title_panel->destroy();
    }
    if(!g_game_panel) {
      new GamePanel(0);
    }
    g_game_panel->loadState(s);
  }

  void Pause()
  {
    if(!g_pause_panel && !g_continue_panel) {
      new PausePanel(g_game_panel);
    }
  }

  void Resume()
  {
    if(g_pause_panel) {
      g_pause_panel->resume();
    }
    if(g_continue_panel) {
      g_continue_panel->resume();
    }
  }

  void CreateContinuePanel()
  {
    new ContinuePanel(g_game_panel);
  }

  sgui::Window* GetGameWindow() { return g_game_panel; }
  sgui::Window* GetTitleWindow() { return g_title_panel; }
} // exception 



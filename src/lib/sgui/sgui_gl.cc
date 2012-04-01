#include <algorithm>
#include <cassert>
#include <cmath>

#include "sgui.h"




#include <SDL/SDL_opengl.h>

namespace sgui {

int GetTicks() { return SDL_GetTicks(); }
void Sleep(int ms) { return SDL_Delay(ms); }


// GL_MODELVIEW_MATRIX | GL_PROJECTION_MATRIX | GL_TEXTURE_MATRIX
MatrixSaver::MatrixSaver(int matrix_type = GL_MODELVIEW_MATRIX) {
  glGetFloatv(matrix_type, m_matrix);

  switch(matrix_type) {
  case GL_MODELVIEW_MATRIX:
    m_matrix_mode = GL_MODELVIEW;
    break;
  case GL_PROJECTION_MATRIX:
    m_matrix_mode = GL_PROJECTION;
    break;
  case GL_TEXTURE_MATRIX:
    m_matrix_mode = GL_TEXTURE;
    break;
  default:
    m_matrix_mode = GL_MODELVIEW;
    break;
  }
}

MatrixSaver::~MatrixSaver() {
  int current_matrix_mode;
  glGetIntegerv(GL_MATRIX_MODE, &current_matrix_mode);

  glMatrixMode(m_matrix_mode);
  glLoadMatrixf(m_matrix);
  glMatrixMode(current_matrix_mode);
}

ProjectionMatrixSaver::ProjectionMatrixSaver() : MatrixSaver(GL_PROJECTION_MATRIX) {}

ModelviewMatrixSaver::ModelviewMatrixSaver() : MatrixSaver(GL_MODELVIEW_MATRIX) {}





void AssignColor(const Color& c)
{
  glColor4fv(c.v);
}

void AssignColorO(const Color& c, float opa)
{
  Color a = c;
  a.a*=opa;
  glColor4fv(a.v);
}

void DrawPoint(const Point& p) {
  glBegin(GL_POINTS);
  glVertex2f(p.getX(), p.getY());
  glEnd();
}

void DrawLine(const Point& p1, const Point& p2)
{
  glBegin(GL_LINES);
  glVertex2f(p1.getX(), p1.getY());
  glVertex2f(p2.getX(), p2.getY());
  glEnd();
}

void DrawRectEdge(const Rect& r)
{
  glBegin(GL_LINE_LOOP);
  glVertex2f(r.getLeft(), r.getTop());
  glVertex2f(r.getRight(), r.getTop());
  glVertex2f(r.getRight(), r.getBottom());
  glVertex2f(r.getLeft(), r.getBottom());
  glEnd();
}

void DrawRect(const Rect& r)
{
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(r.getLeft(), r.getBottom());
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(r.getRight(), r.getBottom());
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(r.getRight(), r.getTop());
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(r.getLeft(), r.getTop());
  glEnd();
}


Font::Font(const string& path, size_t size) : m_font(0), m_size(size)
{
  m_font = new FTGLTextureFont(path.c_str());
  if(m_font->Error()==0) {
    m_font->FaceSize(m_size);
  }
  else {
    delete m_font;
    throw InitializeError("Font::Font()");
  }
}

Font::~Font()
{
  delete m_font;
}

void Font::draw(const wstring& str, const Rect& r, int style)
{
  if(!m_font || str.empty()) {
    return;
  }

  MatrixSaver ms;

  Rect rect = r;
  if(rect.getWidth()==0.0f) {
    rect = Rect(getAdvance(str.c_str(), str.size()), 0.0f, getHeight(), 0.0f);
  }


  Point rel = rect.getTopLeft();
  rel.setY(rel.getY()+getHeight());
  if((style & VCENTER)==VCENTER) {
    rel.setY(rel.getY()+(rect.getHeight()-getHeight(str.c_str(), str.size()))*0.5f-1.0f);
  }
  else if((style & VBOTTOM)==VBOTTOM) {
    rel.setY(rect.getBottom()-getHeight(str.c_str(), str.size()));
  }

  if((style & HCENTER)==HCENTER) {
    rel.setX(rel.getX()+(rect.getWidth()-getAdvance(str.c_str(), str.size()))*0.5f);
  }
  else if((style & HRIGHT)==HRIGHT) {
    rel.setX(rect.getRight()-getAdvance(str.c_str(), str.size()));
  }

  glEnable(GL_TEXTURE_2D);
  glTranslatef(rel.getX(), rel.getY(), 0.0f);
  glScalef(1.0f, -1.0f, 1.0f);

  static wstring buf;
  size_t i = 0;
  float y = 0.0f;
  for(;;) {
    float width = 0.0f;
    for(;;) {
      wchar_t c = str[i];
      if(c=='\n' || c==0) {
        ++i;
        break;
      }
      else {
        float adv = getAdvance(&c, 1);
        width+=adv;
        if(width>rect.getWidth()) {
          break;
        }
        buf+=c;
        ++i;
      }
    }
    glPushMatrix();
    m_font->Render(buf.c_str());
    glPopMatrix();
    buf.clear();

    if(i>=str.size()) {
      break;
    }
    else {
      y+=getHeight();
      if(y>rect.getHeight()) {
        break;
      }
      glTranslatef(0, -getHeight(), 0.0f);
    }
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

float Font::getHeight(const wchar_t *str, size_t len) const
{
  int l = 1;
  for(size_t i=0; i<len; ++i) {
    if(str[i]=='\n') {
      ++l;
    }
  }
  return m_size*l;
}

float Font::getAdvance(const wchar_t *str, size_t len) const
{
  float longest = 0.0f;
  float tmp = 0.0f;
  for(size_t i=0; i<len; ++i) {
    if(str[i]=='\n') {
      if(tmp > longest) {
        longest = tmp;
      }
      tmp = 0.0f;
      continue;
    }
    if(isascii(str[i])) {
      tmp+=(m_size/2);
    }
    else {
      tmp+=(m_size);
    }
  }
  if(tmp > longest) {
    longest = tmp;
  }
  return longest;
}



StopWatch::StopWatch() :
  m_start_time(0), m_end_time(0)
{}

void StopWatch::run(size_t limit)
{
  m_start_time = SDL_GetTicks();
  m_end_time = m_start_time+limit;
}

void StopWatch::reset()
{
  m_start_time = 0;
  m_end_time = 0;
}

void StopWatch::stop()
{
  m_end_time = SDL_GetTicks();
}

bool StopWatch::isRunning() const
{
  size_t t = SDL_GetTicks();
  return t>=m_start_time && t < m_end_time;
}

bool StopWatch::isFinished() const
{
  return m_start_time!=0 && SDL_GetTicks()>=m_end_time;
}

bool StopWatch::isActive() const
{
  return isRunning() || isFinished();
}

size_t StopWatch::getLeft() const
{
  size_t t = SDL_GetTicks();
  if(m_end_time > t) {
    return m_end_time-t;
  }
  return 0;
}

size_t StopWatch::getPast() const
{
  return std::min<size_t>(SDL_GetTicks()-m_start_time, m_end_time-m_start_time);
}


Uint32 Timer::callback(Uint32 interval, void *param)
{
  ((Timer*)param)->onTimer();
  return interval;
}

Timer::Timer() :
  m_timerid(0), m_target(0), m_autorestart(0)
{
}

Timer::~Timer()
{
  stop();
}

void Timer::setTarget(Window *target) { m_target = target; }

void Timer::run(size_t interval, bool autorestart)
{
  if(m_timerid) {
    stop();
  }
  m_autorestart = autorestart;
  m_timerid = SDL_AddTimer(interval, &callback, this);
}

void Timer::stop() {
  if(m_timerid) {
    SDL_RemoveTimer(m_timerid);
    m_timerid = 0;
  }
}

void Timer::onTimer()
{
  App::instance()->getEventServer()->queueEvent(new TimerEvent(EVT_TIMER, 0, m_target));
  if(!m_autorestart) {
    stop();
  }
}





/*
  Window 
*/

void Window::fitViewport() const
{
  View *view = View::instance();
  Point gp = getGlobalPosition();

  if(!isFloating()) {
    Rect rb(gp, getSize());
    Rect r(rb);
    clip(r, this);
    Point gap = rb.getTopLeft()-r.getTopLeft();

    view->setViewport(
      Point(gp.getX()-gap.getX(), View::instance()->getSize().getHeight()-(gp.getY()+r.getHeight()-gap.getY())),
      Size(r.getWidth(), r.getHeight()));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-gap.getX()-1, r.getWidth()+1-gap.getX(), r.getHeight()-gap.getY()+1, -gap.getY()-1, -1,5000);
    glMatrixMode(GL_MODELVIEW);
  }
  else {
    view->fitViewport();
    glTranslatef(gp.getX(), gp.getY(), 0.0f);
  }
}



/*
  View
*/

View* View::m_instance = 0;

View* View::instance()
{
  return m_instance;
}

View::View(const string& title, const Size& window_size, const Size& gl_size, bool full) :
  Window(0), m_width_scale(1.0f), m_height_scale(1.0f), m_fullscreen(full)
{
  assert(m_instance==0);

  m_instance = this;
  if(window_size.getWidth()>0.0f) {
    m_width_scale = gl_size.getWidth()/window_size.getWidth();
    m_height_scale = gl_size.getHeight()/window_size.getHeight();
  }

  setSize(window_size);
  SDL_WM_SetCaption(title.c_str(), 0);
}

void View::setSize(const Size& size)
{
  Window::setSize(Size(size.getWidth()*m_width_scale, size.getHeight()*m_height_scale));
  m_window_size = size;

  const SDL_VideoInfo *info = SDL_GetVideoInfo();
  int bpp = info->vfmt->BitsPerPixel;
  int flag = SDL_OPENGL;
  if(m_fullscreen) {
    flag|=SDL_FULLSCREEN;
    bpp = 32;
  }

  if(SDL_SetVideoMode(int(size.getWidth()), int(size.getHeight()), bpp, flag)==0) {
    throw std::runtime_error(SDL_GetError());
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glClearColor(0, 0, 0, 0);

  App::instance()->initOpenGLResource();
}

namespace {
  typedef std::vector<Window*> draw_queue;
  draw_queue g_draw;

  void gAdd(Window *w)
  {
    if(!w->isVisible()) {
      return;
    }

    g_draw.push_back(w);
    Window::window_cont& wc = w->getChildren();
    for(Window::window_cont::iterator p=wc.begin(); p!=wc.end(); ++p) {
      gAdd(*p);
    }
  }


  struct greater_draw_priority
  {
    bool operator()(const Window *l, const Window *r) {
      return l->getDrawPriority() < r->getDrawPriority();
    }
  };

  void gDraw()
  {
    std::stable_sort(g_draw.begin(), g_draw.end(), greater_draw_priority());
    for(draw_queue::iterator p=g_draw.begin(); p!=g_draw.end(); ++p) {
      MatrixSaver pm(GL_PROJECTION_MATRIX);
      MatrixSaver mm(GL_MODELVIEW_MATRIX);
      (*p)->fitViewport();
      (*p)->draw();
    }
  }

  void gClear()
  {
    g_draw.clear();
  }
}

void View::draw()
{
  setViewport(getPosition(), getSize());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,getSize().getWidth(), getSize().getHeight(),0, -1,5000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  const Color& col = getBackgroundColor();
  glClearColor(col.r, col.g, col.b, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for(window_cont::const_iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    gAdd(*p);
  }
  gDraw();
  gClear();

  SDL_GL_SwapBuffers();
}

void View::setViewport(const Point& pos, const Size& size) const
{
  glViewport(
    GLint(pos.getX()/m_width_scale), GLint(pos.getY()/m_height_scale),
    GLsizei(size.getWidth()/m_width_scale), GLsizei(size.getHeight()/m_height_scale));
}

Point View::toOpenGLCoord(const Point& point) const
{
  return Point(point.getX()*m_width_scale, point.getY()*m_height_scale);
}

Point View::toWindowCoord(const Point& point) const
{
  return Point(point.getX()/m_width_scale, point.getY()/m_height_scale);
}

const Size& View::getWindowSize() const
{
  return m_window_size;
}







/*
  App
*/

MouseButton TranslateSDLButton(Uint8 SDLButton) {
  int Button = MOUSE_LEFT;
  switch (SDLButton) {
  case SDL_BUTTON_LEFT: Button = MOUSE_LEFT; break;
  case SDL_BUTTON_RIGHT: Button = MOUSE_RIGHT; break;
  case SDL_BUTTON_MIDDLE: Button = MOUSE_MIDDLE; break;
  case SDL_BUTTON_WHEELUP: Button = MOUSE_WHEELUP; break;
  case SDL_BUTTON_WHEELDOWN: Button = MOUSE_WHEELDOWN;  break;
  default: break;
  }
  return MouseButton(Button);
}

MouseButton TranslateSDLButtonState(Uint8 SDLButtonState) {
  int Button = MOUSE_LEFT;
  if(SDLButtonState & SDL_BUTTON(1)) Button |= MOUSE_LEFT;
  if(SDLButtonState & SDL_BUTTON(2)) Button |= MOUSE_MIDDLE;
  if(SDLButtonState & SDL_BUTTON(3)) Button |= MOUSE_RIGHT;
  if(SDLButtonState & SDL_BUTTON(4)) Button |= MOUSE_WHEELUP;
  if(SDLButtonState & SDL_BUTTON(5)) Button |= MOUSE_WHEELDOWN;
  return MouseButton(Button);
}

App* App::instance() { return m_instance; }

void handleSDLEvent(const SDL_Event& evt)
{
  App *app = App::instance();
  switch(evt.type) {
  case SDL_ACTIVEEVENT:
    {
      EventType type = EVT_UNKNOWN;
      if(evt.active.state & SDL_APPMOUSEFOCUS) {
        type = evt.active.gain==1 ? EVT_APP_GAININGMOUSEFOCUS : EVT_APP_LOSINGMOUSEFOCUS;
      }
      else if(evt.active.state & SDL_APPINPUTFOCUS) {
        type = evt.active.gain==1 ? EVT_APP_GAININGKEYFOCUS : EVT_APP_LOSINGKEYFOCUS;
      }
      else if(evt.active.state & SDL_APPACTIVE) {
        type = evt.active.gain==1 ? EVT_APP_UNICONIZE : EVT_APP_ICONIZE;
      }
      if(type!=EVT_UNKNOWN) {
        app->getEventServer()->queueEvent(new Event(type, 0, 0));
      }
    }
    break;

  case SDL_VIDEORESIZE:
    app->getEventServer()->queueEvent(new ResizeEvent(
      EVT_APP_RESIZE, 0, 0, Size(evt.resize.w, evt.resize.h), Size()));
    break;

  case SDL_KEYDOWN:
    app->getEventServer()->queueEvent(new KeyboardEvent(
      EVT_KEYDOWN, 0, app->getKeyFocus(), KeyCode(evt.key.keysym.sym),  evt.key.keysym.unicode));
    break;
  case SDL_KEYUP:
    app->getEventServer()->queueEvent(new KeyboardEvent(
      EVT_KEYUP, 0, app->getKeyFocus(), KeyCode(evt.key.keysym.sym), evt.key.keysym.unicode));
    break;

  case SDL_MOUSEBUTTONDOWN:
    app->getEventServer()->queueEvent(new MouseEvent(
      EVT_MOUSE_BUTTONDOWN, 0, app->getMouseFocus(),
      Point(evt.button.x, evt.button.y), Point(),
      TranslateSDLButton(evt.button.button)));
    break;
  case SDL_MOUSEBUTTONUP:
    app->getEventServer()->queueEvent(new MouseEvent(
      EVT_MOUSE_BUTTONUP, 0, app->getMouseFocus(),
      Point(evt.button.x, evt.button.y), Point(),
      TranslateSDLButton(evt.button.button)));
    break;
  case SDL_MOUSEMOTION:
    app->getEventServer()->queueEvent(new MouseEvent(
      EVT_MOUSE_MOVE, 0, app->getMouseFocus(),
      Point(evt.motion.x, evt.motion.y), Point(evt.motion.xrel, evt.motion.yrel),
      TranslateSDLButtonState(evt.motion.state)));
    break;

  case SDL_JOYBUTTONDOWN:
    app->getEventServer()->queueEvent(new JoyEvent(
      EVT_JOY_BUTTONDOWN, 0, app->getKeyFocus(),
      evt.jbutton.which, evt.jbutton.button,
      0, 0));
    break;
  case SDL_JOYBUTTONUP:
    app->getEventServer()->queueEvent(new JoyEvent(
      EVT_JOY_BUTTONUP, 0, app->getKeyFocus(),
      evt.jbutton.which, evt.jbutton.button,
      0, 0));
    break;
  case SDL_JOYAXISMOTION:
    app->getEventServer()->queueEvent(new JoyEvent(
      EVT_JOY_AXIS, 0, app->getKeyFocus(),
      evt.jaxis.which, 0, evt.jaxis.axis, evt.jaxis.value));
    break;

  case SDL_QUIT:
    app->getEventServer()->queueEvent(new Event(EVT_APP_EXIT, 0, 0));
    break;
  }
};

void App::waitSDLEvent()
{
  SDL_Event e;
  while(!m_end_flag && SDL_PollEvent(&e)) {
    handleSDLEvent(e);
    if(e.type==SDL_QUIT) {
      m_end_flag = true;
    }
  }
}

App* App::m_instance = 0;

App::App(int argc, char **argv) :
  m_argc(argc),
  m_argv(argv),
  m_default_font_size(12),
  m_default_font(0),
  m_end_flag(false),
  m_view(0),
  m_key_focus_window(0),
  m_mouse_focus_window(0)
{
  assert(m_instance==0);
  m_instance = this;
  m_evtserver = new EventServer();

  if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)<0) {
    throw std::runtime_error(SDL_GetError());
  }

  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  setDefaultForegroundColor(Color());
  setDefaultBackgroundColor(Color(0.0f, 0.0f, 0.0f, 0.7f));
  setDefaultBorderColor(Color());

#ifdef _WIN32
  char path[256];
  if(GetWindowsDirectoryA(path, 256)) {
    string font = "\\Fonts\\";
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(GetVersionEx(&osvi) && osvi.dwMajorVersion>=6) { // vistaˆÈ~‚Ìê‡meiryo.ttcŽg—p 
      font+="meiryo.ttc";
    }
    else {
      font+="msgothic.ttc";
    }
    setDefaultFont(string(path)+font);
  }
#else
  setDefaultFont("/usr/share/fonts/truetype/ipa/ipagui.ttf");
#endif
}

App::~App()
{
  for(font_cont::iterator p=m_fonts.begin(); p!=m_fonts.end(); ++p) {
    delete p->second;
  }
  m_fonts.clear();

  View::instance()->release();

  SDL_Quit();
  delete m_evtserver;
}

int App::exec()
{
  loop();
  release();
  return 0;
}

void App::exit()
{
  m_end_flag = true;
}

void App::loop()
{
  while(!m_end_flag) {
    update();
    draw();
    SDL_Delay(1);
  }
}

void App::update()
{
  waitSDLEvent();
  getEventServer()->deliverEvent();
  View::instance()->update();
}

bool App::handleEvent(const Event& evt)
{
  std::vector<Window*> destroyed;
  if(evt.getType()==EVT_APP_DESTROY_WINDOW) {
    Window *src = evt.getSrc();
    if(std::find(destroyed.begin(), destroyed.end(), src)==destroyed.end()) {
      src->release();
      destroyed.push_back(src);
    }
    return true;
  }
  else if(evt.getType()==EVT_DD_DROP) {
    if(!evt.isHandled()) {
      if(DDItem *dd = dynamic_cast<DDItem*>(evt.getSrc())) {
        dd->setPosition(dd->getDragBeginPosition());
      }
    }
  }

  return false;
}


void App::draw()
{
  View::instance()->draw();
}

EventServer* App::getEventServer() { return m_evtserver; }
const Color& App::getDefaultForegroundColor() const { return m_fg_color; }
const Color& App::getDefaultBackgroundColor() const { return m_bg_color; }
const Color& App::getDefaultBorderColor() const { return m_bd_color; }
Font* App::getDefaultFont() const { return m_default_font; }
Font* App::getFont(const string& path, size_t size) const
{
  char name[128];
  sprintf(name, "%s_%d", path.c_str(), size);
  font_cont::const_iterator i = m_fonts.find(name);
  if(i!=m_fonts.end()) {
    return i->second;
  }
  return 0;
}


Window* App::getKeyFocus() const { return m_key_focus_window; }
Window* App::getMouseFocus() const { return m_mouse_focus_window; }
View* App::getView() { return m_view; }


void App::initOpenGLResource()
{
  loadFont(m_default_font_name, m_default_font_size);
}

bool App::loadFont(const string& path, size_t size)
{
  if(m_fonts.find(path)!=m_fonts.end()) {
    return true;
  }

  try {
    Font *font = new Font(path, size);
    m_fonts[path] = font;
    if(!m_default_font) {
      m_default_font = font;
    }
    return true;
  }
  catch(const InitializeError&) {
    return false;
  }

  return false;
}

void App::releaseFont(const string& path)
{
  delete m_fonts[path];
  m_fonts[path] = 0;
}

void App::setDefaultForegroundColor(const Color& color) { m_fg_color = color; }
void App::setDefaultBackgroundColor(const Color& color) { m_bg_color = color; }
void App::setDefaultBorderColor(const Color& color) { m_bd_color = color; }
void App::setDefaultFont(Font *font) { if(!!font) m_default_font = font; }
void App::setDefaultFont(const string& name) { m_default_font_name = name; }
void App::setDefaultFontSize(size_t v) { m_default_font_size=v; }

void App::setKeyFocus(Window *window)
{
  if(m_key_focus_window!=window) {
    if(m_key_focus_window) {
      getEventServer()->queueEvent(new Event(EVT_LOSINGKEYFOCUS, 0, m_key_focus_window));
    }
    m_key_focus_window = window;
    getEventServer()->queueEvent(new Event(EVT_GAININGKEYFOCUS, 0, m_key_focus_window));
  }
}

void App::setMouseFocus(Window *window)
{
  if(m_mouse_focus_window!=window) {
    if(m_mouse_focus_window) {
      getEventServer()->queueEvent(new Event(EVT_LOSINGMOUSEFOCUS, 0, m_mouse_focus_window));
    }
    m_mouse_focus_window = window;
    getEventServer()->queueEvent(new Event(EVT_GAININGMOUSEFOCUS, 0, m_mouse_focus_window));
  }
}

void App::setView(View *view)
{
  m_view = view;
}

} // sgui

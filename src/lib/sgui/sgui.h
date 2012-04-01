#ifndef sgui_h
#define sgui_h

#pragma warning(disable : 4996)


// FileDialogに対応する。
#define SGUI_ENABLE_FILEDIALOG

#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <map>

#ifdef WIN32
  #include <windows.h>
#else
  #include <dlfcn.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <boost/thread.hpp>
#include <FTGL/FTGLTextureFont.h>
#include <SDL/SDL.h>
#include "sgui.h"



namespace sgui {

typedef size_t wnd_id;

using std::string;
using std::wstring;


//------------------------------
// 例外各種 
//------------------------------

class Error : public std::runtime_error
{
public:
  Error(const string& message) : std::runtime_error(message) {}
};

class NotImplemented : public Error
{
public:
  NotImplemented(const string& message) : Error(message) {}
};

class InitializeError : public Error
{
public:
  InitializeError(const string& message) : Error(message) {}
};



//------------------------------
// 小物 
//------------------------------


inline string Format(const char *format, ...)
{
  char buf[256];
  vsprintf(buf, format, (char*)(&format+1));
  return buf;
}





//------------------------------
// 図形と色 
//------------------------------

struct Point
{
public:
  explicit Point(float x=0.0f, float y=0.0f) : m_x(x), m_y(y) {}
  float getX() const { return m_x; }
  float getY() const { return m_y; }
  void setX(float x) { m_x = x; }
  void setY(float y) { m_y = y; }

  void operator+=(const Point& p) { m_x+=p.m_x; m_y+=p.m_y; }
  void operator-=(const Point& p) { m_x-=p.m_x; m_y-=p.m_y; }
  Point operator+(const Point& p) const { Point r(*this); r+=p; return r; }
  Point operator-(const Point& p) const { Point r(*this); r-=p; return r; }
  Point operator*(float v) const { return Point(getX()*v, getY()*v); }
  Point operator/(float v) const { return Point(getX()/v, getY()/v); }

private:
  float m_x, m_y;
};

struct Size
{
public:
  explicit Size(float w=0.0f, float h=0.0f) : m_width(w), m_height(h) {}
  float getWidth() const { return m_width; }
  float getHeight() const { return m_height; }
  void setWidth(float w) { m_width = w; }
  void setHeight(float h) { m_height = h; }

  void operator+=(const Size& p) { m_width+=p.m_width; m_height+=p.m_height; }
  void operator-=(const Size& p) { m_width-=p.m_width; m_height-=p.m_height; }
  Size operator+(const Size& p) const { Size r(*this); r+=p; return r; }
  Size operator-(const Size& p) const { Size r(*this); r-=p; return r; }
  Size operator*(float v) const { return Size(getWidth()*v, getHeight()*v); }
  Size operator/(float v) const { return Size(getWidth()/v, getHeight()/v); }

private:
  float m_width, m_height;
};

struct Range
{
public:
  explicit Range(float n=0.0f, float x=0.0f) : m_min(n), m_max(x) {}
  float getMin() const { return m_min; }
  float getMax() const { return m_max; }
  float getLength() const { return getMax()-getMin(); }
  float clamp(float v) const { return std::max<float>(getMin(), std::min<float>(v, getMax())); }

  void setMin(float n) { m_min = n; }
  void setMax(float x) { m_max = x; }

private:
  float m_min, m_max;
};

struct Rect
{
public:
  explicit Rect(float r=0.0f, float l=0.0f, float t=0.0f, float b=0.0f) :
    right(r), left(l), top(t), bottom(b) {}
  explicit Rect(const Point& tl, const Point& br) :
    right(br.getX()), left(tl.getX()), top(tl.getY()), bottom(br.getY()) {}
  explicit Rect(const Point& pos, const Size& s) :
    right(pos.getX()+s.getWidth()), left(pos.getX()), top(pos.getY()), bottom(pos.getY()+s.getHeight()) {}
  explicit Rect(const Size& s) :
    right(s.getWidth()), left(0.0f), top(0.0f), bottom(s.getHeight()) {}

  float getLeft() const   { return left; }
  float getRight() const  { return right; }
  float getTop() const    { return top; }
  float getBottom() const { return bottom; }
  float getWidth() const  { return right-left; }
  float getHeight() const { return bottom-top; }
  Point getTopLeft() const { return Point(getLeft(), getTop()); }
  Point getTopRight() const { return Point(getRight(), getTop()); }
  Point getBottomLeft() const { return Point(getLeft(), getBottom()); }
  Point getBottomRight() const { return Point(getRight(), getBottom()); }
  Size getSize() const { return Size(getWidth(), getHeight()); }

  bool isInside(const Point& p) const
  {
    return (p.getX()>=getLeft() && p.getX()<=getRight() && p.getY()>=getTop() && p.getY()<=getBottom());
  }

  bool isInside(const Rect& r) const
  {
    return (isInside(r.getTopLeft())
      && isInside(r.getTopRight())
      && isInside(r.getBottomLeft())
      && isInside(r.getBottomRight()));
  }

  bool isOverlaped(const Rect& r) const
  {
    return (getRight()>=r.getLeft()
      && getLeft()<=r.getRight()
      && getTop()<=r.getBottom()
      && getBottom()>=r.getTop());
  }

  void clip(Rect& r)
  {
    if(!isOverlaped(r)) {
      r = Rect();
      return;
    }

    if(r.left<left)  r.left = left;
    if(r.right>right) r.right = right;
    if(r.top<top)      r.top = top;
    if(r.bottom>bottom) r.bottom = bottom;
  }

  void operator+=(const Point& p) { right+=p.getX(); left+=p.getX(); top+=p.getY(); bottom+=p.getY(); }
  void operator-=(const Point& p) { right-=p.getX(); left-=p.getX(); top-=p.getY(); bottom-=p.getY(); }
  Rect operator+(const Point& p) const { Rect r(*this); r+=p; return r; }
  Rect operator-(const Point& p) const { Rect r(*this); r-=p; return r; }

private:
  float right, left, top, bottom;
};

struct Color
{
public:
  union {
    struct { float r, g, b, a; };
    float v[4];
  };

  explicit Color(float _r=1.0f, float _g=1.0f, float _b=1.0f, float _a=1.0f)
  {
    r=_r; g=_g; b=_b; a=_a;
  }
};




void AssignColor(const Color& c);
void AssignColorO(const Color& c, float opa);
void DrawPoint(const Point& p);
void DrawLine(const Point& p1, const Point& p2);
void DrawRectEdge(const Rect& r);
void DrawRect(const Rect& r);


class MatrixSaver
{
public:
  MatrixSaver(int matrix_type);
  ~MatrixSaver();

private:
  int m_matrix_mode;
  float m_matrix[16];
};

class ProjectionMatrixSaver : public MatrixSaver
{
public:
  ProjectionMatrixSaver();
};

class ModelviewMatrixSaver : public MatrixSaver
{
public:
  ModelviewMatrixSaver();
};





//  RootObject

class Object
{
public:
  Object();
  virtual ~Object();
  virtual void release();
};


//------------------------------
// イベント各種 
//------------------------------

class EventServer;
class EventClient;
class Window;
class ListItem;
class DDItem;


enum KeyCode {
  KEY_A = SDLK_a,
  KEY_B = SDLK_b,
  KEY_C = SDLK_c,
  KEY_D = SDLK_d,
  KEY_E = SDLK_e,
  KEY_F = SDLK_f,
  KEY_G = SDLK_g,
  KEY_H = SDLK_h,
  KEY_I = SDLK_i,
  KEY_J = SDLK_j,
  KEY_K = SDLK_k,
  KEY_L = SDLK_l,
  KEY_M = SDLK_m,
  KEY_N = SDLK_n,
  KEY_O = SDLK_o,
  KEY_P = SDLK_p,
  KEY_Q = SDLK_q,
  KEY_R = SDLK_r,
  KEY_S = SDLK_s,
  KEY_T = SDLK_t,
  KEY_U = SDLK_u,
  KEY_V = SDLK_v,
  KEY_W = SDLK_w,
  KEY_X = SDLK_x,
  KEY_Y = SDLK_y,
  KEY_Z = SDLK_z,

  KEY_PRINT    = SDLK_PRINT,
  KEY_BACKSPACE= SDLK_BACKSPACE,
  KEY_TAB      = SDLK_TAB,
  KEY_ESCAPE   = SDLK_ESCAPE,
  KEY_RETURN   = SDLK_RETURN,
  KEY_SPACE    = SDLK_SPACE,
  KEY_UP       = SDLK_UP,
  KEY_DOWN     = SDLK_DOWN,
  KEY_RIGHT    = SDLK_RIGHT,
  KEY_LEFT     = SDLK_LEFT,
  KEY_INSERT   = SDLK_INSERT,
  KEY_DELETE   = SDLK_DELETE,
  KEY_HOME     = SDLK_HOME,
  KEY_END      = SDLK_END,

  KEY_LSHIFT   = SDLK_LSHIFT,
  KEY_RSHIFT   = SDLK_RSHIFT,
  KEY_LCTRL    = SDLK_LCTRL,
  KEY_RCTRL    = SDLK_RCTRL,
  KEY_LALT     = SDLK_LALT,
  KEY_RALT     = SDLK_RALT,

  KEY_F1       = SDLK_F1,
  KEY_F2       = SDLK_F2,
  KEY_F3       = SDLK_F3,
  KEY_F4       = SDLK_F4,
  KEY_F5       = SDLK_F5,
  KEY_F6       = SDLK_F6,
  KEY_F7       = SDLK_F7,
  KEY_F8       = SDLK_F8,
  KEY_F9       = SDLK_F9,
  KEY_F10      = SDLK_F10,
  KEY_F11      = SDLK_F11,
  KEY_F12      = SDLK_F12,
};

enum MouseButton {
  MOUSE_NONE       = 0,
  MOUSE_LEFT       = 1<<0,
  MOUSE_MIDDLE     = 1<<1,
  MOUSE_RIGHT      = 1<<2,
  MOUSE_WHEELUP    = 1<<3,
  MOUSE_WHEELDOWN  = 1<<4,
};

enum EventType {
  EVT_UNKNOWN = 0,

  EVT_CONSTRUCT,
  EVT_DESTRUCT,

  EVT_APP_DESTROY_WINDOW,
  EVT_APP_EXIT,
  EVT_APP_RESIZE,
  EVT_APP_ACTIVE,
  EVT_APP_ICONIZE,
  EVT_APP_UNICONIZE,
  EVT_APP_GAININGMOUSEFOCUS,
  EVT_APP_GAININGKEYFOCUS,
  EVT_APP_LOSINGKEYFOCUS,
  EVT_APP_LOSINGMOUSEFOCUS,

  EVT_GAININGKEYFOCUS,
  EVT_GAININGMOUSEFOCUS,
  EVT_LOSINGKEYFOCUS,
  EVT_LOSINGMOUSEFOCUS,

  EVT_MOUSE_BUTTONDOWN,   // MouseEvent
  EVT_MOUSE_BUTTONUP,     // MouseEvent
  EVT_MOUSE_MOVE,         // MouseEvent
  EVT_MOUSE_DOUBLECLICKED,// MouseEvent
  EVT_KEYDOWN,            // KeyboardEvent
  EVT_KEYUP,              // KeyboardEvent

  EVT_JOY_BUTTONDOWN,     // JoyEvent
  EVT_JOY_BUTTONUP,       // JoyEvent
  EVT_JOY_AXIS,           // JoyEvent

  EVT_TIMER,              // TimerEvent

  EVT_RESIZE,             // SizeEvent
  EVT_BUTTON_DOWN,        // Event
  EVT_BUTTON_UP,          // Event
//  EVT_CHECK,            // Event
//  EVT_CHECK_UNCHECK,    // Event
  EVT_EDIT,               // EditEvent
  EVT_EDIT_ENTER,         // EditEvent
  EVT_SCROLL,             // ScrollEvent
//  EVT_SLIDER,           // ScrollEvent
//  EVT_RADIOBUTTON,      // Event
  EVT_LIST_SELECT,        // ListEvent
  EVT_LIST_DISSELECT,     // ListEvent
  EVT_LIST_DOUBLECLICK,   // ListEvent
  EVT_COMBO,              // ListEvent
  EVT_DD_DRAG,            // DragAndDropEvent
  EVT_DD_DRAGING,         // DragAndDropEvent
  EVT_DD_DROP,            // DragAndDropEvent
  EVT_DD_RECIEVE,         // DragAndDropEvent
  EVT_CLOSEDIALOG,        // Event
  EVT_MESSAGEDIALOG,      // DialogEvent
  EVT_CONFIRMDIALOG,      // DialogEvent
  EVT_FILEDIALOG,         // DialogEvent

  EVT_USER, // UserEvent
};

class Event : public Object
{
friend class EventServer;
public:
  Event(EventType type, Window *src, Window *dst);

  EventType getType() const;
  Window* getSrc() const;
  Window* getDst() const;
  int isHandled() const;
  int isHandledByControl() const;
  int isHandledByFrame() const;

private:
  const EventType m_type;
  Window *m_src;
  Window *m_dst;
  int m_hcontrol;
  int m_hframe;
};

class DestructEvent : public Event
{
friend class EventServer;
public:
  DestructEvent(EventType type, Window *src, Window *dst, Window *self);
  Window* getSelf() const { return m_self; } // 既にdeleteされたオブジェクトへのポインタを返す。比較以外に使っちゃダメ 
  int getSelfID() { return m_selfid; }

private:
  Window *m_self;
  int m_selfid;
};

class KeyboardEvent : public Event
{
public:
  KeyboardEvent(EventType MessageType, Window *src, Window *dst, KeyCode key, int unicode);

  KeyCode getKey() const { return m_key; }
  int getUnicode() const { return m_unicode; }

private:
  KeyCode m_key;
  int m_unicode;
};

class MouseEvent : public Event
{
public:
  MouseEvent(EventType type, Window *src, Window *dst, Point pos, Point relative, MouseButton button);
  const Point& getPosition() const { return m_position; } // OpenGL座標 
  const Point& getRelative() const { return m_relative; } //
  const Point& getRealPosition() const { return m_real_position; } // Window座標 
  const Point& getRealRelative() const { return m_real_relative; } //
  MouseButton getButton() const { return m_button; }

private:
  Point m_position;
  Point m_relative;
  Point m_real_position;
  Point m_real_relative;
  MouseButton m_button;
};

class JoyEvent : public Event
{
public:
  JoyEvent(EventType type, Window *src, Window *dst, int device, int button, int axis, int value);
  int getButton() const { return m_button; }
  int getAxis() const { return m_axis; }
  int getAxisValue() const { return m_axis_value; }

private:
  int m_devid;
  int m_button;
  int m_axis;
  int m_axis_value;
};



class DragAndDropEvent : public MouseEvent
{
public:
  DragAndDropEvent(EventType type, Window *src, Window *dst, Point pos, Point relative, MouseButton button, DDItem *item) :
    MouseEvent(type, src, dst, pos, relative, button), m_item(item)
  {}
  DDItem* getItem() const { return m_item; }

private:
  DDItem *m_item;
};

class ResizeEvent : public Event
{
public:
  ResizeEvent(EventType type, Window *src, Window *dst, Size size, Size rel) :
    Event(type, src, dst), m_size(size), m_relative(rel)
  {}
  const Size& getSize() const { return m_size; }
  const Size& getRelative() const { return m_relative; }

private:
  Size m_size;
  Size m_relative;
};

class TimerEvent : public Event
{
public:
  TimerEvent(EventType type, Window *src, Window *dst) :
    Event(type, src, dst)
  {}
};

class EditEvent : public Event
{
public:
  EditEvent(EventType type, Window *src, Window *dst, const wstring& str) :
    Event(type, src, dst), m_string(str)
  {}
  const wstring& getString() const { return m_string; }

private:
  wstring m_string;
};

class ScrollEvent : public Event
{
public:
  ScrollEvent(EventType type, Window *src, Window *dst, float value, float rel) :
    Event(type, src, dst), m_value(value), m_relative(rel)
  {}
  float getValue() const { return m_value; }
  float getRelative() const { return m_relative; }

private:
  float m_value;
  float m_relative;
};

class ListEvent : public Event
{
public:
  ListEvent(EventType type, Window *src, Window *dst, size_t index, ListItem *item) :
    Event(type, src, dst), m_index(index), m_item(item)
  {}

  size_t getIndex() const { return m_index; }
  ListItem* getItem() const { return m_item; }

private:
  size_t m_index;
  ListItem *m_item;
};

class DialogEvent : public Event
{
public:
  DialogEvent(EventType type, Window *src, Window *dst, bool ok, const string& path="") :
    Event(type, src, dst), m_ok(ok), m_path(path)
  {}

  bool isOK() const { return m_ok; }
  const string& getPath() const { return m_path; }

private:
  bool m_ok;
  string m_path;
};

class UserEvent : public Event
{
public:
  UserEvent(EventType type, Window *src, Window *dst, const string& name, void *value) :
    Event(type, src, dst), m_name(name), m_value(value)
  {}
  const string& getName() const { return m_name; }
  void* getValue() const { return m_value; }

private:
  string m_name;
  void *m_value;
};




//  EventServer

class EventServer
{
private:
  typedef std::list<EventClient*> eventclient_cont;
  typedef std::map<EventType, eventclient_cont> eventclient_map;
  typedef std::list<Event*> event_cont;

  eventclient_map m_evtclients;
  eventclient_cont m_toplevel;
  event_cont m_events;
  boost::recursive_mutex m_mutex;

public:
  EventServer();
  ~EventServer();
  void registerEventClient(EventClient *client, EventType type);
  void deregisterEventClient(EventClient *client, EventType type);
  void deregisterEventClient(EventClient *client);
  void setTopLevel(EventClient *client);

  void queueEvent(Event *evt); // synchronized
  void deliverEvent(); // synchronized
  void clearEvent(); // synchronized
};



class EventClient : public Object
{
public:
  EventClient();
  virtual ~EventClient();
  virtual void listenEvent(EventType type);
  virtual void queueEvent(Event *e);
  virtual bool handleEvent(const Event& evt);
};







class Font
{
public:
  enum HAlign {
    HLEFT   = 1,
    HRIGHT  = 2,
    HCENTER = 3,
  };
  enum VAlign {
    VTOP    = 4,
    VBOTTOM = 8,
    VCENTER = 12,
  };

  Font(const string& font, size_t size);
  ~Font();
  void draw(const wstring& str, const Rect& rect, int style=1);
  float getHeight(const wchar_t *str=0, size_t length=0) const;
  float getAdvance(const wchar_t *str, size_t length) const;

private:
  FTFont* m_font;
  size_t m_size;
};

class StopWatch
{
public:
  StopWatch();
  void run(size_t limit);
  void reset();
  void stop();
  bool isRunning() const;
  bool isFinished() const;
  bool isActive() const;
  size_t getLeft() const;
  size_t getPast() const;

private:
  size_t m_start_time;
  size_t m_end_time;
};

class Timer
{
public:
  static Uint32 callback(Uint32 interval, void *param);

  Timer();
  ~Timer();
  void run(size_t interval, bool autorestart=false);
  void stop();
  void onTimer();

  void setTarget(Window *target);

private:
  SDL_TimerID m_timerid;
  Window *m_target;
  bool m_autorestart;
};

int GetTicks();
void Sleep(int ms);



//  Window

class Window : public EventClient
{
public:
  typedef std::list<Window*> window_cont;
  typedef std::vector<Window*> window_vector;

  Window(Window *parent, const Point& pos=Point(), const Size& size=Size(), const wstring& text=L"", wnd_id id=0);
  virtual ~Window();
  virtual void releaseChildren();
  virtual void update();
  virtual void draw();
  virtual void destroy();

  virtual bool isInclude(sgui::Window *w);
  virtual bool isInside(const Point& global) const;
  virtual Point toGlobalCoord(const Point& local) const;
  virtual Point toLocalCoord(const Point& global) const;
  virtual void clip(Rect& r, const Window *wnd=0) const;
  virtual void fitViewport() const;

  virtual wnd_id getID() const;
  virtual float getDrawPriority() const;
  virtual const Color&   getForegroundColor() const;
  virtual const Color&   getBackgroundColor() const;
  virtual const Color&   getBorderColor() const;
  virtual const wstring& getText() const;
  virtual const Point&   getPosition() const;
  virtual const Size&    getSize() const;
  virtual const Point&   getClientPosition() const;
  virtual const Size&    getClientSize() const;
  virtual bool isVisible() const;
  virtual bool isFloating() const;
  virtual Point getGlobalPosition() const;

  virtual void setDrawPriority(float priority);
  virtual void setForegroundColor(const Color& color);
  virtual void setBackgroundColor(const Color& color);
  virtual void setBorderColor(const Color& color);
  virtual void setText(const wstring& text);
  virtual void setPosition(const Point& pos);
  virtual void setSize(const Size& size);
  virtual void setFloating(bool floating);
  virtual void setVisible(bool visible);
  virtual void setTopLevel(Window *window);
  virtual void move(const Point& rel);
  virtual void toTopLevel();
  virtual void toTopLevelEvent();

  virtual Window* getParent() const;
  virtual window_cont& getChildren();
  virtual Window* getChildByID(wnd_id id) const;
  virtual void setParent(Window *parent);
  virtual void registerChild(Window *child);
  virtual void deregisterChild(Window *child);

  virtual Font* getFont() const;
  virtual void setFont(Font* font);

protected:
  virtual void drawText(const wstring& str, const Rect& rect, int style=1);

  wnd_id m_id;
  Font *m_font;
  wstring m_text;
  Color m_fg_color;
  Color m_bg_color;
  Color m_bd_color;
  Point m_position;
  Size m_size;
  float m_priority;
  bool m_visible;
  bool m_floating;

  Window *m_parent;
  window_cont m_children;
  window_vector m_tmp;
};








//  Label

class Label : public Window
{
public:
  Label(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id=0);
  virtual void draw();
};




//  Button
//
//  EVT_BUTTON_DOWN: ボタンが押されたとき 
//  EVT_BUTTON_UP: ボタンが離されたとき 

class Button : public Window
{
public:
  enum State {
    UP,
    DOWN,
    DISABLED,
  };

  Button(Window *parent, const Point& pos, const Size& size, const wstring& text=L"", wnd_id id=0);
  virtual void draw();
  virtual bool handleEvent(const Event& evt);

  virtual void up();
  virtual void down();
  virtual State getButtonState() const;
  virtual void setButtonState(State state);

private:
  State m_button_state;
};


class ToggleButton : public Button
{
public:
  ToggleButton(Window *parent, const Point& pos, const Size& size, const wstring& text=L"", wnd_id id=0);
  virtual bool handleEvent(const Event& evt);

  virtual void toggle();

private:
  bool m_clicked;
};




//  RadioButton

/*
enum {
  RB_GROUP = 1,
};

class RadioButton : public Window
{
public:
  RadioButton(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id=0, int style=0);
  virtual void draw();
  virtual bool handleEvent(const Event& evt);

private:
};
*/


//  ToolTip

class ToolTip : public Window
{
public:
  ToolTip(Window *parent, const wstring& text, wnd_id id=0);
  virtual bool handleEvent(const Event& evt);
  virtual void draw();
  virtual float getDrawPriority() const;

private:
  Timer m_timer;
  size_t m_appear_time;
};




//  Edit
//
//  EVT_EDIT : 文字が編集されたとき 
//  EVT_ENTER : フォーカスされていてEnterが押されたとき 

class Edit : public Window
{
public:
  Edit(Window *parent, const Point& pos, const Size& size, const wstring& text=L"", wnd_id id=0);
  virtual void draw();
  virtual bool handleEvent(const Event& evt);

  virtual void focus();
  virtual void defocus();

  virtual bool isReadOnly() const;
  virtual bool isFocused() const;
  virtual void setReadOnly(bool ro);
  virtual void setBlinkerPos(int pos);
  virtual void setText(const wstring& text);

private:
  bool m_readonly;
  int m_blinker;
  bool m_focused;
};




//  DDItem
//
//  EVT_DD_DRAG : クリックされたとき 
//  EVT_DD_DRAGING : ドラッグされたとき 
//  EVT_DD_DROP : ドロップされたとき 

class DDItem : public Window
{
public:
  DDItem(Window *parent, const Point& pos, const Size& size, const wstring& text=L"", wnd_id id=0);
  virtual void draw();
  virtual bool handleEvent(const Event& evt);
  virtual bool isDragging() const;
  virtual const Point& getDragBeginPosition() const;

private:
  StopWatch m_stopwatch; // ダブルクリック検出用 
  Point m_drag_pos;
  bool m_dragging;
};



//  DDReciever
//
//  EVT_DD_RECIEVE : DDItemが範囲内にドロップされたとき 

class DDReciever : public Window
{
public:
  DDReciever(Window *parent, const Point& pos, const Size& size, const wstring& text=L"", wnd_id id=0);
  virtual void draw();
  virtual bool handleEvent(const Event& evt);
  virtual void deregisterChild(Window *child);

  virtual void setItem(DDItem *item);
  virtual DDItem* getItem() const;

private:
  DDItem *m_item;
};



//  ScrollBar
//
//  EVT_SCROLL : 値(setValue)か最大値(setRange)が変更されたとき 

enum {
  SB_HORIZONTAL = 0,
  SB_VERTICAL = 1,
};

class ScrollBar : public Window
{
public:
  enum {
    HORIZONTAL,
    VERTICAL,
  };

  ScrollBar(Window *parent, int style=0, float size=0.0f, wnd_id id=0);
  virtual void draw();
  virtual bool handleEvent(const Event& event);

  virtual void fitTo(Window *target);
  virtual float getValue() const;
  virtual float getThick() const;
  virtual float getStep() const;
  virtual float getRange() const;

  virtual void setScrollBarSize(float size);
  virtual void setThick(float value);
  virtual void setValue(float value);
  virtual void setStep(float value);
  virtual void setRange(float range);

protected:
  float getBarWidth() const;
  Range getScrollBarRange() const;
  void resetBarRect();
  void mouseToScroll(const Point& mouse_move);

private:
  int m_align;
  bool m_dragging;
  float m_thick;
  float m_step;
  float m_scroll_pos;
  float m_range;
  float m_bar_width;
  Button *m_button_tl;
  Button *m_button_br;
  Rect m_bar;
};




enum {
  VSCROLLBAR = 1,
  HSCROLLBAR = 2,
};

class ScrolledWindow : public Window
{
public:
  ScrolledWindow(Window *parent, const Point& pos, const Size& size,
    const wstring& text=L"", wnd_id id=0, int style=VSCROLLBAR);
  virtual void draw();
  virtual bool handleEvent(const Event& event);

  virtual const Point& getScrollPosition() const;
  virtual const Size& getSizeNS() const; // スクロールバーの幅を除いたSize 
  virtual const Size& getScrollSize() const;
  virtual ScrollBar* getVScrollBar() const;
  virtual ScrollBar* getHScrollBar() const;
  virtual void clip(Rect& r, const Window *wnd) const;

  virtual void setSize(const Size& size);
  virtual void setScrollSize(const Size& size);
  virtual void setScrollPosition(const Point& rel);

private:
  Point m_scroll_pos;
  Size m_scroll_size;
  Size m_size_ns;
  ScrollBar *m_hscrollbar;
  ScrollBar *m_vscrollbar;
};




class ListItem : public Window
{
public:
  ListItem(const wstring& text);
  virtual void draw();

  virtual bool isSelected() const;
  virtual void setSelect(bool selected);

  bool m_selected;
};


//  List
//
//  EVT_LIST_SELECT : 要素が選択されたとき 
//  EVT_LIST_DISSELECT : 要素が未選択状態になったとき 
//  EVT_LIST_DOUBLECLICK : 要素がダブルクリックされたとき 

enum {
  LS_MULTISELECT = 1,
};

class List : public ScrolledWindow
{
public:
  typedef std::vector<ListItem*> item_cont;

  List(Window *parent, const Point& pos, const Size& size, wnd_id id=0, int style=0);
  virtual void draw();
  virtual bool handleEvent(const Event& event);

  virtual float getItemHeight() const;
  virtual ListItem* getItem(size_t i);
  virtual ListItem* getSelectedItem(size_t i=0);
  virtual size_t getItemCount() const;
  virtual size_t getItemIndex(ListItem *l) const;
  virtual size_t getSelection(size_t i=0) const;
  virtual size_t getSelectionCount() const;
  virtual size_t hitItem(const Point& p) const;

  virtual void addItem(ListItem *item);
  virtual void removeItem(size_t i);
  virtual void removeItem(ListItem *i);
  virtual void setSelection(size_t i);
  virtual void setSelection(ListItem *i);
  virtual void clearItem();
  virtual void clearSelection();
  virtual void alignItems();
  virtual void resetScrollSize();

private:
  bool m_multiselect;
  float m_item_height;
  item_cont m_items;

  StopWatch m_stopwatch; // ダブルクリック検出用 
  ListItem *m_last_selected;
};



//  Combo
//
//  EVT_COMBO : 要素を選択したとき 

class Combo : public Window
{
public:
  Combo(Window *parent, const Point& pos, const Size& size, wnd_id id=0, int style=0);
  virtual bool handleEvent(const Event& event);
  virtual void draw();

  virtual void addItem(ListItem *item);
  virtual void clearItem();

  virtual List* getList();
  virtual ListItem* getSelection();
  virtual void setSelection(size_t i);
  virtual void setVisible(bool visible);

private:
  Button *m_list_button;
  List *m_list;
  ListItem *m_selection;
};



//  Panel
//
//  EVT_RESIZE : サイズが変更されたとき 

class Panel : public Window
{
public:
  Panel(Window *parent, const Point& pos, const Size& size,
    const wstring& text=wstring(), wnd_id id=0, int style=0);
  virtual void draw();
  virtual bool handleEvent(const Event& event);
  virtual void setSize(const Size& size);
};



//  Dialog
//
//  EVT_CLOSEDIALOG : 閉じたとき 

enum {
  MOVABLE = 1,
  RESIZABLE = 2,
  CLOSE_BUTTON = 4,
  MAXIMIZE_BUTTON = 8,
  MINIMIZE_BUTTON = 16,
  STAY_ON_TOP = 32,
};

class Dialog : public Panel
{
public:
  Dialog(Window *parent, const Point& pos, const Size& size,
    const wstring& text=L"", wnd_id id=0, int style= MOVABLE | RESIZABLE | CLOSE_BUTTON);
  virtual void destroy();
  virtual void draw();
  virtual bool handleEvent(const Event& event);

  virtual const Point& getClientPosition() const;
  virtual const Size& getClientSize() const;
  virtual void setSize(const Size& size);
  virtual void setWidthRange(const Range& r);
  virtual void setHeightRange(const Range& r);

  virtual void onCloseButton();
  virtual void onMaximizeButton();
  virtual void onMinimizeButton();

protected:
  Point m_client_pos;
  Size m_client_size;
  Button *m_close_button;
  Button *m_maximize_button;
  Button *m_minimize_button;

  bool m_movable;
  bool m_moving;

  bool m_resizable;
  int m_resizing;
  Range m_width_range;
  Range m_height_range;
};

//  MessageDialog

class MessageDialog : public Dialog
{
public:
  MessageDialog(const wstring& title, const wstring& message, Window *parent=0,
    const Point& pos=Point(), const Size& size=Size(320, 200));
  virtual bool handleEvent(const Event& event);

protected:
  Button *m_ok_button;
  Label *m_message;
};


//  ConfirmDialog
//
//  EVT_CONFIRMDIALOG

class ConfirmDialog : public Dialog
{
public:
  ConfirmDialog(const wstring& title, const wstring& message, wnd_id id, Window *parent=0,
    const Point& pos=Point(200,200), const Size& size=Size(240.0f, 160.0f));
  virtual bool handleEvent(const Event& event);

protected:
  Button *m_ok_button;
  Button *m_cancel_button;
  Label *m_message;
};


//  FileDialog
//
//  EVT_FILEDIALOG
#ifdef SGUI_ENABLE_FILEDIALOG

bool IsFile(const string& path);
bool IsDir(const string& path);
bool MakeDir(const string& path);
bool MakeDeepDir(const string& path);
bool RemoveDir(const string& path);
bool Remove(const string& path);
string GetCWD();

wstring _L(const string& src);
string _S(const wstring& src);

class Dir
{
public:
  typedef std::vector<string> path_cont;
  typedef path_cont::iterator iterator;
  typedef path_cont::const_iterator const_iterator;

  Dir();
  explicit Dir(const string& path);
  bool open(const string& path);
  bool openRecursive(const string& path);

  size_t size() const;
  const string& operator[](size_t i) const;
  const string& getPath() const;

  iterator begin() { return m_files.begin(); }
  iterator end()   { return m_files.end(); }
  const_iterator begin() const { return m_files.begin(); }
  const_iterator end() const   { return m_files.end(); }

private:
  string m_path;
  path_cont m_files;
};


class FileDialog : public Dialog
{
public:
  FileDialog(const wstring& title, Window *parent=0, const Point& pos=Point(), const Size& size=Size());
  virtual bool handleEvent(const Event& event);
  virtual bool openDir(const string& path);

protected:
  Button *m_ok_button;
  Button *m_cancel_button;
  List *m_filelist;
  Combo *m_locate;
  string m_path;
  string m_file;
};

#endif


//  View

class View : public Window
{
public:
  static View* instance();

  View(const string& title, const Size& window_size, const Size& gl_size=Size(), bool full=false);
  virtual void draw();

  virtual void setViewport(const Point& pos, const Size& size) const;
  virtual Point toOpenGLCoord(const Point& point) const; // Window座標をOpenGL内座標に変換 
  virtual Point toWindowCoord(const Point& point) const; // OpenGL内座標をWindow座標に変換 
  virtual void setSize(const Size& size);

  const Size& getWindowSize() const;

private:
  static View *m_instance;
  string m_title;
  float m_width_scale;
  float m_height_scale;
  bool m_fullscreen;
  Size m_window_size;
  Size m_screen_size;
  Point m_client_pos;
};




/*
  App
*/

class App : public Object
{
friend class View;
public:
  static App* instance();

  App(int argc=0, char **argv=0);
  virtual ~App();

  virtual void loop();
  virtual void update();
  virtual void draw();

  virtual bool handleEvent(const Event& event);
  virtual int exec();
  virtual void exit();
  virtual void initOpenGLResource();

  virtual EventServer* getEventServer();
  virtual const Color& getDefaultForegroundColor() const;
  virtual const Color& getDefaultBackgroundColor() const;
  virtual const Color& getDefaultBorderColor() const;
  virtual Font* getDefaultFont() const;
  virtual Font* getFont(const string& path, size_t size) const;
  virtual Window* getKeyFocus() const;
  virtual Window* getMouseFocus() const;
  virtual View* getView();

  virtual bool loadFont(const string& path, size_t size);
  virtual void releaseFont(const string& path);
  virtual void setDefaultForegroundColor(const Color& color);
  virtual void setDefaultBackgroundColor(const Color& color);
  virtual void setDefaultBorderColor(const Color& color);
  virtual void setDefaultFont(Font *font);
  virtual void setDefaultFont(const string& name);
  virtual void setDefaultFontSize(size_t);
  virtual void setKeyFocus(Window *window);
  virtual void setMouseFocus(Window *window);
  virtual void setView(View *view);

protected:
  typedef std::map<string, Font*> font_cont;

  void waitSDLEvent();

  static App *m_instance;

  EventServer *m_evtserver;
  int m_argc;
  char **m_argv;

  Color m_fg_color;
  Color m_bg_color;
  Color m_bd_color;
  string m_default_font_name;
  size_t m_default_font_size;
  Font *m_default_font;
  font_cont m_fonts;

  bool m_end_flag;
  View *m_view;
  Window *m_key_focus_window;
  Window *m_mouse_focus_window;
};

} // sgui


#endif // SGUI_H

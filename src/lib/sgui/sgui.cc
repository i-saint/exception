#include <algorithm>
#include "sgui.h"


namespace sgui {



//  RootObject

Object::Object() {}
Object::~Object() {}
void Object::release() {
//  printf("sgui::Object::~Object() : %s\n", typeid(*this).name());
  delete this;
}

#ifdef SG_BUILD_DSO
void* Object::operator new(size_t size) { return malloc(size); }
void Object::operator delete(void *p) { return free(p); }
#endif // SG_BUILD_DSO



//  Event

Event::Event(EventType type, Window *src, Window *dst) :
  m_type(type), m_src(src), m_dst(dst), m_hcontrol(0), m_hframe(0)
{
}

EventType Event::getType(void) const { return m_type; }
Window* Event::getSrc() const { return m_src; }
Window* Event::getDst() const { return m_dst; }
int Event::isHandled() const { return m_hcontrol+m_hframe; }
int Event::isHandledByControl() const { return m_hcontrol; }
int Event::isHandledByFrame() const { return m_hframe; }


DestructEvent::DestructEvent(EventType type, Window *src, Window *dst, Window *self) :
  Event(type, src, dst), m_self(self), m_selfid(self->getID())
{
}

KeyboardEvent::KeyboardEvent(EventType type, Window *src, Window *dst, KeyCode key, int unicode) :
  Event(type, src, dst), m_key(key), m_unicode(unicode)
{
}

MouseEvent::MouseEvent(EventType type, Window *src, Window *dst,
  Point point, Point relative, MouseButton button) :
  Event(type, src, dst), m_real_position(point), m_real_relative(relative), m_button(button)
{
  m_position = View::instance()->toOpenGLCoord(m_real_position);
  m_relative = View::instance()->toOpenGLCoord(m_real_relative);
}


JoyEvent::JoyEvent(EventType type, Window *src, Window *dst,
  int devid, int button, int axis, int value) :
  Event(type, src, dst), m_devid(devid), m_button(button), m_axis(axis), m_axis_value(value)
{
}







//  EventServer

EventServer::EventServer() {}
EventServer::~EventServer() { clearEvent(); }

void EventServer::registerEventClient(EventClient *client, EventType type)
{
  if(!client) {
    return;
  }

  eventclient_cont& cont = m_evtclients[type];
  eventclient_cont::iterator p = std::find(cont.begin(), cont.end(), client);
  if(p==cont.end()) {
    cont.push_front(client);
  }
}

void EventServer::deregisterEventClient(EventClient *client, EventType type)
{
  eventclient_cont& cont = m_evtclients[type];
  cont.remove(client);
}

void EventServer::deregisterEventClient(EventClient *client)
{
  for(eventclient_map::iterator p=m_evtclients.begin(); p!=m_evtclients.end(); ++p) {
    p->second.remove(client);
  }
  m_toplevel.remove(client);

  for(event_cont::iterator p=m_events.begin(); p!=m_events.end(); /**/) {
    Event *e = *p;
    if(e->getSrc()==client || e->getDst()==client) {
      m_events.erase(p++);
      e->release();
    }
    else {
      ++p;
    }
  }
}

void EventServer::setTopLevel(EventClient *client)
{
  m_toplevel.remove(client);
  m_toplevel.push_back(client);
}

void EventServer::queueEvent(Event *evt)
{
  boost::recursive_mutex::scoped_lock lock(m_mutex);

  if(evt) {
    m_events.push_back(evt);
  }
}

void EventServer::deliverEvent()
{
  boost::recursive_mutex::scoped_lock lock(m_mutex);

  while(!m_events.empty()) {
    Event *e = m_events.front();
    m_events.pop_front();
    eventclient_cont& cli = m_evtclients[e->getType()];

    for(eventclient_cont::iterator q=cli.begin(); q!=cli.end(); ++q) {
      EventClient *c = *q;
      if(c->handleEvent(*e)) {
        if(Panel *panel = dynamic_cast<Panel*>(c)) {
          e->m_hframe++;
        }
        else {
          e->m_hcontrol++;
        }
      }
    }

    for(eventclient_cont::iterator q=m_toplevel.begin(); q!=m_toplevel.end(); ++q) {
      for(eventclient_map::iterator p=m_evtclients.begin(); p!=m_evtclients.end(); ++p) {
        eventclient_cont& cont = p->second;
        cont.remove(*q);
        cont.push_front(*q);
      }
    }
    m_toplevel.clear();

    App::instance()->handleEvent(*e);

    e->release();
  }
}

void EventServer::clearEvent()
{
  boost::recursive_mutex::scoped_lock lock(m_mutex);

  while(!m_events.empty()) {
    Event *e = m_events.front();
    m_events.pop_front();
    e->release();
  }
}



//  EventClient

EventClient::EventClient()
{}

EventClient::~EventClient()
{
  App::instance()->getEventServer()->deregisterEventClient(this);
}

void EventClient::listenEvent(EventType type)
{
  App::instance()->getEventServer()->registerEventClient(this, type);
}

void EventClient::queueEvent(Event *e)
{
  App::instance()->getEventServer()->queueEvent(e);
}

bool EventClient::handleEvent(const Event& e)
{
  return false;
}


//  Window

Window::Window(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  m_id(id), m_font(0), m_priority(1.0f), m_visible(true), m_floating(false), m_parent(0)
{
  setFont(App::instance()->getDefaultFont());
  setParent(parent);
  setPosition(pos);
  setSize(size);
  setText(text);

  setForegroundColor(App::instance()->getDefaultForegroundColor());
  setBackgroundColor(App::instance()->getDefaultBackgroundColor());
  setBorderColor(App::instance()->getDefaultBorderColor());

  queueEvent(new Event(EVT_CONSTRUCT, this, 0));
}

Window::~Window()
{
  releaseChildren();
  queueEvent(new DestructEvent(EVT_DESTRUCT, 0, 0, this));
  setParent(0);
}

void Window::releaseChildren()
{
  while(!m_children.empty()) {
    (*m_children.begin())->release();
  }
}

bool Window::isInclude(Window *cw)
{
    if(this==cw) { return true; }

    sgui::Window::window_cont& wc = getChildren();
    for(sgui::Window::window_cont::iterator p=wc.begin(); p!=wc.end(); ++p) {
      sgui::Window *c = *p;
      if(c==cw || c->isInclude(cw)) {
        return true;
      }
    }
    return false;
}

bool Window::isInside(const Point& point) const
{
  Rect rect(getGlobalPosition(), getSize());
  if(!isFloating() && getParent()) {
    getParent()->clip(rect, this);
  }
  return rect.isInside(point);
}

Point Window::toGlobalCoord(const Point& local) const
{
  Point global = local;
  global+=getClientPosition();
  global+=getPosition();
  if(getParent()) {
    global = getParent()->toGlobalCoord(global);
  }
  return global;
}

Point Window::toLocalCoord(const Point& global) const
{
  Point local = global;
  local-=getClientPosition();
  local-=getPosition();
  if(getParent()) {
    local = getParent()->toLocalCoord(local);
  }
  return local;
}

void Window::clip(Rect& r, const Window *wnd) const
{
  if(wnd && wnd->isFloating()) {
    return;
  }

  Rect(getGlobalPosition(), getSize()).clip(r);
  if(Window *p = getParent()) {
    p->clip(r, this);
  }
}


void Window::update()
{
  // 子->update()でm_childrenの内容が変わってiteratorが無効になって死ぬ可能性があるため、 
  // 一時コンテナに突っ込んでそっちで処理。 
  for(window_cont::iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    m_tmp.push_back(*p);
  }
  for(size_t i=0; i<m_tmp.size(); ++i) {
    m_tmp[i]->update();
  }
  m_tmp.clear();
}

void Window::draw() {}

void Window::drawText(const wstring& str, const Rect& rect, int style)
{
  if(m_font) {
    m_font->draw(str, rect, style);
  }
}

void Window::destroy()
{
  queueEvent(new Event(EVT_APP_DESTROY_WINDOW, this, this));
  setParent(0);
}

Window* Window::getParent() const
{
  return m_parent;
}

Window::window_cont& Window::getChildren()
{
  return m_children;
}

Window* Window::getChildByID(wnd_id id) const
{
  for(window_cont::const_iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    if((*p)->getID()!=0 && (*p)->getID()==id) {
      return *p;
    }
  }
  return 0;
}

void Window::setParent(Window *parent)
{
  if(m_parent) {
    m_parent->deregisterChild(this);
  }
  m_parent = parent;
  if(parent) {
    parent->registerChild(this);
  }
}

void Window::registerChild(Window *child)
{
  if(child) {
    m_children.push_back(child);
  }
}

void Window::deregisterChild(Window *child)
{
  m_children.remove(child);
}

void Window::setTopLevel(Window *window)
{
  window_cont::iterator p = std::find(m_children.begin(), m_children.end(), window);
  if(p!=m_children.end()) {
    m_children.erase(p);
    m_children.push_back(window);
  }
}

void Window::move(const Point& rel)
{
  setPosition(getPosition()+rel);
}

void Window::toTopLevel()
{
  if(Window *p = getParent()) {
    p->setTopLevel(this);
  }
  toTopLevelEvent();
}

void Window::toTopLevelEvent()
{
  App::instance()->getEventServer()->setTopLevel(this);
  for(window_cont::iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    (*p)->toTopLevelEvent();
  }
}

wnd_id Window::getID() const { return m_id; }
float Window::getDrawPriority() const { return m_priority; }
const Color& Window::getForegroundColor() const { return m_fg_color; }
const Color& Window::getBackgroundColor() const { return m_bg_color; }
const Color& Window::getBorderColor() const     { return m_bd_color; }
const wstring& Window::getText() const     { return m_text; }
const Point&   Window::getPosition() const      { return m_position; }
const Size&    Window::getSize() const          { return m_size; }
const Point&   Window::getClientPosition() const{ static Point dummy; return dummy; }
const Size&    Window::getClientSize() const    { return m_size; }
bool Window::isFloating() const                 { return m_floating; }
Point Window::getGlobalPosition() const         { return toGlobalCoord(Point()); }
bool Window::isVisible() const
{
  for(Window *p=getParent(); p; p=p->getParent()) {
    if(!p->isVisible()) {
      return false;
    }
  }
  return m_visible;
}

void Window::setDrawPriority(float v) { m_priority = v; }
void Window::setForegroundColor(const Color& color) { m_fg_color = color; }
void Window::setBackgroundColor(const Color& color) { m_bg_color = color; }
void Window::setBorderColor(const Color& color)     { m_bd_color = color; }
void Window::setText(const wstring& text)           { m_text = text; }
void Window::setPosition(const Point& pos)          { m_position = pos; }
void Window::setSize(const Size& size)              { m_size = size; }
void Window::setFloating(bool floating)             { m_floating = floating; }
void Window::setVisible(bool visible)               { m_visible = visible; }
Font* Window::getFont() const    { return m_font; }
void Window::setFont(Font *font) { m_font = font; }







//  Label

Label::Label(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Window(parent, pos, size, text, id)
{
}

void Label::draw()
{
//  DrawRectEdge(Rect(getSize()));
  AssignColor(getForegroundColor());
  drawText(getText(), Rect(getSize()));
  AssignColor(Color());
}




//  Button

Button::Button(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Window(parent, pos, size, text, id), m_button_state(UP)
{
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_MOVE);

  if(size.getWidth()==0.0f && size.getHeight()==0.0f && m_font) {
    setSize(Size(m_font->getAdvance(m_text.c_str(), m_text.size())+2.0f, m_font->getHeight()+2.0f));
  }
}

void Button::draw()
{
  if(getButtonState()==UP) {
    AssignColor(getBackgroundColor());
    DrawRect(Rect(getSize()));
  }
  else if(getButtonState()==DOWN) {
    AssignColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
    DrawRect(Rect(getSize()));
  }

  AssignColor(getForegroundColor());
  drawText(getText(), Rect(getSize()), Font::HCENTER | Font::VCENTER);
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

void Button::up()
{
  setButtonState(UP);
  queueEvent(new Event(EVT_BUTTON_UP, this, getParent()));
}

void Button::down()
{
  setButtonState(DOWN);
  queueEvent(new Event(EVT_BUTTON_DOWN, this, getParent()));
}

bool Button::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(getButtonState()==UP && e.getButton()==MOUSE_LEFT && isInside(e.getPosition())) {
      down();
      return true;
    }
  }
  else if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONUP) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(getButtonState()==DOWN && e.getButton()==MOUSE_LEFT && isInside(e.getPosition())) {
      up();
      return true;
    }
  }
  else if(evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(getButtonState()==DOWN && !isInside(e.getPosition())) {
      setButtonState(UP);
    }
  }
  return false;
}

Button::State Button::getButtonState() const { return m_button_state; }
void Button::setButtonState(State state) { m_button_state = state; }


ToggleButton::ToggleButton(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Button(parent, pos, size, text, id), m_clicked(false)
{
}

void ToggleButton::toggle()
{
  if(getButtonState()==DOWN) {
    setButtonState(UP);
    queueEvent(new Event(EVT_BUTTON_UP, this, getParent()));
  }
  else {
    setButtonState(DOWN);
    queueEvent(new Event(EVT_BUTTON_DOWN, this, getParent()));
  }
}

bool ToggleButton::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(e.getButton()==MOUSE_LEFT && isInside(e.getPosition())) {
      m_clicked = true;
      return true;
    }
  }
  else if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONUP) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(e.getButton()==MOUSE_LEFT && m_clicked && isInside(e.getPosition())) {
      m_clicked = false;
      toggle();
      return true;
    }
  }
  else if(evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(m_clicked && !isInside(e.getPosition())) {
      m_clicked = false;
    }
  }
  return false;
}





//  ToolTip

ToolTip::ToolTip(Window *parent, const wstring& text, wnd_id id) :
  Window(parent, Point(), Size(), text, id), m_appear_time(0)
{
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_MOUSE_MOVE);
  listenEvent(EVT_TIMER);

  setVisible(false);
  setFloating(true);
  if(m_font) {
    setSize(Size(
      m_font->getAdvance(text.c_str(), text.size())+6.0f,
      m_font->getHeight(text.c_str(), text.size())+6.0f) );
  }
  m_timer.setTarget(this);
}

bool ToolTip::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    setVisible(false);
  }
  else if(evt.getType()==EVT_MOUSE_BUTTONUP) {
    setVisible(false);
  }
  else if(evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);

    m_timer.stop();
    setVisible(false);
    if(getParent()->isInside(e.getPosition())) { // 親Windowにカーソルが乗ってたら、一定時間後表示 
      Point pos = getParent()->toLocalCoord(e.getPosition());
      pos.setY(pos.getY()+15.0f);
      setPosition(pos);
      m_timer.run(300);
      return true;
    }
  }
  else if(evt.getType()==EVT_TIMER) {
    if(evt.getDst()==this) {
      setVisible(true);
      m_appear_time = GetTicks();
      return true;
    }
  }
  return false;
}

void ToolTip::draw()
{
  float opa = std::min<float>(float(GetTicks()-m_appear_time)/100.0f, 1.0f);
  AssignColorO(getBackgroundColor(), opa);
  DrawRect(Rect(getSize()));
  AssignColorO(getForegroundColor(), opa);
  drawText(getText(), Rect(Point(2,2), getSize()));
  AssignColorO(getBorderColor(), opa);
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

float ToolTip::getDrawPriority() const { return 2.0f; }


//  Edit

Edit::Edit(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Window(parent, pos, size, text, id), m_readonly(false), m_blinker(0), m_focused(false)
{
  listenEvent(EVT_KEYDOWN);
  listenEvent(EVT_KEYUP);
  listenEvent(EVT_MOUSE_BUTTONDOWN);
}

void Edit::draw()
{
  AssignColor(getBackgroundColor());
  DrawRect(Rect(getSize()));
  AssignColor(getForegroundColor());
  drawText(getText(), Rect(getSize()));
  if(m_focused && m_font) {
    float x = m_font->getAdvance(m_text.c_str(), m_blinker);
    DrawLine(Point(x, 0), Point(x, getSize().getHeight()));
  }
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

void Edit::focus()
{
  m_focused = true;
}

void Edit::defocus()
{
  m_focused = false;
}

bool Edit::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(!evt.isHandled() && e.getButton()==MOUSE_LEFT && isInside(e.getPosition())) {
      focus();

      Point l = toLocalCoord(e.getPosition());
      float adv = 0;
      size_t i;
      for(i=0; i<m_text.size(); ++i) {
        if(m_font) {
          adv+=m_font->getAdvance(&m_text[i], 1);
        }
        if(l.getX()<=adv) {
          break;
        }
      }
      setBlinkerPos(i);
      return true;
    }
    else {
      defocus();
      return false;
    }
  }
  else if(evt.getType()==EVT_KEYDOWN) {
    if(!isFocused() || isReadOnly()) {
      return false;
    }

    const KeyboardEvent& e = dynamic_cast<const KeyboardEvent&>(evt);
    bool r = false;
    wstring value = getText();
    if(e.getKey()==KEY_RIGHT) {
      if(m_blinker<m_text.size()) {
        ++m_blinker;
        r = true;
      }
    }
    else if(e.getKey()==KEY_LEFT) {
      if(m_blinker>0) {
        --m_blinker;
        r = true;
      }
    }
    else if(e.getKey()==KEY_DELETE) {
      if(!m_text.empty() && m_blinker<m_text.size()) {
        value.erase(value.begin()+m_blinker);
        setText(value);
        r = true;
      }
    }
    else if(e.getKey()==KEY_BACKSPACE) {
      if(!m_text.empty() && m_blinker>0 && m_blinker<=m_text.size()) {
        value.erase(value.begin()+(m_blinker-1));
        --m_blinker;
        setText(value);
        queueEvent(new EditEvent(EVT_EDIT, this, getParent(), getText()));
        r = true;
      }
    }
    else if(e.getKey()==KEY_RETURN) {
      queueEvent(new EditEvent(EVT_EDIT_ENTER, this, getParent(), getText()));
      r = true;
    }
    else {
      if((e.getUnicode() & 0xFF80)==0) {
        char c = char(e.getUnicode() & 0x7F);
        if(isprint(c)) {
          value.insert(value.begin()+m_blinker, 1, c);
          ++m_blinker;
          setText(value);
          queueEvent(new EditEvent(EVT_EDIT, this, getParent(), getText()));
          r = true;
        }
      }
    }
    return r;
  }
  else if(evt.getType()==EVT_KEYUP) {
    if(!isFocused() || isReadOnly()) {
      return false;
    }
    return true;
  }

  return false;
}

void Edit::setBlinkerPos(int i)
{
  m_blinker = std::max<int>(0, std::min<int>(m_text.size(), i));
}

void Edit::setText(const wstring& text)
{
  Window::setText(text);
  if(m_blinker > text.size()) {
    m_blinker = text.size();
  }
}

bool Edit::isReadOnly() const { return m_readonly; }
bool Edit::isFocused() const { return m_focused; }
void Edit::setReadOnly(bool ro) { m_readonly = ro; }



//  DragAndDrop

DDItem::DDItem(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Window(parent, pos, size, text, id), m_dragging(false)
{
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_MOUSE_MOVE);
}

void DDItem::draw()
{
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

bool DDItem::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(!evt.isHandled() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(e.getButton()==MOUSE_LEFT && isInside(e.getPosition())) {
      m_drag_pos = getPosition();
      m_dragging = true;
      setFloating(true);
      queueEvent(
        new DragAndDropEvent(EVT_DD_DRAG, this, getParent(),
          e.getRealPosition(), e.getRealRelative(), e.getButton(), this));
      return true;
    }
  }
  else if(evt.getType()==EVT_MOUSE_BUTTONUP) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(isDragging()) {
      m_dragging = false;
      setFloating(false);
      queueEvent(
        new DragAndDropEvent(EVT_DD_DROP, this, getParent(),
          e.getRealPosition(), e.getRealRelative(), e.getButton(), this));
      return false;
    }
  }
  else if(evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(isDragging()) {
      move(e.getRelative());
      queueEvent(new DragAndDropEvent(EVT_DD_DRAGING, this, getParent(),
        e.getRealPosition(), e.getRealRelative(), e.getButton(), this));
    }
  }
  return false;
}

bool DDItem::isDragging() const { return m_dragging; }
const Point& DDItem::getDragBeginPosition() const { return m_drag_pos; }




DDReciever::DDReciever(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id) :
  Window(parent, pos, size, text, id), m_item(0)
{
  listenEvent(EVT_DD_DROP);
}

void DDReciever::draw()
{
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

bool DDReciever::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(evt.getType()==EVT_DD_DROP) {
    const DragAndDropEvent& e = dynamic_cast<const DragAndDropEvent&>(evt);
    if(!getItem() && isInside(e.getPosition())) {
      if(DDItem *item = dynamic_cast<DDItem*>(e.getSrc())) {
        setItem(item);
        queueEvent(new DragAndDropEvent(EVT_DD_RECIEVE, this, getParent(),
          e.getRealPosition(), e.getRealRelative(), e.getButton(), item));
        return true;
      }
    }
  }
  return false;
}

void DDReciever::deregisterChild(Window *child)
{
  Window::deregisterChild(child);
  if(child==m_item) {
    setItem(0);
  }
}

void DDReciever::setItem(DDItem *item)
{
  m_item = item;
  if(m_item) {
    Size gap = getSize()-m_item->getSize();
    m_item->setPosition(Point(gap.getWidth()/2.0f, gap.getHeight()/2.0f));
    m_item->setParent(this);
  }
}
DDItem* DDReciever::getItem() const { return m_item; }




//  ScrollBar

ScrollBar::ScrollBar(Window *parent, int style, float size, wnd_id id) :
  Window(parent, Point(), Size(), L"", id),
  m_align(HORIZONTAL), m_dragging(false), m_thick(0.0f), m_step(0.0f), m_scroll_pos(0.0f), m_range(0.0f), m_bar_width(10.0f),
  m_button_tl(0), m_button_br(0)
{
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_MOUSE_MOVE);
  listenEvent(EVT_BUTTON_UP);

  setThick(12.0f);
  setStep(10.0f);
  setRange(100.0f);
  setValue(0.0f);
  resetBarRect();

  if(style & SB_VERTICAL) {
    m_align = VERTICAL;
  }

  if(m_align==HORIZONTAL) {
    m_button_tl = new Button(this, Point(), Size(getThick(), getThick()));
    m_button_br = new Button(this, Point(getSize().getWidth()-getThick(), 0.0f), Size(getThick(), getThick()));
  }
  else if(m_align==VERTICAL) {
    m_button_tl = new Button(this, Point(), Size(getThick(), getThick()));
    m_button_br = new Button(this, Point(0.0f, getSize().getHeight()-getThick()), Size(getThick(), getThick()));
  }

  setScrollBarSize(size);
}

void ScrollBar::draw()
{
  AssignColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
  DrawRect(m_bar);
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

bool ScrollBar::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(evt.getType()==EVT_BUTTON_UP) {
    if(m_button_tl && evt.getSrc()==m_button_tl) {
      setValue(getValue()-getStep());
      return true;
    }
    else if(m_button_br && evt.getSrc()==m_button_br) {
      setValue(getValue()+getStep());
      return true;
    }
  }
  else if(!evt.isHandled() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(m_bar.isInside(toLocalCoord(e.getPosition()))) {
      m_dragging = true;
      return true;
    }
  }
  else if(evt.getType()==EVT_MOUSE_BUTTONUP) {
    m_dragging = false;
  }
  else if(evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(m_dragging) {
      mouseToScroll(e.getRelative());
    }
  }
  return Window::handleEvent(evt);
}

void ScrollBar::fitTo(Window *target)
{
  if(m_align==HORIZONTAL) {
    setPosition(Point(0.0f, target->getSize().getHeight()-getThick()));
    setSize(Size(target->getSize().getWidth()-getThick(), getThick()));
    m_button_tl->setPosition(Point());
    m_button_br->setPosition(Point(getSize().getWidth()-getThick(), 0.0f));
  }
  else if(m_align==VERTICAL) {
    setPosition(Point(target->getSize().getWidth()-getThick(), 0.0f));
    setSize(Size(getThick(), target->getSize().getHeight()-getThick()));
    m_button_tl->setPosition(Point());
    m_button_br->setPosition(Point(0.0f, getSize().getHeight()-getThick()));
  }
}

float ScrollBar::getBarWidth() const
{
  return m_bar_width;
}

Range ScrollBar::getScrollBarRange() const
{
  Range r;
  if(m_align==HORIZONTAL) {
    r.setMin(getThick());
    r.setMax(getSize().getWidth()-getThick()-m_bar_width);
  }
  else if(m_align==VERTICAL) {
    r.setMin(getThick());
    r.setMax(getSize().getHeight()-getThick()-m_bar_width);
  }
  return r;
}

void ScrollBar::resetBarRect()
{
  Range r = getScrollBarRange();
  if(m_align==HORIZONTAL) {
    float width = getSize().getWidth()-getThick()*2.0f;
    float par = std::min<float>(1.0f, getParent()->getSize().getWidth()/getRange());
    m_bar_width = std::max<float>(10.0f, width*par);
    m_bar = Rect(Point(r.getMin()+r.getLength()*(getValue()/getRange()), 0.0f), Size(m_bar_width, getThick()));
  }
  else if(m_align==VERTICAL) {
    float width = getSize().getHeight()-getThick()*2.0f;
    float par = std::min<float>(1.0f, getParent()->getSize().getHeight()/getRange());
    m_bar_width = std::max<float>(10.0f, width*par);
    m_bar = Rect(Point(0.0f, r.getMin()+r.getLength()*(getValue()/getRange())), Size(getThick(), m_bar_width));
  }
}

void ScrollBar::mouseToScroll(const Point& mouse_move)
{
  Range r = getScrollBarRange();
  if(m_align==HORIZONTAL) {
    setValue(getValue()+mouse_move.getX()/r.getLength()*getRange());
  }
  else if(m_align==VERTICAL) {
    setValue(getValue()+mouse_move.getY()/r.getLength()*getRange());
  }
}

float ScrollBar::getValue() const { return m_scroll_pos; }
float ScrollBar::getThick() const { return m_thick; }
float ScrollBar::getStep() const  { return m_step; }
float ScrollBar::getRange() const { return m_range; }

void ScrollBar::setScrollBarSize(float size)
{
  if(m_align==HORIZONTAL) {
    setSize(Size(size, getThick()));
    m_button_tl->setPosition(Point());
    m_button_br->setPosition(Point(getSize().getWidth()-getThick(), 0.0f));
  }
  else if(m_align==VERTICAL) {
    setSize(Size(getThick(), size));
    m_button_tl->setPosition(Point());
    m_button_br->setPosition(Point(0.0f, getSize().getHeight()-getThick()));
  }
  resetBarRect();
}

void ScrollBar::setValue(float value)
{
  float old_value = m_scroll_pos;
  m_scroll_pos = std::max<float>(0.0f, std::min<float>(value, getRange()));
  resetBarRect();

  float scale = 1.0f;
  if(m_align==HORIZONTAL) {
    scale = std::max<float>(0.0f, (getRange()-getParent()->getSize().getWidth())/getRange());
  }
  else if(m_align==VERTICAL) {
    scale = std::max<float>(0.0f, (getRange()-getParent()->getSize().getHeight())/getRange());
  }
  queueEvent(new ScrollEvent(EVT_SCROLL, this, getParent(), m_scroll_pos*scale, (m_scroll_pos-old_value)*scale));
    
}

void ScrollBar::setRange(float range)
{
  float old = getValue();
  setValue(0.0f);
  m_range = range;
  setValue(old);
}
void ScrollBar::setThick(float thick) { m_thick = thick; }
void ScrollBar::setStep(float value) { m_step = value; }



//  ScrolledWindow

ScrolledWindow::ScrolledWindow(Window *parent, const Point& pos, const Size& size,
  const wstring& text, wnd_id id, int style) :
  Window(parent, pos, size, text, id), m_hscrollbar(0), m_vscrollbar(0)
{
  if(style & VSCROLLBAR) {
    m_vscrollbar = new ScrollBar(this, SB_VERTICAL);
    m_vscrollbar->setPosition(Point(getSize().getWidth()-m_vscrollbar->getThick(), 0.0f));
    if(style & HSCROLLBAR) {
      m_vscrollbar->setScrollBarSize(getSize().getHeight()-m_vscrollbar->getThick());
    }
    else {
      m_vscrollbar->setScrollBarSize(getSize().getHeight());
    }
  }
  if(style & HSCROLLBAR) {
    m_hscrollbar = new ScrollBar(this, SB_HORIZONTAL);
    m_hscrollbar->setPosition(Point(0.0f, getSize().getHeight()-m_hscrollbar->getThick()));
    if(style & VSCROLLBAR) {
      m_hscrollbar->setScrollBarSize(getSize().getWidth()-m_hscrollbar->getThick());
    }
    else {
      m_hscrollbar->setScrollBarSize(getSize().getWidth());
    }
  }
  setSize(getSize());

  listenEvent(EVT_SCROLL);
}

bool ScrolledWindow::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(evt.getType()==EVT_SCROLL) {
    if((m_vscrollbar && evt.getSrc()==m_vscrollbar) || (m_hscrollbar && evt.getSrc()==m_hscrollbar)) {
      const ScrollEvent& se = dynamic_cast<const ScrollEvent&>(evt);
      if(m_vscrollbar && se.getSrc()==m_vscrollbar) {
        setScrollPosition(Point(0.0f, -se.getValue()));
        return true;
      }
      else if(m_hscrollbar && se.getSrc()==m_hscrollbar) {
        setScrollPosition(Point(-se.getValue(), 0.0f));
        return true;
      }
    }
  }
  return false;
}

void ScrolledWindow::draw()
{
  AssignColor(getBackgroundColor());
  DrawRect(Rect(Point(), Window::getSize()));
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(Point(), Window::getSize()));
  AssignColor(Color());
}

void ScrolledWindow::setScrollPosition(const Point& _pos)
{
  const Size lim = getScrollSize()-getSize();
  Point pos = Point(
    std::min<float>(0.0f, std::max<float>(_pos.getX(), -lim.getWidth())),
    std::min<float>(0.0f, std::max<float>(_pos.getY(), -lim.getHeight())));
  Point rel = pos-m_scroll_pos;
  m_scroll_pos = pos;
  for(window_cont::iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    if(*p!=m_vscrollbar && *p!=m_hscrollbar) {
      (*p)->move(rel);
    }
  }
}

const Point& ScrolledWindow::getScrollPosition() const { return m_scroll_pos; }
const Size& ScrolledWindow::getSizeNS() const { return m_size_ns; }
const Size& ScrolledWindow::getScrollSize() const { return m_scroll_size; }
ScrollBar* ScrolledWindow::getVScrollBar() const { return m_vscrollbar; }
ScrollBar* ScrolledWindow::getHScrollBar() const { return m_hscrollbar; }

void ScrolledWindow::setSize(const Size& size)
{
  Window::setSize(size);
  m_size_ns = getSize();
  if(m_hscrollbar) {
    m_size_ns.setHeight(m_size_ns.getHeight()-12.0f);
  }
  if(m_vscrollbar) {
    m_size_ns.setWidth(m_size_ns.getWidth()-12.0f);
  }
}

void ScrolledWindow::setScrollSize(const Size& size)
{
  m_scroll_size = size;
  if(m_hscrollbar) {
    m_hscrollbar->setRange(getScrollSize().getWidth());
  }
  if(m_vscrollbar) {
    m_vscrollbar->setRange(getScrollSize().getHeight());
  }
}

void ScrolledWindow::clip(Rect& r, const Window *wnd) const
{
  if(wnd && wnd->isFloating()) {
    return;
  }

  if(wnd && (wnd==this || wnd==m_vscrollbar || wnd==m_hscrollbar)) {
    Window::clip(r, wnd);
  }
  else {
    Rect(getGlobalPosition(), getSizeNS()).clip(r);
    if(Window *p = getParent())
      p->clip(r, this);
  }
}




//  List

ListItem::ListItem(const wstring& text) :
  Window(0, Point(), Size(), text, 0), m_selected(false)
{
}

void ListItem::draw()
{
  if(isSelected()) {
    AssignColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
    DrawRect(Rect(getSize()));
  }
  AssignColor(getForegroundColor());
  drawText(getText(), Rect(getSize()));
  AssignColor(Color());
}

bool ListItem::isSelected() const { return m_selected; }
void ListItem::setSelect(bool selected) { m_selected = selected; }




List::List(Window *parent, const Point& pos, const Size& size, wnd_id id, int style) :
  ScrolledWindow(parent, pos, size, L"", id, VSCROLLBAR),
  m_multiselect(false), m_item_height(12.0f), m_last_selected(0)
{
  Font *font = App::instance()->getDefaultFont();
  m_item_height = font ? font->getHeight() : 12.0f;

  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_TIMER);
}

void List::draw()
{
  AssignColor(getBackgroundColor());
  DrawRect(Rect(getSize()));
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}


bool List::handleEvent(const Event& evt)
{
  if(!isVisible()) {
    return false;
  }

  if(!evt.isHandled() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(e.getButton()==MOUSE_LEFT && getItemCount()!=0 && isInside(e.getPosition())) {
      size_t index = hitItem(e.getPosition());
      if(ListItem *item = getItem(index)) {
        if(m_stopwatch.isRunning() && item==m_last_selected) {
          queueEvent(new ListEvent(EVT_LIST_DOUBLECLICK, this, getParent(), index, item));
          m_stopwatch.stop();
          m_last_selected = 0;
        }
        else {
          setSelection(index);
          m_last_selected = item;
          m_stopwatch.run(300); // ダブルクリック検出開始 
        }
      }
      return true;
    }
  }
  return ScrolledWindow::handleEvent(evt);
}

void List::setSelection(size_t i)
{
  if(i>=m_items.size()) {
    return;
  }

  if(!m_multiselect) {
    clearSelection();
  }

  ListItem *item = m_items[i];
  if(!item->isSelected()) {
    item->setSelect(true);
    queueEvent(new ListEvent(EVT_LIST_SELECT, this, getParent(), i, item));
  }
  else if(m_multiselect) {
    item->setSelect(false);
    queueEvent(new ListEvent(EVT_LIST_DISSELECT, this, getParent(), i, item));
  }
}

void List::setSelection(ListItem *l)
{
  setSelection(getItemIndex(l));
}

size_t List::hitItem(const Point& p) const
{
  size_t i;
  for(i=0; i<m_items.size(); ++i) {
    if(m_items[i]->isInside(p)) {
      break;
    }
  }
  return i;
}

ListItem* List::getItem(size_t i)
{
  if(i>=m_items.size()) {
    return 0;
  }
  return m_items[i];
}

ListItem* List::getSelectedItem(size_t i)
{
  if(i < getSelectionCount()) {
    return getItem(getSelection(i));
  }
  return 0;
}

size_t List::getItemCount() const
{
  return m_items.size();
}

size_t List::getItemIndex(ListItem *l) const
{
  size_t i = 0;
  for(i=0; i<m_items.size(); ++i) {
    if(l==m_items[i]) {
      break;
    }
  }
  return i;
}

size_t List::getSelectionCount() const
{
  size_t count = 0;
  for(size_t i=0; i<m_items.size(); ++i) {
    if(m_items[i]->isSelected()) {
      ++count;
    }
  }
  return count;
}

size_t List::getSelection(size_t index) const
{
  size_t count = 0;
  size_t i;
  for(i=0; i<m_items.size(); ++i) {
    if(m_items[i]->isSelected()) {
      if(count==index) {
        return i;
      }
      ++count;
    }
  }
  return i;
}

void List::addItem(ListItem *item)
{
  if(item) {
    item->setParent(this);
    m_items.push_back(item);
    alignItems();
    resetScrollSize();
  }
}

void List::removeItem(size_t i)
{
  if(i>=m_items.size()) {
    return;
  }
  m_items[i]->destroy();
  m_items.erase(m_items.begin()+i);
  alignItems();
  resetScrollSize();
}

void List::removeItem(ListItem *l)
{
  removeItem(getItemIndex(l));
}

void List::clearItem()
{
  for(item_cont::iterator p=m_items.begin(); p!=m_items.end(); ++p) {
    (*p)->destroy();
  }
  m_items.clear();
  resetScrollSize();
}

void List::clearSelection()
{
  for(size_t i=0; i<m_items.size(); ++i) {
    if(m_items[i]->isSelected()) {
      m_items[i]->setSelect(false);
      queueEvent(new ListEvent(EVT_LIST_DISSELECT, this, getParent(), i, m_items[i]));
    }
  }
}

void List::alignItems()
{
  for(size_t i=0; i<m_items.size(); ++i) {
    m_items[i]->setPosition(getScrollPosition()+Point(0.0f, getItemHeight()*i));
    m_items[i]->setSize(Size(getSize().getWidth(), getItemHeight()));
  }
}

void List::resetScrollSize()
{
  setScrollSize(Size(getSize().getWidth(), getItemHeight()*getItemCount()+2.0f));
}

float List::getItemHeight() const { return m_item_height; }




//  Combo

Combo::Combo(Window *parent, const Point& pos, const Size& size, wnd_id id, int style) :
  Window(parent, pos, size, L"", id), m_list_button(0), m_list(0)
{
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_BUTTON_DOWN);
  listenEvent(EVT_LIST_SELECT);

  m_list_button = new Button(this, Point(getSize().getWidth()-16.0f, 0.0f), Size(16.0f, 16.0f));
  m_list = new List(this, Point(0.0f, getSize().getHeight()), Size(getSize().getWidth(), 150.0f));
  m_list->setFloating(true);
  m_list->setVisible(false);
}

bool Combo::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_BUTTON_DOWN) {
    if(m_list_button && evt.getSrc()==m_list_button) {
      if(m_list->isVisible()) {
        m_list->setVisible(false);
      }
      else {
        m_list->setVisible(true);
      }
      return true;
    }
  }
  else if(evt.getType()==EVT_LIST_SELECT) {
    if(m_list && evt.getSrc()==m_list) {
      setSelection(m_list->getSelection());
      m_list->setVisible(false);
      return true;
    }
  }

  if(!isVisible()) {
    return false;
  }

  if(evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(!m_list_button->isInside(e.getPosition())) {
      if(m_list->isVisible() && !m_list->isInside(e.getPosition())) {
        m_list->setVisible(false);
        return true;
      }
      else if(!evt.isHandled() && !m_list->isVisible() && isInside(e.getPosition())) {
        m_list->setVisible(true);
        return true;
      }
    }
  }
  else if(evt.getType()==EVT_MOUSE_BUTTONUP) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(!m_list_button->isInside(e.getPosition())) {
      if(m_list->isVisible() && !isInside(e.getPosition()) && !m_list->isInside(e.getPosition())) {
        m_list->setVisible(false);
        return true;
      }
    }
  }
  return Window::handleEvent(evt);
}

void Combo::draw()
{
  AssignColor(getForegroundColor());
  drawText(getText(), Rect(getSize()));
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  AssignColor(Color());
}

void Combo::addItem(ListItem *item)
{
  m_list->addItem(item);
}

void Combo::clearItem()
{
  m_list->clearItem();
  m_selection = 0;
}

List* Combo::getList() { return m_list; }
ListItem* Combo::getSelection() { return m_selection; }

void Combo::setSelection(size_t i)
{
  m_selection = m_list->getItem(i);
  setText(m_selection->getText());
  queueEvent(new ListEvent(EVT_COMBO, this, getParent(), i, m_selection));
}

void Combo::setVisible(bool visible)
{
  m_visible = visible;
  for(window_cont::iterator p=m_children.begin(); p!=m_children.end(); ++p) {
    if(!dynamic_cast<ToolTip*>(*p) && *p!=m_list) {
      (*p)->setVisible(visible);
    }
  }
}





//  Panel

Panel::Panel(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id, int style) :
  Window(parent, pos, size, text, id)
{
}

void Panel::draw()
{
  AssignColor(getBackgroundColor());
  DrawRect(Rect(getSize()));
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));

  AssignColor(Color());
}

bool Panel::handleEvent(const Event& evt)
{
  return false;
}

void Panel::setSize(const Size& size)
{
  Size old = getSize();
  Window::setSize(size);
  queueEvent(new ResizeEvent(EVT_RESIZE, this, getParent(), getSize(), getSize()-old));
}




//  Dialog

Dialog::Dialog(Window *parent, const Point& pos, const Size& size, const wstring& text, wnd_id id, int style) :
  Panel(parent, pos, size, text, id, style),
  m_close_button(0), m_maximize_button(0), m_minimize_button(0),
  m_movable(false), m_moving(false),
  m_resizable(false), m_resizing(0)
{
  listenEvent(EVT_MOUSE_BUTTONDOWN);
  listenEvent(EVT_MOUSE_BUTTONUP);
  listenEvent(EVT_MOUSE_MOVE);
  listenEvent(EVT_BUTTON_UP);

  if(getParent()==0) {
    setParent(View::instance());
  }
  setWidthRange(Range(120.0f, getParent()->getSize().getWidth()));
  setHeightRange(Range(25.0f, getParent()->getSize().getHeight()));
//  m_client_pos = Point(0.0f, 24.0f);

  if(style & MOVABLE) {
    m_movable = true;
  }
  if(style & RESIZABLE) {
    m_resizable = true;
  }

  int buttons = 1;
  if(style & CLOSE_BUTTON) {
    m_close_button = new Button(this, Point(size.getWidth()-20*buttons, 4), Size(16, 16));
    ++buttons;
  }
  if(style & MAXIMIZE_BUTTON) {
    m_maximize_button = new Button(this, Point(size.getWidth()-20*buttons, 4), Size(16, 16));
    ++buttons;
  }
  if(style & MINIMIZE_BUTTON) {
    m_minimize_button = new Button(this, Point(size.getWidth()-20*buttons, 4), Size(16, 16));
    ++buttons;
  }
}

void Dialog::destroy()
{
  queueEvent(new Event(EVT_CLOSEDIALOG, this, getParent()));
  Panel::destroy();
}

void Dialog::draw()
{
  AssignColor(getBackgroundColor());
  DrawRect(Rect(getSize()));
  AssignColor(getBorderColor());
  DrawRectEdge(Rect(getSize()));
  DrawLine(Point(0, 24), Point(getSize().getWidth(), 24));
  drawText(getText(), Rect(Point(4,0), Size(getSize().getWidth(), 24)),
    Font::HLEFT | Font::VCENTER);

  AssignColor(Color());
}

void Dialog::onCloseButton()
{
  destroy();
}

void Dialog::onMaximizeButton()
{
  setSize(Size(m_width_range.getMax(), m_height_range.getMax()));
  setPosition(Point(0.0f, 0.0f));
}

void Dialog::onMinimizeButton()
{
  setSize(Size(m_width_range.getMin(), m_height_range.getMin()));
}

bool Dialog::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_BUTTON_UP) {
    if(m_close_button && evt.getSrc()==m_close_button) {
      onCloseButton();
      return true;
    }
    else if(m_maximize_button && evt.getSrc()==m_maximize_button) {
      onMaximizeButton();
      return true;
    }
    else if(m_minimize_button && evt.getSrc()==m_minimize_button) {
      onMinimizeButton();
      return true;
    }
  }

  if(!isVisible()) {
    return Panel::handleEvent(evt);
  }

  if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONDOWN) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(isInside(e.getPosition())) {
      toTopLevel();

      if(e.getButton()==MOUSE_LEFT) {
        const Size& s = getSize();
        Point gp = getGlobalPosition();

        // タイトルバーがクリックされたか 
        if(Rect(gp, Size(getSize().getWidth(), 24)).isInside(e.getPosition())) {
          if(m_movable && !evt.isHandled()) {
            m_moving = true;
          }
        }
        else {
          // 端がクリックされたか 
          if(Rect(gp, Size(5, s.getHeight())).isInside(e.getPosition())) {
            m_resizing|=1;
          }
          else if(Rect(gp+Point(s.getWidth()-5.0f, 0.0f), Size(5.0f, s.getHeight())).isInside(e.getPosition())) {
            m_resizing|=2;
          }

          if(Rect(gp+Point(0.0f, s.getHeight()-5.0f), Size(s.getWidth(), 5.0f)).isInside(e.getPosition())) {
            m_resizing|=4;
          }
        }
      }
      return true;
    }
  }
  else if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_BUTTONUP) {
    m_moving = false;
    m_resizing = 0;

    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(isInside(e.getPosition())) {
      return true;
    }
  }
  else if(!evt.isHandledByFrame() && evt.getType()==EVT_MOUSE_MOVE) {
    const MouseEvent& e = dynamic_cast<const MouseEvent&>(evt);
    if(m_moving) { // タイトルバードラッグで位置変更 
      setPosition(getPosition()+e.getRelative());
    }
    if(m_resizing!=0) { // 端ドラッグでウィンドウサイズ変更 
      Size size = getSize();
      Point pos = getPosition();
      if(m_resizing&1) { // 左 
        size.setWidth(size.getWidth()-e.getRelative().getX());
        pos.setY(pos.getY()+e.getRelative().getX());
      }
      else if(m_resizing&2) { // 右 
        size.setWidth(size.getWidth()+e.getRelative().getX());
      }
      if(m_resizing&4) { // 下 
        size.setHeight(size.getHeight()+e.getRelative().getY());
      }
      size.setWidth(m_width_range.clamp(size.getWidth()));
      size.setHeight(m_height_range.clamp(size.getHeight()));
      setSize(size);
      setPosition(pos);

      if(isInside(e.getPosition())) {
        return true;
      }
    }
  }
  return Panel::handleEvent(evt);
}

const Point& Dialog::getClientPosition() const { return m_client_pos; }
const Size& Dialog::getClientSize() const { return m_client_size; }

void Dialog::setSize(const Size& size)
{
  Size old = getSize();
  Panel::setSize(size);

  Size gap = getSize()-old;
  Point rel(gap.getWidth(), 0.0f);
  if(m_close_button) { m_close_button->move(rel); }
  if(m_minimize_button) { m_minimize_button->move(rel); }
  if(m_maximize_button) { m_maximize_button->move(rel); }
}

void Dialog::setWidthRange(const Range& r) { m_width_range = r; }
void Dialog::setHeightRange(const Range& r) { m_height_range = r; }




// MessageDialog

MessageDialog::MessageDialog(const wstring& title, const wstring& message, Window *parent, const Point& pos, const Size& size):
  Dialog(parent, pos, size, title, 0, MOVABLE | CLOSE_BUTTON),
  m_ok_button(0), m_message(0)
{
  m_ok_button = new Button(this, Point(size.getWidth()-55, size.getHeight()-25), Size(50, 20), L"OK");
  m_message = new Label(this, Point(5,30), Size(size-Size(10, 35)), message);
}

bool MessageDialog::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_BUTTON_UP) {
    if(evt.getDst()==this && evt.getSrc()==m_ok_button) {
      destroy();
      return true;
    }
  }
  return Dialog::handleEvent(evt);
}


// ConfirmDialog

ConfirmDialog::ConfirmDialog(const wstring& title, const wstring& message, wnd_id id, Window *parent, const Point& pos, const Size& size):
  Dialog(parent, pos, size, title, id, MOVABLE | CLOSE_BUTTON),
  m_ok_button(0), m_cancel_button(0), m_message(0)
{
  m_ok_button = new Button(this, Point(60, 135), Size(50, 20), L"OK");
  m_cancel_button = new Button(this, Point(140, 135), Size(50, 20), L"Cancel");
  m_message = new Label(this, Point(5, 30), Size(230, 100), message);
}

bool ConfirmDialog::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_BUTTON_UP) {
    if(evt.getSrc()==m_ok_button || evt.getSrc()==m_cancel_button) {
      if(evt.getSrc()==m_ok_button) {
        queueEvent(new DialogEvent(EVT_CONFIRMDIALOG, this, getParent(), true));
      }
      else {
        queueEvent(new DialogEvent(EVT_CONFIRMDIALOG, this, getParent(), false));
      }
      destroy();
      return true;
    }
  }
  return Dialog::handleEvent(evt);
}






// FileDialog

#ifdef SGUI_ENABLE_FILEDIALOG




class CWConverter
{
public:
  static CWConverter& getDefault()
  {
    static CWConverter c;
    return c;
  }

  CWConverter(const char *l = "") {
    setLocale(l);
  }

  void setLocale(const char *l)
  {
    loc = l;
    ::setlocale(LC_ALL, l);
  }

  void MBS2WCS(wstring& dst, const string& src)
  {
    size_t len = mbstowcs(0, src.c_str(), 0)+1;
    if(len==(size_t)(-1)) {
      return;
    }
    wchar_t *buf(new wchar_t[len]);
    ::mbstowcs(buf, src.c_str(), len);
    dst = buf;
    delete[] buf;
  }

  void WCS2MBS(string& dst, const wstring& src)
  {
    size_t len = wcstombs(0, src.c_str(), 0)+1;
    if(len==(size_t)(-1)) {
      return;
    }
    char *buf(new char[len]);
    ::wcstombs(buf, src.c_str(), len);
    dst = buf;
    delete[] buf;
  }

  string loc;
};

wstring _L(const string& src)
{
  wstring dst;
  CWConverter::getDefault().MBS2WCS(dst, src);
  return dst;
}

string _S(const wstring& src)
{
  string dst;
  CWConverter::getDefault().WCS2MBS(dst, src);
  return dst;
}



bool IsFile(const string& path)
{
#ifdef _WIN32
  DWORD ret = ::GetFileAttributes(path.c_str());
  return (ret!=(DWORD)-1) && !(ret & FILE_ATTRIBUTE_DIRECTORY);
#else
  struct stat st;
  return ::stat(path.c_str(), &st)==0 && S_ISREG(st.st_mode);
#endif
}

bool IsDir(const string& path)
{
#ifdef _WIN32
  DWORD ret = ::GetFileAttributes(path.c_str());
  return (ret!=(DWORD)-1) && (::GetFileAttributes(path.c_str())&FILE_ATTRIBUTE_DIRECTORY)!=0;
#else
  struct stat st;
  return ::stat(path.c_str(), &st)==0 && S_ISDIR(st.st_mode);
#endif
}

bool MakeDir(const string& path)
{
#ifdef _WIN32
  return ::CreateDirectory(path.c_str(), NULL)==TRUE;
#else
  return ::mkdir(path.c_str(), 0777)==0;
#endif
}

bool MakeDeepDir(const string& path)
{
  size_t num = 0;
  for(size_t i=0; i<path.size(); ++i) {
    if(path[i]=='/') {
      string p(path.begin(), path.begin()+i);
      if(MakeDir(p.c_str())) {
        ++num;
      }
    }
  }
  return num!=0;
}

bool RemoveDir(const string& path)
{
#ifdef _WIN32
  return ::RemoveDirectory(path.c_str())==TRUE;
#else
  return ::rmdir(path.c_str())==0;
#endif
}

bool Remove(const string& path)
{
  if(IsDir(path)) {
    return RemoveDir(path.c_str());
  }
  else {
    return std::remove(path.c_str())==0;
  }
}

bool RemoveRecursive(const string& path)
{
  bool res = false;
  if(IsDir(path)) {
    Dir dir(path);
    for(Dir::iterator p=dir.begin(); p!=dir.end(); ++p) {
      if(*p=="." || *p=="..") {
        continue;
      }
      string next = dir.getPath()+*p;
      res = RemoveRecursive(next.c_str());
    }
    res = RemoveDir(path);
  }
  else {
    res = std::remove(path.c_str())==0;
  }
  return res;
}

string GetCWD()
{
  char buf[256];
#ifdef _WIN32
  ::GetCurrentDirectory(256, buf);
  for(size_t i=0; i<strlen(buf); ++i) {
    if(buf[i]=='\\') {
      buf[i] = '/';
    }
  }
  return buf;
#else
  ::getcwd(buf, 256);
  return buf;
#endif
}

bool SetCWD(const string& path)
{
#ifdef _WIN32
  return ::SetCurrentDirectory(path.c_str())!=0;
#else
  return ::chdir(path.c_str())==0;
#endif
}


Dir::Dir() {}
Dir::Dir(const string& path) { open(path); }
size_t Dir::size() const { return m_files.size(); }
const string& Dir::operator[](size_t i) const { return m_files[i]; }
const string& Dir::getPath() const { return m_path; }

bool Dir::open(const string& path)
{
  m_files.clear();
  m_path = path;
  if(m_path[m_path.size()-1]!='/') {
    m_path+='/';
  }

#ifdef _WIN32
  WIN32_FIND_DATA wfdata;
  HANDLE handle = ::FindFirstFile((m_path+"*").c_str(), &wfdata);
  if(handle!=INVALID_HANDLE_VALUE) {
    do {
      m_files.push_back(wfdata.cFileName);
    } while(::FindNextFile(handle, &wfdata));
    ::FindClose(handle);
  }
  else {
    return false;
  }
  return true;
#else
  DIR *dir = ::opendir(m_path.c_str());
  if(dir!=0) {
    dirent *dr;
    while((dr=::readdir(dir))!=0) {
      m_files.push_back(dr->d_name);
    }
    ::closedir(dir);
  }
  else {
    return false;
  }
  return true;
#endif
}

bool Dir::openRecursive(const string& path)
{
  Dir dir(path);
  for(iterator p=dir.begin(); p!=dir.end(); ++p) {
    if(*p=="." || *p=="..") {
      continue;
    }
    string next = dir.getPath()+*p;
    if(IsDir(next.c_str())) {
      openRecursive(next.c_str());
    }
    else {
      m_files.push_back(next);
    }
  }
  return size()!=0;
}



FileDialog::FileDialog(const wstring& title, Window *parent, const Point& pos, const Size& size):
  Dialog(parent, pos, size, title, 0, MOVABLE | CLOSE_BUTTON),
  m_ok_button(0), m_cancel_button(0), m_filelist(0)
{
  setSize(Size(320, 250));
  m_ok_button = new Button(this, Point(260, 30), Size(50, 16), L"OK");
  m_cancel_button = new Button(this, Point(260, 50), Size(50, 16), L"Cancel");
  m_filelist = new List(this, Point(5, 50), Size(250, 180));
  m_locate = new Combo(this, Point(5, 30), Size(250, 16));
  openDir(GetCWD());
}

bool FileDialog::handleEvent(const Event& evt)
{
  if(evt.getType()==EVT_BUTTON_UP) {
    if(evt.getSrc()==m_ok_button) {
      queueEvent(new DialogEvent(EVT_FILEDIALOG, this, getParent(), true));
      if(IsDir(m_path+m_file)) {
        openDir(m_path+m_file);
      }
      else if(IsFile(m_path+m_file)) {
        queueEvent(new DialogEvent(EVT_FILEDIALOG, this, getParent(), true, m_path+m_file));
        destroy();
      }
      return true;
    }
    else if(evt.getSrc()==m_cancel_button) {
      queueEvent(new DialogEvent(EVT_FILEDIALOG, this, getParent(), false));
      destroy();
      return true;
    }
  }
  else if(evt.getType()==EVT_LIST_DOUBLECLICK) {
    if(evt.getDst()==this) {
      const ListEvent& e = dynamic_cast<const ListEvent&>(evt);
      if(IsDir(m_path+m_file)) {
        openDir(m_path+m_file);
      }
      else if(IsFile(m_path+m_file)) {
        queueEvent(new DialogEvent(EVT_FILEDIALOG, this, getParent(), true, m_path+m_file));
        destroy();
      }
      return true;
    }
  }
  else if(evt.getType()==EVT_LIST_SELECT) {
    if(evt.getDst()==this) {
      const ListEvent& e = dynamic_cast<const ListEvent&>(evt);
      m_file = _S(e.getItem()->getText());
      return true;
    }
  }
  else if(evt.getType()==EVT_COMBO) {
    if(evt.getDst()==this) {
      const ListEvent& e = dynamic_cast<const ListEvent&>(evt);
      openDir(_S(e.getItem()->getText()));
      return true;
    }
  }
  return Dialog::handleEvent(evt);
}

bool FileDialog::openDir(const string& _path)
{
  string path = _path;

  if(path.empty()) {
    return false;
  }
  if(path[path.size()-1]!='/') {
    path+='/';
  }
  if(!IsDir(path) || m_path==path) {
    return false;
  }

  if(path.size()>4 && strncmp(path.c_str()+path.size()-3, "/./", 3)==0) {
    return true;
  }
  else if(path.size()>5 && strncmp(path.c_str()+path.size()-4, "/../", 4)==0) {
    string p = path;
    size_t s = p.size()-5;
    for(; s!=0; --s) {
      if(p[s]=='/') {
        break;
      }
    }
    p.erase(p.begin()+s, p.end());
    m_path = p;
  }
  else {
    m_path = path;
  }

  if(m_path[m_path.size()-1]!='/') {
    m_path+='/';
  }

  m_locate->clearItem();
  for(int i=int(m_path.size()-1); i>=0; --i) {
    if(m_path[i]=='/' || m_path[i]=='\\') {
      m_locate->addItem(new ListItem(_L(string(m_path.begin(), m_path.begin()+i))));
    }
  }
  m_locate->setSelection(0);

  m_file.clear();
  m_filelist->clearItem();
  Dir dir(path);
  for(size_t i=0; i<dir.size(); ++i) {
    if(IsDir(m_path+dir[i])) {
      m_filelist->addItem(new ListItem(_L(dir[i]+'/')));
    }
  }
  for(size_t i=0; i<dir.size(); ++i) {
    if(!IsDir(m_path+dir[i])) {
      m_filelist->addItem(new ListItem(_L(dir[i])));
    }
  }
  return true;
}

#endif


} // sgui

#include <typeinfo>
#include "sgui.h"

// string⇔wstring変換関数 
using sgui::_L;
using sgui::_S;


enum {
  BU_EXIT = 1,
  BU_MYDIALOG,
  BU_FILEDIALOG,
};


class MyDialog : public sgui::Dialog
{
typedef sgui::Dialog Super;
private:
  sgui::Label *m_text;
  sgui::List *m_list;
  sgui::Edit *m_edit;
  sgui::Combo *m_combo;

public:
  MyDialog(sgui::Window *parent) :
    Super(parent, sgui::Point(50, 50), sgui::Size(350, 160), L"MyDialog")
  {
    listenEvent(sgui::EVT_EDIT_ENTER);
    listenEvent(sgui::EVT_LIST_DOUBLECLICK);
    listenEvent(sgui::EVT_COMBO);
    listenEvent(sgui::EVT_DD_RECIEVE);

    m_text = new sgui::Label(this, sgui::Point(180, 30), sgui::Size(150, 16), L"");
    m_edit = new sgui::Edit(this, sgui::Point(10, 30), sgui::Size(150, 16), L"Enterでアイテム追加");
    m_list = new sgui::List(this, sgui::Point(10, 50), sgui::Size(120, 100));

    m_combo = new sgui::Combo(this, sgui::Point(180, 50), sgui::Size(120, 16));
    m_combo->addItem(new sgui::ListItem(L"hoge"));
    m_combo->addItem(new sgui::ListItem(L"hage"));
    m_combo->addItem(new sgui::ListItem(L"hige"));

    sgui::DDReciever *ddr;
    sgui::DDItem *ddi;
    ddr = new sgui::DDReciever(this, sgui::Point(140, 50), sgui::Size(20, 20));
    ddi = new sgui::DDItem(this, sgui::Point(), sgui::Size(16, 16));
    new sgui::ToolTip(ddi, L"ほげー");
    ddr->setItem(ddi);

    ddr = new sgui::DDReciever(this, sgui::Point(140, 70), sgui::Size(20, 20));
    ddi = new sgui::DDItem(this, sgui::Point(), sgui::Size(16, 16));
    new sgui::ToolTip(ddi, L"もげー");
    ddr->setItem(ddi);

    ddr = new sgui::DDReciever(this, sgui::Point(140, 90), sgui::Size(20, 20));
    ddr->setItem(new sgui::DDItem(this, sgui::Point(), sgui::Size(16, 16)));

    ddr = new sgui::DDReciever(this, sgui::Point(140, 110), sgui::Size(20, 20));
    ddr = new sgui::DDReciever(this, sgui::Point(140, 130), sgui::Size(20, 20));
  }

  bool handleEvent(const sgui::Event& evt)
  {
    if(evt.getType()==sgui::EVT_EDIT_ENTER) {
      if(evt.getSrc()==m_edit) {
        m_list->addItem(new sgui::ListItem(dynamic_cast<const sgui::EditEvent&>(evt).getString()));
      }
    }
    else if(evt.getType()==sgui::EVT_LIST_DOUBLECLICK) {
      if(evt.getSrc()==m_list) {
        m_text->setText(dynamic_cast<const sgui::ListEvent&>(evt).getItem()->getText());
        return true;
      }
    }
    else if(evt.getType()==sgui::EVT_COMBO) {
      if(evt.getSrc()==m_combo) {
        m_text->setText(dynamic_cast<const sgui::ListEvent&>(evt).getItem()->getText());
        return true;
      }
    }
    else if(evt.getType()==sgui::EVT_DD_RECIEVE) {
      wchar_t buf[64];
      swprintf(buf, 64, L"%p", dynamic_cast<const sgui::DragAndDropEvent&>(evt).getItem());
      m_text->setText(buf);
      return true;
    }

    return Super::handleEvent(evt);
  }
};


class MyView : public sgui::View
{
typedef sgui::View Super;
private:
  sgui::Label *m_text;

public:
  MyView(const std::string& title, const sgui::Size& size, const sgui::Size& gsize=sgui::Size()) :
    Super(title, size, gsize)
  {
    listenEvent(sgui::EVT_BUTTON_UP);
    listenEvent(sgui::EVT_BUTTON_DOWN);
    listenEvent(sgui::EVT_FILEDIALOG);
    setBackgroundColor(sgui::Color(0.2f, 0.2f, 0.2f));

    new sgui::Button(this, sgui::Point(10, 20), sgui::Size(80, 20), L"exit", BU_EXIT);
    new sgui::Button(this, sgui::Point(10, 45), sgui::Size(80, 20), L"MyDialog", BU_MYDIALOG);
    new sgui::Button(this, sgui::Point(10, 70), sgui::Size(80, 20), L"FileDialog", BU_FILEDIALOG);
    m_text = new sgui::Label(this, sgui::Point(10, 200), sgui::Size(250, 250), L"");
  }

  bool handleEvent(const sgui::Event& evt)
  {
    int id = evt.getSrc() ? evt.getSrc()->getID() : 0;
    m_text->setText(_L(typeid(evt).name())+L"\n");

    if(evt.getType()==sgui::EVT_BUTTON_UP) {
      if(id==BU_MYDIALOG) {
        new MyDialog(this);
        return true;
      }
      else if(id==BU_FILEDIALOG) {
        new sgui::FileDialog(L"file", 0, sgui::Point(100,100));
        return true;
      }
      else if(id==BU_EXIT) {
        sgui::App::instance()->stop();
        return true;
      }
    }
    else if(evt.getType()==sgui::EVT_FILEDIALOG) {
      m_text->setText(m_text->getText()+_L(dynamic_cast<const sgui::DialogEvent&>(evt).getPath()));
      return true;
    }
    return sgui::View::handleEvent(evt);
  }
};

class MyApp : public sgui::App
{
public:
  MyApp(int argc, char **argv) : sgui::App(argc, argv)
  {
  //  setDefaultFont("ipag-mona.ttf");
    setView(new MyView("test", sgui::Size(640, 480), sgui::Size(640, 480)));
  }

  bool handleEvent(const sgui::Event& e) {
    if(e.getType()==sgui::EVT_KEYUP) {
      const sgui::KeyboardEvent& k = dynamic_cast<const sgui::KeyboardEvent&>(e);
      if(k.getKey()==SDLK_ESCAPE) {
        stop();
        return true;
      }
    }
    return sgui::App::handleEvent(e);
  }
};



int main(int argc, char *argv[])
{
  return (new MyApp(argc, argv))->exec();
}

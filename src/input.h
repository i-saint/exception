#include "stdafx.h"

#ifndef input_h
#define input_h


namespace exception {


  class BaseInput : public IInput
  {
  public:
    ushort m_state;
    ushort m_pstate;

  public:
    enum {
      UP = 1<<0,
      DOWN = 1<<1,
      LEFT = 1<<2,
      RIGHT = 1<<3,
    };

    BaseInput() : m_state(0), m_pstate(0) {}

    BaseInput(Deserializer& s)
    {
      s >> m_state >> m_pstate;
    }

    virtual void serialize(Serializer& s) const
    {
      s << m_state << m_pstate;
    }

    bool up() const { return (m_state&UP)!=0; }
    bool down() const { return (m_state&DOWN)!=0; }
    bool left() const { return (m_state&LEFT)!=0; }
    bool right() const { return (m_state&RIGHT)!=0; }
    bool button(int b) const { return (m_state&(1<<(b+4)))!=0; }

    bool buttonPressed(int b) const
    {
      return (m_state&(1<<(b+4)))!=0 && (m_pstate&(1<<(b+4)))==0;
    }

    bool buttonReleased(int b) const
    {
      return (m_state&(1<<(b+4)))==0 && (m_pstate&(1<<(b+4)))!=0;
    }

    void setState(ushort v)
    {
      m_pstate = m_state;
      m_state = v;
    }

    ushort getState() { return m_state; }
  };



  SDL_Joystick* GetJoystick();          // app.cc 
  const vector2& GetMousePosition();    // 



  class InputStream : public BaseInput
  {
  typedef BaseInput Super;
  private:
    typedef std::vector<ushort> input_data;
    input_data m_data;
    size_t m_index;

  public:
    InputStream(Deserializer& s) : Super(s)
    {
      ist::deserialize_container(s, m_data);
      s >> m_index;
    }

    virtual void serialize(Serializer& s) const
    {
      Super::serialize(s);
      ist::serialize_container(s, m_data);
      s << m_index;
    }

  public:
    InputStream() : m_index(0)
    {
      m_data.reserve(256*1026);
    }

    InputStream(ist::gzbstream& s) : m_index(0)
    {
      ist::deserialize_container(s, m_data);
    }

    virtual void write(ist::gzbstream& s)
    {
      ist::serialize_container(s, m_data);
    }

    virtual void update()
    {
      if(m_index>=m_data.size()) {
        m_data.push_back(0);
      }
      setState(m_data[m_index++]);
    }

    void push(ushort v) { m_data.push_back(v); }
    size_t getLength() { return m_data.size(); }
    size_t getIndex() { return m_index; }
  };
  typedef intrusive_ptr<InputStream> input_ptr;


}

#endif

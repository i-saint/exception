#include "stdafx.h"

namespace exception {


  template<class T>
  class MessagePool
  {
  private:
    typedef T message_type;
    std::vector<message_ptr> m_cont;
    size_t m_counter;

  public:
    MessagePool() : m_counter(0)
    {
    }

    ~MessagePool()
    {
      for(size_t i=0; i<m_cont.size(); ++i) {
        delete m_cont[i];
      }
      m_cont.clear();
    }

    message_type* create()
    {
      while(m_counter<m_cont.size() && m_cont[m_counter]->isShared()) {
        ++m_counter;
      }
      if(m_counter==m_cont.size()) {
        m_cont.push_back(new message_type());
      }
      return static_cast<message_type*>(m_cont[m_counter++]);
    }

    void reset()
    {
      m_counter = 0;
    }

    size_t size()
    {
      return m_cont.size();
    }

    void clear()
    {
      m_counter = 0;
      m_cont.clear();
    }

    string p()
    {
      string mes;
      char buf[256];
      sprintf(buf, "%s : %d", typeid(message_type).name(), m_cont.size());
      mes+=buf;
      return mes;
    }
  };





  class ThreadSpecificMethod : public IThreadSpecificMethod
  {
  private:
    MessagePool<ConstructMessage> m_construct;
    MessagePool<UpdateMessage>    m_update;
    MessagePool<DamageMessage>    m_damage;
    MessagePool<DestroyMessage>   m_destroy;
    MessagePool<KillMessage>      m_kill;
    MessagePool<CollideMessage>   m_collide;
    MessagePool<AccelMessage>     m_accel;
    MessagePool<CallMessage>      m_call;
    gobj_vector m_objs;
    gvector_iter m_iter;
    IGame *m_game;

  public:
    ThreadSpecificMethod()
    {
      m_game = GetGame();
    }

    ~ThreadSpecificMethod()
    {}

    void resetMessageCache()
    {
      m_construct.reset();
      m_update.reset();
      m_damage.reset();
      m_destroy.reset();
      m_kill.reset();
      m_collide.reset();
      m_accel.reset();
      m_call.reset();
    }


    gobj_iter& getObjects(const box& box)
    {
      m_game->getObjects(m_objs, box);
      m_iter.reset(m_objs);
      return m_iter;
    }

    gobj_iter& getObjects(const sphere& sphere)
    {
      m_game->getObjects(m_objs, sphere);
      m_iter.reset(m_objs);
      return m_iter;
    }

    gobj_iter& getAllObjects()
    {
      m_iter.reset(m_game->getAllObjects());
      return m_iter;
    }


    virtual void sendMessage(gobj_ptr to, message_ptr m)=0;

    void sendConstructMessage(gobj_ptr from, gobj_ptr to)
    {
      if(!to || to->isDead()) { return; }

      ConstructMessage *m = m_construct.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      sendMessage(to, m);
    }

    void sendUpdateMessage(gobj_ptr from, gobj_ptr to)
    {
      UpdateMessage *m = m_update.create();
      m->m_from = from;
      m->m_to = to;
      sendMessage(to, m);
    }

    void sendDamageMessage(gobj_ptr from, gobj_ptr to, float d, gobj_ptr source=0)
    {
      if(!to || to->isDead()) { return; }

      DamageMessage *m = m_damage.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      m->m_damage = d;
      m->m_source = source && !source->isDead() ? source : 0;
      sendMessage(to, m);
    }

    void sendDestroyMessage(gobj_ptr from, gobj_ptr to, int stat=0)
    {
      if(!to || to->isDead()) { return; }

      DestroyMessage *m = m_destroy.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      m->m_stat = stat;
      sendMessage(to, m);
    }

    void sendKillMessage(gobj_ptr from, gobj_ptr to)
    {
      if(!to || to->isDead()) { return; }

      KillMessage *m = m_kill.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      sendMessage(to, m);
    }

    void sendCollideMessage(gobj_ptr from, gobj_ptr to, const vector4& p, const vector4& n, float d)
    {
      if(!to || to->isDead()) { return; }

      CollideMessage *m = m_collide.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      m->m_pos = p;
      m->m_normal = n;
      m->m_normal.w = 0.0f;
      m->m_distance = d;
      sendMessage(to, m);
    }

    void sendAccelMessage(gobj_ptr from, gobj_ptr to, const vector4& a)
    {
      if(!to || to->isDead()) { return; }

      AccelMessage *m = m_accel.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      m->m_accel = a;
      sendMessage(to, m);
    }

    void sendCallMessage(gobj_ptr from, gobj_ptr to, const string& name, const any& value)
    {
      if(!to || to->isDead()) { return; }

      CallMessage *m = m_call.create();
      m->m_from = from && !from->isDead() ? from : 0;
      m->m_to = to;
      m->m_name = name;
      m->m_value = value;
      sendMessage(to, m);
    }

    message_ptr deserializeMessage(Deserializer& s)
    {
      int type;
      s >> type;
      message_ptr m;
      switch(type) {
        case Message::CONSTRUCT: m = m_construct.create(); break;
        case Message::UPDATE:    m = m_update.create();    break;
        case Message::DAMAGE:    m = m_damage.create();    break;
        case Message::DESTROY:   m = m_destroy.create();   break;
        case Message::KILL:      m = m_kill.create();      break;
        case Message::COLLIDE:   m = m_collide.create();   break;
        case Message::ACCEL:     m = m_accel.create();     break;
        case Message::CALL:      m = m_call.create();    break;
        default: throw Error(
                   string("ThreadSpecificMethod::deserializeMessage() : unknown type\n")
                   + boost::lexical_cast<string>(type)); break;
      }
      m->deserialize(s);
      return m;
    }
  };


  class ThreadSpecificMethodSync : public ThreadSpecificMethod
  {
  public:
    void sendMessage(gobj_ptr to, message_ptr m)
    {
      to->pushMessage(m);
    }
  };


  class ThreadSpecificMethodAsync : public ThreadSpecificMethod
  {
  typedef ThreadSpecificMethod Super;
  private:
    message_queue m_store;
    gobj_ptr m_current;

  public:
    ThreadSpecificMethodAsync() : m_current(0)
    {}

    void setCurrent(gobj_ptr obj)
    {
      m_current = obj;
    }

    void flushMessage()
    {
      for(size_t i=0; i<m_store.size(); ++i) {
        message_ptr m = m_store[i];
        m->getTo()->pushMessage(m);
      }
      m_store.clear();
      resetMessageCache();
    }

    void sendMessage(gobj_ptr to, message_ptr m)
    {
      if(to==m_current) {
        to->pushMessage(m);
      }
      else {
        m_store.push_back(m);
      }
    }
  };



} // exception

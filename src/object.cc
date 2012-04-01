#include "stdafx.h"

namespace exception {

  void SerializeMessage(Serializer& s, const message_ptr m)
  {
    s << m->getType();
    m->serialize(s);
  }

  message_ptr DeserializeMessage(Deserializer& s)
  {
    return GetMainTSM().deserializeMessage(s);
  }




  RefCounter::RefCounter() : m_ref(0) {}
  RefCounter::~RefCounter() {}
  void RefCounter::addRef() { ++m_ref; }
  void RefCounter::release()
  {
    if(--m_ref==0) {
      delete this;
    }
  }
  size_t RefCounter::getRefCount() const { return m_ref; }



  linkage_info& GetLinkageInfo()
  {
    static linkage_info li;
    return li;
  }


#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  std::map<gid, Object*> g_objects;
  void PrintLeakObject()
  {
    puts("leak object:");
    for(std::map<gid, Object*>::iterator i=g_objects.begin(); i!=g_objects.end(); ++i) {
      puts(typeid(*i->second).name());
    }
  }
#endif
  size_t Object::s_idgen = 0;
  size_t Object::s_count = 0;

  gid Object::createID()
  {
    return ++s_idgen;
  }

  size_t Object::getCount() { return s_count; }

  Object::Object() : m_id(createID())
  {
    ++s_count;
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    g_objects[m_id] = this;
#endif
  }

  Object::Object(Deserializer& s) : m_id(0)
  {
    ++s_count;
    s >> m_id;
    s_idgen = std::max<gid>(s_idgen, m_id+1);
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    g_objects[m_id] = this;
#endif
  }

  Object::~Object()
  {
    --s_count;
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
    g_objects.erase(m_id);
#endif
  }

  void Object::reconstructLinkage()
  {}

  void Object::serialize(Serializer& s) const
  {
    s << m_id;
  }

  void Object::release() { delete this; }
  gid Object::getID() const { return m_id; }
  bool Object::call(const string& name, const any& value) { return false; }

  string Object::p()
  {
    string r = typeid(*this).name();
    r+="\n";
    char buf[64];
    sprintf(buf, "  id: %d\n", getID());
    r+=buf;
    return r;
  }


  void Message::deserialize(Deserializer& s)
  {
    DeserializeLinkage(s, m_from);
    DeserializeLinkage(s, m_to);
    s >> m_shared;
  }

  void Message::reconstructLinkage()
  {
    ReconstructLinkage(m_from);
    ReconstructLinkage(m_to);
  }

  void Message::serialize(Serializer& s) const
  {
    SerializeLinkage(s, m_from);
    SerializeLinkage(s, m_to);
    s  << m_shared;
  }


  void DestroyMessage::deserialize(Deserializer& s)
  {
    Super::deserialize(s);
    s >> m_stat;
  }

  void DestroyMessage::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_stat;
  }


  void DamageMessage::deserialize(Deserializer& s)
  {
    Super::deserialize(s);
    DeserializeLinkage(s, m_source);
    s >> m_damage;
  }

  void DamageMessage::reconstructLinkage()
  {
    Super::reconstructLinkage();
    ReconstructLinkage(m_source);
  }

  void DamageMessage::serialize(Serializer& s) const
  {
    Super::serialize(s);
    SerializeLinkage(s, m_source);
    s << m_damage;
  }


  void CollideMessage::deserialize(Deserializer& s)
  {
    Super::deserialize(s);
    s >> m_pos >> m_normal >> m_distance >> m_force;
  }

  void CollideMessage::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_pos << m_normal << m_distance << m_force;
  }


  void AccelMessage::deserialize(Deserializer& s)
  {
    Super::deserialize(s);
    s >> m_accel;
  }

  void AccelMessage::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_accel;
  }


  void CallMessage::deserialize(Deserializer& s)
  {
    Super::deserialize(s);
    s >> m_name >> m_value;
  }

  void CallMessage::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_name << m_value;
  }




  GameObject::GameObject() : m_killed(false), m_destroyed(false)
  {
    setTSM(GetMainTSM());
    SendConstructMessage(0, this);
  }

  GameObject::GameObject(Deserializer& s) : Super(s)
  {
    setTSM(GetMainTSM());
    s >> m_killed >> m_destroyed;
  }

  void GameObject::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_killed << m_destroyed;
  }

  void GameObject::update()
  {
    processMessageQueue();
  }

  void GameObject::asyncupdate() {}
  void GameObject::draw() {}
  float GameObject::getDrawPriority() { return 1.0f; }
  gobj_ptr GameObject::getParent() { return 0; }

  bool GameObject::isDestroyed() { return m_destroyed; }
  bool GameObject::isDead() { return m_killed; }
  bool GameObject::isKilled() { return m_killed; }

  void GameObject::onKill(KillMessage& m)
  {
    m_killed = true;
    clearMessage();
  }

  void GameObject::onDestroy(DestroyMessage& m)
  {
    m_destroyed = true;
    SendKillMessage(m.getFrom(), this);
  }

  void GameObject::onCall(CallMessage& m)
  {
    call(m.getName(), m.getValue());
  }




  size_t Solid::s_groupgen = 0;

  gid Solid::createGroupID()
  {
    return ++s_groupgen;
  }


  Solid::Solid(Deserializer& s) : Super(s)
  {
    s >> m_group;
    s_groupgen = std::max<gid>(s_groupgen, m_group+1);
  }

  void Solid::serialize(Serializer& s) const
  {
    Super::serialize(s);
    s << m_group;
  }

  Solid::Solid() : m_group(createGroupID())
  {}

  bool Solid::call(const string& name, const any& value)
  {
    if(name=="setGroup") setGroup(any_cast<gid>(value));
    else return Super::call(name, value);
    return true;
  }

  string Solid::p()
  {
    string r = Super::p();
    char buf[64];
    sprintf(buf, "  group: %d\n", getGroup());
    r+=buf;
    return r;
  }


  CacheInfo::info_cont CacheInfo::s_info;

} // exception 

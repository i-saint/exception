#include "stdafx.h"
#include "network.h"

namespace exception {

  iserver_ptr g_iserver;
  iclient_ptr g_iclient;

  IInputServer* GetInputServer() { return g_iserver.get(); }
  IInputClient* GetInputClient() { return g_iclient.get(); }

  void SendNMessage(const NMessage& t)
  {
    if(g_iclient) {
      g_iclient->push(t);
    }
  }

  bool IsLocalMode()
  {
    return g_iserver && typeid(*g_iserver)==typeid(InputClientLocal&);
  }

  bool IsServerMode()
  {
    return !!g_iserver;
  }

  bool IsClientMode()
  {
#ifdef EXCEPTION_ENABLE_NETPLAY
    return !g_iserver && g_iclient && typeid(*g_iclient)==typeid(InputClientIP&);
#else
    return false;
#endif // EXCEPTION_ENABLE_NETPLAY 
  }

  bool IsReplayMode()
  {
    return g_iclient && typeid(*g_iclient)==typeid(InputClientReplay&);
  }


  size_t GetSessionCount()
  {
    return g_iclient ? g_iclient->getSessionCount() : 0;
  }

  size_t GetSessionID()
  {
    return g_iclient->getSessionID();
  }

  session_ptr GetSession(size_t i)
  {
    return g_iclient->getSession(i);
  }

  session_ptr GetSessionByID(size_t id)
  {
    return g_iclient->getSessionByID(id);
  }

} // namespace exception 

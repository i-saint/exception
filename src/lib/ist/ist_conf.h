#ifndef IST_CONF_H
#define IST_CONF_H


#pragma warning(disable : 4996)

#define IST_BUILD_STATIC
//#define IST_BUILD_DYNAMIC


//#define IST_BIG_ENDIAN


#define IST_WITH_PNG
//#define IST_WITH_JPG




#ifdef _WIN32
  #if !defined(IST_BUILD_DYNAMIC) && !defined(IST_BUILD_STATIC)
    #pragma comment(lib,"ist.lib")
  #endif
#endif



#ifdef IST_BUILD_DYNAMIC
  #ifdef _WIN32
    #define IST_EXPORT __declspec(dllexport)
    #define IST_CLASS __declspec(dllexport)
    #define IST_DECLARE_DSO_ENTRYPOINT \
BOOL WINAPI DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)\
{\
  return TRUE;\
}
  #else // WIN32
    #define IST_EXPORT
    #define IST_CLASS
    #define IST_DECLARE_DSO_ENTRYPOINT
  #endif // WIN32
#else // IST_BUILD_DYNAMIC
  #ifdef _WIN32
    #define IST_EXPORT
    #define IST_CLASS
    #define IST_DECLARE_DSO_ENTRYPOINT
  #else // WIN32
    #define IST_EXPORT
    #define IST_CLASS
    #define IST_DECLARE_DSO_ENTRYPOINT
  #endif // WIN32
#endif // IST_BUILD_DYNAMIC


namespace ist {
  void Init();
  void Quit();
}
#include "ist_object.h"

#endif

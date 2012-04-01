#include "stdafx.h"
#include "ist/ist_sys.h"

#ifdef WIN32
  #include <tlhelp32.h>

  #pragma comment(lib,"SDL.lib")
  #pragma comment(lib,"SDLmain.lib")
  #pragma comment(lib,"SDL_mixer.lib")
  #pragma comment(lib,"opengl32.lib")
  #pragma comment(lib,"glu32.lib")
  #pragma comment(lib,"glew32.lib")
  #pragma comment(lib,"ftgl.lib")
  #pragma comment(lib,"zlib.lib")
  #pragma comment(lib,"libpng.lib")
#endif


namespace exception {
  sgui::App* CreateApp(int argc, char *argv[]);
#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  void PrintLeakObject();
#endif
}


#ifdef WIN32
bool FindProcess(const char *exe)
{
  HANDLE snap = (HANDLE)-1;
  snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if(snap==(HANDLE)-1) {
    return false;
  }

  bool res = false;
  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(pe);
  BOOL n = Process32First(snap, &pe);
  while(n) {
    if(lstrcmpi(exe, pe.szExeFile)==0) {
      res = true;
      break;
    }
    n = Process32Next(snap, &pe);
  }
  CloseHandle(snap);
  return res;
}
#endif

int main(int argc, char *argv[])
{
#ifdef EXCEPTION_CHECK_LEAK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  char *hoge = new char[128];
#endif


#ifdef WIN32
  while(FindProcess("updater.exe")) {
    Sleep(100);
  }
  while(FindProcess("exception_config.exe")) {
    Sleep(100);
  }

  if(ist::IsFile("updater.exe.tmp")) {
    if(ist::IsFile("updater.exe")) {
      remove("updater.exe");
    }
    rename("updater.exe.tmp", "updater.exe");
  }
  if(ist::IsFile("exception_config.exe.tmp")) {
    if(ist::IsFile("exception_config.exe")) {
      remove("exception_config.exe");
    }
    rename("exception_config.exe.tmp", "exception_config.exe");
  }
#endif

  try {
    exception::CreateApp(argc, argv)->exec();
  }
  catch(const std::exception& e) {
#ifdef WIN32
    MessageBox(NULL, e.what(), "error", MB_OK);
#else
    puts(e.what());
#endif
  }

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  exception::PrintLeakObject();
#endif

  return 0;
}

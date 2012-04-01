#include "ist_conf.h"
#include "gl_include.h"

#ifdef _WIN32
  #include <windows.h>
#endif

#ifdef IST_BUILD_DYNAMIC
IST_DECLARE_DSO_ENTRYPOINT
#endif

namespace ist {
  void Init() {
  }

  void Quit() {
  }
}

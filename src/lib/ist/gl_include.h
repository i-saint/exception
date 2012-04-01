#ifndef GL_INCLUDE_INCLUDE
#define GL_INCLUDE_INCLUDE

#ifdef _WIN32
  #ifndef WIN32
    #define WIN32
  #endif
#endif


#include <GL/glew.h>
#include <GL/glu.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

/*
#ifdef _WIN32
  #define WIN32
  #include <GL/gl.h>
  #include <GL/glu.h>
#elif linux
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glx.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif
*/

#endif

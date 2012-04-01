#ifndef ist_frame_buffer_object_h
#define ist_frame_buffer_object_h

#include <vector>
#include <boost/smart_ptr.hpp>
#include <GL/glew.h>

#include "ist_conf.h"

namespace ist {

  class FrameBufferObject : public Object
  {
  private:
    GLuint m_fbo;
    GLuint m_color;
    GLuint m_depth;
    GLuint m_rb_depth;
    GLsizei m_width;
    GLsizei m_height;
    GLsizei m_screen_width;
    GLsizei m_screen_height;

    int m_viewport[4];
    int m_prev_target;

    void gen(int flag)
    {
      bool enable_color = (flag & GL_COLOR_BUFFER_BIT)!=0;
      bool enable_depth = (flag & GL_DEPTH_BUFFER_BIT)!=0;

      m_screen_width = m_width;
      m_screen_height = m_height;
      if(!GLEW_ARB_texture_non_power_of_two) {
        GLsizei w = 16;
        GLsizei h = 16;
        while(w<m_width) { w*=2; }
        while(h<m_height) { h*=2; }
        m_width = w;
        m_height = h;
      }

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      if(enable_color) {
        glGenTextures(1, &m_color);
        glBindTexture(GL_TEXTURE_2D, m_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        CheckGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
      if(enable_depth) {
      /*
        glGenTextures(1, &m_depth);
        glBindTexture(GL_TEXTURE_2D, m_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      */
      }

      glGenFramebuffersEXT(1, &m_fbo);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
      if(enable_color) {
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_color, 0);
      }
      if(enable_depth) {
      //  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depth, 0);

        glGenRenderbuffersEXT(1, &m_rb_depth);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_rb_depth);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, m_width, m_height);
        CheckGLError();
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_rb_depth);
      }
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    }

  public:
    FrameBufferObject(int flag=GL_COLOR_BUFFER_BIT) :
        m_fbo(0), m_color(0), m_depth(0), m_rb_depth(0), m_width(0), m_height(0)
    {
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      m_width = viewport[2]-viewport[0];
      m_height = viewport[3]-viewport[1];

      gen(flag);
    }

    FrameBufferObject(GLsizei width, GLsizei height, int flag=GL_COLOR_BUFFER_BIT) :
        m_fbo(0), m_color(0), m_depth(0), m_rb_depth(0), m_width(width), m_height(height)
    {
      gen(flag);
    }

    ~FrameBufferObject()
    {
      glDeleteRenderbuffersEXT(1, &m_rb_depth);
      glDeleteTextures(1, &m_depth);
      glDeleteTextures(1, &m_color);
      glDeleteFramebuffersEXT(1, &m_fbo);
    }

    GLsizei getWidth() const { return m_width; }
    GLsizei getHeight() const { return m_height; }

    void enable()
    {
      // 現在のviewportを退避し、FBOのサイズに合わせる 
      glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_prev_target);
      glGetIntegerv(GL_VIEWPORT, m_viewport);
      glViewport(0,0, getWidth(), getHeight());

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
      CheckGLError();
    }

    void disable()
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_prev_target);

      // 退避したviewportを戻す 
      glViewport(m_viewport[0],m_viewport[1], m_viewport[2],m_viewport[3]);
    }


    void assignColor() { glBindTexture(GL_TEXTURE_2D, m_color); }
    void assignDepth() { glBindTexture(GL_TEXTURE_2D, m_depth); }

    void assign()
    {
      glBindTexture(GL_TEXTURE_2D, m_color);
      CheckGLError();
    }

    void disassign()
    {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  };

} // ist

/*
 // 使用例: 

  FrameBufferObject *fbo;
  fbo = new FrameBufferObject(640, 480);
  //fbo->attach(new RenderBuffer(GL_DEPTH_COMPONENT)); // depthバッファを使いたい時付け加える 


  fbo->enable();
  // ここでFBOに描きたいもんを描画 
  fbo->disable();
  fbo->assign(); // FBOをテクスチャとして使用 
  glEnable(GL_TEXTURE_2D);
  // 何か描画 
  glDisable(GL_TEXTURE_2D);
*/

#endif

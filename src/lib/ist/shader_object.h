#ifndef ist_shader_object
#define ist_shader_object

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/smart_ptr.hpp>
#include <GL/glew.h>

#include "ist_conf.h"

namespace ist
{

  class ShaderObject : public Object
  {
  private:
    GLhandleARB m_handle;
    std::string m_name;
    std::string m_source;
    bool m_compiled;

  public:
    ShaderObject(const std::string& filename, const GLenum shader_type)
      : m_handle(0), m_name(filename), m_compiled(false)
    {
      // create
      m_handle = glCreateShaderObjectARB(shader_type);
      if(glGetError() != GL_NO_ERROR) {
        std::cerr << "ShaderObject::ShaderObject(): cannot create shader object: " << m_name << std::endl;
      }

      // read source file
      std::ifstream  f_in(filename.c_str(), std::ios::binary);
      if(f_in.fail()) {
        std::cerr << "ShaderObject::ShaderObject(): cannot open file: " << m_name << std::endl;
      }
      std::ostringstream str_out;
      str_out << f_in.rdbuf();
      m_source = str_out.str();
      f_in.close();

      // set shader source
      const char *s = m_source.c_str();
      int l = m_source.length();
      glShaderSourceARB(m_handle, 1, &s, &l);
      if(glGetError() != GL_NO_ERROR) {
        std::cerr << "ShaderObject::ShaderObject(): cannot set shader source: " << m_name << std::endl;
      }

      CheckGLError();

      // compile
      compile();
    }

    virtual ~ShaderObject()
    {
      glDeleteObjectARB(m_handle);
    }

    bool operator!() const { return !m_compiled; }
    GLhandleARB getHandle() const { return m_handle; }
    const std::string& getFileName() const { return m_name; }
    const std::string& getSource() const   { return m_source; }

    bool compile()
    {
      if(!m_handle) {
        return false;
      }

      // compile
      glCompileShaderARB(m_handle);

      // get errors
      GLint result;
      glGetObjectParameterivARB(m_handle, GL_OBJECT_COMPILE_STATUS_ARB, &result);
      if(glGetError()!=GL_NO_ERROR || result==GL_FALSE) {
        std::string error_message;
        int length;
        glGetObjectParameterivARB(m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
        if(length > 0) {
          int l;
          GLcharARB *info_log = new GLcharARB[length];
          glGetInfoLogARB(m_handle, length, &l, info_log);
          error_message = info_log;
          delete[] info_log;
        }
        throw std::runtime_error("ShaderObject::compile(): cannot compile shader: "+m_name+"\n"+error_message);
      }
      m_compiled = true;
      CheckGLError();

      return true;
    }
  };


  class ProgramObject : public Object
  {
  public:
    typedef boost::intrusive_ptr<ShaderObject> so_ptr;
  private:
    GLhandleARB m_handle;
    std::vector<so_ptr> m_attachment;

  public:
    ProgramObject() : m_handle(0)
    {
      m_handle = glCreateProgramObjectARB();
      CheckGLError();
    }

    virtual ~ProgramObject()
    {
      m_attachment.clear();
      glDeleteObjectARB(m_handle);
    }

    bool operator!() const { return !m_handle; }

    void attach(so_ptr s)
    {
      glAttachObjectARB(m_handle, s->getHandle());
      CheckGLError();
      m_attachment.push_back(s);
    }

    bool link()
    {
      if(!m_handle) {
        return false;
      }

      // link
      glLinkProgramARB(m_handle);

      // get errors
      GLint result;
      glGetObjectParameterivARB(m_handle, GL_OBJECT_LINK_STATUS_ARB, &result);

      if(glGetError() != GL_NO_ERROR || result==GL_FALSE) {
        std::string error_message;
        int length;
        glGetObjectParameterivARB(m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
        if(length > 0) {
          int  l;
          GLcharARB *info_log = new GLcharARB[length];
          glGetInfoLogARB(m_handle, length, &l, info_log);
          error_message = info_log;
          std::cerr << info_log << std::endl;
          delete[] info_log;
        }
        throw std::runtime_error("ProgramObject::link(): cannot link program object" + error_message);
      }
      CheckGLError();

      return true;
    }

    void enable()
    {
      glUseProgramObjectARB(m_handle);
      CheckGLError();
    }

    void disable()
    {
      glUseProgramObjectARB(0);
    }


    GLint getUniformLocation(const char *name)
    {
      GLint ul = glGetUniformLocationARB(m_handle, name);
      if(ul == -1) {
        std::cerr << "ProgramObject::getUniformLocation(): no such uniform named " << name << std::endl;
      }
      return ul;
    }

    GLint getAttribLocation(const char *name)
    {
      GLint al = glGetAttribLocationARB(m_handle, name);
      if(al == -1) {
        std::cerr << "ProgramObject::getAttribLocation(): no such attribute named " << name << std::endl;
      }
      return al;
    }

    // uniform variable
    // int
    void setUniform1i(const char *name, GLint v0) { glUniform1iARB(getUniformLocation(name), v0); }
    void setUniform2i(const char *name, GLint v0, GLint v1) { glUniform2iARB(getUniformLocation(name), v0, v1); }
    void setUniform3i(const char *name, GLint v0, GLint v1, GLint v2) { glUniform3iARB(getUniformLocation(name), v0, v1, v2); }
    void setUniform4i(const char *name, GLint v0, GLint v1, GLint v2, GLint v3) { glUniform4iARB(getUniformLocation(name), v0, v1, v2, v3); }

    // float
    void setUniform1f(const char *name, GLfloat v0) { glUniform1fARB(getUniformLocation(name), v0); }
    void setUniform2f(const char *name, GLfloat v0, GLfloat v1) { glUniform2fARB(getUniformLocation(name), v0, v1); }
    void setUniform3f(const char *name, GLfloat v0, GLfloat v1, GLfloat v2) { glUniform3fARB(getUniformLocation(name), v0, v1, v2); }
    void setUniform4f(const char *name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { glUniform4fARB(getUniformLocation(name), v0, v1, v2, v3); }

    // int array
    void setUniform1iv(const char *name, GLuint count, const GLint *v) { glUniform1ivARB(getUniformLocation(name), count, v); }
    void setUniform2iv(const char *name, GLuint count, const GLint *v) { glUniform2ivARB(getUniformLocation(name), count, v); }
    void setUniform3iv(const char *name, GLuint count, const GLint *v) { glUniform3ivARB(getUniformLocation(name), count, v); }
    void setUniform4iv(const char *name, GLuint count, const GLint *v) { glUniform4ivARB(getUniformLocation(name), count, v); }

    // float array
    void setUniform1fv(const char *name, GLuint count, const GLfloat *v) { glUniform1fvARB(getUniformLocation(name), count, v); }
    void setUniform2fv(const char *name, GLuint count, const GLfloat *v) { glUniform2fvARB(getUniformLocation(name), count, v); }
    void setUniform3fv(const char *name, GLuint count, const GLfloat *v) { glUniform3fvARB(getUniformLocation(name), count, v); }
    void setUniform4fv(const char *name, GLuint count, const GLfloat *v) { glUniform4fvARB(getUniformLocation(name), count, v); }

    // matrix
    void setMatrix2fv(const char *name, GLuint count, GLboolean transpose, const GLfloat *v) { glUniformMatrix2fvARB(getUniformLocation(name), count, transpose, v); }
    void setMatrix3fv(const char *name, GLuint count, GLboolean transpose, const GLfloat *v) { glUniformMatrix3fvARB(getUniformLocation(name), count, transpose, v); }
    void setMatrix4fv(const char *name, GLuint count, GLboolean transpose, const GLfloat *v) { glUniformMatrix4fvARB(getUniformLocation(name), count, transpose, v); }

    // attribute variable
    // float
    void setAttrib1f(GLint al, GLfloat v0) { glVertexAttrib1fARB(al, v0); }
    void setAttrib2f(GLint al, GLfloat v0, GLfloat v1) { glVertexAttrib2fARB(al, v0, v1); }
    void setAttrib3f(GLint al, GLfloat v0, GLfloat v1, GLfloat v2) { glVertexAttrib3fARB(al, v0, v1, v2); }
    void setAttrib4f(GLint al, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { glVertexAttrib4fARB(al, v0, v1, v2, v3); }

    // float array
    void setAttrib1fv(GLint al, const GLfloat *v) { glVertexAttrib1fvARB(al, v); }
    void setAttrib2fv(GLint al, const GLfloat *v) { glVertexAttrib2fvARB(al, v); }
    void setAttrib3fv(GLint al, const GLfloat *v) { glVertexAttrib3fvARB(al, v); }
    void setAttrib4fv(GLint al, const GLfloat *v) { glVertexAttrib4fvARB(al, v); }
  };


  class FragmentShader : public ShaderObject
  {
  public:
    FragmentShader(const std::string& filename)
      : ShaderObject(filename, GL_FRAGMENT_SHADER_ARB)
    {}
  };

  class VertexShader : public ShaderObject
  {
  public:
    VertexShader(const std::string& filename)
      : ShaderObject(filename, GL_VERTEX_SHADER_ARB)
    {}
  };
}

#endif

/*
 // Žg—p—á: 

  ProgramObject *po;
  po = new ProgramObject();
  po->attach(new VertexShader("hoge.vsh"));
  po->attach(new FragmentShader("hoge.fsh"));
  po->link();

  po->enable();
  // ‚±‚±‚Å•’Ê‚É•`‰æ 
  po->disable();
*/

#include <stdexcept>
#include "ist_i3d.h"

#define BUFFER_OFFSET(i) ((char*)0 + (i))



namespace ist {

  
  void _CheckGLError()
  {
    int code = glGetError();
    if(code!=GL_NO_ERROR) {
      throw std::runtime_error((const char*)gluErrorString(code));
    }
  }



/*
  GLInfo
*/

void GLInfo::print(std::ostream& out) {
  out << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;
  out << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
  out << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;

  out << std::endl;
  out << "GL_EXTENSIONS:" << std::endl;
  for(const GLubyte *ext = glGetString(GL_EXTENSIONS); *ext!='\0'; ++ext) {
    if(*ext==' ') {
      out << std::endl;
    }
    else {
      out << *ext;
    }
  }

  out << std::endl;
  int v[2];
  float fv[2];
  glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, v);
  out << "GL_MAX_ATTRIB_STACK_DEPTH:     " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_CLIP_PLANES, v);
  out << "GL_MAX_CLIP_PLANES:            " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_EVAL_ORDER, v);
  out << "GL_MAX_EVAL_ORDER:             " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_LIGHTS, v);
  out << "GL_MAX_LIGHTS:                 " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_LIST_NESTING, v);
  out << "GL_MAX_LIST_NESTING:           " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, v);
  out << "GL_MAX_MODELVIEW_STACK_DEPTH:  " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, v);
  out << "GL_MAX_NAME_STACK_DEPTH:       " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, v);
  out << "GL_MAX_PIXEL_MAP_TABLE:        " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, v);
  out << "GL_MAX_PROJECTION_STACK_DEPTH: " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, v);
  out << "GL_MAX_TEXTURE_SIZE:           " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, v);
  out << "GL_MAX_TEXTURE_STACK_DEPTH:    " << v[0] << std::endl;

  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
  out << "GL_MAX_VIEWPORT_DIMS:          " << v[0] << " " << v[1] << std::endl;


  glGetIntegerv(GL_SUBPIXEL_BITS, v);
  out << "GL_SUBPIXEL_BITS:              " << v[0] << std::endl;

  glGetFloatv(GL_POINT_SIZE_RANGE, fv);
  out << "GL_POINT_SIZE_RANGE:           " << fv[0] << " " << fv[1] << std::endl;

  glGetFloatv(GL_POINT_SIZE_GRANULARITY, fv);
  out << "GL_POINT_GRANULARITY:          " << fv[0] << std::endl;

  glGetFloatv(GL_LINE_WIDTH_RANGE, fv);
  out << "GL_LINE_WIDTH_RANGE:           " << fv[0] << " " << fv[1] << std::endl;

  glGetFloatv(GL_LINE_WIDTH_GRANULARITY, fv);
  out << "GL_LINE_WIDTH_GRANULARITY:     " << fv[0] << std::endl;
  out << std::endl;
  out << std::endl;
}








/*
  Matrix
*/

void MatrixOp::ScreenMatrix() {
  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,viewport[2], 0,viewport[3], -1,5000);
//  gluOrtho2D(0,viewport[2], 0,viewport[3]);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void MatrixOp::ScreenMatrixIV()
{
  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,viewport[2], viewport[3],0, -1,5000);
//  gluOrtho2D(0,viewport[2], viewport[3],0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

MatrixSaver::MatrixSaver(int matrix_mode)
{
  switch(matrix_mode) {
  case GL_MODELVIEW_MATRIX:
    m_matrix_mode = GL_MODELVIEW;
    break;
  case GL_PROJECTION_MATRIX:
    m_matrix_mode = GL_PROJECTION;
    break;
  case GL_TEXTURE_MATRIX:
    m_matrix_mode = GL_TEXTURE;
    break;
  default:
    m_matrix_mode = GL_MODELVIEW;
    break;
  }

  glGetFloatv(matrix_mode, m_matrix.v);

  /*
  int mode;
  glGetIntegerv(GL_MATRIX_MODE, &mode);
  glMatrixMode(m_matrix_mode);
  glPushMatrix();
  glMatrixMode(mode);
  */
}

MatrixSaver::~MatrixSaver()
{
  int mode;
  glGetIntegerv(GL_MATRIX_MODE, &mode);

  glMatrixMode(m_matrix_mode);

  glLoadMatrixf(m_matrix.v);
  /*
  glPopMatrix();
  */

  glMatrixMode(mode);
}

const matrix44& MatrixSaver::getMatrix() const { return m_matrix; }









/*
  Material
*/

Material::Material() :
  m_ambient(0.2f, 0.2f, 0.2f, 1.0f),
  m_diffuse(0.8f, 0.8f, 0.8f, 1.0f),
  m_specular(0.0f, 0.0f, 0.0f, 0.0f),
  m_emission(0.0f, 0.0f, 0.0f, 0.0f),
  m_shininess(60.0f)
{};

void Material::assign() const
{
  glMaterialfv(GL_FRONT, GL_AMBIENT,   m_ambient.v);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   m_diffuse.v);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  m_specular.v);
  glMaterialfv(GL_FRONT, GL_EMISSION,  m_emission.v);
  glMaterialfv(GL_FRONT, GL_SHININESS,&m_shininess);
}

void Material::serialize(bostream& b) const
{
  b << m_ambient << m_diffuse << m_specular << m_emission << m_shininess;
}

void Material::deserialize(bistream& b)
{
  b >> m_ambient >> m_diffuse >> m_specular >> m_emission >> m_shininess;
}


Color::Color() :
  m_color(1.0f, 1.0f, 1.0f, 1.0f)
{}

Color::Color(float r, float g, float b, float a) :
  m_color(r, g, b, a)
{}

void Color::assign() const {
  glColor4fv(m_color.v);
}

void Color::serialize(bostream& b) const
{
  b << m_color;
}

void Color::deserialize(bistream& b)
{
  b >> m_color;
}



/*
  Light
*/

namespace light {
  int light_table[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
  const Light *lights[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  int assign_id(const Light *lp) {
    for(int i=0; i<8; ++i) {
      if(lights[i]==0) {
        lights[i] = lp;
        return light_table[i];
      }
    }
    return 0;
  }

  bool remove_id(const Light *lp) {
    for(int i=0; i<8; ++i) {
      if(lights[i]==lp) {
        lights[i] = 0;
        return true;
      }
    }
    return false;
  }
}

Light::Light()
  : m_ambient(0.2f, 0.2f, 0.2f, 1.0f),
    m_diffuse(0.8f, 0.8f, 0.8f, 1.0f),
    m_specular(1.0f, 1.0f, 1.0f, 1.0f),
    m_constant_att(1.0f), m_linear_att(0.0f), m_quadratic_att(0.0f),
    m_enabled(false)
{
  m_lightid = light::assign_id(this);
}
  
Light::~Light()
{
  disable();
  light::remove_id(this);
}

void Light::enable()
{
  if(m_lightid!=0) {
    m_enabled = true;
    glEnable(m_lightid);
    glLightfv(m_lightid, GL_POSITION, m_position.v);
    glLightfv(m_lightid, GL_AMBIENT,  m_ambient.v);
    glLightfv(m_lightid, GL_DIFFUSE,  m_diffuse.v);
    glLightfv(m_lightid, GL_SPECULAR, m_specular.v);
    glLightf(m_lightid, GL_CONSTANT_ATTENUATION, m_constant_att);
    glLightf(m_lightid, GL_LINEAR_ATTENUATION, m_linear_att);
    glLightf(m_lightid, GL_QUADRATIC_ATTENUATION, m_quadratic_att);
  }
}

void Light::disable()
{
  if(m_lightid!=0) {
    m_enabled = false;
    glDisable(m_lightid);
  }
}

bool Light::isValid() const
{
  return m_lightid!=0;
}

bool Light::isEnabled() const
{
  return m_enabled;
}

void Light::serialize(bostream& b) const
{
  b << m_position << m_ambient << m_diffuse << m_specular
    << m_constant_att << m_linear_att << m_quadratic_att
    << m_enabled;
}

void Light::deserialize(bistream& b)
{
  b >> m_position >> m_ambient >> m_diffuse >> m_specular
    >> m_constant_att >> m_linear_att >> m_quadratic_att
    >> m_enabled;

  if(m_enabled) {
    enable();
  }
}



/*
  Fog
*/

Fog::Fog() : m_near(0), m_far(100.0f), m_enabled(false) {}
Fog::~Fog() { disable(); }

void Fog::enable()
{
  m_enabled = true;
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, m_near);
  glFogf(GL_FOG_END, m_far);
  glFogfv(GL_FOG_COLOR, m_color.v);
}

void Fog::disable()
{
  glDisable(GL_FOG);
  m_enabled = false;
}

void Fog::serialize(bostream& b) const
{
  b << m_position << m_color << m_near << m_far << m_enabled;
}

void Fog::deserialize(bistream& b)
{
  b >> m_position >> m_color >> m_near >> m_far >> m_enabled;

  if(m_enabled) {
    enable();
  }
}


/*
  Camera
*/

ViewFrustum::ViewFrustum() : m_znear(1.0f), m_zfar(2000.0f) {
  m_vertex.resize(5);
  m_normal.resize(6);
  m_index.resize(18);

  m_index[0]=0; m_index[1]=1; m_index[2]=2;
  m_index[3]=0; m_index[4]=2; m_index[5]=3;
  m_index[6]=0; m_index[7]=3; m_index[8]=4;
  m_index[9]=0; m_index[10]=4; m_index[11]=1;
  m_index[12]=1; m_index[13]=4; m_index[14]=3;
  m_index[15]=3; m_index[16]=2; m_index[17]=1;
}

void ViewFrustum::setup(const vector4& pos, const vector4& target, const vector3& up, float znear, float zfar, float fovy, float aspect) {
  float fovyx = fovy*aspect;
  m_front = vector3((target-pos).v).normalize();
  m_pos = pos;
  m_znear = znear;
  m_zfar = zfar;

  vector3 vx;
  vector3 vy;
  vx.cross(m_front, up);
  vx.normalize();
  vy.cross(vx, m_front);
  vy.normalize();

  matrix33 rot_t = matrix33().rotateA(vx, -90-fovy/2.0f);
  matrix33 rot_b = matrix33().rotateA(vx,  90+fovy/2.0f);
  matrix33 rot_r = matrix33().rotateA(vy,  90+fovyx/2.0f);
  matrix33 rot_l = matrix33().rotateA(vy, -90-fovyx/2.0f);
  m_top   = rot_t*m_front;
  m_bottom= rot_b*m_front;
  m_left  = rot_l*m_front;
  m_right = rot_r*m_front;
  m_vertex[0] = pos;
  m_vertex[1] = pos - (m_top+m_right).v4()*m_zfar;
  m_vertex[2] = pos - (m_top+m_left).v4()*m_zfar;
  m_vertex[3] = pos - (m_bottom+m_left).v4()*m_zfar;
  m_vertex[4] = pos - (m_bottom+m_right).v4()*m_zfar;
  for(size_t i=0; i<m_index.size()/3; ++i) {
    m_normal[i] = vector4().cross(
      m_vertex[m_index[i*3+1]]-m_vertex[m_index[i*3]],
      m_vertex[m_index[i*3+2]]-m_vertex[m_index[i*3]]).normalize();
  }

  setBoundingBox();
}

int ViewFrustum::test(const vector4& center, float r) const {
  vector4 rel_pos = center-m_pos;
  float dist = m_front.dot(rel_pos);

  if(dist<m_znear-r || dist>m_zfar+r) {
    return 0;
  }
  float dot_top = m_top.dot(rel_pos);
  float dot_bottom = m_bottom.dot(rel_pos);
  float dot_right = m_right.dot(rel_pos);
  float dot_left = m_left.dot(rel_pos);
  if(dot_top>r || dot_bottom>r || dot_right>r || dot_left>r) {
    return 0;
  }
  else if(dot_top>-r || dot_bottom>-r || dot_right>-r || dot_left>-r) {
    return 1;
  }
  return 2;
}

int ViewFrustum::test(const ICollision& cm) const {
  if(cm.getType()==CM_TRUE) {
    return 2;
  }
  else if(cm.getType()==CM_POINT) {
    const IPointCollision& pcm = static_cast<const IPointCollision&>(cm);
    return test(pcm.getPosition(), pcm.getRadius());
  }
  else if(cm.getType()==CM_LINE) {
    const ILineCollision& lcm = static_cast<const ILineCollision&>(cm);
    return test(lcm.getHead(), lcm.getRadius());
  }

  return false;
}

void ViewFrustum::serialize(bostream& b) const
{
  b << m_pos << m_front << m_top << m_bottom << m_right << m_left << m_znear << m_zfar;
}

void ViewFrustum::deserialize(bistream& b)
{
  b >> m_pos >> m_front >> m_top >> m_bottom >> m_right >> m_left >> m_znear >> m_zfar;
}



Camera::Camera() :
  m_position(0.0f, 0.0f, 100.0f),
  m_target(0.0f, 0.0f, 0.0f),
  m_direction(0.0f, 1.0f, 0.0f),
  m_znear(1.0f), m_zfar(1000.0f)
{}

Camera::~Camera() {}

void Camera::setPosition(const vector4& v)  { m_position=v; }
void Camera::setTarget(const vector4& v)    { m_target=v; }
void Camera::setDirection(const vector3& v) { m_direction=v; }
void Camera::setZNear(float v) { m_znear=v; }
void Camera::setZFar(float v)  { m_zfar=v; }

const vector4& Camera::getPosition() const  { return m_position; }
const vector4& Camera::getTarget() const    { return m_target; }
const vector3& Camera::getDirection() const { return m_direction; }
float Camera::getZNear() const { return m_znear; }
float Camera::getZFar() const  { return m_zfar; }

void Camera::serialize(bostream& b) const
{
  b << m_position << m_target << m_direction << m_znear << m_zfar;
}

void Camera::deserialize(bistream& b)
{
  b >> m_position >> m_target >> m_direction >> m_znear >> m_zfar;
}



OrthographicCamera::OrthographicCamera() :
  m_left(0.0f),
  m_right(100.0f),
  m_bottom(100.0f),
  m_top(0.0f)
{
  setZNear(-1.0f);
}

void OrthographicCamera::look() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(getLeft(), getRight(), getBottom(), getTop(), getZNear(), getZFar());
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  const vector4& pos = getPosition();
  const vector4& tar = getTarget();
  const vector3& dir = getDirection();
  gluLookAt(
    pos.x, pos.y, pos.z,
    tar.x, tar.y, tar.z,
    dir.x, dir.y, dir.z);
}

void OrthographicCamera::setScreen(float l, float r, float b, float t) {
  m_left = l;
  m_right = r;
  m_bottom = b;
  m_top = t;
}

float OrthographicCamera::getLeft() const   { return m_left; }
float OrthographicCamera::getRight() const  { return m_right; }
float OrthographicCamera::getBottom() const { return m_bottom; }
float OrthographicCamera::getTop() const    { return m_top; }

void OrthographicCamera::serialize(bostream& b) const
{
  Super::serialize(b);
  b << m_left << m_right << m_bottom << m_top;
}

void OrthographicCamera::deserialize(bistream& b)
{
  Super::deserialize(b);
  b >> m_left >> m_right >> m_bottom >> m_top;
}



PerspectiveCamera::PerspectiveCamera() :
  m_fovy(60.0f), m_aspect(1.3333333f), m_mod(true)
{}

void PerspectiveCamera::look() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(getFovy(), getAspect(), getZNear(), getZFar());
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  const vector4& pos = getPosition();
  const vector4& tar = getTarget();
  const vector3& dir = getDirection();
  gluLookAt(
    pos.x, pos.y, pos.z,
    tar.x, tar.y, tar.z,
    dir.x, dir.y, dir.z);
}

const ViewFrustum& PerspectiveCamera::getViewFrustum() const
{
  if(m_mod) {
    m_frustum.setup(getPosition(), getTarget(), getDirection(), getZNear(), getZFar(), getFovy(), getAspect());
    m_mod = false;
  }
  return m_frustum;
}

void PerspectiveCamera::setPosition(const vector4& v)  { Camera::setPosition(v);  m_mod=true; }
void PerspectiveCamera::setTarget(const vector4& v)    { Camera::setTarget(v);    m_mod=true; }
void PerspectiveCamera::setDirection(const vector3& v) { Camera::setDirection(v); m_mod=true; }
void PerspectiveCamera::setZNear(float v)  { Camera::setZNear(v); m_mod=true; }
void PerspectiveCamera::setZFar(float v)   { Camera::setZFar(v);  m_mod=true; }
void PerspectiveCamera::setFovy(float v)   { m_fovy=v;   m_mod=true; }
void PerspectiveCamera::setAspect(float v) { m_aspect=v; m_mod=true; }

float PerspectiveCamera::getFovy() const   { return m_fovy; }
float PerspectiveCamera::getAspect() const { return m_aspect; }

void PerspectiveCamera::serialize(bostream& b) const
{
  Super::serialize(b);
  b << m_fovy << m_aspect << m_mod << m_frustum;
}

void PerspectiveCamera::deserialize(bistream& b)
{
  Super::deserialize(b);
  b >> m_fovy >> m_aspect >> m_mod >> m_frustum;
}


/*
  Texture
*/
Texture::Texture() : m_tex_name(0) { setDefaultParam(); }

Texture::Texture(const std::string& filename) : m_tex_name(0)
{
  setDefaultParam();
  load(Bitmap(filename));
}

Texture::Texture(const Bitmap& src) : m_tex_name(0)
{
  setDefaultParam();
  load(src);
}

Texture::~Texture() { clear(); }

void Texture::setDefaultParam()
{
  setType(GL_RGBA);
  setMagFilter(GL_LINEAR); // GL_LINEAR | GL_NEAREST
  setMinFilter(GL_LINEAR);
  setWarpS(GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE | GL_REPEAT
  setWarpT(GL_CLAMP_TO_EDGE);
  setEnvMode(GL_MODULATE); // GL_MODULATE | GL_DECAL | GL_BLEND | GL_ADD
}

void Texture::clear()
{
  if(m_tex_name) {
    glDeleteTextures(1, &m_tex_name);
    m_tex_name = 0;
  }
}

void Texture::setParam() const
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_warp_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_warp_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_mag_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_min_filter);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_env_mode);
  
  float ecolor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_env_mode);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, ecolor);
}

bool Texture::load(const std::string& filename)
{
  Bitmap bmp;
  if(!bmp.load(filename)) {
    return false;
  }
  return load(bmp);
}

bool Texture::load(const Bitmap& src)
{
  clear();
  if(src.getWidth() < 16) {
    return false;
  }

  glGenTextures(1, &m_tex_name);
  if(m_tex_name==0) {
    return false;
  }
  glBindTexture(GL_TEXTURE_2D, m_tex_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, m_type, src.getWidth(), src.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, src[0][0].v);
  CheckGLError();
  glBindTexture(GL_TEXTURE_2D, 0);

  return true;
}


void Texture::assign() const
{
  if(m_tex_name) {
    glBindTexture(GL_TEXTURE_2D, m_tex_name);
    CheckGLError();
    setParam();
  }
  else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void Texture::disassign() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::operator !()  const { return m_tex_name==0; }
GLuint Texture::getHandle() { return m_tex_name; }

void Texture::setType(int a)      { m_type=a; }
void Texture::setMagFilter(int a) { m_mag_filter=a; }
void Texture::setMinFilter(int a) { m_min_filter=a; }
void Texture::setWarpS(int a)     { m_warp_s=a; }
void Texture::setWarpT(int a)     { m_warp_t=a; }
void Texture::setEnvMode(int a)   { m_env_mode=a; }




/*
  PolygonModel
*/

PolygonChunc::PolygonChunc()
{}

PolygonChunc::~PolygonChunc()
{}

void PolygonChunc::setup() {
  BPolygonChunc::setup();

  m_vertex_array.resize(m_vvertex.size()*3+m_vnormal.size()*3+m_vtexcoord.size()*2);

  size_t offset = 0;
  for(size_t i=0; i<m_vvertex.size(); ++i) {
    m_vertex_array[i*3+0] = m_vvertex[i].x;
    m_vertex_array[i*3+1] = m_vvertex[i].y;
    m_vertex_array[i*3+2] = m_vvertex[i].z;
  }
  offset+=m_vvertex.size()*3;

  for(size_t i=0; i<m_vnormal.size(); ++i) {
    m_vertex_array[offset+i*3+0] = m_vnormal[i].x;
    m_vertex_array[offset+i*3+1] = m_vnormal[i].y;
    m_vertex_array[offset+i*3+2] = m_vnormal[i].z;
  }
  offset+=m_vnormal.size()*3;

  if(!m_vtexcoord.empty()) {
    for(size_t i=0; i<m_vtexcoord.size(); ++i) {
      m_vertex_array[offset+i*2+0] = m_vtexcoord[i].x;
      m_vertex_array[offset+i*2+1] = m_vtexcoord[i].y;
    }
  }
  else {
    for(size_t i=0; i<m_vtexcoord.size(); ++i) {
      m_vertex_array[offset+i*2+0] = 0;
      m_vertex_array[offset+i*2+1] = 0;
    }
  }
//  vvertex().swap(position_cont());
}


void PolygonChunc::draw() const {
  if(m_vertex_array.empty()) {
    return;
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  const float *vertex_pointer = &m_vertex_array[0];
  glVertexPointer(3, GL_FLOAT, 0, vertex_pointer);
  glNormalPointer(GL_FLOAT, 0, vertex_pointer+m_face_num*3*3 );
  glTexCoordPointer(2, GL_FLOAT, 0, vertex_pointer+m_face_num*3*6 );

  glDrawArrays(GL_TRIANGLES, 0, m_face_num*3);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}



VBOPolygonChunc::VBOPolygonChunc() : m_vbo_id(0)
{}

VBOPolygonChunc::~VBOPolygonChunc() {
  if(m_vbo_id) {
    glDeleteBuffersARB(1, &m_vbo_id);
  }
}
void VBOPolygonChunc::setup() {
  PolygonChunc::setup();

  glGenBuffersARB(1, &m_vbo_id);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo_id);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_index.size()*8*sizeof(float), &m_vertex_array[0], GL_STATIC_DRAW_ARB);
}


void VBOPolygonChunc::draw() const {
  if(!m_vbo_id) {
    return;
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo_id);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glNormalPointer(GL_FLOAT, 0, BUFFER_OFFSET(m_index.size()*3*sizeof(float)) );
  glTexCoordPointer(2, GL_FLOAT, 0, BUFFER_OFFSET(m_index.size()*6*sizeof(float)) );

  glDrawArrays(GL_TRIANGLES, 0, m_face_num*3);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}


void MorphModel::draw(iterator p, float t) const {
  if(p==m_vtx_clip.end() || p->second.empty()) {
    return;
  }

  static vertex_cont t_vertex;
  static vertex_cont vertex_array;
  p->second.morph(t, t_vertex);

  vertex_array.resize(m_idx_array.size()*3);
  for(size_t i=0; i<m_idx_array.size(); ++i) {
    vertex_array[i] = t_vertex[m_idx_array[i]];
  }

  glVertexPointer(3, GL_FLOAT, 0, vertex_array.front().v);
  glTexCoordPointer(2, GL_FLOAT, 0, m_tex_array.front().v);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glDrawArrays(GL_TRIANGLES, 0, m_idx_array.size());
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}


void CachedMorphModel::draw(iterator p, int t) const {
  if(p==m_vtx_clip.end()) {
    return;
  }

  const vertex_sequence& seq = p->second;
  if(seq.m_vertex.empty()) {
    return;
  }

  const vertex_cont *vertex_array;
  if(t<=seq.beginFrame()) {
    vertex_array = &seq.m_vertex.front();
  }
  else if(t>=seq.endFrame()) {
    vertex_array = &seq.m_vertex.back();
  }
  else {
    vertex_array = &seq.m_vertex[t-seq.beginFrame()];
  }

  if(m_enable_normal) {
    const normal_cont *normal_array;
    if(t<seq.beginFrame()) {
      normal_array = &seq.m_normal.front();
    }
    else if(t>=seq.endFrame()) {
      normal_array = &seq.m_normal.back();
    }
    else {
      normal_array = &seq.m_normal[t-seq.beginFrame()];
    }

    glVertexPointer(3, GL_FLOAT, 0, vertex_array->front().v);
    glNormalPointer(GL_FLOAT, 0, normal_array->front().v);
    glTexCoordPointer(2, GL_FLOAT, 0, m_tex_array.front().v);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, m_idx_array.size());
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
  else {
    glVertexPointer(3, GL_FLOAT, 0, vertex_array->front().v);
    glTexCoordPointer(2, GL_FLOAT, 0, m_tex_array.front().v);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, m_idx_array.size());
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
}




/*
  Kanji
*/

static void sjis2jis(unsigned char &c1, unsigned char &c2) {
  if (c1>=0xe0) { c1 = c1-0x40; }
  if (c2>=0x80) { c2 = c2-1; }
  if (c2>=0x9e) {
    c1 = (c1-0x70) * 2;
    c2 = c2-0x7d;
  } else {
    c1 = ((c1-0x70)*2)-1;
    c2 = c2-0x1f;
  }
}

static void euc2jis(unsigned char &c1, unsigned char &c2) {
  c1-=0x80;
  c2-=0x80;
}




Kanji::Kanji(int _code)
  : m_code(_code)
{
  init();
}

Kanji::Kanji(const std::string& filename, int _code)
  : m_code(_code)
{
  init();
  load(filename);
}

void Kanji::setCode(int _code) { m_code=_code; }
int Kanji::getFontSize() { return m_font_size; }

void Kanji::init()
{
  m_font_size = 0;
  m_char_byte = 0;
  m_max_index = 0;
}

void Kanji::print(int x, int y, const char *_str)
{
  if(_str==0 || m_data.empty()) {
    return;
  }

  const unsigned char *str = reinterpret_cast<const unsigned char*>(_str);

  MatrixSaver mmsaver(GL_MODELVIEW_MATRIX);  // 変換行列を退避
  MatrixSaver pmsaver(GL_PROJECTION_MATRIX);
  MatrixOp::ScreenMatrixIV();    // 座標系をスクリーンにあわせる

  glRasterPos2i(x, y+m_font_size);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  int down = 0;

  while(*str!=0) {
    if(*str=='\n') {
      down+=m_font_size+1;
      ++str;
      glRasterPos2i(x, y+m_font_size+down);
      continue;
    }
    if(isprint(*str)) {  // ascii文字の場合
      if(*str < m_max_index) {
        glBitmap(m_font_size, m_font_size, m_font_size/2, 0, m_font_size/2, 0, getChar(*str) );
      }
      ++str;
    }
    else {  // 日本語文字の場合
      unsigned char c1=str[0];
      unsigned char c2=str[1];

      switch(m_code) {
        case SJIS: sjis2jis(c1,c2); break;
        case EUC:  euc2jis (c1,c2); break;
      }

      int index = (c1-0x20)*96 + c2-0x20 +0xff;
      if(index<m_max_index && index>=0) {
        glBitmap(m_font_size, m_font_size, 0, 0, m_font_size, 0, getChar(index) );
      }
      str+=2;
    }
  }
}


bool Kanji::load(const std::string& filename)
{
  m_data.clear();
  m_font_size = 0;
  m_char_byte = 0;

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  biostream bf(f);
  if(!f) {
    return false;
  }

  bf >> m_font_size;

  int n = 0;
  while(n<m_font_size) { n+=8; }
  int char_bit = n*m_font_size;
  m_char_byte = char_bit/8;


  unsigned char length, temp;
  while(!f.eof()) {
    bf >> length;
    if(length<0x80) {
      for(int i=0; i<length+1; ++i) {
        bf >> temp;
        m_data.push_back(temp);
      }
    }
    else {
      bf >> temp;
      for(int i=0x80; i<length+1; ++i) {
        m_data.push_back(temp);
      }
    }
  }

  m_max_index = m_data.size()/m_char_byte;
  return true;
}

const unsigned char* Kanji::getChar(int index) { return &m_data[index*m_char_byte]; }



}// ist

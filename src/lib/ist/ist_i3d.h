#ifndef ist_i3d
#define ist_i3d

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include "ist_conf.h"
#include "vector.h"
#include "bitmap.h"
#include "bstream.h"
#include "gl_include.h"

#ifdef ENABLE_GL_ERRORCKECK
#define CheckGLError() ist::_CheckGLError()
#else
#define CheckGLError()
#endif


namespace ist {

/*
  Diagram
*/

class Circle
{
private:
  vector2 m_pos;
  float m_radius;

public:
  Circle();
  explicit Circle(const vector2& p, float r);
  float getRadius() const;
  const vector2& getPosition() const;
  void setRadius(float v);
  void setPosition(const vector2& v);
  bool isInner(const vector2& pos) const;
  float getVolume() const;

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class Rect
{
private:
  vector2 m_ur;
  vector2 m_bl;

public:
  Rect();
  explicit Rect(const vector2& ur);
  explicit Rect(const vector2& ur, const vector2& bl);
  const vector2& getUpperRight() const;
  const vector2& getBottomLeft() const;
  void setUpperRight(const vector2& v);
  void setBottomLeft(const vector2& v);
  float getWidth() const;
  float getHeight() const;
  vector2 getCenter() const;
  float getVolume() const;

  bool isInner(const vector2& pos) const;
  bool isInner(const Circle& circle) const;
  bool isInner(const Rect& rect) const;
  bool isOverlapped(const Rect& rect) const;

  Rect& operator+=(const vector2&);
  Rect operator+(const vector2&);
  Rect& operator-=(const vector2&);
  Rect operator-(const vector2&);
  Rect& operator+=(const Rect&);
  Rect operator+(const Rect&);
  Rect& operator-=(const Rect&);
  Rect operator-(const Rect&);

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class Box;
class Sphere
{
private:
  vector4 m_pos;
  float m_radius;

public:
  Sphere();
  explicit Sphere(const vector4& p, float r=0.0f);
  float getRadius() const;
  const vector4& getPosition() const;
  void setRadius(float v);
  void setPosition(const vector4& v);
  bool isInner(const vector4& pos) const;
  float getVolume() const;
  Box getBoundingBox() const;

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class Box
{
private:
  vector4 m_ur;
  vector4 m_bl;

public:
  Box();
  explicit Box(const vector4& ur);
  explicit Box(const vector4& ur, const vector4& bl);
  const vector4& getUpperRight() const;
  const vector4& getBottomLeft() const;
  void setUpperRight(const vector4& v);
  void setBottomLeft(const vector4& v);
  float getWidth() const;
  float getHeight() const;
  float getLength() const;
  vector4 getCenter() const;
  float getVolume() const;
  Sphere getBoundingSphere() const;

  Box getAABB(const matrix44& trans) const;
  bool isInner(const vector4& pos) const;
  bool isInner(const Sphere& sphere) const;
  bool isInner(const Box& box) const;
  bool isOverlapped(const Box& box) const;
  int innerTriangle(const vector4& v1, const vector4& v2, const vector4& v3) const;

  Box& operator+=(const vector4&);
  Box operator+(const vector4&);
  Box& operator-=(const vector4&);
  Box operator-(const vector4&);
  Box& operator+=(const Box&);
  Box operator+(const Box&);
  Box& operator-=(const Box&);
  Box operator-(const Box&);

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};




/*
  CollisionDetector
*/

enum {
  CM_FALSE,
  CM_TRUE,
  CM_POINT,
  CM_LINE,
  CM_BOX,
  CM_POLYGON,
  CM_POLYGON2D,
  CM_COMPOSITE,
};

class IST_CLASS ICollision : public Object
{
public:
  virtual ~ICollision()=0;
  virtual int getType() const=0;
  virtual const Box& getBoundingBox() const=0;
//  virtual const Sphere& getBoundingSphere() const=0;
};

class IST_CLASS IPointCollision : public ICollision
{
public:
  int getType() const { return CM_POINT; }
  virtual const Sphere& getSphere() const=0;
  virtual const vector4& getPosition() const=0;
  virtual float getRadius() const=0;
};

class IST_CLASS ILineCollision : public ICollision
{
public:
  int getType() const { return CM_LINE; }
  virtual float getRadius() const=0;
  virtual const vector4& getTail() const=0;
  virtual const vector4& getHead() const=0;
};

class IST_CLASS IBoxCollision : public ICollision
{
public:
  int getType() const { return CM_BOX; }
  virtual const Box& getBox() const=0;
  virtual const matrix44& getMatrix() const=0;
  virtual const matrix44& getInvertMatrix() const=0;
};

class IST_CLASS IPolygonCollision : public ICollision
{
public:
  typedef std::vector<vector4> vertex_cont;
  typedef std::vector<vector4> normal_cont;
  typedef std::vector<int>     index_cont;

  int getType() const { return CM_POLYGON; }
  virtual const vertex_cont& getVertexCont() const=0;
  virtual const normal_cont& getNormalCont() const=0;
  virtual const index_cont&  getIndexCont() const=0;
};


class IST_CLASS FalseCollision : public ICollision
{
public:
  virtual int getType() const { return CM_FALSE; }

  virtual const Box& getBoundingBox() const
  {
    static Box dummy;
    return dummy;
  }

  virtual const Sphere& getBoundingSphere() const
  {
    static Sphere dummy;
    return dummy;
  }
};

class IST_CLASS TrueCollision : public ICollision
{
public:
  virtual int getType() const { return CM_TRUE; }

  virtual const Box& getBoundingBox() const
  {
    static Box dummy;
    return dummy;
  }

  virtual const Sphere& getBoundingSphere() const
  {
    static Sphere dummy;
    return dummy;
  }
};


class IST_CLASS ICompositeCollision : public ICollision
{
protected:
  mutable Box m_aabb;

public:
  virtual size_t getCollisionCount() const=0;
  virtual const ICollision& getCollision(size_t i) const=0;
  virtual int getType() const { return CM_COMPOSITE; }

  virtual const Box& getBoundingBox() const
  {
    vector4 ur;
    vector4 bl;
    for(size_t i=0; i<getCollisionCount(); ++i) {
      const ICollision& c = getCollision(i);
      const Box& b = c.getBoundingBox();
      const vector4& tur = b.getUpperRight();
      const vector4& tbl = b.getBottomLeft();
      if(i==0) {
        ur = tur;
        bl = tbl;
      }
      else {
        bl.x = std::min<float>(bl.x, tbl.x);
        bl.y = std::min<float>(bl.y, tbl.y);
        bl.z = std::min<float>(bl.z, tbl.z);
        ur.x = std::max<float>(ur.x, tur.x);
        ur.y = std::max<float>(ur.y, tur.y);
        ur.z = std::max<float>(ur.z, tur.z);
      }
    }
    m_aabb = Box(ur, bl);
    return m_aabb;
  }
};


class IST_CLASS PointCollision : public IPointCollision
{
protected:
  Sphere m_sphere;
  mutable bool m_mod;
  mutable Box m_aabb;

public:
  PointCollision() : m_mod(false) {}
  PointCollision(const vector4& p, float r) : m_mod(false) { setPosition(p); setRadius(r); }

  const Sphere& getSphere() const    { return m_sphere; }
  const vector4& getPosition() const { return m_sphere.getPosition(); }
  float getRadius() const            { return m_sphere.getRadius(); }

  void setSphere(const Sphere& v)    { m_mod=true; m_sphere=v; }
  void setPosition(const vector4& v) { m_mod=true; m_sphere.setPosition(v); }
  void setRadius(float v)            { m_mod=true; m_sphere.setRadius(v); }

  virtual const Box& getBoundingBox() const
  {
    if(m_mod) {
      float r = getRadius();
      m_aabb.setUpperRight(getPosition()+vector4(r, r, r));
      m_aabb.setBottomLeft(getPosition()-vector4(r, r, r));
      m_mod = false;
    }
    return m_aabb;
  }

  virtual const Sphere& getBoundingSphere() const
  {
    return m_sphere;
  }

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class IST_CLASS LineCollision : public ILineCollision
{
protected:
  vector4 m_head;
  vector4 m_tail;
  float m_radius;
  mutable bool m_mod;
  mutable Box m_aabb;

public:
  LineCollision() : m_radius(0), m_mod(false) {}
  float getRadius() const { return m_radius; }
  const vector4& getHead() const { return m_head; }
  const vector4& getTail() const { return m_tail; }

  void setHead(const vector4& v) { m_head=v; m_mod=true; }
  void setTail(const vector4& v) { m_tail=v; m_mod=true; }
  void setRadius(float v)        { m_radius=v; m_mod=true; }

  virtual const Box& getBoundingBox() const
  {
    if(m_mod) {
      const vector4& h = getHead();
      const vector4& t = getTail();
      float r = getRadius();
      vector4 ur = h;
      vector4 bl = h;
      bl.x = std::min<float>(h.x-r, t.x-r);
      bl.y = std::min<float>(h.y-r, t.y-r);
      bl.z = std::min<float>(h.z-r, t.z-r);
      ur.x = std::max<float>(h.x+r, t.x+r);
      ur.y = std::max<float>(h.y+r, t.y+r);
      ur.z = std::max<float>(h.z+r, t.z+r);
      m_aabb.setBottomLeft(bl);
      m_aabb.setUpperRight(ur);
    }
    return m_aabb;
  }

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class IST_CLASS BoxCollision : public IBoxCollision
{
private:
  matrix44 m_mat;
  matrix44 m_imat;
  Box m_box;
  mutable bool m_mod;
  mutable Box m_aabb;

public:
  BoxCollision() : m_mod(false) {}
  void setBox(const Box& v) { m_mod=true; m_box=v; }
  void setUpperRight(const vector4& v) { m_mod=true; m_box.setUpperRight(v); }
  void setBottomLeft(const vector4& v) { m_mod=true; m_box.setBottomLeft(v); }

  void setMatrix(const matrix44& v)
  {
    m_mod = true;
    m_mat = v;
    m_imat = v;
    m_imat.invert();
  }

  void setMatrix(const matrix44& v, const matrix44& iv)
  {
    m_mod = true;
    m_mat = v;
    m_imat = iv;
  }

  virtual const Box& getBox() const { return m_box; }
  virtual const matrix44& getMatrix() const { return m_mat; }
  virtual const matrix44& getInvertMatrix() const { return m_imat; }

  virtual const Box& getBoundingBox() const
  {
    if(m_mod) {
      const vector4& mur = m_box.getUpperRight();
      const vector4& mbl = m_box.getBottomLeft();
      vector4 v[8] = {
        vector4(mbl.x, mur.y, mbl.z),
        vector4(mbl.x, mur.y, mur.z),
        vector4(mur.x, mur.y, mur.z),
        vector4(mur.x, mur.y, mbl.z),
        vector4(mbl.x, mbl.y, mbl.z),
        vector4(mbl.x, mbl.y, mur.z),
        vector4(mur.x, mbl.y, mur.z),
        vector4(mur.x, mbl.y, mbl.z),
      };

      vector4 ur;
      vector4 bl;
      for(int i=0; i<8; ++i) {
        v[i] = m_mat*v[i];
        if(i==0) {
          ur = bl = v[i];
          continue;
        }
        bl.x = std::min<float>(bl.x, v[i].x);
        bl.y = std::min<float>(bl.y, v[i].y);
        bl.z = std::min<float>(bl.z, v[i].z);
        ur.x = std::max<float>(ur.x, v[i].x);
        ur.y = std::max<float>(ur.y, v[i].y);
        ur.z = std::max<float>(ur.z, v[i].z);
      }
      m_aabb.setBottomLeft(bl);
      m_aabb.setUpperRight(ur);
      m_mod = false;
    }
    return m_aabb;
  }

  void serialize(bostream& b) const;
  void deserialize(bistream& b);
};

class IST_CLASS SimplePolygonCollision : public IPolygonCollision
{
protected:
  vertex_cont m_vertex;
  normal_cont m_normal;
  index_cont  m_index;
  Box m_aabb;

public:
  SimplePolygonCollision() {}
  const vertex_cont& getVertexCont() const { return m_vertex; }
  const normal_cont& getNormalCont() const { return m_normal; }
  const index_cont&  getIndexCont()  const { return m_index; }
  const Box& getBoundingBox() const  { return m_aabb; }

  void setBoundingBox()
  {
    vector4 ur = m_vertex.front();
    vector4 bl = m_vertex.front();
    for(vertex_cont::iterator p=m_vertex.begin(); p!=m_vertex.end(); ++p) {
      bl.x = std::min<float>(bl.x, p->x);
      bl.y = std::min<float>(bl.y, p->y);
      bl.z = std::min<float>(bl.z, p->z);
      ur.x = std::max<float>(ur.x, p->x);
      ur.y = std::max<float>(ur.y, p->y);
      ur.z = std::max<float>(ur.z, p->z);
      m_aabb.setBottomLeft(bl);
      m_aabb.setUpperRight(ur);
    }
  }
};



class IST_CLASS FrustumCollision : public SimplePolygonCollision
{
private:
  float m_znear;
  float m_zfar;

public:
  FrustumCollision() :
    m_znear(0),
    m_zfar(1500.0f)
  {
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

  void setZNear(float v) { m_znear=v; }
  void setZFar(float v)  { m_zfar=v; }

  void setup(const vector4& _0, const vector4& _1, const vector4& _2, const vector4& _3, const vector4& _4) {
    m_vertex[0] = _0;
    m_vertex[1] = _1;
    m_vertex[2] = _2;
    m_vertex[3] = _3;
    m_vertex[4] = _4;
    for(size_t i=0; i<m_index.size()/3; ++i) {
      m_normal[i] = vector4().cross(
        m_vertex[m_index[i*3+1]]-m_vertex[m_index[i*3]], m_vertex[m_index[i*3+2]]-m_vertex[m_index[i*3]]).normalize();
    }

    setBoundingBox();
  }

  bool test(const vector4& center, float r) const {
    vector4 rel_pos = center-m_vertex[0];
    float dist = m_normal[4].dot(rel_pos);

    if(  m_vertex[1]==m_vertex[2]
      || m_vertex[2]==m_vertex[3]
      || dist < m_znear-r
      || dist > m_zfar+r) {
      return false;
    }
    if(  m_normal[0].dot(rel_pos)>r
      || m_normal[2].dot(rel_pos)>r
      || m_normal[3].dot(rel_pos)>r
      || m_normal[1].dot(rel_pos)>r) {
      return false;
    }
    return true;
  }

  bool test(const ICollision& cm) const {
    if(cm.getType()==CM_TRUE) {
      return true;
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
};




class IST_CLASS CollisionDetector : public Object
{
private:
  template <class T, class U> bool detect_(const T &base, const U &target);

  bool m_result;
  float m_distance;
  vector4 m_pos;
  vector4 m_normal;

public:
  CollisionDetector();
  bool detect(const ICollision& base, const ICollision& target);

  bool getResult() const    { return m_result; }
  float getDistance() const { return m_distance; }
  const vector4& getPosition() const { return m_pos; }
  const vector4& getNormal() const   { return m_normal; }
};



void _CheckGLError();

/*
  GLInfo
*/

class IST_CLASS GLInfo
{
public:
  void print(std::ostream& out);
};



/*
  Matrix
*/

class IST_CLASS MatrixOp
{
public:
  static void ScreenMatrix();
  static void ScreenMatrixIV();
};


class IST_CLASS MatrixSaver : public Object
{
private:
  int m_matrix_mode;
  matrix44 m_matrix;

public:

// GL_MODELVIEW_MATRIX | GL_PROJECTION_MATRIX | GL_TEXTURE_MATRIX
  MatrixSaver(int matrix_type);
  ~MatrixSaver();
  const matrix44& getMatrix() const;
};

class IST_CLASS ModelviewMatrixSaver : public MatrixSaver
{
public:
  ModelviewMatrixSaver() : MatrixSaver(GL_MODELVIEW_MATRIX) {}
};

class IST_CLASS ProjectionMatrixSaver : public MatrixSaver
{
public:
  ProjectionMatrixSaver() : MatrixSaver(GL_PROJECTION_MATRIX) {}
};

class IST_CLASS TextureMatrixSaver : public MatrixSaver
{
public:
  TextureMatrixSaver() : MatrixSaver(GL_TEXTURE_MATRIX) {}
};


/*
  Material
*/

class IST_CLASS Material : public Object
{
private:
  vector4 m_ambient;
  vector4 m_diffuse;
  vector4 m_specular;
  vector4 m_emission;
  float m_shininess;

public:
  Material();
  virtual void assign() const;

  void setAmbient(const vector4& v)  { m_ambient=v; }
  void setDiffuse(const vector4& v)  { m_diffuse=v; }
  void setSpecular(const vector4& v) { m_specular=v; }
  void setEmission(const vector4& v) { m_emission=v; }
  void setShininess(float v) { m_shininess=v; }

  const vector4& getAmbient() const  { return m_ambient; }
  const vector4& getDiffuse() const  { return m_diffuse; }
  const vector4& getSpecular() const { return m_specular; }
  const vector4& getEmission() const { return m_emission; }
  float getShininess() const { return m_shininess; }

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};


class IST_CLASS Color : public Object
{
private:
  vector4 m_color;

public:
  Color();
  Color(float r, float g, float b, float a);
  const vector4& getColor() const { return m_color; }
  void setColor(const vector4& v) { m_color=v; }
  virtual void assign() const;

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};


/*
  Light
*/

class IST_CLASS Light : public Object
{
private:
  int m_lightid;
  vector4 m_position;
  vector4 m_ambient;
  vector4 m_diffuse;
  vector4 m_specular;
  float m_constant_att;
  float m_linear_att;
  float m_quadratic_att;
  bool m_enabled;

  Light(const Light&);
  Light& operator=(const Light&);

public:
  Light();
  ~Light();

  void enable();
  void disable();
  bool isEnabled() const;
  bool isValid() const;

  void setPosition(const vector4& v) { m_position=v; }
  void setAmbient(const vector4& v)  { m_ambient=v; }
  void setDiffuse(const vector4& v)  { m_diffuse=v; }
  void setSpecular(const vector4& v) { m_specular=v; }
  void setConstantAttenuation(float v)  { m_constant_att=v; }
  void setLinearAttenuation(float v)    { m_linear_att=v; }
  void setQuadraticAttenuation(float v) { m_quadratic_att=v; }

  const vector4& getPosition() const { return m_position; }
  const vector4& getAmbient() const  { return m_ambient; }
  const vector4& getDiffuse() const  { return m_diffuse; }
  const vector4& getSpecular() const { return m_specular; }
  float getConstantAttenuation() const  { return m_constant_att; }
  float getLinearAttenuation() const    { return m_linear_att; }
  float getQuadraticAttenuation() const { return m_quadratic_att; }

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};


/*
  Fog
*/

class IST_CLASS Fog : public Object
{
private:
  vector4 m_position;
  vector4 m_color;
  float m_near;
  float m_far;
  bool m_enabled;

public:
  Fog();
  ~Fog();

  void enable();
  void disable();

  const vector4& getColor() { return m_color; }
  float getNear() { return m_near; }
  float getFar() { return m_far; }

  void setColor(const vector4& v) { m_color=v; }
  void setNear(float v) { m_near=v; }
  void setFar(float v) { m_far=v; }

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};



/*
  Camera
*/


class IST_CLASS ViewFrustum : public SimplePolygonCollision
{
private:
  vector4 m_pos;
  vector3 m_front;
  vector3 m_top;
  vector3 m_bottom;
  vector3 m_right;
  vector3 m_left;
  float m_znear;
  float m_zfar;

public:
  ViewFrustum();
  void setup(const vector4& pos, const vector4& target4, const vector3& up, float znear, float zfar, float fovy, float aspect);
  int test(const vector4& center, float r) const;
  int test(const ICollision& cm) const;

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};


class IST_CLASS Camera : public Object
{
private:
  vector4 m_position;
  vector4 m_target;
  vector3 m_direction;
  float m_znear;
  float m_zfar;

public:
  Camera();
  virtual ~Camera();
  virtual void look()=0;

  virtual void setPosition(const vector4& v);
  virtual void setTarget(const vector4& v);
  virtual void setDirection(const vector3& v);
  virtual void setZNear(float v);
  virtual void setZFar(float v);

  const vector4& getPosition() const;
  const vector4& getTarget() const;
  const vector3& getDirection() const;
  float getZNear() const;
  float getZFar() const;

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};

class IST_CLASS OrthographicCamera : public Camera
{
typedef Camera Super;
private:
  float m_left;
  float m_right;
  float m_bottom;
  float m_top;

public:
  OrthographicCamera();
  virtual void look();
  virtual void setScreen(float l, float r, float b, float t);
  float getLeft() const;
  float getRight() const;
  float getBottom() const;
  float getTop() const;

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};

class IST_CLASS PerspectiveCamera : public Camera
{
typedef Camera Super;
private:
  float m_fovy;
  float m_aspect;
  mutable bool m_mod;
  mutable ViewFrustum m_frustum;

public:
  PerspectiveCamera();
  virtual void look();
  virtual const ViewFrustum& getViewFrustum() const;

  virtual void setPosition(const vector4& v);
  virtual void setTarget(const vector4& v);
  virtual void setDirection(const vector3& v);
  virtual void setZNear(float v);
  virtual void setZFar(float v);
  void setFovy(float v);
  void setAspect(float v);
  float getFovy() const;
  float getAspect() const;

  virtual void serialize(bostream& b) const;
  virtual void deserialize(bistream& b);
};







/*
  Texture
*/

inline size_t power_of_two(size_t size) {
  size_t i=1;
  while(i<size) {
    i*=2;
  }
  return i;
}

class IST_CLASS ITexture : public Object
{
public:
  virtual ~ITexture() {}

  virtual bool load(const std::string& filename) =0;
  virtual bool load(const Bitmap& src) =0;

  virtual void clear()=0;
  virtual void assign() const =0;
  virtual void disassign() const =0;
  virtual bool operator !()  const=0;
};

// ・ｽR・ｽs・ｽ[・ｽs・ｽ・ｽ 
class IST_CLASS Texture : public ITexture
{
private:
  int m_type;
  int m_warp_s;
  int m_warp_t;
  int m_mag_filter;
  int m_min_filter;
  int m_env_mode;

  GLuint m_tex_name;

  Texture(const Texture&);
  Texture& operator =(const Texture&);

  void setDefaultParam();
  void setParam() const;

public:
  Texture();
  Texture(const std::string& filename);
  Texture(const Bitmap& src);
  virtual ~Texture();

  virtual bool load(const std::string& filename);
  virtual bool load(const Bitmap& src);
  virtual void clear();
  virtual void assign() const;
  virtual void disassign() const;
  virtual bool operator !()  const;
  GLuint getHandle();

  void setType(int a);
  void setMagFilter(int a);
  void setMinFilter(int a);
  void setWarpS(int a);
  void setWarpT(int a);
  void setEnvMode(int a);
};


/*
  PolygonModel
*/

struct vertex_t
{
  vector4 position;
  size_t bone_id[8];
  float weight[8];

  vertex_t() {
    for(size_t i=0; i<8; ++i) {
      bone_id[i] = 0;
      if(i==0) {
        weight[i] = 100.0f;
      }
      else {
        weight[i] = 0.0f;
      }
    }
  }
};


class IST_CLASS IPolygonChunc : public Object
{
public:
  typedef std::vector<vertex_t> vertex_cont;
  typedef std::vector<vector3>  position_cont;
  typedef std::vector<vector3>  normal_cont;
  typedef std::vector<vector2>  texcoord_cont;
  typedef std::vector<matrix44> bone_cont;
  typedef std::vector<size_t>   index_cont;

  virtual void draw() const=0;
  virtual const vertex_cont& getVertex() const=0;
  virtual const normal_cont& getNormal() const=0;
  virtual const index_cont&  getIndex() const=0;
  virtual const Sphere& getBSphere() const=0;
  virtual const Box&  getBBox() const=0;
};

class IST_CLASS IPolygonModel : public Object
{
public:
  typedef boost::intrusive_ptr<IPolygonChunc> chunc_ptr;

  virtual bool load(const std::string& filename)=0;
  virtual void draw() const=0;

  virtual size_t getNumChunc()=0;
  virtual chunc_ptr getChunc(int i)=0;
  virtual chunc_ptr getChunc(const std::string& chuncname)=0;
};







class IST_CLASS OBJLoader : public Object
{
public:
  typedef std::vector<vector3> position_cont;
  typedef std::vector<vector3> normal_cont;
  typedef std::vector<vector2> texcoord_cont;
  typedef std::vector<size_t>  index_cont;

  struct chunc
  {
    std::string m_label;
    index_cont m_vindex;
    index_cont m_tindex;
    index_cont m_nindex;
    float m_facet;

    chunc() : m_facet(60.0f) {}
  };
  typedef std::list<chunc> chunc_cont;

  chunc_cont    m_chuncs;
  position_cont m_vertex;
  normal_cont   m_normal;
  texcoord_cont m_texcoord;
  int m_status;
  std::iostream& m_in;

  OBJLoader(std::iostream& ios);
  void assignVertexArray(chunc& c, position_cont& varray, index_cont& vindex);
  int loadVertex(char *buf);
  void loadFace(char *buf);
  int loadIndex(char *buf, index_cont& vindex, index_cont& tindex, index_cont& nindex);
};

class IST_CLASS MQOLoader : public Object
{
public:
  typedef std::vector<vector3> position_cont;
  typedef std::vector<vector2> texcoord_cont;
  typedef std::vector<size_t>  index_cont;

  struct chunc
  {
    std::string   m_label;
    position_cont m_vertex;
    texcoord_cont m_vtexcoord;
    index_cont    m_index;
    float m_facet;

    chunc() : m_facet(60.0f) {}
  };
  typedef std::list<chunc> chunc_cont;

  chunc_cont m_chuncs;
  std::iostream& m_in;

  MQOLoader(std::iostream& ios);
  bool loadFace(char *buf);
  bool loadVertex(char *buf);
  bool loadAttrib(char *buf);
};


class IST_CLASS BPolygonChunc : public IPolygonChunc
{
public:
  BPolygonChunc();
  virtual ~BPolygonChunc();

  virtual void clear();
  virtual void draw() const;

  virtual void setup();
  virtual void setNormal();
  virtual void setBoundingVolume();

  virtual const vertex_cont& getVertex() const { return m_vertex; }
  virtual const normal_cont& getNormal() const { return m_normal; }
  virtual const index_cont&  getIndex() const { return m_index; }
  virtual const Sphere& getBSphere() const { return m_bsphere; }
  virtual const Box&  getBBox() const { return m_aabb; }


  index_cont  m_index;
  vertex_cont m_vertex;
  bone_cont   m_bone;

  position_cont m_vvertex;
  normal_cont   m_normal;
  normal_cont   m_vnormal;
  texcoord_cont m_vtexcoord;
  Box         m_aabb;
  Sphere        m_bsphere;
  size_t m_face_num;
  float m_facet;
};


class IST_CLASS BPolygonModel : public IPolygonModel
{
private:
  typedef boost::intrusive_ptr<BPolygonChunc> cchunc_ptr;
  typedef std::list<cchunc_ptr> chunc_cont;
  typedef std::list<std::string> label_cont;
  chunc_cont m_chuncs;
  label_cont m_labels;

public:

  BPolygonModel() {}
  BPolygonModel(const std::string& filename) { load(filename); }

  virtual ~BPolygonModel() {}
  virtual bool load(const std::string& filename);
  virtual void draw() const;
  virtual size_t getNumChunc() { return m_chuncs.size(); }
  virtual chunc_ptr getChunc(int i);
  virtual chunc_ptr getChunc(const std::string& chuncname);

  virtual void addChunc(cchunc_ptr p, const std::string& l) { m_chuncs.push_back(p); m_labels.push_back(l); }
  virtual void setup();

  virtual BPolygonChunc* createChunc() { return new BPolygonChunc(); }

protected:
  bool loadMQO(const std::string& filename);
  bool loadOBJ(const std::string& filename);
};



template<class T>
class TPolygonModel : public BPolygonModel
{
public:
  typedef T chunc_type;

  TPolygonModel() {}
  TPolygonModel(const std::string& filename) { load(filename); }

  virtual chunc_type* createChunc() { return new chunc_type(); }
};




class IST_CLASS PolygonChunc : public BPolygonChunc
{
public:
  typedef std::vector<float> vertex_array_cont;
  vertex_array_cont m_vertex_array;

  PolygonChunc();
  virtual ~PolygonChunc();

  virtual void draw() const;
  virtual void setup();
};


class IST_CLASS VBOPolygonChunc : public PolygonChunc
{
private:
  unsigned int m_vbo_id;

  VBOPolygonChunc(const VBOPolygonChunc&);
  const VBOPolygonChunc& operator=(const VBOPolygonChunc&);

public:
  VBOPolygonChunc();
  virtual ~VBOPolygonChunc();

  virtual void draw() const;
  virtual void setup();
};

typedef TPolygonModel<PolygonChunc> PolygonModel;
typedef TPolygonModel<VBOPolygonChunc> VBOPolygonModel;





/*
  MorphModel
*/

class IST_CLASS IMorphModel : public Object
{
public:
  virtual ~IMorphModel() {}
  virtual bool load(const std::string& filename)=0;
  virtual void draw(const std::string& clipname, float t) const=0;

  virtual bool isExistClip(const std::string& clipname) const=0;
  virtual float beginFrame(const std::string& clipname) const=0;
  virtual float endFrame(const std::string& clipname) const=0;
  virtual const Sphere& getBSphere() const=0;
  virtual const Box&  getBBox() const=0;
};

class IST_CLASS BMorphModel : public IMorphModel
{
public:
  typedef std::vector<vector3> vertex_cont;
  typedef std::vector<vector2> texcoord_cont;
  typedef std::vector<size_t>  index_cont;

  class vertex_sequence : public std::map<float, vertex_cont>
  {
  public:
    typedef std::pair<const_iterator, const_iterator> range_t;

    float beginFrame() const {
      if(empty()) {
        return 0;
      }
      return begin()->first;
    }

    float endFrame() const {
      if(empty()) {
        return 0;
      }
      return (--end())->first;
    }

    bool changeFrame(float before, float after) {
      iterator p = find(before);
      if(p==end()) {
        return false;
      }

      (*this)[after] = p->second;
      erase(p);
      return true;
    }

    range_t range(float f) const {
      return equal_range(f);
    }
    void morph(float t, vertex_cont& t_vertex) const;
  };

  typedef std::map<std::string, vertex_sequence> vertex_clip;
  typedef vertex_clip::const_iterator iterator;

public:
  Box m_aabb;
  Sphere m_bsphere;
  vertex_clip   m_vtx_clip;
  texcoord_cont m_tex_array;
  index_cont    m_idx_array;
  unsigned int  m_num_vertex;

public:
  BMorphModel();
  BMorphModel(const std::string& filename) { load(filename); }
  virtual ~BMorphModel();
  virtual void draw(const std::string& clipname, float t) const;
  virtual void draw(iterator p, float t) const;
  virtual void setup();
  virtual bool load(const std::string& filename);
  virtual bool save(const std::string& filename);
  virtual void clear();

  iterator find(const std::string& clipname) { return m_vtx_clip.find(clipname); }
  iterator begin() { return m_vtx_clip.begin(); }
  iterator end()   { return m_vtx_clip.end(); }

  float beginFrame(const std::string& clipname) const {
    vertex_clip::const_iterator p = m_vtx_clip.find(clipname);
    if(p==m_vtx_clip.end()) {
      return 0;
    }
    return p->second.beginFrame();
  }

  float endFrame(const std::string& clipname) const {
    vertex_clip::const_iterator p = m_vtx_clip.find(clipname);
    if(p==m_vtx_clip.end()) {
      return 0;
    }
    return p->second.endFrame();
  }

  bool isExistClip(const std::string& clipname) const { return m_vtx_clip.find(clipname)!=m_vtx_clip.end(); }
  const Sphere& getBSphere() const{ return m_bsphere; }
  const Box&  getBBox() const   { return m_aabb; }
};



class IST_CLASS ICachedMorphModel : public Object
{
public:
  virtual ~ICachedMorphModel() {}
  virtual bool load(const std::string& filename)=0;
  virtual void draw(const std::string& clipname, int t) const=0;

  virtual bool isExistClip(const std::string& clipname) const=0;
  virtual int beginFrame(const std::string& clipname) const=0;
  virtual int endFrame(const std::string& clipname) const=0;
  virtual const Sphere& getBSphere() const=0;
  virtual const Box& getBBox() const=0;
};

class IST_CLASS BCachedMorphModel : public ICachedMorphModel
{
public:
  typedef std::vector<vector3> vertex_cont;
  typedef std::vector<vector3> normal_cont;
  typedef std::vector<vector2> texcoord_cont;
  typedef std::vector<size_t>  index_cont;

  class vertex_sequence
  {
  public:
    int m_begin_frame;
    int m_end_frame;
    std::vector<vertex_cont> m_vertex;
    std::vector<normal_cont> m_normal;

    vertex_sequence() : m_begin_frame(0), m_end_frame(0) {}
    bool empty() const     { return m_vertex.empty() || m_normal.empty(); }
    int beginFrame() const { return m_begin_frame; }
    int endFrame() const   { return m_end_frame; }
  };

  typedef std::map<std::string, vertex_sequence> vertex_clip;
  typedef vertex_clip::const_iterator iterator;

public:
  Box m_aabb;
  Sphere m_bsphere;
  vertex_clip   m_vtx_clip;
  texcoord_cont m_tex_array;
  index_cont    m_idx_array;
  unsigned int  m_num_vertex;
  float m_facet;
  bool m_enable_normal;

public:
  BCachedMorphModel();
  BCachedMorphModel(const std::string& filename, bool en=true);
  virtual ~BCachedMorphModel();
  virtual void draw(const std::string& clipname, int t) const;
  virtual void draw(iterator p, int t) const;

  virtual void setup();
  virtual bool load(const std::string& filename);
  virtual void setVertex(vertex_cont& vtx_array, vertex_cont& vvtx_array);
  virtual void setNormal(vertex_cont& vtx_array, normal_cont& vnml_array);
  virtual void clear();
  virtual void enableNormal(bool en) { m_enable_normal = en; }

  iterator find(const std::string& clipname) { return m_vtx_clip.find(clipname); }
  iterator begin() { return m_vtx_clip.begin(); }
  iterator end()   { return m_vtx_clip.end(); }

  int beginFrame(const std::string& clipname) const {
    vertex_clip::const_iterator p = m_vtx_clip.find(clipname);
    if(p==m_vtx_clip.end()) {
      return 0;
    }
    return p->second.beginFrame();
  }

  int endFrame(const std::string& clipname) const {
    vertex_clip::const_iterator p = m_vtx_clip.find(clipname);
    if(p==m_vtx_clip.end()) {
      return 0;
    }
    return p->second.endFrame();
  }

  bool isExistClip(const std::string& clipname) const { return m_vtx_clip.find(clipname)!=m_vtx_clip.end(); }
  const Sphere& getBSphere() const { return m_bsphere; }
  const Box&  getBBox() const { return m_aabb; }
};




class IST_CLASS MorphModel : public BMorphModel
{
public:
  MorphModel() {}
  MorphModel(const std::string& filename) { load(filename); }
  virtual void draw(const std::string& clipname, float t) const { BMorphModel::draw(clipname, t); }
  virtual void draw(iterator p, float t) const;
};

/*
class IST_CLASS VSHMorphModel : public BMorphModel
{
public:
  VSHMorphModel() {}
  VSHMorphModel(const std::string& filename) { load(filename); }
  void draw(iterator p, float t) const;
};
*/


class IST_CLASS CachedMorphModel : public BCachedMorphModel
{
public:
  CachedMorphModel() {}
  CachedMorphModel(const std::string& filename, bool en=true) { enableNormal(en); load(filename); }
  virtual void draw(const std::string& clipname, int t) const { BCachedMorphModel::draw(clipname, t); }
  virtual void draw(iterator p, int t) const;
};



/*
  Kanji
*/

class IST_CLASS Kanji : public Object
{
public:
  enum { SJIS, EUC, JIS };

  Kanji(int _code=SJIS);
  Kanji(const std::string& filename, int _code=SJIS);

  void print(int x, int y, const char *str);

  void setCode(int _code);
  int getFontSize();
  bool load(const std::string& filename);

protected:
  void init();
  const unsigned char* getChar(int index);

  int m_font_size;
  int m_char_byte;
  int m_max_index;
  int m_code;
  std::vector<unsigned char> m_data;

};

}  // ist



inline ist::bostream& operator<<(ist::bostream& b, const ist::Circle& v) { v.serialize(b); return b; }
inline ist::bistream& operator>>(ist::bistream& b, ist::Circle& v)       { v.deserialize(b); return b; }
inline ist::bostream& operator<<(ist::bostream& b, const ist::Rect& v)   { v.serialize(b); return b; }
inline ist::bistream& operator>>(ist::bistream& b, ist::Rect& v)         { v.deserialize(b); return b; }
inline ist::bostream& operator<<(ist::bostream& b, const ist::Sphere& v) { v.serialize(b); return b; }
inline ist::bistream& operator>>(ist::bistream& b, ist::Sphere& v)       { v.deserialize(b); return b; }
inline ist::bostream& operator<<(ist::bostream& b, const ist::Box& v)    { v.serialize(b); return b; }
inline ist::bistream& operator>>(ist::bistream& b, ist::Box& v)          { v.deserialize(b); return b; }

inline ist::bostream& operator<<(ist::bostream& b, const ist::PointCollision& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::PointCollision& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::LineCollision& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::LineCollision& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::BoxCollision& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::BoxCollision& v)
{
  v.deserialize(b);
  return b;
}


inline ist::bostream& operator<<(ist::bostream& b, const ist::Material& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::Material& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::Color& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::Color& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::Light& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::Light& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::Fog& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::Fog& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::ViewFrustum& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::ViewFrustum& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::Camera& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::Camera& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::OrthographicCamera& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::OrthographicCamera& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::PerspectiveCamera& v)
{
  v.serialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::PerspectiveCamera& v)
{
  v.deserialize(b);
  return b;
}


#endif // ist_i3d 

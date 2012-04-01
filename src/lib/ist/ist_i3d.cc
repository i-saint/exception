#include <stdexcept>
#include "ist_i3d.h"


namespace ist {


void Circle::serialize(bostream& b) const { b << m_pos << m_radius; }
void Circle::deserialize(bistream& b)     { b >> m_pos >> m_radius; }

void Rect::serialize(bostream& b) const   { b << m_ur << m_bl; }
void Rect::deserialize(bistream& b)       { b >> m_ur >> m_bl; }

void Sphere::serialize(bostream& b) const { b << m_pos << m_radius; }
void Sphere::deserialize(bistream& b)     { b >> m_pos >> m_radius; }

void Box::serialize(bostream& b) const    { b << m_ur << m_bl; }
void Box::deserialize(bistream& b)        { b >> m_ur >> m_bl; }

void PointCollision::serialize(bostream& b) const
{
  b << m_sphere << m_mod << m_aabb;
}
void PointCollision::deserialize(bistream& b)
{
  b >> m_sphere >> m_mod >> m_aabb;
}

void LineCollision::serialize(bostream& b) const
{
  b << m_head << m_tail << m_radius << m_mod << m_aabb;
}
void LineCollision::deserialize(bistream& b)
{
  b >> m_head >> m_tail >> m_radius >> m_mod >> m_aabb;
}

void BoxCollision::serialize(bostream& b) const
{
  b << m_mat << m_imat << m_box << m_mod << m_aabb;
}
void BoxCollision::deserialize(bistream& b)
{
  b >> m_mat >> m_imat >> m_box >> m_mod >> m_aabb;
}



/*
  Diagram
*/

Circle::Circle() : m_radius(0) {}
Circle::Circle(const vector2& p, float r) : m_pos(p), m_radius(r) {}
float Circle::getRadius() const            { return m_radius; }
const vector2& Circle::getPosition() const { return m_pos; }
void Circle::setRadius(float v)            { m_radius=v; }
void Circle::setPosition(const vector2& v) { m_pos=v; }
float Circle::getVolume() const            { return m_radius*m_radius*pi; }
bool Circle::isInner(const vector2& pos) const
{
  return (pos-m_pos).norm()<=m_radius;
}



Rect::Rect() {}

Rect::Rect(const vector2& ur)
{
  m_ur.x = std::max<float>(ur.x, -ur.x);
  m_bl.x = std::min<float>(ur.x, -ur.x);
  m_ur.y = std::max<float>(ur.y, -ur.y);
  m_bl.y = std::min<float>(ur.y, -ur.y);
}

Rect::Rect(const vector2& ur, const vector2 &bl)
{
  m_ur.x = std::max<float>(ur.x, bl.x);
  m_bl.x = std::min<float>(ur.x, bl.x);
  m_ur.y = std::max<float>(ur.y, bl.y);
  m_bl.y = std::min<float>(ur.y, bl.y);
}

const vector2& Rect::getUpperRight() const { return m_ur; }
const vector2& Rect::getBottomLeft() const { return m_bl; }
void Rect::setUpperRight(const vector2& v) { m_ur=v; }
void Rect::setBottomLeft(const vector2& v) { m_bl=v; }

float Rect::getWidth() const  { return m_ur.x-m_bl.x;}
float Rect::getHeight() const { return m_ur.y-m_bl.y;}
vector2 Rect::getCenter() const { return m_bl+((m_ur-m_bl)/2.0f); }
float Rect::getVolume() const { return getWidth()*getHeight(); }

bool Rect::isInner(const vector2& pos) const
{
  const vector2& ur = getUpperRight();
  const vector2& bl = getBottomLeft();
  if(  pos.x < ur.x
    && pos.x > bl.x
    && pos.y < ur.y
    && pos.y > bl.y)
  {
    return true;
  }
  else {
    return false;
  }
}

bool Rect::isInner(const Circle& circle) const
{
  float r = circle.getRadius();
  vector2 ur = getUpperRight()+vector2(r,r);
  vector2 bl = getBottomLeft()-vector2(r,r);
  const vector2& pos = circle.getPosition();
  if(  pos.x < ur.x
    && pos.x > bl.x
    && pos.y < ur.y
    && pos.y > bl.y)
  {
    return true;
  }
  else {
    return false;
  }
}

bool Rect::isInner(const Rect& rect) const
{
  return isInner(rect.getUpperRight()) && isInner(rect.getBottomLeft());
}

bool Rect::isOverlapped(const Rect& bb) const
{
  const vector2& ur = getUpperRight();
  const vector2& bl = getBottomLeft();
  const vector2& aur = bb.getUpperRight();
  const vector2& abl = bb.getBottomLeft();
  if(  (ur.x < abl.x || bl.x < aur.x)
    && (ur.y < abl.y || bl.y < aur.y) )
  {
    return true;
  }
  else {
    return false;
  }
}

Rect& Rect::operator+=(const vector2& v) { *this=*this+v; return *this; }
Rect Rect::operator+(const vector2& v)   { return Rect(m_ur+v, m_bl+v); }
Rect& Rect::operator-=(const vector2& v) { *this=*this-v; return *this; }
Rect Rect::operator-(const vector2& v)   { return Rect(m_ur-v, m_bl-v); }

Rect& Rect::operator+=(const Rect& v) { *this=*this+v; return *this; }
Rect Rect::operator+(const Rect& v)   { return Rect(m_ur+v.getUpperRight(), m_bl+v.getBottomLeft()); }
Rect& Rect::operator-=(const Rect& v) { *this=*this-v; return *this; }
Rect Rect::operator-(const Rect& v)   { return Rect(m_ur-v.getUpperRight(), m_bl-v.getBottomLeft()); }


Sphere::Sphere() : m_radius(0) {}
Sphere::Sphere(const vector4& p, float r) : m_pos(p), m_radius(r) {}
float Sphere::getRadius() const            { return m_radius; }
const vector4& Sphere::getPosition() const { return m_pos; }
void Sphere::setRadius(float v)            { m_radius=v; }
void Sphere::setPosition(const vector4& v) { m_pos=v; }
float Sphere::getVolume() const            { return (4.0f/3.0f)*m_radius*m_radius*m_radius*pi; }
bool Sphere::isInner(const vector4& pos) const
{
  return (pos-m_pos).norm()<=m_radius;
}

Box Sphere::getBoundingBox() const
{
  vector4 r(m_radius, m_radius, m_radius);
  return Box(m_pos+r, m_pos-r);
}


Box::Box() {}

Box::Box(const vector4& ur)
{
  m_ur.x = std::max<float>(ur.x, -ur.x);
  m_bl.x = std::min<float>(ur.x, -ur.x);
  m_ur.y = std::max<float>(ur.y, -ur.y);
  m_bl.y = std::min<float>(ur.y, -ur.y);
  m_ur.z = std::max<float>(ur.z, -ur.z);
  m_bl.z = std::min<float>(ur.z, -ur.z);
}

Box::Box(const vector4& ur, const vector4 &bl)
{
  m_ur.x = std::max<float>(ur.x, bl.x);
  m_bl.x = std::min<float>(ur.x, bl.x);
  m_ur.y = std::max<float>(ur.y, bl.y);
  m_bl.y = std::min<float>(ur.y, bl.y);
  m_ur.z = std::max<float>(ur.z, bl.z);
  m_bl.z = std::min<float>(ur.z, bl.z);
}

const vector4& Box::getUpperRight() const { return m_ur; }
const vector4& Box::getBottomLeft() const { return m_bl; }
void Box::setUpperRight(const vector4& v) { m_ur=v; }
void Box::setBottomLeft(const vector4& v) { m_bl=v; }

float Box::getWidth() const  { return m_ur.x-m_bl.x;}
float Box::getHeight() const { return m_ur.y-m_bl.y;}
float Box::getLength() const { return m_ur.z-m_bl.z;}
vector4 Box::getCenter() const { return m_bl+((m_ur-m_bl)/2.0f); }
float Box::getVolume() const { return getWidth()*getHeight()*getLength(); }

Sphere Box::getBoundingSphere() const
{
  vector4 center = getCenter();
  return Sphere(center, (m_ur-center).norm());
}

Box Box::getAABB(const matrix44 &trans) const
{
  vector4 ur = getUpperRight();
  vector4 bl = getBottomLeft();
  const vector4 vertex[8] = {
    vector4(bl.x, ur.y, bl.z),
    vector4(bl.x, ur.y, ur.z),
    vector4(ur.x, ur.y, ur.z),
    vector4(ur.x, ur.y, bl.z),
    vector4(bl.x, bl.y, bl.z),
    vector4(bl.x, bl.y, ur.z),
    vector4(ur.x, bl.y, ur.z),
    vector4(ur.x, bl.y, bl.z),
  };

  for(int i=0; i<8; ++i) {
    vector4 pos = trans*vertex[i];
    if(i==0) {
      ur = pos;
      bl = pos;
      continue;
    }

    if     (pos.x > ur.x) ur.x=pos.x;
    else if(pos.x < bl.x) bl.x=pos.x;
    if     (pos.y > ur.y) ur.y=pos.y;
    else if(pos.y < bl.y) bl.y=pos.y;
    if     (pos.z > ur.z) ur.z=pos.z;
    else if(pos.z < bl.z) bl.z=pos.z;
  }

  return Box(ur, bl);
}

bool Box::isInner(const vector4& pos) const
{
  const vector4& ur = getUpperRight();
  const vector4& bl = getBottomLeft();
  if(  pos.x < ur.x
    && pos.x > bl.x
    && pos.y < ur.y
    && pos.y > bl.y
    && pos.z < ur.z
    && pos.z > bl.z)
  {
    return true;
  }
  else {
    return false;
  }
}

bool Box::isInner(const Sphere& sphere) const
{
  float r = sphere.getRadius();
  vector4 ur = getUpperRight()+vector4(r,r,r);
  vector4 bl = getBottomLeft()-vector4(r,r,r);
  const vector4& pos = sphere.getPosition();
  if(  pos.x < ur.x
    && pos.x > bl.x
    && pos.y < ur.y
    && pos.y > bl.y
    && pos.z < ur.z
    && pos.z > bl.z)
  {
    return true;
  }
  else {
    return false;
  }
}

bool Box::isInner(const Box& box) const
{
  return isInner(box.getUpperRight()) && isInner(box.getBottomLeft());
}

int Box::innerTriangle(const vector4& v1, const vector4& v2, const vector4& v3) const
{
  const vector4& ur = getUpperRight();
  const vector4& bl = getBottomLeft();
  int result = 0;

  if(  bl.x < v1.x && ur.x > v1.x
    && bl.y < v1.y && ur.y > v1.y
    && bl.z < v1.z && ur.z > v1.z ) ++result;

  if(  bl.x < v2.x && ur.x > v2.x
    && bl.y < v2.y && ur.y > v2.y
    && bl.z < v2.z && ur.z > v2.z ) ++result;

  if(  bl.x < v3.x && ur.x > v3.x
    && bl.y < v3.y && ur.y > v3.y
    && bl.z < v3.z && ur.z > v3.z ) ++result;

  return result;
}

bool Box::isOverlapped(const Box& bb) const
{
  const vector4& ur = getUpperRight();
  const vector4& bl = getBottomLeft();
  const vector4& aur = bb.getUpperRight();
  const vector4& abl = bb.getBottomLeft();
  if(  (ur.x < abl.x || bl.x < aur.x)
    && (ur.y < abl.y || bl.y < aur.y)
    && (ur.z < abl.z || bl.z < aur.z) )
  {
    return true;
  }
  else {
    return false;
  }
}

Box& Box::operator+=(const vector4& v) { *this=*this+v; return *this; }
Box Box::operator+(const vector4& v)   { return Box(m_ur+v, m_bl+v); }
Box& Box::operator-=(const vector4& v) { *this=*this-v; return *this; }
Box Box::operator-(const vector4& v)   { return Box(m_ur-v, m_bl-v); }

Box& Box::operator+=(const Box& v) { *this=*this+v; return *this; }
Box Box::operator+(const Box& v)   { return Box(m_ur+v.getUpperRight(), m_bl+v.getBottomLeft()); }
Box& Box::operator-=(const Box& v) { *this=*this-v; return *this; }
Box Box::operator-(const Box& v)   { return Box(m_ur-v.getUpperRight(), m_bl-v.getBottomLeft()); }




/*
  CollisionDetector
*/


ICollision::~ICollision() {}

CollisionDetector::CollisionDetector() :
  m_result(false), m_distance(0)
{}

//ポリゴン-ポリゴン
template <>
bool CollisionDetector::detect_<IPolygonCollision, IPolygonCollision>
  (const IPolygonCollision& base, const IPolygonCollision& target)
{
  throw std::runtime_error("not implemented");

/*  if(base.chunc==0 || target.chunc==0
  || !base.bb.isOverlapped(target.bb) )
    return false;

  const BasePolygonChunc::line_cont &line_array = base.chunc->line_array;
  for(BasePolygonChunc::line_cont::const_iterator p=line_array.begin(); p!=line_array.end(); ++p) {
    const vector4& head = base.t_vertex[ p->getIndex(0) ];
    const vector4& tail = base.t_vertex[ p->getIndex(1) ];

    for(unsigned int i=0; i<target.chunc->getFaceNum(); ++i) {
      const vector4& normal = target.t_normal[i];
      const vector4 *vtx[3] = {
        &target.t_vertex[ target.chunc->idx_array[i*3+0] ],
        &target.t_vertex[ target.chunc->idx_array[i*3+1] ],
        &target.t_vertex[ target.chunc->idx_array[i*3+2] ],
      };
    
      float dist_tri = normal.dot( *vtx[0] );
      float dist_tail= normal.dot(tail);
      if( (normal.dot( head )-dist_tri)*(dist_tail-dist_tri)>0)
        continue;

      vector4 direction=(head-tail).normalize();
      float t=-(normal.dot(tail)-normal.dot(*vtx[0]))/normal.dot(direction);
      vector4 proj_pos=tail+direction*t;

      float inside=0;
      for(unsigned int j=0; j<3; ++j)
        inside+=(*vtx[j]-proj_pos).cos( *vtx[(j+1)%3]-proj_pos );

      if( inside>358.0f*radian ) {
        distance = base.chunc->bounding_sphere.radius;
        m_pos=proj_pos;
        m_normal=normal;
        return true;
      }
    }
  }
  
  return false;
*/
}

//ポリゴン-点
template <>
bool CollisionDetector::detect_<IPolygonCollision, IPointCollision>
  (const IPolygonCollision& base, const IPointCollision& target)
{
  const IPolygonCollision::vertex_cont &b_vtx = base.getVertexCont();
  const IPolygonCollision::normal_cont &b_nml = base.getNormalCont();
  const IPolygonCollision::index_cont  &b_idx = base.getIndexCont();
  const vector4& t_pos = target.getPosition();
  float t_radius = target.getRadius();

  if(!base.getBoundingBox().isInner(target.getSphere())) {
    return false;
  }


  bool is_inside = false;
  float min_distance = 0;
  vector4 last_pos;
  vector4 last_normal;

  for(unsigned int i=0; i<(b_idx.size()/3); ++i) {
    const int *triangle_index = &b_idx[i*3];
    const vector4& normal = b_nml[i];
    const vector4 *vtx[3] = {
      &b_vtx[ triangle_index[0] ],
      &b_vtx[ triangle_index[1] ],
      &b_vtx[ triangle_index[2] ],
    };

    for(int i=0; i<3; ++i) { // 頂点と衝突しているか
      float d = (*vtx[i]-t_pos).norm();
      if(d < t_radius) {
        m_distance = d;
        m_pos = *vtx[i];
        m_normal = normal;
        return true;
      }
    }

    float dist1 = normal.dot( t_pos );  //原点-点間距離
    float dist2 = normal.dot( *vtx[0] );    //原点-面間距離
    float dist3 = dist1-dist2;        //面-点間距離、これが負だったら面の反対側にいる

    if( fabsf(dist3)<fabsf(min_distance) || !is_inside ) {  //点に一番近い面を探す
      vector4 proj_pos = t_pos-normal*dist3;      //面に投影された点の位置

      float inside = 0;
      for(unsigned int j=0; j<3; ++j) {    //三角形との内外判定。各頂点と成す角が360°なら内側にいるはず。
        inside+=(*vtx[j]-proj_pos).cos( *vtx[(j+1)%3]-proj_pos );
      }

      if(inside>358.0f*radian) {  //2°の誤差を認める
        is_inside = true;
        min_distance = dist3;

        last_pos = proj_pos;
        last_normal = normal;
      }
    }
  }

  if(min_distance<t_radius && is_inside) {
    m_distance = t_radius;
    m_pos = last_pos;
    m_normal = last_normal;
    return true;
  }
  else
    return false;
}

//点-ポリゴン
template <>
bool CollisionDetector::detect_<IPointCollision, IPolygonCollision>
  (const IPointCollision& base, const IPolygonCollision& target)
{
  return detect_(target, base);
}

//ポリゴン-線
template<>
bool CollisionDetector::detect_<IPolygonCollision, ILineCollision>
  (const IPolygonCollision& base, const ILineCollision& target)
{
  if( !base.getBoundingBox().isOverlapped(target.getBoundingBox()) )
    return false;

  const IPolygonCollision::vertex_cont &b_vtx = base.getVertexCont();
  const IPolygonCollision::normal_cont &b_nml = base.getNormalCont();
  const IPolygonCollision::index_cont  &b_idx = base.getIndexCont();
  const vector4& t_head = target.getHead();
  const vector4& t_tail = target.getTail();

  bool is_inside = false;
  float min_distance = 0;
  vector4 last_pos;
  vector4 last_normal;

  for(unsigned int i=0; i<(b_idx.size()/3); ++i) {
    const vector4& normal = b_nml[i];
    const vector4 *vtx[3] = {
      &b_vtx[ b_idx[i*3+0] ],
      &b_vtx[ b_idx[i*3+1] ],
      &b_vtx[ b_idx[i*3+2] ],
    };
  
    float dist_tri  = normal.dot(*vtx[0]);
    float dist_tail = normal.dot(t_tail);
    if( (normal.dot(t_head)-dist_tri)*(dist_tail-dist_tri)>0)
      continue;

    vector4 direction = (t_head-t_tail).normalize();
    float t = -(normal.dot(t_tail)-normal.dot(*vtx[0]))/normal.dot(direction);
    vector4 proj_pos = t_tail+direction*t;

    float inside=0;
    for(unsigned int j=0; j<3; ++j) {
      if((proj_pos-*vtx[j]).square()<0.01) {
        inside=360.0f*radian;
        break;
      }
      inside+=(*vtx[j]-proj_pos).cos( *vtx[(j+1)%3]-proj_pos );
    }

    if( (inside>358.0f*radian)
    &&  (fabsf(dist_tail)<fabsf(min_distance) || !is_inside) ) {
      is_inside = true;
      min_distance = dist_tail;
      last_pos = proj_pos;
      last_normal = normal;
    }
  }

  if(is_inside) {
    m_distance = target.getRadius()+((t_tail-last_pos).norm()+min_distance);
    m_pos = last_pos;
    m_normal = last_normal;
    return true;
  }
  else
    return false;
}

//線-ポリゴン
template<>
bool CollisionDetector::detect_<ILineCollision, IPolygonCollision>
  (const ILineCollision& base, const IPolygonCollision& target)
{
  return detect_(target, base);
}



//点-点
template <>
bool CollisionDetector::detect_<IPointCollision, IPointCollision>
  (const IPointCollision& base, const IPointCollision& target)
{
  if(!base.getBoundingBox().isOverlapped(target.getBoundingBox())) {
    return false;
  }

  const vector4 &b_pos = base.getPosition();
  const vector4 &t_pos = target.getPosition();

  float pos_gap = (t_pos-b_pos).norm();
  float union_radius = base.getRadius() + target.getRadius();

  if(pos_gap < union_radius) {
    m_distance = union_radius-pos_gap;
    m_normal = (t_pos-b_pos).normalize();
    m_pos = b_pos+((t_pos-b_pos)*(base.getRadius()/union_radius));
    return true;
  }

  return false;
}


//線-点
template <>
bool CollisionDetector::detect_<ILineCollision, IPointCollision>
  (const ILineCollision& base, const IPointCollision& target)
{
  if( !base.getBoundingBox().isOverlapped(target.getBoundingBox()) ) {
    return false;
  }

  const vector4 &b_head = base.getHead();
  const vector4 &b_tail = base.getTail();
  float b_radius = base.getRadius();
  const vector4 &t_pos = target.getPosition();
  float t_radius = target.getRadius();

  bool result = false;
  vector4 segment = b_head-b_tail;
  float union_radius = b_radius+t_radius;

  float u = 0;
  if(b_tail!=b_head) {
    u = (t_pos-b_tail).dot(segment) / segment.square();
  }

  if(u>0 && u<1) {
    vector4 nearest = b_tail+(segment*u);
    float distance = (t_pos-nearest).norm();

    if( distance < union_radius ) {
      m_normal = (t_pos-nearest).normalize();
      m_pos = nearest;
      result = true;
    }
  }
  else if((t_pos-b_head).norm()<union_radius) {
    m_normal = (t_pos-b_head).normalize();
    m_pos = b_head;
    result = true;
  }
  else if((t_pos-b_tail).norm()<union_radius) {
    m_normal = (t_pos-b_tail).normalize();
    m_pos = b_tail;
    result = true;
  }

  if(result) {
    m_distance = union_radius;
  }

  return result;
}


//点-線
template <>
bool CollisionDetector::detect_<IPointCollision, ILineCollision>
  (const IPointCollision& base, const ILineCollision& target)
{
  return detect_(target, base);
}


//線-線
template <>
bool CollisionDetector::detect_<ILineCollision, ILineCollision>
  (const ILineCollision& base, const ILineCollision& target)
{
  if( !base.getBoundingBox().isOverlapped(target.getBoundingBox()) ) {
    return false;
  }

  return false;
}


//点-直方体
template <>
bool CollisionDetector::detect_<IPointCollision, IBoxCollision>
  (const IPointCollision& base, const IBoxCollision& target)
{
  if( !base.getBoundingBox().isOverlapped(target.getBoundingBox()) ) {
    return false;
  }

  float rad = base.getRadius();
  const Box& tbox = target.getBox();
  vector4 pos = target.getInvertMatrix()*base.getPosition();
  if(tbox.isInner(Sphere(pos, rad))) {
    vector4 tur = tbox.getUpperRight()+vector4(rad);
    vector4 tbl = tbox.getBottomLeft()-vector4(rad);
    vector4 normal;
    float distance = 0.0f;
    float r = tur.x-pos.x;
    float l = pos.x-tbl.x;
    float u = tur.y-pos.y;
    float d = pos.y-tbl.y;
    float f = tur.z-pos.z;
    float b = pos.z-tbl.z;

    if     (r<=l && r<u && r<d && r<f && r<b) { normal = vector4(-1.0f, 0.0f, 0.0f, 0.0f);distance=r; pos.x=tur.x; }
    else if(l<=u && l<d && l<f && l<b)        { normal = vector4(1.0f, 0.0f, 0.0f, 0.0f); distance=l; pos.x=tbl.x; }
    else if(u<=d && u<f && u<b)               { normal = vector4(0.0f, -1.0f, 0.0f, 0.0f);distance=u; pos.y=tur.y; }
    else if(d<=f && d<b)                      { normal = vector4(0.0f, 1.0f, 0.0f, 0.0f); distance=d; pos.y=tbl.y; }
    else if(f<=b)                             { normal = vector4(0.0f, 0.0f, -1.0f, 0.0f);distance=f; pos.z=tur.z; }
    else                                      { normal = vector4(0.0f, 0.0f, 1.0f, 0.0f); distance=b; pos.z=tbl.z; }

    const matrix44& tmat = target.getMatrix();
    m_pos = tmat*pos;
    m_normal = tmat*normal;
    m_distance = distance;
    return true;
  }
  return false;
}

//直方体-点
template <>
bool CollisionDetector::detect_<IBoxCollision, IPointCollision>
  (const IBoxCollision& base, const IPointCollision& target)
{
  if(detect_(target, base)) {
    m_normal*=-1.0f;
    return true;
  }
  return false;
}

//直方体-直方体
template<>
bool CollisionDetector::detect_<IBoxCollision, IBoxCollision>
  (const IBoxCollision& obb1, const IBoxCollision& obb2)
{
  if( !obb1.getBoundingBox().isOverlapped(obb2.getBoundingBox()) ) {
    return false;
  }

  const matrix44& im1 = obb1.getInvertMatrix();
  const matrix44& im2 = obb2.getInvertMatrix();
  const matrix44& m1 = obb1.getMatrix();
  const matrix44& m2 = obb2.getMatrix();
  const Box& b1 = obb1.getBox();
  const Box& b2 = obb2.getBox();
  vector4 b2pos = im1*(m2*b2.getCenter());

  // obb1の3軸がxyz軸になる空間に変換
  vector4 Axis[3];
  Axis[0] = im1*(m2*vector4(1.0f, 0.0f, 0.0f, 0.0f));
  Axis[1] = im1*(m2*vector4(0.0f, 1.0f, 0.0f, 0.0f));
  Axis[2] = im1*(m2*vector4(0.0f, 0.0f, 1.0f, 0.0f));

  vector4 VecAtoBTrans = b2pos - b1.getCenter();

  // 無駄な計算を省くために下ごしらえ
  float B[3][3] = {
    { Axis[0].x, Axis[0].y, Axis[0].z },
    { Axis[1].x, Axis[1].y, Axis[1].z },
    { Axis[2].x, Axis[2].y, Axis[2].z }
  };

  float absB[3][3] = {
    { fabs( B[0][0] ), fabs( B[0][1] ), fabs( B[0][2] ) },
    { fabs( B[1][0] ), fabs( B[1][1] ), fabs( B[1][2] ) },
    { fabs( B[2][0] ), fabs( B[2][1] ), fabs( B[2][2] ) }
  };

  float T[3] = { VecAtoBTrans.x, VecAtoBTrans.y, VecAtoBTrans.z }; 

  float a[3] = { b1.getWidth()/2.0f, b1.getHeight()/2.0f, b1.getLength()/2.0f };
  float b[3] = { b2.getWidth()/2.0f, b2.getHeight()/2.0f, b2.getLength()/2.0f };

  // 以下、判定ルーチン

  // 分離軸 A.Axis[?]
  for( int i = 0; i < 3; ++i ){
    if( fabs( T[i] )
      > a[i] + b[0] * absB[0][i] + b[1] * absB[1][i] + b[2] * absB[2][i] ) return false;
  }

  // 分離軸 B.Axis[?]
  for( int i = 0; i < 3; ++i ){
    if( fabs( T[0] * B[i][0] + T[1] * B[i][1] + T[2] * B[i][2] )
      > b[i] + a[0] * absB[i][0] + a[1] * absB[i][1] + a[2] * absB[i][2] ) return false;
  }

  // 分離軸 A.Axis[?] x B.Axis[?]
  for( int j0 = 0; j0 < 3; ++j0 ){
    int j1 = ( j0 + 1 ) % 3;
    int j2 = ( j0 + 2 ) % 3;
    for( int i0 = 0; i0 < 3; ++i0 ){
      int i1 = ( i0 + 1 ) % 3;
      int i2 = ( i0 + 2 ) % 3;
      if( fabs( -T[j1] * B[i0][j2] + T[j2] * B[i0][j1] ) 
        > a[j1] * absB[i0][j2] + a[j2] * absB[i0][j1]
        + b[i1] * absB[i2][j0] + b[i2] * absB[i1][j0] ) return false;
    }
  }

  {
    vector4 pos;

    {
      float dist = 0.0f;
      vector4 b2c = b2.getCenter();
      vector4 ur = b1.getUpperRight();
      vector4 bl = b1.getBottomLeft();
      vector4 v[19] = {
        b1.getCenter(),

        ur-vector4(b1.getWidth()/2.0f, b1.getHeight()/2.0f, 0.0f),
        ur-vector4(b1.getWidth()/2.0f, 0.0f, b1.getLength()/2.0f),
        ur-vector4(0.0f, b1.getHeight()/2.0f, b1.getLength()/2.0f),
        bl+vector4(b1.getWidth()/2.0f, b1.getHeight()/2.0f, 0.0f),
        bl+vector4(b1.getWidth()/2.0f, 0.0f, b1.getLength()/2.0f),
        bl+vector4(0.0f, b1.getHeight()/2.0f, b1.getLength()/2.0f),

        bl+vector4(b1.getWidth()/2.0f, 0.0f, 0.0f),
        bl+vector4(0.0f, 0.0f, b1.getLength()/2.0f),
        bl+vector4(b1.getWidth()/2.0f, 0.0f, b1.getLength()),
        bl+vector4(b1.getWidth(), 0.0f, b1.getLength()/2.0f),

        bl+vector4(0.0f, b1.getHeight()/2.0f, 0.0f),
        bl+vector4(b1.getWidth(), b1.getHeight()/2.0f, 0.0f),
        bl+vector4(b1.getWidth(), b1.getHeight()/2.0f, b1.getLength()),
        bl+vector4(0.0f, b1.getHeight()/2.0f, b1.getLength()),

        bl+vector4(b1.getWidth()/2.0f, b1.getHeight(), 0.0f),
        bl+vector4(0.0f, b1.getHeight(), b1.getLength()/2.0f),
        bl+vector4(b1.getWidth(), b1.getHeight(), b1.getLength()/2.0f),
        bl+vector4(b1.getWidth()/2.0f, b1.getHeight(), b1.getLength()),
      };
      for(int i=0; i<19; ++i) {
        vector4 tv = im2*(m1*v[i]);
        if(b2.isInner(tv)) {
          pos = tv;
          break;
        }
        else {
          float d = (tv-b2c).norm();
          if(i==0 || d<dist) {
            pos = tv;
            dist = d;
          }
        }
      }
    }

    vector4 tur = b2.getUpperRight();
    vector4 tbl = b2.getBottomLeft();
    vector4 normal;
    float distance = 0.0f;
    float r = tur.x-pos.x;
    float l = pos.x-tbl.x;
    float u = tur.y-pos.y;
    float d = pos.y-tbl.y;
    float f = tur.z-pos.z;
    float b = pos.z-tbl.z;
    if     (r<=l && r<u && r<d && r<f && r<b) { normal = vector4(-1.0f, 0.0f, 0.0f, 0.0f);distance=r; pos.x=tur.x; }
    else if(l<=u && l<d && l<f && l<b)        { normal = vector4(1.0f, 0.0f, 0.0f, 0.0f); distance=l; pos.x=tbl.x; }
    else if(u<=d && u<f && u<b)               { normal = vector4(0.0f, -1.0f, 0.0f, 0.0f);distance=u; pos.y=tur.y; }
    else if(d<=f && d<b)                      { normal = vector4(0.0f, 1.0f, 0.0f, 0.0f); distance=d; pos.y=tbl.y; }
    else if(f<=b)                             { normal = vector4(0.0f, 0.0f, -1.0f, 0.0f);distance=f; pos.z=tur.z; }
    else                                      { normal = vector4(0.0f, 0.0f, 1.0f, 0.0f); distance=b; pos.z=tbl.z; }
    m_normal = m2*normal;
    m_pos = m2*pos;
    m_distance = fabsf(distance);
    return true;
  }


  return true;
}





//基底クラスから型判別
bool CollisionDetector::detect(const ICollision &base, const ICollision &target)
{
  m_result = false;

  int btype = base.getType();
  int ttype = target.getType();
  if(btype==CM_TRUE || ttype==CM_TRUE) {
    m_result = true;
  }
  else if(btype==CM_FALSE || ttype==CM_FALSE) {
    m_result = false;
  }
  else if(btype==CM_POINT) {
    const IPointCollision& b = static_cast<const IPointCollision&>(base);
    if(ttype==CM_POINT) {
      m_result = detect_(b, static_cast<const IPointCollision&>(target));
    }
    else if(ttype==CM_LINE) {
      m_result = detect_(b, static_cast<const ILineCollision&>(target));
    }
    else if(ttype==CM_BOX) {
      m_result = detect_(b, static_cast<const IBoxCollision&>(target));
    }
    else if(ttype==CM_POLYGON) {
      m_result = detect_(b, static_cast<const IPolygonCollision&>(target));
    }
    else if(ttype==CM_COMPOSITE) {
      const ICompositeCollision& t = static_cast<const ICompositeCollision&>(target);
      for(size_t i=0; i<t.getCollisionCount(); ++i) {
        if(m_result = detect(base, t.getCollision(i))) {
          break;
        }
      }
    }
  }
  else if(btype==CM_LINE) {
    const ILineCollision& b = static_cast<const ILineCollision&>(base);
    if(ttype==CM_POINT) {
      m_result = detect_(b, static_cast<const IPointCollision&>(target));
    }
    else if(ttype==CM_LINE) {
      m_result = detect_(b, static_cast<const ILineCollision&>(target));
    }
    else if(ttype==CM_POLYGON) {
      m_result = detect_(b, static_cast<const IPolygonCollision&>(target));
    }
    else if(ttype==CM_COMPOSITE) {
      const ICompositeCollision& t = static_cast<const ICompositeCollision&>(target);
      for(size_t i=0; i<t.getCollisionCount(); ++i) {
        if(m_result = detect(base, t.getCollision(i))) {
          break;
        }
      }
    }
  }
  else if(btype==CM_BOX) {
    const IBoxCollision& b = static_cast<const IBoxCollision&>(base);
    if(ttype==CM_POINT) {
      m_result = detect_(b, static_cast<const IPointCollision&>(target));
    }
    else if(ttype==CM_BOX) {
      m_result = detect_(b, static_cast<const IBoxCollision&>(target));
    }
    else if(ttype==CM_COMPOSITE) {
      const ICompositeCollision& t = static_cast<const ICompositeCollision&>(target);
      for(size_t i=0; i<t.getCollisionCount(); ++i) {
        if(m_result = detect(base, t.getCollision(i))) {
          break;
        }
      }
    }
  }
  else if(btype==CM_POLYGON) {
    const IPolygonCollision& b = static_cast<const IPolygonCollision&>(base);
    if(ttype==CM_POINT) {
      m_result = detect_(b, static_cast<const IPointCollision&>(target));
    }
    else if(ttype==CM_LINE) {
      m_result = detect_(b, static_cast<const ILineCollision&>(target));
    }
    else if(ttype==CM_POLYGON) {
      m_result = detect_(b, static_cast<const IPolygonCollision&>(target));
    }
    else if(ttype==CM_COMPOSITE) {
      const ICompositeCollision& t = static_cast<const ICompositeCollision&>(target);
      for(size_t i=0; i<t.getCollisionCount(); ++i) {
        if(m_result = detect(base, t.getCollision(i))) {
          break;
        }
      }
    }
  }
  else if(btype==CM_COMPOSITE) {
    const ICompositeCollision& b = static_cast<const ICompositeCollision&>(base);
    for(size_t i=0; i<b.getCollisionCount(); ++i) {
      if(m_result = detect(b.getCollision(i), target)) {
        break;
      }
    }
  }

  m_normal.w = 0.0f;
  return m_result;
}




/*
  PolygonModel
*/


BPolygonChunc::BPolygonChunc() :
  m_facet(0.0f),
  m_face_num(0)
{}

BPolygonChunc::~BPolygonChunc() {
}

void BPolygonChunc::clear() {
  m_index.clear();
  m_vertex.clear();
  m_vtexcoord.clear();
  m_normal.clear();
  m_vnormal.clear();
  m_aabb = Box();
  m_bsphere = Sphere();
  m_facet = 0.0f;
}

void BPolygonChunc::draw() const {
  assert(0);
}


void BPolygonChunc::setNormal() {
  if(m_normal.empty() && m_vnormal.empty()) {
    for(size_t i=0; i<m_face_num; ++i) {
      vector4 vec1 = m_vertex[m_index[i*3+1]].position - m_vertex[m_index[i*3]].position;
      vector4 vec2 = m_vertex[m_index[i*3+2]].position - m_vertex[m_index[i*3]].position;

      vector4 cur_normal = vector4().cross(vec1, vec2);
      m_normal.push_back(cur_normal.v3());
    }
  }

  if(m_vnormal.empty()) {
    m_vnormal.reserve(m_index.size());
    for(size_t i=0; i<m_index.size(); ++i) {
      int union_num = 0;
      vector3 std_normal = m_normal[i/3];
      vector3 cur_normal = std_normal;

      for(size_t j=0; j<m_face_num; ++j) {
        if(m_index[i]==m_index[j*3+0] || m_index[i]==m_index[j*3+1] || m_index[i]==m_index[j*3+2] ) {
          if(std_normal!=m_normal[j] && std_normal.cos(m_normal[j]) <= m_facet*radian) {
            cur_normal+=m_normal[j];
            union_num++;
          }
        }
      }

      cur_normal.normalize();
      m_vnormal.push_back(cur_normal);
    }
  }

  for(unsigned int i=0; i<m_normal.size(); ++i) {
    m_normal[i].normalize();
  }
}

void BPolygonChunc::setBoundingVolume()
{
  if(!m_vertex.empty()) {
    vector4 ur = m_vertex.front().position;
    vector4 bl = m_vertex.front().position;
    for(vertex_cont::iterator p=m_vertex.begin(); p!=m_vertex.end(); ++p) {
      const vector4& v = p->position;
      if(v.x > ur.x)      ur.x = v.x;
      else if(v.x < bl.x) bl.x = v.x;
      if(v.y > ur.y)      ur.y = v.y;
      else if(v.y < bl.y) bl.y = v.y;
      if(v.z > ur.z)      ur.z = v.z;
      else if(v.z < bl.z) bl.z = v.z;
    }
    m_aabb.setUpperRight(ur);
    m_aabb.setBottomLeft(bl);
  }
  else if(!m_vvertex.empty()) {
    vector4 ur = m_vvertex.front().v4();
    vector4 bl = m_vvertex.front().v4();
    for(position_cont::iterator p=m_vvertex.begin(); p!=m_vvertex.end(); ++p) {
      const vector3& v = *p;
      if(v.x > ur.x)      ur.x = v.x;
      else if(v.x < bl.x) bl.x = v.x;
      if(v.y > ur.y)      ur.y = v.y;
      else if(v.y < bl.y) bl.y = v.y;
      if(v.z > ur.z)      ur.z = v.z;
      else if(v.z < bl.z) bl.z = v.z;
    }
    m_aabb.setUpperRight(ur);
    m_aabb.setBottomLeft(bl);
  }

  vector4 center = (m_aabb.getUpperRight()+m_aabb.getBottomLeft())/2.0f;
  m_bsphere.setPosition(center);
  m_bsphere.setRadius((m_aabb.getUpperRight()-center).norm());
}


void BPolygonChunc::setup() {
  m_face_num = m_index.size()/3;

  if(m_vvertex.empty()) {
    m_vvertex.resize(m_index.size());
    for(size_t i=0; i<m_index.size(); ++i) {
      m_vvertex[i] = m_vertex[m_index[i]].position.v3();
    }
  }

  setNormal();
  setBoundingVolume();
}





BPolygonModel::chunc_ptr BPolygonModel::getChunc(int index)
{
  chunc_cont::iterator p = m_chuncs.begin();
  advance(p, index);
  return (*p).get();
}

BPolygonModel::chunc_ptr BPolygonModel::getChunc(const std::string &label)
{
  int i = 0;
  for(label_cont::iterator p=m_labels.begin(); p!=m_labels.end(); ++p) {
    if(*p==label) {
      return getChunc(i);
    }
    ++i;
  }

  return chunc_ptr();
}



void BPolygonModel::draw() const
{
  for(chunc_cont::const_iterator p=m_chuncs.begin(); p!=m_chuncs.end(); ++p) {
    (*p)->draw();
  }
}


bool BPolygonModel::load(const std::string& filename)
{
  m_chuncs.clear();
  m_labels.clear();

  if(  (strncmp(&filename[filename.size()]-4, ".mqo", 4)==0 && loadMQO(filename))
    || (strncmp(&filename[filename.size()]-4, ".obj", 4)==0 && loadOBJ(filename)) ) {

    setup();
    return true;
  }

  return false;
}

void BPolygonModel::setup()
{
  for(chunc_cont::iterator p=m_chuncs.begin(); p!=m_chuncs.end(); /* */) {
    (*p)->setup();
    ++p;
  }
}



// OBJLoader


OBJLoader::OBJLoader(std::iostream& ios) : m_status(0), m_in(ios) {
  char buf[1024];

  while(true) {
    m_in.getline(buf, 1024);
    if(strncmp(buf, "g ", 2)==0 || m_chuncs.empty()) {
      m_chuncs.push_back(chunc());
      char label[128];
      if(sscanf(buf, "g %s", label)==1) {
        m_chuncs.back().m_label = label;
      }

      continue;
    }

    int s = loadVertex(buf);
    if(s!=0) {
      m_status|=s;
    }
    else {
      loadFace(buf);
    }
  }

//  for(chunc_cont::iterator p=chuncs.begin(); p!=chuncs.end(); ++p)
//    assignVertexArray();

  for(chunc_cont::iterator p=m_chuncs.begin(); p!=m_chuncs.end(); ) {
    if(p->m_vindex.empty()) {
      m_chuncs.erase(p++);
    }
    else {
      ++p;
    }
  }
}

void OBJLoader::assignVertexArray(chunc& c, position_cont& varray, index_cont& vindex)
{
  if(c.m_vindex.empty()) {
    return;
  }

  size_t max_index = *max_element( c.m_vindex.begin(), c.m_vindex.end() )+1;
  size_t min_index = *min_element( c.m_vindex.begin(), c.m_vindex.end() );

  varray.assign(m_vertex.begin()+min_index, m_vertex.begin()+max_index);  // 必要な頂点だけコピー
  for(index_cont::iterator p=vindex.begin(); p!=vindex.end(); ++p) {
    *p-=min_index;
  }
}

int OBJLoader::loadVertex(char *buf)
{
  if(buf[0]!='v') {
    return 0;
  }

  vector3 vtx;
  vector3 nml;
  vector2 tex;

  if(sscanf(buf, "v %f %f %f", &vtx.x, &vtx.y, &vtx.z)==3) {
    m_vertex.push_back(vtx);
    return 1;
  }
  else if( sscanf(buf, "vt %f %f", &tex.x, &tex.y)==2 ) {
    m_texcoord.push_back(tex);
    return 2;
  }
  else if( sscanf(buf, "vn %f %f %f", &nml.x, &nml.y, &nml.z)==3 ) {
    m_normal.push_back(nml);
    return 4;
  }
  else {
    return 0;
  }
}

void OBJLoader::loadFace(char *buf)
{
  if(strncmp(buf, "f ", 2)!=0) {
    return;
  }
  if(m_chuncs.empty()) {
    m_chuncs.push_back(chunc());
  }

  chunc& c = m_chuncs.back();
  index_cont vindex, tindex, nindex;
  int ngon = loadIndex(buf, vindex, tindex, nindex);

  for(int i=0; i<ngon-2; i++) {  // 全部3角ポリゴンに変換 
    int index[3];
    index[0] = 0;
    index[1] = i+1;
    index[2] = i+2;

    for(int j=0; j<3; ++j) {
      if(m_status&1) {
        c.m_vindex.push_back(vindex[index[j]]);
      }
      if(m_status&2) {
        c.m_tindex.push_back(tindex[index[j]]);
      }
      if(m_status&4) {
        c.m_nindex.push_back(nindex[index[j]]);
      }
    }
  }
}

int OBJLoader::loadIndex(char *buf, index_cont& vindex, index_cont& tindex, index_cont& nindex)
{
  int tvi, tti, tni;
  int ngon = 0;

  char *token;
  token = strtok(buf, " "); // v. 
  token = strtok(0, " ");

  while(token) {
    switch(m_status) {
    case(1):
      sscanf(token, "%d", &tvi);
      vindex.push_back(tvi-1);
      break;

    case(3):
      sscanf(token, "%d/%d", &tvi, &tti);
      vindex.push_back(tvi-1);
      tindex.push_back(tti-1);
      break;

    case(5):
      sscanf(token, "%d/%d", &tvi, &tni);
      vindex.push_back(tvi-1);
      nindex.push_back(tni-1);
      break;

    case(7):
      sscanf(token, "%d/%d/%d", &tvi, &tti, &tni);
      vindex.push_back(tvi-1);
      tindex.push_back(tti-1);
      nindex.push_back(tni-1);
      break;

    default:
      break;
    }

    ++ngon;
    token = strtok(0, " ");
  }

  return ngon;
}


bool BPolygonModel::loadOBJ(const std::string& filename) {
  std::fstream in(filename.c_str(), std::ios::in);
  if(!in)
    return false;
  
  OBJLoader loader(in);
  for(OBJLoader::chunc_cont::iterator p=loader.m_chuncs.begin(); p!=loader.m_chuncs.end(); ++p) {
    cchunc_ptr c = createChunc();
    for(size_t i=0; i<loader.m_vertex.size(); ++i) {
      vertex_t v;
      v.position = loader.m_vertex[i].v4();
      c->m_vertex.push_back(v);
    }
    c->m_index = p->m_vindex;
    for(size_t i=0; i<p->m_tindex.size(); ++i) {
      c->m_vtexcoord.push_back(loader.m_texcoord[p->m_tindex[i]]);
    }
    // 今はnormal無視仕様 

    c->setup();
    addChunc(c, p->m_label);
  }
  return true;
}


// MQOLoader

MQOLoader::MQOLoader(std::iostream& ios) : m_in(ios) {
  char buf[1024];
  char label[128];
  while(!m_in.eof()) {
    m_in.getline(buf, 1024);
    if(strcmp(buf, "Eof")==0) {
      break;
    }
    if(sscanf(buf, "Object \"%[^\"]\" {", label)==1) {
      m_chuncs.push_back(chunc());
      m_chuncs.back().m_label = label;

      while(true) {
        m_in.getline(buf, 1024);
        if(buf[0]=='}') {
          break;
        }
        loadAttrib(buf);
        loadVertex(buf);
        loadFace(buf);
      }
    }
  }
}

bool MQOLoader::loadFace(char *buf) {
  size_t num_face;
  if(sscanf(buf, "\tface %d", &num_face)==0)
    return false;

  chunc& c = m_chuncs.back();
  for(size_t i=0; i<num_face; ++i) {
    int ngon = 0;
    int index[4];
    vector2 tex[4];

    m_in.getline(buf, 1024);
    char *token = strtok(buf, " ");
    sscanf(token, " %d", &ngon);

    while(token) {
      if(sscanf(token, " V(%d %d %d %d", &index[0], &index[1], &index[2], &index[3])==ngon ) {
        for(int i=0; i<ngon-2; ++i) {
          c.m_index.push_back(index[0]);
          c.m_index.push_back(index[i+2]);
          c.m_index.push_back(index[i+1]);
        }
      }
      else if(sscanf(token, " UV(%f %f %f %f %f %f %f %f",
           &tex[0].x,&tex[0].y, &tex[1].x,&tex[1].y, &tex[2].x,&tex[2].y, &tex[3].x,&tex[3].y)==ngon*2 ) {
        for(int i=0; i<ngon-2; ++i) {
          c.m_vtexcoord.push_back(tex[0]);
          c.m_vtexcoord.push_back(tex[i+2]);
          c.m_vtexcoord.push_back(tex[i+1]);
        }
      }
      token = strtok(0, ")");
    }
  }
  m_in.getline(buf, 1024); // "  }"
  return true;
}

bool MQOLoader::loadVertex(char *buf) {
  size_t num_vertex;
  if(sscanf(buf, "\tvertex %d", &num_vertex)==0) {
    return false;
  }

  chunc& c = m_chuncs.back();
  for(size_t i=0; i<num_vertex; ++i) {
    vector3 vtx;
    m_in.getline(buf, 1024);
    if(sscanf(buf, "\t\t%f %f %f", &vtx.x, &vtx.y, &vtx.z)==3) {
      c.m_vertex.push_back(vtx);
    }
  }
  m_in.getline(buf, 1024); // "  }"
  return true;
}

bool MQOLoader::loadAttrib(char *buf) {
  float facet;
  if(sscanf(buf, "\tfacet %f", &facet)==1) {
    chunc& c = m_chuncs.back();
    c.m_facet = facet;
    return true;
  }
  return false;
}



bool BPolygonModel::loadMQO(const std::string& filename)
{
  std::fstream in(filename.c_str(), std::ios::in);
  if(!in) {
    return false;
  }
  
  MQOLoader loader(in);
  for(MQOLoader::chunc_cont::iterator p=loader.m_chuncs.begin(); p!=loader.m_chuncs.end(); ++p) {
    cchunc_ptr c = createChunc();
    for(size_t i=0; i<p->m_vertex.size(); ++i) {
      vertex_t v;
      v.position = p->m_vertex[i].v4();
      c->m_vertex.push_back(v);
    }
    c->m_index = p->m_index;
    c->m_vtexcoord = p->m_vtexcoord;
    c->m_facet = p->m_facet;
    c->setup();
    addChunc(c, p->m_label);
  }
  return true;
}






/*
  MorphModel
*/

struct AVTXHeader {
  unsigned int version_number;
  unsigned int index_count;
  unsigned int vertex_count;
  unsigned int vertex_clip_count;
};

struct AVTXClipHeader {
  unsigned int frame_count;
  unsigned int label_length;
};


BMorphModel::BMorphModel() :
  m_num_vertex(0)
{}

BMorphModel::~BMorphModel() {
}

void BMorphModel::draw(const std::string& clipname, float t) const {
  draw(m_vtx_clip.find(clipname), t);
}

void BMorphModel::draw(iterator iter, float t) const {
}

void BMorphModel::setup() {
  if(m_vtx_clip.empty() || m_vtx_clip.begin()->second.empty()) {
    return;
  }

  vertex_cont& vc = m_vtx_clip.begin()->second.begin()->second;
  vector4 ur = vc.front().v4();
  vector4 bl = vc.front().v4();
  for(vertex_cont::iterator p=vc.begin(); p!=vc.end(); ++p) {
    vector3& v = *p;
    if(v.x > ur.x)      ur.x = v.x;
    else if(v.x < bl.x) bl.x = v.x;
    if(v.y > ur.y)      ur.y = v.y;
    else if(v.y < bl.y) bl.y = v.y;
    if(v.z > ur.z)      ur.z = v.z;
    else if(v.z < bl.z) bl.z = v.z;
  }
  m_aabb.setUpperRight(ur);
  m_aabb.setBottomLeft(bl);

  vector4 center = (m_aabb.getUpperRight()+m_aabb.getBottomLeft())/2;
  m_bsphere.setPosition(center);
  m_bsphere.setRadius((m_aabb.getUpperRight()-center).norm());
}



bool BMorphModel::load(const std::string& filename) {
  clear();

  if(strncmp(&filename[filename.size()]-4, ".avx", 5)!=0) {
    return false;
  }

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  biostream bf(f);
  if(!f) {
    return false;
  }

  AVTXHeader header;
  bf>> header.version_number
    >> header.index_count
    >> header.vertex_count
    >> header.vertex_clip_count;
  if(header.version_number!=100) {
    return false;
  }

  m_idx_array.resize(header.index_count);
  m_tex_array.resize(header.index_count);
  m_num_vertex = header.vertex_count;


  for(unsigned int i=0; i<m_idx_array.size(); ++i) {
    bf >> m_idx_array[i];
  }
  for(unsigned int i=0; i<m_tex_array.size(); ++i) {
    bf >> m_tex_array[i].x;
    bf >> m_tex_array[i].y;
  }

  for(unsigned int i=0; i<header.vertex_clip_count; ++i) {
    AVTXClipHeader clip_header;
    std::vector<char> temp_string(256, '\0');
    bf >> clip_header.frame_count;
    bf >> clip_header.label_length;
    bf.read(&temp_string[0], clip_header.label_length);

    vertex_sequence& seq = m_vtx_clip[&temp_string[0]];

    for(unsigned int j=0; j<clip_header.frame_count; ++j) {
      float t;
      bf >> t;

      vertex_cont& vcont = seq[t];
      vcont.resize(header.vertex_count);

      for(unsigned int k=0; k<vcont.size(); ++k) {
        bf >> vcont[k].x >> vcont[k].y >> vcont[k].z;
      }
    }
  }
  setup();

  return true;
}

bool BMorphModel::save(const std::string& filename) {
  std::fstream f(filename.c_str(), std::ios::out | std::ios::binary);
  biostream bf(f);
  if(!f) {
    return false;
  }

  AVTXHeader header={
    100,
    m_idx_array.size(),
    m_num_vertex,
    m_vtx_clip.size()
  };

  bf<< header.version_number
    << header.index_count
    << header.vertex_count
    << header.vertex_clip_count;

  for(unsigned int i=0; i<m_idx_array.size(); ++i) {
    bf << m_idx_array[i];
  }
  for(unsigned int i=0; i<m_tex_array.size(); ++i) {
    bf << m_tex_array[i].x;
    bf << m_tex_array[i].y;
  }

  for(vertex_clip::iterator p=m_vtx_clip.begin(); p!=m_vtx_clip.end(); ++p) {
    vertex_sequence& seq = p->second;
    AVTXClipHeader clip_header={
      seq.size(),
      p->first.size(),
    };

    bf<< clip_header.frame_count
      << clip_header.label_length;
    bf.write(p->first.c_str(), p->first.size());

    for(vertex_sequence::iterator q=seq.begin(); q!=seq.end(); ++q) {
      bf << q->first;

      vertex_cont& vcont = q->second;
      for(unsigned int i=0; i<vcont.size(); ++i) {
        bf << vcont[i].x << vcont[i].y << vcont[i].z;
      }
    }
  }

  return true;
}

void BMorphModel::clear() {
  m_vtx_clip.clear();
  m_tex_array.clear();
  m_idx_array.clear();
  m_num_vertex = 0;
}

void BMorphModel::vertex_sequence::morph(float t, vertex_cont& t_vertex) const
{
  if(t<=beginFrame()) {
    t_vertex = begin()->second;
  }
  else if(t>=endFrame()) {
    t_vertex = (--end())->second;
  }
  else {
    vertex_sequence::range_t range = equal_range(t);
    if(range.first->first==t) {
      t_vertex = range.first->second;
    }
    else {
      --range.first;
      const float& keyframe1 = range.first->first;
      const float& keyframe2 = range.second->first;
      const vertex_cont& vcont1 = range.first->second;
      const vertex_cont& vcont2 = range.second->second;
      t_vertex.resize(vcont1.size());
      for(size_t i=0; i<vcont1.size(); ++i) {
        vector3 dir = (vcont2[i]-vcont1[i])/(keyframe2-keyframe1);
        t_vertex[i] = vcont1[i] + dir*(t-keyframe1);
      }
    }
  }
}




BCachedMorphModel::BCachedMorphModel() :
  m_num_vertex(0),
  m_facet(60.0f),
  m_enable_normal(true)
{}

BCachedMorphModel::BCachedMorphModel(const std::string& filename, bool en) :
  m_num_vertex(0),
  m_facet(60.0f),
  m_enable_normal(true)
{
  enableNormal(en);
  load(filename);
}

BCachedMorphModel::~BCachedMorphModel() {
}

bool BCachedMorphModel::load(const std::string& filename) {
  BMorphModel tpolygon;
  if(!tpolygon.load(filename)) {
    return false;
  }

  m_tex_array = tpolygon.m_tex_array;
  m_idx_array = tpolygon.m_idx_array;
  m_num_vertex = tpolygon.m_num_vertex;

  vertex_cont t_vertex;
  typedef BMorphModel::vertex_clip::iterator clip_iterator;
  for(clip_iterator p=tpolygon.m_vtx_clip.begin(); p!=tpolygon.m_vtx_clip.end(); ++p) {
    vertex_sequence& seq = m_vtx_clip[p->first];
    seq.m_begin_frame = int(tpolygon.beginFrame(p->first));
    seq.m_end_frame = int(tpolygon.endFrame(p->first));
    int frame_length = seq.endFrame()-seq.beginFrame();
    seq.m_vertex.resize(std::max<int>(1, frame_length));
    seq.m_normal.resize(std::max<int>(1, frame_length));
    int t=0;
    do {
      p->second.morph(t+seq.beginFrame(), t_vertex);
      setVertex(t_vertex, seq.m_vertex[t]);
      if(m_enable_normal) {
        setNormal(t_vertex, seq.m_normal[t]);
      }
      ++t;
    } while(t<frame_length);
  }

  setup();
  return true;
}

void BCachedMorphModel::setVertex(vertex_cont& vtx_array, vertex_cont& vvtx_array) {
  vvtx_array.resize(m_idx_array.size());
  for(size_t i=0; i<m_idx_array.size(); ++i) {
    vvtx_array[i] = vtx_array[m_idx_array[i]];
  }
}

void BCachedMorphModel::setNormal(vertex_cont& vtx_array, normal_cont& vnml_array) {
  normal_cont nml_array;

  for(size_t i=0; i<m_idx_array.size()/3; ++i) {
    vector3 vec1 = vtx_array[ m_idx_array[i*3+1] ] - vtx_array[ m_idx_array[i*3] ];
    vector3 vec2 = vtx_array[ m_idx_array[i*3+2] ] - vtx_array[ m_idx_array[i*3] ];
    nml_array.push_back(vector3().cross(vec1, vec2));
  }

  vnml_array.reserve(m_idx_array.size());
  for(size_t i=0; i<m_idx_array.size(); ++i) {
    int union_num = 0;
    vector3 std_normal = nml_array[i/3];
    vector3 cur_normal = std_normal;

    for(size_t j=0; j<m_idx_array.size()/3; ++j) {
      if(m_idx_array[i]==m_idx_array[j*3+0] || m_idx_array[i]==m_idx_array[j*3+1] || m_idx_array[i]==m_idx_array[j*3+2]) {
        if(std_normal!=nml_array[j] && std_normal.cos(nml_array[j]) <= m_facet*radian) {
          cur_normal+=nml_array[j];
          union_num++;
        }
      }
    }

    cur_normal.normalize();
    vnml_array.push_back(cur_normal);
  }
}

void BCachedMorphModel::draw(const std::string& clipname, int t) const {
  draw(m_vtx_clip.find(clipname), t);
}

void BCachedMorphModel::draw(iterator p, int t) const {
}

void BCachedMorphModel::setup() {
  if(m_vtx_clip.empty()) {
    return;
  }

  vertex_cont& m_vtx_array = m_vtx_clip.begin()->second.m_vertex.front();
  vector4 ur = m_vtx_array.front().v4();
  vector4 bl = m_vtx_array.front().v4();
  for(vertex_cont::iterator p=m_vtx_array.begin(); p!=m_vtx_array.end(); ++p) {
    const vector3& v = *p;
    if(v.x > ur.x)      ur.x = v.x;
    else if(v.x < bl.x) bl.x = v.x;
    if(v.y > ur.y)      ur.y = v.y;
    else if(v.y < bl.y) bl.y = v.y;
    if(v.z > ur.z)      ur.z = v.z;
    else if(v.z < bl.z) bl.z = v.z;
  }

  vector4 center = (m_aabb.getUpperRight()+m_aabb.getBottomLeft())/2;
  m_bsphere.setPosition(center);
  m_bsphere.setRadius((m_aabb.getUpperRight()-center).norm());
}

void BCachedMorphModel::clear() {
  m_vtx_clip.clear();
  m_tex_array.clear();
  m_idx_array.clear();
  m_num_vertex=0;
}


}


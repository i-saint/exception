#ifndef ist_vector
#define ist_vector

#include <cmath>
#include "bstream.h"

namespace ist {

const float pi = 3.141592653589793238462643383279f;
const float radian = pi/180.0f;


struct vector2;
struct vector3;
struct vector4;
struct matrix22;
struct matrix33;
struct matrix44;

struct vector2
{
  union {
    struct { float x,y; };
    float v[2];
  };

  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  vector2 operator - () const { return vector2(-x, -y); }
  vector2 operator * (float f) const { return vector2(x*f, y*f); }
  vector2 operator / (float f) const { return vector2(x/f, y/f); }
  void operator *=(float f) { x*=f; y*=f; }
  void operator /=(float f) { x/=f; y/=f; }
  vector2 operator + (const vector2 &a) const { return vector2(x+a.x, y+a.y); }
  vector2 operator - (const vector2 &a) const { return vector2(x-a.x, y-a.y); }
  vector2 operator * (const vector2 &a) const { return vector2(x*a.x, y*a.y); }
  vector2 operator / (const vector2 &a) const { return vector2(x/a.x, y/a.y); }
  void operator +=(const vector2 &a) { x+=a.x; y+=a.y; }
  void operator -=(const vector2 &a) { x-=a.x; y-=a.y; }
  void operator *=(const vector2 &a) { x*=a.x; y*=a.y; }
  void operator /=(const vector2 &a) { x/=a.x; y/=a.y; }
  bool operator ==(const vector2 &a) const { return x==a.x && y==a.y; }
  bool operator !=(const vector2 &a) const { return x!=a.x || y!=a.y; }


  vector2() : x(0), y(0) { }
  explicit vector2(float v) : x(v), y(v) {}
  explicit vector2(float _x, float _y) : x(_x), y(_y) {}
  explicit vector2(const float *fv) : x(fv[0]), y(fv[1]) {}

  vector2& setX(float a) { x=a; return *this; }
  vector2& setY(float a) { y=a; return *this; }

  float square() const
  {
    return (x*x + y*y);
  }

  float norm() const
  {
    return std::sqrt(x*x + y*y);
  }

  float dot(const vector2 &v) const
  {
    return (x*v.x + y*v.y);
  }

  vector2 normal()
  {
    return vector2(*this).normalize();
  }

  vector2& normalize()
  {
    float l=norm();
    if(l!=0) {
      *this/=l;
    }
    return *this;
  }

  float cos(const vector2 &a) const
  {
    return std::acos( dot(a) / (a.norm()*norm()) );
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<2; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<2; ++i) { b >> v[i]; }
  }
};


struct vector3
{
  union {
    struct { float x,y,z; };
    float v[3];
  };

  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  vector3 operator - () const { return vector3(-x, -y, -z); }
  vector3 operator * (float f) const { return vector3(x*f, y*f, z*f); }
  vector3 operator / (float f) const { return vector3(x/f, y/f, z/f); }
  void operator *=(float f) { x*=f; y*=f; z*=f; }
  void operator /=(float f) { x/=f; y/=f; z/=f; }
  vector3 operator + (const vector3 &a) const { return vector3(x+a.x, y+a.y, z+a.z); }
  vector3 operator - (const vector3 &a) const { return vector3(x-a.x, y-a.y, z-a.z); }
  vector3 operator * (const vector3 &a) const { return vector3(x*a.x, y*a.y, z*a.z); }
  vector3 operator / (const vector3 &a) const { return vector3(x/a.x, y/a.y, z/a.z); }
  void operator +=(const vector3 &a) { x+=a.x; y+=a.y; z+=a.z; }
  void operator -=(const vector3 &a) { x-=a.x; y-=a.y; z-=a.z; }
  void operator *=(const vector3 &a) { x*=a.x; y*=a.y; z*=a.z; }
  void operator /=(const vector3 &a) { x/=a.x; y/=a.y; z/=a.z; }
  bool operator ==(const vector3 &a) const { return x==a.x && y==a.y && z==a.z; }
  bool operator !=(const vector3 &a) const { return x!=a.x || y!=a.y || z!=a.z; }


  vector3() : x(0), y(0), z(0) {}
  explicit vector3(float v) : x(v), y(v), z(v) {}
  explicit vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
  explicit vector3(const float *fv) : x(fv[0]), y(fv[1]), z(fv[2]) {}
  explicit vector3(const vector4& fv);

  vector2 v2() const { return vector2(x,y); }
  vector2 v2xz() const { return vector2(x,z); }
  vector4 v4() const;

  vector3& setX(float a) { x=a; return *this; }
  vector3& setY(float a) { y=a; return *this; }
  vector3& setZ(float a) { z=a; return *this; }

  float square() const
  {
    return (x*x + y*y + z*z);
  }

  float norm() const
  {
    return std::sqrt(x*x + y*y + z*z);
  }

  vector3 normal() const
  {
    return vector3(*this).normalize();
  }

  vector3& normalize()
  {
    float l=norm();
    if(l!=0) {
      *this/=l;
    }
    return *this;
  }

  template<class VecType>
  float dot(const VecType &v) const
  {
    return (x*v.x + y*v.y + z*v.z);
  }

  template<class VecType>
  vector3& cross(const VecType &a, const VecType &b)
  {
    x = a.y*b.z - a.z*b.y;
    y = a.z*b.x - a.x*b.z;
    z = a.x*b.y - a.y*b.x;
    return *this;
  }

  template<class VecType>
  float cos(const VecType &a) const
  {
    return std::acos( dot(a) / (a.norm()*norm()) );
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<3; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<3; ++i) { b >> v[i]; }
  }
};


struct vector4
{
  union {
    struct { float x,y,z,w; };
    float v[4];
  };

  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  vector4 operator - () const { return vector4(-x, -y, -z); }
  vector4 operator * (float f) const { return vector4(x*f, y*f, z*f); }
  vector4 operator / (float f) const { return vector4(x/f, y/f, z/f); }
  void operator *=(float f) { x*=f; y*=f; z*=f; }
  void operator /=(float f) { x/=f; y/=f; z/=f; }
  vector4 operator + (const vector4 &a) const { return vector4(x+a.x, y+a.y, z+a.z); }
  vector4 operator - (const vector4 &a) const { return vector4(x-a.x, y-a.y, z-a.z); }
  vector4 operator * (const vector4 &a) const { return vector4(x*a.x, y*a.y, z*a.z); }
  vector4 operator / (const vector4 &a) const { return vector4(x/a.x, y/a.y, z/a.z); }
  void operator +=(const vector4 &a) { x+=a.x; y+=a.y; z+=a.z; }
  void operator -=(const vector4 &a) { x-=a.x; y-=a.y; z-=a.z; }
  void operator *=(const vector4 &a) { x*=a.x; y*=a.y; z*=a.z; }
  void operator /=(const vector4 &a) { x/=a.x; y/=a.y; z/=a.z; }
  bool operator ==(const vector4 &a) const { return x==a.x && y==a.y && z==a.z && w==a.w; }
  bool operator !=(const vector4 &a) const { return x!=a.x || y!=a.y || z!=a.z || w!=a.w; }


  vector4() : x(0), y(0), z(0), w(1) {}
  explicit vector4(float v) : x(v), y(v), z(v), w(1) {}
  explicit vector4(float _x, float _y, float _z, float _w=1) : x(_x), y(_y), z(_z), w(_w) {}
  explicit vector4(const float *fv) : x(fv[0]), y(fv[1]), z(fv[2]), w(fv[3]) {}

  vector2 v2() const { return vector2(x,y); }
  vector2 v2xz() const { return vector2(x,z); }
  vector3 v3() const { return vector3(v); }

  vector4& setX(float a) { x=a; return *this; }
  vector4& setY(float a) { y=a; return *this; }
  vector4& setZ(float a) { z=a; return *this; }
  vector4& setW(float a) { w=a; return *this; }

  float square() const
  {
    return (x*x + y*y + z*z);
  }

  float norm() const
  {
    return std::sqrt(x*x + y*y + z*z);
  }

  vector4 normal() const
  {
    return vector4(*this).normalize();
  }

  vector4& normalize()
  {
    float l=norm();
    if(l!=0)
      *this/=l;
    return *this;
  }

  template<class VecType>
  float dot(const VecType &v) const
  {
    return (x*v.x + y*v.y + z*v.z);
  }

  template<class VecType>
  float dot4(const VecType &v) const
  {
    return (x*v.x + y*v.y + z*v.z + w*v.w);
  }

  template<class VecType>
  vector4& cross(const VecType &a, const VecType &b)
  {
    x = a.y*b.z - a.z*b.y;
    y = a.z*b.x - a.x*b.z;
    z = a.x*b.y - a.y*b.x;
    return *this;
  }

  template<class VecType>
  float cos(const VecType &a) const
  {
    return std::acos( dot(a) / (a.norm()*norm()) );
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<4; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<4; ++i) { b >> v[i]; }
  }
};

inline vector4 vector3::v4() const { return vector4(x,y,z); }

inline vector3::vector3(const vector4& v) : x(v.x), y(v.y), z(v.z) {}



struct float4 : public vector4
{
public:
  float4 operator - () const { return float4(-x, -y, -z, -w); }
  float4 operator * (float f) const { return float4(x*f, y*f, z*f, w*f); }
  float4 operator / (float f) const { return float4(x/f, y/f, z/f, w/f); }
  void operator *=(float f) { x*=f; y*=f; z*=f; }
  void operator /=(float f) { x/=f; y/=f; z/=f; }
  float4 operator + (const float4 &a) const { return float4(x+a.x, y+a.y, z+a.z, w+a.w); }
  float4 operator - (const float4 &a) const { return float4(x-a.x, y-a.y, z-a.z, w-a.w); }
  float4 operator * (const float4 &a) const { return float4(x*a.x, y*a.y, z*a.z, w*a.w); }
  float4 operator / (const float4 &a) const { return float4(x/a.x, y/a.y, z/a.z, w/a.w); }
  void operator +=(const float4 &a) { x+=a.x; y+=a.y; z+=a.z; w+=a.w; }
  void operator -=(const float4 &a) { x-=a.x; y-=a.y; z-=a.z; w-=a.w; }
  void operator *=(const float4 &a) { x*=a.x; y*=a.y; z*=a.z; w*=a.w; }
  void operator /=(const float4 &a) { x/=a.x; y/=a.y; z/=a.z; w/=a.w; }
  bool operator ==(const float4 &a) const { return x==a.x && y==a.y && z==a.z && w==a.w; }
  bool operator !=(const float4 &a) const { return x!=a.x || y!=a.y || z!=a.z || w!=a.w; }
  float4& operator =(const float4& a) {x=a.x; y=a.y; z=a.z; w=a.w; return *this;}

  float4() {}
  float4(float v) : vector4(v) {}
  float4(float _x, float _y, float _z, float _w=1) : vector4(_x, _y, _z, _w) {}
  explicit float4(const float *fv) : vector4(fv) {}
};







struct matrix22
{
  union {
    struct {
        float x0, y0;
        float x1, y1;
    };
    float v[4];
    float m[2][2];
  };

  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  matrix22 operator * (const matrix22 &mlt) const
  {
    return matrix22(
      v[0]*mlt.v[0] + v[2]*mlt.v[1],
      v[0]*mlt.v[2] + v[2]*mlt.v[3],
      v[1]*mlt.v[0] + v[3]*mlt.v[1],
      v[1]*mlt.v[2] + v[3]*mlt.v[3] );
  }

  void operator *= (const matrix22 &mlt)
  {
    *this=*this*mlt;
  }

  template<typename VecType>
  VecType operator * (const VecType &vec) const
  {
    return VecType(
      v[0]*vec.x + v[2]*vec.y,
      v[1]*vec.x + v[3]*vec.y );
  }

  matrix22()
  {
    identity();
  }

  matrix22(const float *fv):
    x0(fv[0]), y0(fv[1]),
    x1(fv[2]), y1(fv[3]) {}

  matrix22(
    float _0, float _2,
    float _1, float _3 ):
    x0(_0), y0(_1),
    x1(_2), y1(_3) {}

  void identity()
  {
    v[0]=1; v[2]=0;
    v[1]=0; v[3]=1;
  }

  matrix22& transpose()
  {
    float t = m[0][1];
    m[0][1] = m[1][0];
    m[1][0] = t;
    return *this;
  }

  matrix22& invert()
  {
    const int dim=2;
    int i, j, k;
    float p, q;

    for(k=0; k<dim; k++) {
      p=m[k][k];
      m[k][k]=1.0;
      for(j=0; j<dim; j++)
        m[k][j]/=p;
      for(i=0; i<dim; i++) {
        if(i!=k) {
          q=m[i][k];
          m[i][k]=0.0;
          for(j=0; j<dim; j++)
            m[i][j]-=q*m[k][j];
        }
      }
    }

    return *this;
  }

  matrix22& scale(const float *fv)
  {
    scale(fv[0], fv[1]);
    return *this;
  }

  matrix22& scale(float sx, float sy)
  {
    matrix22 sm(sx,0,
          0,sy );
    *this*=sm;
    return *this;
  }


  matrix22& rotate(float ang)
  {
    ang=ang*radian; 
    float cz=std::cos(ang), sz=std::sin(ang);

    matrix22 rm(cz,-sz,
          sz, cz );
    *this*=rm;
    return *this;
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<4; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<4; ++i) { b >> v[i]; }
  }
};


struct matrix33
{
  union {
    struct {
        float x0, y0, z0;
        float x1, y1, z1;
        float x2, y2, z2;
    };
    float v[9];
    float m[3][3];
  };

  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  matrix33 operator * (const matrix33 &mlt) const
  {
    return matrix33(
      v[0]*mlt.v[0] + v[3]*mlt.v[1] + v[6]*mlt.v[2],
      v[0]*mlt.v[3] + v[3]*mlt.v[4] + v[6]*mlt.v[5],
      v[0]*mlt.v[6] + v[3]*mlt.v[7] + v[6]*mlt.v[8],

      v[1]*mlt.v[0] + v[4]*mlt.v[1] + v[7]*mlt.v[2],
      v[1]*mlt.v[3] + v[4]*mlt.v[4] + v[7]*mlt.v[5],
      v[1]*mlt.v[6] + v[4]*mlt.v[7] + v[7]*mlt.v[8],

      v[2]*mlt.v[0] + v[5]*mlt.v[1] + v[8]*mlt.v[2],
      v[2]*mlt.v[3] + v[5]*mlt.v[4] + v[8]*mlt.v[5],
      v[2]*mlt.v[6] + v[5]*mlt.v[7] + v[8]*mlt.v[8] );
  }

  void operator *= (const matrix33 &mlt)
  {
    *this=*this*mlt;
  }

  template<typename VecType>
  VecType operator * (const VecType &vec) const
  {
    return VecType(
      v[0]*vec.x + v[3]*vec.y + v[6]*vec.z,
      v[1]*vec.x + v[4]*vec.y + v[7]*vec.z,
      v[2]*vec.x + v[5]*vec.y + v[8]*vec.z );
  }

  matrix33()
  {
    identity();
  }

  matrix33(const float *fv):
    x0(fv[0]), y0(fv[1]), z0(fv[2]),
    x1(fv[3]), y1(fv[4]), z1(fv[5]),
    x2(fv[6]), y2(fv[7]), z2(fv[8]) {}

  matrix33(
    float _0, float _3, float _6,
    float _1, float _4, float _7,
    float _2, float _5, float _8 ):
    x0(_0), y0(_1), z0(_2),
    x1(_3), y1(_4), z1(_5),
    x2(_6), y2(_7), z2(_8) {}

  void identity()
  {
    v[0]=1; v[3]=0; v[6]=0;
    v[1]=0; v[4]=1; v[7]=0;
    v[2]=0; v[5]=0; v[8]=1;
  }

  matrix33& invert()
  {
    const int dim=3;
    int i, j, k;
    float p, q;

    for(k=0; k<dim; k++) {
      p=m[k][k];
      m[k][k]=1.0;
      for(j=0; j<dim; j++) {
        m[k][j]/=p;
      }
      for(i=0; i<dim; i++) {
        if(i!=k) {
          q=m[i][k];
          m[i][k]=0.0;
          for(j=0; j<dim; j++) {
            m[i][j]-=q*m[k][j];
          }
        }
      }
    }

    return *this;
  }

  matrix33& transpose()
  {
    float t;
    t=x1; x1=y0; y0=t;
    t=x2; x2=z0; z0=t;
    t=y2; y2=z1; z1=t;

    return *this;
  }

  matrix33& scale(const float *fv)
  {
    scale(fv[0], fv[1], fv[2]);
    return *this;
  }

  matrix33& scale(float sx, float sy, float sz)
  {
    matrix33 S(
      sx, 0, 0,
       0,sy, 0,
       0, 0,sz );
    *this*=S;
    return *this;
  }


  matrix33& rotate(const float *fv)
  {
    rotate(fv[0], fv[1], fv[2]);
    return *this;
  }

  matrix33& rotate(float p, float h, float r)
  {
    p*=radian;
    h*=radian;
    r*=radian; 
    float sp=std::sin(p), cp=std::cos(p);
    float sh=std::sin(h), ch=std::cos(h);
    float sr=std::sin(r), cr=std::cos(r);

    matrix33 em(
      cr*ch-sr*sp*sh,-sr*cp, cr*sh+sr*sp*ch,
      sr*ch+cr*sp*sh, cr*cp, sr*sh-cr*sp*ch,
              -cp*sh,    sp,          cp*ch );

    *this=*this*em;
    return *this;
  }


  matrix33& rotateX(float rx)
  {
    rx*=radian;
    float cx=std::cos(rx), sx=std::sin(rx);

    matrix33 rm(
      1,  0,  0,
      0, cx,-sx,
      0, sx, cx );
    *this*=rm;
    return *this;
  }

  matrix33& rotateY(float ry)
  {
    ry*=radian;
    float cy=std::cos(ry), sy=std::sin(ry);

    matrix33 rm(
      cy, 0, sy,
       0, 1,  0,
     -sy, 0, cy );
    *this*=rm;
    return *this;
  }

  matrix33& rotateZ(float rz)
  {
    rz*=radian; 
    float cz=std::cos(rz), sz=std::sin(rz);

    matrix33 rm(
      cz,-sz, 0,
      sz, cz, 0,
       0,  0, 1 );
    *this*=rm;
    return *this;
  }


  template<typename VecType>
  matrix33& rotateA(const VecType &ax, float ang)
  {
    ang*=radian;
    const float &x=ax.x, &y=ax.y, &z=ax.z;
    float c=std::cos(ang), s=std::sin(ang);
    float ic=1-c;

    matrix33 rm(
      c+ic*x*x, ic*x*y-z*s, ic*x*z+y*s,
    ic*x*y+z*s,   c+ic*y*y, ic*y*z-x*s,
    ic*x*z-y*s, ic*y*z+x*s,   c+ic*z*z );
    *this*=rm;
    return *this;
  }


  template<typename VecType>
  matrix33& aimVector(const VecType &vec)
  {
    {
      static const matrix33 turn90degree = matrix33().rotateY(90);
      static const VecType std_vec(0, 1.0f, 0);
      VecType rotate_axis = VecType(vec.x, 0, vec.z);
      if(rotate_axis.square()==0) {
        return *this;
      }
      rotate_axis = (turn90degree*rotate_axis).normalize();
      float ang_xz=std_vec.cos(vec);

      rotateA(rotate_axis, ang_xz/radian);
    }
    {
      static const vector2 std_vec(-1.0f, 0);
      vector2 proj_y=vector2(vec.x, vec.z).normalize();
      float ang_y=std_vec.cos(proj_y);
      if(proj_y.y < 0)
        ang_y*=-1;

      rotateY(ang_y/radian-90);
    }

    return *this;
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<9; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<9; ++i) { b >> v[i]; }
  }
};




struct matrix44
{
  union {
    struct {
        float x0, y0, z0, w0;
        float x1, y1, z1, w1;
        float x2, y2, z2, w2;
        float x3, y3, z3, w3;
    };
    float v[16];
    float m[4][4];
  };


  float& operator [](size_t i) { return v[i]; }
  const float& operator [](size_t i) const { return v[i]; }

  // どうせ*=でもコピーを作らんといかんので値返し
  matrix44 operator * (const matrix44 &mlt) const
  {
    return matrix44(
    v[0]*mlt.v[ 0] + v[4]*mlt.v[ 1] + v[ 8]*mlt.v[ 2] + v[12]*mlt.v[ 3],
    v[0]*mlt.v[ 4] + v[4]*mlt.v[ 5] + v[ 8]*mlt.v[ 6] + v[12]*mlt.v[ 7],
    v[0]*mlt.v[ 8] + v[4]*mlt.v[ 9] + v[ 8]*mlt.v[10] + v[12]*mlt.v[11],
    v[0]*mlt.v[12] + v[4]*mlt.v[13] + v[ 8]*mlt.v[14] + v[12]*mlt.v[15],

    v[1]*mlt.v[ 0] + v[5]*mlt.v[ 1] + v[ 9]*mlt.v[ 2] + v[13]*mlt.v[ 3],
    v[1]*mlt.v[ 4] + v[5]*mlt.v[ 5] + v[ 9]*mlt.v[ 6] + v[13]*mlt.v[ 7],
    v[1]*mlt.v[ 8] + v[5]*mlt.v[ 9] + v[ 9]*mlt.v[10] + v[13]*mlt.v[11],
    v[1]*mlt.v[12] + v[5]*mlt.v[13] + v[ 9]*mlt.v[14] + v[13]*mlt.v[15],

    v[2]*mlt.v[ 0] + v[6]*mlt.v[ 1] + v[10]*mlt.v[ 2] + v[14]*mlt.v[ 3],
    v[2]*mlt.v[ 4] + v[6]*mlt.v[ 5] + v[10]*mlt.v[ 6] + v[14]*mlt.v[ 7],
    v[2]*mlt.v[ 8] + v[6]*mlt.v[ 9] + v[10]*mlt.v[10] + v[14]*mlt.v[11],
    v[2]*mlt.v[12] + v[6]*mlt.v[13] + v[10]*mlt.v[14] + v[14]*mlt.v[15],

    v[3]*mlt.v[ 0] + v[7]*mlt.v[ 1] + v[11]*mlt.v[ 2] + v[15]*mlt.v[ 3],
    v[3]*mlt.v[ 4] + v[7]*mlt.v[ 5] + v[11]*mlt.v[ 6] + v[15]*mlt.v[ 7],
    v[3]*mlt.v[ 8] + v[7]*mlt.v[ 9] + v[11]*mlt.v[10] + v[15]*mlt.v[11],
    v[3]*mlt.v[12] + v[7]*mlt.v[13] + v[11]*mlt.v[14] + v[15]*mlt.v[15] );
  }

  void operator *= (const matrix44 &mlt)
  {
    *this=*this*mlt;
  }

  template<typename VecType>
  VecType operator * (const VecType &vec) const
  {
    return VecType(
      v[0]*vec.x + v[4]*vec.y + v[ 8]*vec.z + v[12]*vec.w,
      v[1]*vec.x + v[5]*vec.y + v[ 9]*vec.z + v[13]*vec.w,
      v[2]*vec.x + v[6]*vec.y + v[10]*vec.z + v[14]*vec.w,
      v[3]*vec.x + v[7]*vec.y + v[11]*vec.z + v[15]*vec.w );
  }



  matrix44()
  {
    identity();
  }

  matrix44(const float *fv):
    x0(fv[ 0]), y0(fv[ 1]), z0(fv[ 2]), w0(fv[3]), 
    x1(fv[ 4]), y1(fv[ 5]), z1(fv[ 6]), w1(fv[7]), 
    x2(fv[ 8]), y2(fv[ 9]), z2(fv[10]), w2(fv[11]), 
    x3(fv[12]), y3(fv[13]), z3(fv[14]), w3(fv[15]) {}

  matrix44(
    float _00, float _04, float _08, float _12,
    float _01, float _05, float _09, float _13,
    float _02, float _06, float _10, float _14,
    float _03, float _07, float _11, float _15):
    x0(_00), y0(_01), z0(_02), w0(_03),
    x1(_04), y1(_05), z1(_06), w1(_07),
    x2(_08), y2(_09), z2(_10), w2(_11),
    x3(_12), y3(_13), z3(_14), w3(_15) {}


  void identity()
  {
    v[0]=1; v[4]=0; v[ 8]=0; v[12]=0;
    v[1]=0; v[5]=1; v[ 9]=0; v[13]=0;
    v[2]=0; v[6]=0; v[10]=1; v[14]=0;
    v[3]=0; v[7]=0; v[11]=0; v[15]=1;
  }

  matrix44& invert()
  {
    const int dim=4;
    int i, j, k;
    float p, q;

    for(k=0; k<dim; k++) {
      p=m[k][k];
      m[k][k]=1.0;
      for(j=0; j<dim; j++)
        m[k][j]/=p;
      for(i=0; i<dim; i++) {
        if(i!=k) {
          q=m[i][k];
          m[i][k]=0.0;
          for(j=0; j<dim; j++)
            m[i][j]-=q*m[k][j];
        }
      }
    }

    return *this;
  }

  // 回転と平行移動だけしかかかってない行列(剛体変換)用の高速な逆行列算出法 
  matrix44& transpose()
  {
    float t;
    t=x1; x1=y0; y0=t;
    t=x2; x2=z0; z0=t;
    t=y2; y2=z1; z1=t;
    matrix44 it(
      1,   0,  0, -x3,
      0,   1,  0, -y3,
      0,   0,  1, -z3,
      0,   0,  0, 1 );
    x3 = 0;
    y3 = 0;
    z3 = 0;
    (*this)*=it;

    return *this;
  }

  matrix44& translate(const float *fv)
  {
    translate(fv[0], fv[1], fv[2]);
    return *this;
  }

  matrix44& translate(float tx, float ty, float tz)
  {
    matrix44 tm(
      1, 0, 0, tx,
      0, 1, 0, ty,
      0, 0, 1, tz,
      0, 0, 0,  1 );
    *this*=tm;
    return *this;
  }


  matrix44& scale(const float *fv)
  {
    scale(fv[0], fv[1], fv[2]);
    return *this;
  }

  matrix44& scale(float sx, float sy, float sz)
  {
    matrix44 sm(
      sx,0, 0, 0,
      0,sy, 0, 0,
      0, 0,sz, 0,
      0, 0, 0, 1 );
    *this*=sm;
    return *this;
  }


  matrix44& rotate(const float *fv)
  {
    rotate(fv[0], fv[1], fv[2]);
    return *this;
  }

  matrix44& rotate(float p, float h, float r)
  {
    p*=radian;
    h*=radian;
    r*=radian; 
    float sp=std::sin(p), cp=std::cos(p);
    float sh=std::sin(h), ch=std::cos(h);
    float sr=std::sin(r), cr=std::cos(r);

    matrix44 em(
      cr*ch-sr*sp*sh,-sr*cp, cr*sh+sr*sp*ch, 0,
      sr*ch+cr*sp*sh, cr*cp, sr*sh-cr*sp*ch, 0,
              -cp*sh,    sp,          cp*ch, 0,
                   0,     0,              0, 1 );

    *this=*this*em;
    return *this;
  }


  matrix44& rotateX(float rx)
  {
    rx*=radian;
    float cx=std::cos(rx), sx=std::sin(rx);

    matrix44 rm(
      1,  0,  0, 0,
      0, cx,-sx, 0,
      0, sx, cx, 0,
      0,  0,  0, 1 );
    *this*=rm;
    return *this;
  }

  matrix44& rotateY(float ry)
  {
    ry*=radian;
    float cy=std::cos(ry), sy=std::sin(ry);

    matrix44 rm(
      cy, 0, sy, 0,
       0, 1,  0, 0,
     -sy, 0, cy, 0,
       0, 0,  0, 1 );
    *this*=rm;
    return *this;
  }

  matrix44& rotateZ(float rz)
  {
    rz*=radian; 
    float cz=std::cos(rz), sz=std::sin(rz);

    matrix44 rm(
      cz,-sz, 0, 0,
      sz, cz, 0, 0,
       0,  0, 1, 0,
       0,  0, 0, 1 );
    *this*=rm;
    return *this;
  }


  template<typename VecType>
  matrix44& rotateA(const VecType &ax, float ang)
  {
    ang*=radian;
    const float &x=ax.x, &y=ax.y, &z=ax.z;
    float c=std::cos(ang), s=std::sin(ang);
    float ic=1-c;

    matrix44 rm(
      c+ic*x*x, ic*x*y-z*s, ic*x*z+y*s, 0,
    ic*x*y+z*s,   c+ic*y*y, ic*y*z-x*s, 0,
    ic*x*z-y*s, ic*y*z+x*s,   c+ic*z*z, 0,
             0,          0,          0, 1 );
    *this*=rm;
    return *this;
  }


  template<typename VecType>
  matrix44& aimVector(const VecType &vec)
  {
    const vector3 up(0, 1.0f, 0);
    vector3 x = vector3(vec.v).normal();
    if(fabsf(x.y)>=1.0f) {
      x.x = 0.001f;
      x.normalize();
    }
    vector3 z = vector3().cross(x, up).normal();
    vector3 y = vector3().cross(z, x).normal();
    matrix44 m = matrix44(
      x.x, y.x, z.x, 0,
      x.y, y.y, z.y, 0,
      x.z, y.z, z.z, 0,
        0,   0,   0, 1
      );
    (*this)*=m;
    return *this;
  }


  template<typename VecType>
  matrix44& aimVector2(const VecType &vec)
  {
    const vector3 up(0, 1.0f, 0);
    vector3 x = vector3(vec.v).normal();
    if(fabsf(x.y)>=1.0f) {
      x.x = 0.001f;
      x.normalize();
    }
    vector3 z = vector3().cross(x, up).normal();
    vector3 y = vector3().cross(z, x).normal();
    matrix44 m = matrix44(
      x.x, y.x, z.x, 0,
      x.y, y.y, z.y, 0,
      x.z, y.z, z.z, 0,
        0,   0,   0, 1
      );
    (*this)*=m;

    if(x.x<0.0f) {
      rotateX(180);
    }
    return *this;
  }


  void serialize(bostream& b) const
  {
    for(size_t i=0; i<16; ++i) { b << v[i]; }
  }

  void deserialize(bistream& b)
  {
    for(size_t i=0; i<16; ++i) { b >> v[i]; }
  }
};


  template<>
  inline vector3 matrix44::operator *<vector3> (const vector3 &vec) const
  {
    return vector3(
      v[0]*vec.x + v[4]*vec.y + v[ 8]*vec.z ,
      v[1]*vec.x + v[5]*vec.y + v[ 9]*vec.z,
      v[2]*vec.x + v[6]*vec.y + v[10]*vec.z);
  }

} // ist



inline ist::bostream& operator<<(ist::bostream& b, const ist::vector2& v)
{
  v.serialize(b);
  return b;
}
inline ist::bostream& operator<<(ist::bostream& b, const ist::vector3& v)
{
  v.serialize(b);
  return b;
}
inline ist::bostream& operator<<(ist::bostream& b, const ist::vector4& v)
{
  v.serialize(b);
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const ist::matrix22& v)
{
  v.serialize(b);
  return b;
}
inline ist::bostream& operator<<(ist::bostream& b, const ist::matrix33& v)
{
  v.serialize(b);
  return b;
}
inline ist::bostream& operator<<(ist::bostream& b, const ist::matrix44& v)
{
  v.serialize(b);
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, ist::vector2& v)
{
  v.deserialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::vector3& v)
{
  v.deserialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::vector4& v)
{
  v.deserialize(b);
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, ist::matrix22& v)
{
  v.deserialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::matrix33& v)
{
  v.deserialize(b);
  return b;
}
inline ist::bistream& operator>>(ist::bistream& b, ist::matrix44& v)
{
  v.deserialize(b);
  return b;
}

#endif // ist_vector 


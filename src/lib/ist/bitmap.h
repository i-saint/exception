#ifndef IST_BITMAP_H
#define IST_BITMAP_H

#include <vector>
#include <string>
#include "ist_conf.h"
#include "array2.h"


namespace ist {

template <typename T>
struct RGBA {
  union {
    struct { T r,g,b,a; };
    T v[4];
  };

  RGBA<T> operator + (RGBA<T> &t) const { return RGBA<T>(r+t.r, g+t.g, b+t.b, a+t.a ); }
  RGBA<T> operator - (RGBA<T> &t) const { return RGBA<T>(r-t.r, g-t.g, b-t.b, a-t.a ); }
  RGBA<T> operator * (RGBA<T> &t) const { return RGBA<T>(r*t.r, g*t.g, b*t.b, a*t.a ); }
  RGBA<T> operator / (RGBA<T> &t) const { return RGBA<T>(r/t.r, g/t.g, b/t.b, a/t.a ); }
  RGBA<T>& operator +=(RGBA<T> &t) { *this=*this+t; return *this; }
  RGBA<T>& operator -=(RGBA<T> &t) { *this=*this-t; return *this; }
  RGBA<T>& operator *=(RGBA<T> &t) { *this=*this*t; return *this; }
  RGBA<T>& operator /=(RGBA<T> &t) { *this=*this/t; return *this; }
  bool operator ==(const RGBA<T> &t) const { return (r==t.r && g==t.g && b==t.b && a==t.a); }
  bool operator !=(const RGBA<T> &t) const { return !(*this==t); }
  T& operator [](int i) { return v[i]; }
  const T& operator [](int i) const { return v[i]; }

  RGBA<T>() : r(0), g(0), b(0), a(0) {}
  RGBA<T>(T _r, T _g, T _b, T _a) : r(_r), g(_g), b(_b), a(_a) {}

  T shin() const { return (r*0.299f + g*0.587f + b*0.114f); }
};

typedef RGBA<unsigned char> bRGBA;
typedef RGBA<float> fRGBA;
inline fRGBA bRGBA_to_fRGBA(const bRGBA& b) { return fRGBA(float(b.r)/255.0f, float(b.g)/255.0f, float(b.b)/255.0f, float(b.a)/255.0f ); }



class IST_CLASS Bitmap : public array2<bRGBA>, public ist::Object
{
public:
  Bitmap() {}
  explicit Bitmap(const std::string& filename);
  Bitmap(size_t w, size_t h) { resize(w, h); }
  Bitmap(Bitmap& v, size_t x, size_t y, size_t width, size_t height) { assign(v, x, y, width, height); }

  bool load(const std::string& filename);
  bool save(const std::string& filename) const;

  bool loadBMP(const std::string& filename);
  bool loadTGA(const std::string& filename);
  bool saveBMP(const std::string& filename) const;
  bool saveTGA(const std::string& filename) const;

  bool loadPNG(const std::string& filename);
  bool savePNG(const std::string& filename) const;
  bool loadJPG(const std::string& filename);
  bool saveJPG(const std::string& filename, int quality=75) const;
};


class IST_CLASS fBitmap1 : public array2<float>, public ist::Object
{
public:
  fBitmap1() { clear(); }
  explicit fBitmap1(const std::string& filename) { clear(); load(filename); }
  fBitmap1(size_t w, size_t h) { clear(); resize(w, h); }

  bool load(const std::string& filename);
  bool save(const std::string& filename) const;
};



} //ist

#endif

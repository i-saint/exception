#ifndef ist_bstream
#define ist_bstream

#define IST_BSTREAM_ENABLE_ZLIB

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#ifdef IST_BSTREAM_ENABLE_ZLIB
  #include <zlib.h>
#endif
#include "ist_conf.h"

namespace ist {


class bistream
{
public:
  virtual ~bistream() {}
  virtual bool read(void *p, size_t s)=0;
  virtual size_t rtell() const=0;
  virtual void rseek(int p)=0;
  virtual void rskip(int p)=0;
  bool eread(void *p, size_t s)
  {
#ifdef IST_BIG_ENDIAN
    char buf[8] = {0,0,0,0,0,0,0,0};
    size_t res = read(buf, s);
    std::reverse(buf, buf+s);
    std::copy(buf, buf+s, p);
    return res;
#else
    return this->read(p, s);
#endif // IST_BIG_ENDIAN
  }
};

class bostream
{
public:
  virtual ~bostream() {}
  virtual void write(const void *p, size_t s)=0;
  virtual size_t wtell() const=0;
  virtual void wseek(int p)=0;
  virtual void wskip(int p)=0;
  void ewrite(const void *p, size_t s)
  {
#ifdef IST_BIG_ENDIAN
    char buf[8] = {0,0,0,0,0,0,0,0};
    std::copy((const char*)p, (const char*)p+s, buf);
    std::reverse(buf, buf+s);
    write(buf, s);
#else
    write(p, s);
#endif // IST_BIG_ENDIAN
  }
};

class bstream : public bistream, public bostream
{
public:
  virtual size_t size() const=0;
  virtual bool eof() const=0;
  virtual bool operator!() const=0;
};



class biostream : public bstream
{
private:
  std::iostream& m_stream;

public:
  biostream(std::iostream& f) : m_stream(f) {}

  void write(const void *p, size_t s)
  {
    m_stream.write((char*)p, std::streamsize(s));
  }

  bool read(void *p, size_t s)
  {
    m_stream.read((char*)p, std::streamsize(s));
    return true;
  }

  size_t wtell() const { return size_t(m_stream.tellp()); }
  void wseek(int p) { m_stream.seekp(p, std::ios::beg); }
  void wskip(int p) { m_stream.seekp(p, std::ios::cur); }

  size_t rtell() const { return size_t(m_stream.tellg()); }
  void rseek(int p) { m_stream.seekg(p, std::ios::beg); }
  void rskip(int p) { m_stream.seekg(p, std::ios::cur); }

  size_t size() const
  {
    std::streampos cur = m_stream.tellg();
    m_stream.seekg(0, std::ios::end);
    std::streampos filesize = m_stream.tellg();
    m_stream.seekg(cur, std::ios::beg);
    return size_t(filesize);
  }

  bool eof() const { return m_stream.eof(); }
  bool operator!() const { return !m_stream; }
};


#ifdef IST_BSTREAM_ENABLE_ZLIB

class gzbstream : public bstream
{
private:
  gzFile m_gz;

public:
  gzbstream(const std::string& path, const std::string& mode)
  {
    m_gz = gzopen(path.c_str(), mode.c_str());
  }

  ~gzbstream()
  {
    if(m_gz) {
      gzclose(m_gz);
    }
  }

  void write(const void *p, size_t s)
  {
    gzwrite(m_gz, p, s);
  }

  bool read(void *p, size_t s)
  {
    return gzread(m_gz, p, s)==s;
  }

  size_t wtell() const { return gztell(m_gz); }
  void wseek(int p) { gzseek(m_gz, SEEK_SET, p); }
  void wskip(int p) { gzseek(m_gz, SEEK_CUR, p); }

  size_t rtell() const { return gztell(m_gz); }
  void rseek(int p) { gzseek(m_gz, SEEK_SET, p); }
  void rskip(int p) { gzseek(m_gz, SEEK_CUR, p); }

  size_t size() const
  {
    std::runtime_error("gzbstream::size() : not implemented");
    return 0;
  }

  bool eof() const { return gzeof(m_gz)==1; }
  bool operator!() const { return !m_gz; }

};

#endif


class bbuffer : public bstream, public std::vector<char>
{
private:
  size_t m_read_pos;
  size_t m_write_pos;

public:
  typedef std::vector<char> BaseClass;

  bbuffer() : m_read_pos(0), m_write_pos(0)
  {
    reserve(128);
  }

  void write(const void *p, size_t s)
  {
    if(m_write_pos+s>size()) {
      resize(m_write_pos+s);
    }
    memcpy(ptr()+m_write_pos, p, s);
    m_write_pos+=s;
  }

  bool read(void *p, size_t s)
  {
    if(m_read_pos+s>size()) {
      m_read_pos = size();
      return false;
    }
    memcpy(p, ptr()+m_read_pos, s);
    m_read_pos+=s;
    return true;
  }

  size_t wtell() const { return m_write_pos; }
  void wseek(int i) { m_write_pos=size_t(i); }
  void wskip(int i) { m_write_pos+=i; }

  size_t rtell() const { return m_read_pos; }
  void rseek(int i) { m_read_pos=size_t(i); }
  void rskip(int i) { m_read_pos+=i; }

  size_t size() const { return BaseClass::size(); }
  bool eof() const { return m_read_pos>=size(); }
  bool operator!() const { return m_read_pos>=size(); }


  void clear()
  {
    std::vector<char>::clear();
    m_read_pos = 0;
    m_write_pos = 0;
  }

  bool load(const std::string& path)
  {
    std::ifstream in(path.c_str(), std::ios::binary);
    if(!in) {
      return false;
    }
    size_t size = 0;
    in.seekg(0, std::ios::end);
    size = in.tellg();
    resize(size);
    in.seekg(0, std::ios::beg);
    in.read(ptr(), std::streamsize(size));
    return true;
  }

  bool save(const std::string& path) const
  {
    std::ofstream out(path.c_str(), std::ios::binary);
    if(!out) {
      return false;
    }
    out.write(ptr(), std::streamsize(size()));
    return true;
  }


  char* ptr() { return &(*this)[0]; }
  const char* ptr() const { return &(*this)[0]; }
  std::string str() const { return std::string(ptr(), size()); }
};


template<class T>
inline void serialize_container(ist::bostream& b, const T& v)
{
  b << v.size();
  for(typename T::const_iterator i=v.begin(); i!=v.end(); ++i) {
    b << *i;
  }
}

template<class T>
inline void deserialize_container(ist::bistream& b, T& v)
{
  v.clear();
  size_t size;
  b >> size;
  typename T::value_type tmp;
  for(size_t i=0; i<size; ++i) {
    b >> tmp;
    v.push_back(tmp);
  }
}

template<class T>
inline void serialize_object_container(ist::bostream& b, const T& v)
{
  b << v.size();
  for(typename T::const_iterator i=v.begin(); i!=v.end(); ++i) {
    i->serialize(b);
  }
}

template<class T>
inline void deserialize_object_container(ist::bistream& b, T& v)
{
  v.clear();
  size_t size;
  b >> size;
  typename T::value_type tmp;
  for(size_t i=0; i<size; ++i) {
    tmp.deserialize(b);
    v.push_back(tmp);
  }
}
} // namespace ist 




inline void operator+=(ist::bbuffer& b, const ist::bbuffer& a) { b.write(a.ptr(), a.size()); }

template<typename T, size_t N>
inline ist::bostream& operator<<(ist::bostream& b, const T (&v)[N])
{
  for(size_t i=0; i<N; ++i) {
    b << v[i];
  }
  return b;
}

template<typename T, size_t N>
inline ist::bistream& operator>>(ist::bistream& b, T (&v)[N])
{
  for(size_t i=0; i<N; ++i) {
    b >> v[i];
  }
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, char v)          { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, unsigned char v) { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, short v)         { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, unsigned short v){ b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, int v)           { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, unsigned int v)  { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, float v)         { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, double v)        { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, bool v)          { b.ewrite(&v, sizeof(v)); return b; }
inline ist::bostream& operator<<(ist::bostream& b, wchar_t v)       { b.ewrite(&v, sizeof(v)); return b; }

inline ist::bistream& operator>>(ist::bistream& b, char& v)          { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, unsigned char& v) { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, short& v)         { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, unsigned short& v){ b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, int& v)           { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, unsigned int& v)  { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, float& v)         { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, double& v)        { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, bool& v)          { b.eread(&v, sizeof(v)); return b; }
inline ist::bistream& operator>>(ist::bistream& b, wchar_t& v)       { b.eread(&v, sizeof(v)); return b; }


inline ist::bostream& operator<<(ist::bostream& b, const ist::bbuffer& v)
{
  b << v.size();
  b.write(v.ptr(), v.size());
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, ist::bbuffer& v)
{
  size_t size;
  b >> size;

  v.clear();
  v.resize(size);
  b.read(v.ptr(), size);
  return b;
}


inline ist::bostream& operator<<(ist::bostream& b, const std::string& v)
{
  b << v.size();
  for(size_t i=0; i<v.size(); ++i) {
    b << v[i];
  }
  return b;
}

inline ist::bostream& operator<<(ist::bostream& b, const std::wstring& v)
{
  b << v.size();
  for(size_t i=0; i<v.size(); ++i) {
    b << v[i];
  }
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, std::string& v)
{
  size_t size;
  b >> size;
  v.resize(size);
  for(size_t i=0; i<size; ++i) { b >> v[i]; }
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, std::wstring& v)
{
  size_t size;
  b >> size;
  v.resize(size);
  for(size_t i=0; i<size; ++i) { b >> v[i]; }
  return b;
}


inline std::iostream& operator<<(std::iostream& ios, const ist::bbuffer& v)
{
  size_t size = v.size();
  ios.write((char*)&size, sizeof(size_t));
  ios.write(v.ptr(), std::streamsize(v.size()));
  return ios;
}

inline std::iostream& operator>>(std::iostream& ios, ist::bbuffer& v)
{
  v.clear();
  size_t size = 0;
  ios.read((char*)&size, std::streamsize(sizeof(size_t)));
  v.clear();
  v.resize(size);
  ios.read(v.ptr(), size);
  return ios;
}


#endif // ist_bstream 

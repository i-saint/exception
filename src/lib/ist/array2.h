#ifndef IST_ARRAY2_H
#define IST_ARRAY2_H


namespace ist {

template<class T>
class array2
{
public:
  typedef T value_type;
  typedef std::vector<value_type> container_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;

  array2() : m_width(0), m_height(0) {}
  array2(size_t w, size_t h) : m_width(0), m_height(0) { resize(w, h); }

  void assign(const array2& v, size_t x, size_t y, size_t width, size_t height) {
    resize(width, height);
    copy(0, 0, v, x, y, width, height);
  }

  void copy(size_t sx, size_t sy, const array2& v, size_t x, size_t y, size_t width, size_t height) {
    for(size_t i=y; i<y+height; ++i) {
      if(sy>=getHeight() || i>=v.getHeight())
        break;
      size_t sbx = sx;
      for(size_t j=x; j<x+width; ++j) {
        if(sx>=getWidth() || j>=v.getWidth())
          break;
        (*this)[sy][sx] = v[i][j];
        ++sx;
      }
      sx = sbx;
      ++sy;
    }
  }

  bool empty() const { return data.empty(); }
  void resize(size_t w, size_t h) { m_width=w; m_height=h; data.resize(m_width*m_height); }
  void clear() { m_width=0; m_height=0; data.clear(); }
  iterator begin() { return data.begin(); }
  iterator end()   { return data.end(); }
  const_iterator begin() const { return data.begin(); }
  const_iterator end()   const { return data.end(); }

  // bmp[y][x] 
  T* operator [] (size_t i) { return &data[m_width*i]; }
  const T* operator [] (size_t i) const { return &data[m_width*i]; }
  T& at(size_t w, size_t h) { return data[m_width*h+w]; }
  const T& at(size_t w, size_t h) const { return data[m_width*h+w]; }
  T& at(size_t i) { return data[i]; }
  const T& at(size_t i) const { return data[i]; }

  size_t getWidth() const { return m_width; }
  size_t getHeight() const { return m_height; }


private:
  size_t m_width, m_height;
  container_type data;
};


template<class T>
class array3
{
public:
  typedef T value_type;
  typedef std::vector<value_type> container_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
private:
  size_t m_width;
  size_t m_height;
  size_t m_length;
  container_type data;

public:
  array3() : m_width(0), m_height(0), m_length(0) {}
  array3(size_t w, size_t h, size_t l) : m_width(0), m_height(0), m_length(0) { resize(w, h, l); }

  bool empty() const { return data.empty(); }
  size_t size() const { return data.size(); }
  void resize(size_t w, size_t h, size_t l) { m_width=w; m_height=h; m_length=l; data.resize(m_width*m_height*m_length); }
  void clear() { m_width=0; m_height=0;  m_length=0; data.clear(); }
  iterator begin() { return data.begin(); }
  iterator end()   { return data.end(); }
  const_iterator begin() const { return data.begin(); }
  const_iterator end()   const { return data.end(); }

  size_t getWidth() const { return m_width; }
  size_t getLength() const { return m_length; }
  size_t getHeight() const { return m_height; }

  T& at(size_t w, size_t h, size_t l) { return data[m_width*m_length*h + m_width*l + w]; }
  const T& at(size_t w, size_t h, size_t l) const { return data[m_width*m_length*h + m_width*l + w]; }
  T& at(size_t i) { return data[i]; }
  const T& at(size_t i) const { return data[i]; }
};

} // ist

#endif

#ifndef IST_GRID_H
#define IST_GRID_H

#include <vector>
#include "ist/vector.h"
#include "iterator.h"
#include "array2.h"

namespace ist {


template<class T>
class grid_element : public std::vector<T>
{
typedef std::vector<T> Super;
public:
  void insert(const T& value)
  {
    Super::push_back(value);
  }

  void erase(const T& value)
  {
    typename Super::iterator i = std::find(this->begin(), this->end(), value);
    if(i!=this->end()) {
      Super::erase(i);
    }
  }
};




template<
  class T,
  class C = grid_element<T>
>
class grid_structure3
{
public:
  typedef T value_type;
  typedef C block_type;
  typedef array3<block_type> block_cont;

  class point3
  {
  public:
    int x, y, z;

    point3(int x_=0, int _y=0, int z_=0) : x(x_), y(_y), z(z_) {}
    point3 operator+(const point3& v) { return point3(x+v.x, y+v.y, z+v.z); }
  };

private:
  block_cont m_blocks;
  int m_xdiv;
  int m_ydiv;
  int m_zdiv;
  vector4 m_bl;
  vector4 m_ur;
  vector4 m_area;
  vector4 m_cell;

public:
  grid_structure3() : m_xdiv(0), m_ydiv(0), m_zdiv(0)
  {
    m_cell = vector4(100.0f, 100.0f, 100.0f); // 0œŽZ–hŽ~ 
  }

  grid_structure3(const vector3& ur, const vector2& bl, int xd, int yd, int zd)
  {
    resize(ur, bl, xd, yd, zd);
  }

  size_t size() const { return m_blocks.size(); }
  bool empty() const { return m_blocks.empty(); }

  void resize(const vector4& ur, const vector4& bl, int xd, int yd, int zd)
  {
    m_ur = ur;
    m_bl = bl;
    m_xdiv = xd;
    m_ydiv = yd;
    m_zdiv = zd;
    m_area = m_ur-m_bl;
    m_cell.x = m_area.x/m_xdiv;
    m_cell.y = m_area.y/m_ydiv;
    m_cell.z = m_area.z/m_zdiv;
    m_blocks.resize(m_xdiv, m_ydiv, m_zdiv);
  }


  struct inserter
  {
    const value_type& m_value;
    inserter(const value_type& v) : m_value(v) {}
    void operator()(block_type& b) const
    {
      b.insert(m_value);
    }
  };

  void insert(const Box& box, const value_type& value)
  {
    eachBlocks(box, inserter(value));
  }


  struct eraser
  {
    const value_type& m_value;
    eraser(const value_type& v) : m_value(v) {}
    void operator()(block_type& b) const
    {
      b.erase(m_value);
    }
  };

  void erase(const Box& box, const value_type& value)
  {
    eachBlocks(box, eraser(value));
  }


  void clear()
  {
    for(typename block_cont::iterator p=m_blocks.begin(); p!=m_blocks.end(); ++p) {
      p->clear();
    }
  }


  int getXDivision() const { return m_xdiv; }
  int getYDivision() const { return m_ydiv; }
  int getZDivision() const { return m_zdiv; }
  const vector4& getBottomLeft() const { return m_bl; }
  const vector4& getUpperRight() const { return m_ur; }

  point3 getCoord(const vector4& pos) const
  {
    vector4 rel = pos-m_bl;
    return point3(int(rel.x/m_cell.x), int(rel.y/m_cell.y), int(rel.z/m_cell.z));
  }

  block_type& getBlock(size_t i)
  {
    return m_blocks.at(i);
  }

  const block_type& getBlock(size_t i) const
  {
    return m_blocks.at(i);
  }

  block_type* getBlock(const point3& coord)
  {
    int x=coord.x, y=coord.y, z=coord.z;
    if(x<0 || y<0 || z<0 || x>=m_xdiv || y>=m_ydiv || z>=m_zdiv) {
      return 0;
    }
    return &m_blocks.at(x, y, z);
  }

  block_type* getBlock(const vector4& pos)
  {
    return getBlock(getCoord(pos));
  }


  template<class Functer>
  void eachBlocks(const point3& ur, const point3& bl, const Functer& f)
  {
    for(int i=bl.z; i<ur.z; ++i) {
      for(int j=bl.y; j<ur.y; ++j) {
        for(int k=bl.x; k<ur.x; ++k) {
          if(block_type *b = getBlock(point3(k, j, i))) {
            f(*b);
          }
        }
      }
    }
  }

  template<class Functer>
  void eachBlocks(const ist::Box& box, const Functer& f)
  {
    eachBlocks(
      getCoord(box.getUpperRight())+point3(1,1,1),
      getCoord(box.getBottomLeft()),
      f);
  }

  template<class Functer>
  void eachBlocks(const ist::Sphere& sphere, const Functer& f)
  {
    const vector4& pos = sphere.getPosition();
    float radius = sphere.getRadius();
    eachBlocks(
      getCoord(vector4(pos.x+radius, pos.y+radius, pos.z+radius))+point3(1,1,1),
      getCoord(vector4(pos.x-radius, pos.y-radius, pos.z-radius)),
      f);
  }


  template<class Functer>
  void eachBlocks(const Functer& f)
  {
    for(typename block_cont::iterator p=m_blocks.begin(); p!=m_blocks.end(); ++p) {
      f(*p);
    }
  }
};



} // namespace ist 

#endif // IST_GRID_H

#ifndef IST_SPLINE
#define IST_SPLINE

#include <vector>
#include <utility>
#include "vector.h"
#include "bstream.h"


namespace ist {

class Spline2D
{
public:
  struct point_t
  {
    vector2 in, pos, out;

    point_t() {}
    point_t(const vector2& p) : in(p), pos(p), out(p) {}
    point_t(const vector2& i, const vector2& p, const vector2& o) : in(i), pos(p), out(o) {}

    void serialize(bostream& b) const { b << in << pos << out; }
    void deserialize(bistream& b)     { b >> in >> pos >> out; }
  };

  template<typename T>
  struct less_equal_elem_x {
    bool operator()(const T& lhs, const T& rhs) {
      return lhs.pos.x <= rhs.pos.x;
    }
  };

  typedef std::vector<point_t> point_cont;
  typedef point_cont::iterator iterator;
  typedef std::pair<iterator, iterator> range_t;

private:
  point_cont m_points;
  std::vector<float> m_dist;
  bool m_mod;

public:
  Spline2D() : m_mod(false) {}
  size_t size() const { return m_points.size(); }
  point_t& operator[](size_t i) { return m_points[i]; }
  const point_t& operator[](size_t i) const { return m_points[i]; }
  point_t& front() { return m_points.front(); }
  const point_t& front() const { return m_points.front(); }
  point_t& back() { return m_points.back(); }
  const point_t& back() const { return m_points.back(); }

  bool empty() const { return m_points.empty(); }
  void resize(size_t s) { m_points.resize(s); m_mod=true; }
  void clear() { m_points.clear(); }
  void erase(iterator i) { m_points.erase(i); }
  void sort() { std::sort(m_points.begin(), m_points.end(), less_equal_elem_x<point_t>()); }
  iterator begin() { return m_points.begin(); }
  iterator end()   { return m_points.end(); }


  range_t get_edge(float x)
  {
    range_t pi;

    if(m_points.empty()) {
      pi.first = end();
      pi.second = end();
    }
    else if(m_points.size()==1) {
      if(x > m_points.front().pos.x) {
        pi.first = end();
        pi.second = begin();
      }
      else {
        pi.first = begin();
        pi.second = end();
      }
    }
    else {
      pi = std::equal_range(begin(), end(), point_t(vector2(x, 0)), less_equal_elem_x<point_t>());
      --pi.first;
    }

    return pi;
  }

  void insert(const vector2& p)
  {
    m_points.push_back(point_t(p));
    m_mod = true;

    range_t r = get_edge(p.x);
    if(r.first!=end() && r.first->out.x > p.x) {
      r.first->out.x = p.x;
    }
    if(r.second!=end() && r.second->in.x < p.x) {
      r.second->in.x = p.x;
    }
  }

  void insert(const vector2& i, const vector2& p, const vector2& o)
  {
    m_points.push_back(point_t(i, p, o));
    m_mod = true;
  }

  vector2 linear(float x)
  {
    if(m_points.empty()) {
      return vector2();
    }
    else if(x<=m_points.front().pos.x) {
      return vector2(x, m_points.front().pos.y);
    }
    else if(x>=m_points.back().pos.x) {
      return vector2(x, m_points.back().pos.y);
    }

    range_t range = get_edge(x);
    point_t& prev = *range.first;
    point_t& next = *range.second;

    float dir = (next.pos.y - prev.pos.y)/(next.pos.x - prev.pos.x);
    return vector2(x, prev.pos.y + dir*(x-prev.pos.x));
  }

  vector2 bezier(float x)
  {
    if(m_points.empty()) {
      return vector2();
    }
    else if(x<=m_points.front().pos.x) {
      return vector2(x, m_points.front().pos.y);
    }
    else if(x>=m_points.back().pos.x) {
      return vector2(x, m_points.back().pos.y);
    }

    range_t range = get_edge(x);
    point_t& prev=*range.first;
    point_t& next=*range.second;

    int sw=0;
    if(next.pos==next.in)  sw|=1;
    if(prev.pos==prev.out) sw|=2;

    float u = (x - prev.pos.x) / (next.pos.x - prev.pos.x);

    if(sw!=0) {
      if(sw==3) {
        float dir=(next.pos.y-prev.pos.y)/(next.pos.x-prev.pos.x);
        return vector2(x, prev.pos.y + dir*(x-prev.pos.x));
      }
      else {
        float w[3] = {
        (1-u)*(1-u),
            u*(1-u) * 2,
            u*   u,
        };

        if(sw==1)
          return vector2(
            (w[0]*prev.pos.x + w[1]*prev.out.x + w[2]*next.pos.x),
            (w[0]*prev.pos.y + w[1]*prev.out.y + w[2]*next.pos.y));
        if(sw==2)
          return vector2(
            (w[0]*prev.pos.x + w[1]*next.in.x + w[2]*next.pos.x),
            (w[0]*prev.pos.y + w[1]*next.in.y + w[2]*next.pos.y));
      }
    }
    else {
      float w[4] = {
      (1-u)*(1-u)*(1-u),
          u*(1-u)*(1-u)*3,
          u*    u*(1-u)*3,
          u*    u*   u,
      };

      return vector2(
        (w[0]*prev.pos.x + w[1]*prev.out.x + w[2]*next.in.x + w[3]*next.pos.x),
        (w[0]*prev.pos.y + w[1]*prev.out.y + w[2]*next.in.y + w[3]*next.pos.y));
    }

    return vector2();
  }



  void calcDistance()
  {
    m_mod = false;
    m_dist.resize(m_points.size()-1);
    for(size_t i=0; i<m_points.size()-1; ++i) {
      float d = 0.0f;
      vector2 pre = m_points[i].pos;
      for(int j=1; j<=100; ++j) {
        vector2 cur = bezierS(float(i)+(float(j)/100.0f));
        d+=(cur-pre).norm();
        pre = cur;
      }
      m_dist[i] = d;
    }
  }

  float advance(float current, float progress)
  {
    if(m_mod) {
      calcDistance();
    }

    size_t i = size_t(current);
    if(i < m_dist.size()) {
      return current+progress/m_dist[i];
    }
    return current;
  }

  vector2 linearS(float s)
  {
    int i = int(s);
    float u = fmod(s, 1.0f);
    if(empty()) {
      return vector2();
    }
    else if(i<0) {
      return front().pos;
    }
    else if(i>=size()) {
      return back().pos;
    }
    else if(u==0.0f) {
      return m_points[i].pos;
    }

    point_t& prev = m_points[i];
    point_t& next = m_points[i+1];

    vector2 dir = (next.pos - prev.pos);
    return prev.pos+(dir*u);
  }

  vector2 bezierS(float s)
  {
    int i = int(s);
    float u = fmod(s, 1.0f);
    if(empty()) {
      return vector2();
    }
    else if(i<0) {
      return front().pos;
    }
    else if(i>=size()-1) {
      return back().pos;
    }
    else if(u==0.0f) {
      return m_points[i].pos;
    }

    point_t& prev = m_points[i];
    point_t& next = m_points[i+1];

    int sw = 0;
    if(next.pos==next.in)  sw|=1;
    if(prev.pos==prev.out) sw|=2;

    if(sw!=0) {
      if(sw==3) {
        vector2 dir = (next.pos - prev.pos);
        return prev.pos+(dir*u);
      }
      else {
        float w[3] = {
        (1-u)*(1-u),
            u*(1-u) * 2,
            u*   u,
        };

        if(sw==1)
          return vector2(
            (w[0]*prev.pos.x + w[1]*prev.out.x + w[2]*next.pos.x),
            (w[0]*prev.pos.y + w[1]*prev.out.y + w[2]*next.pos.y));
        if(sw==2)
          return vector2(
            (w[0]*prev.pos.x + w[1]*next.in.x + w[2]*next.pos.x),
            (w[0]*prev.pos.y + w[1]*next.in.y + w[2]*next.pos.y));
      }
    }
    else {
      float w[4] = {
      (1-u)*(1-u)*(1-u),
          u*(1-u)*(1-u)*3,
          u*    u*(1-u)*3,
          u*    u*   u,
      };

      return vector2(
        (w[0]*prev.pos.x + w[1]*prev.out.x + w[2]*next.in.x + w[3]*next.pos.x),
        (w[0]*prev.pos.y + w[1]*prev.out.y + w[2]*next.in.y + w[3]*next.pos.y));
    }
    return vector2();
  }

  void serialize(bostream& b) const
  {
    ist::serialize_object_container(b, m_points);
    ist::serialize_container(b, m_dist);
    b << m_mod;
  }

  void deserialize(bistream& b)
  {
    ist::deserialize_object_container(b, m_points);
    ist::deserialize_container(b, m_dist);
    b >> m_mod;
  }
};

} // namespace ist


inline ist::bostream& operator<<(ist::bostream& b, const ist::Spline2D& v)
{
  v.serialize(b);
  return b;
}

inline ist::bistream& operator>>(ist::bistream& b, ist::Spline2D& v)
{
  v.deserialize(b);
  return b;
}

#endif

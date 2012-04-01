#ifndef IST_ITERATOR_H
#define IST_ITERATOR_H

#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>

namespace ist {


struct non_const_tag {};
struct const_tag {};



// インターフェース 

template<class T>
class IIterator
{
public:
  typedef T value_type;

  virtual ~IIterator() {}
  virtual void reset()=0;
  virtual bool has_next()=0;
  virtual value_type& iterate()=0;
};

template<class T>
class IConstIterator
{
public:
  typedef T value_type;

  virtual ~IConstIterator() {}
  virtual void reset()=0;
  virtual bool has_next()=0;
  virtual const value_type& iterate()=0;
};



namespace impl {

  // ConstTagにconst_tagを指定したらconst Tが返る 
  template<class T, class ConstTag>
  struct as_const {
    typedef T type;
  };

  template<class T>
  struct as_const<T, const_tag> {
    typedef const T type;
  };


  // ConstTagにconst_tagを指定したらconst_iteratorが返る 
  template<class C, class ConstTag>
  struct as_const_iterator {
    typedef typename C::iterator type;
  };

  template<class C>
  struct as_const_iterator<C, const_tag> {
    typedef typename C::const_iterator type;
  };




  // STLコンテナ内蔵型
  template <
    class Container,
    class ConstTag = non_const_tag
  >
  class Position
  {
  public:
    typedef typename as_const<Container, ConstTag>::type container;
    typedef typename as_const_iterator<Container, ConstTag>::type iterator;

    Position() : m_cont(0) {}

    void reset()
    {
      if(m_cont) {
        reset(m_cont->begin(), m_cont->end());
      }
    }

    void reset(container& c)
    {
      m_cont = &c;
      reset();
    }

    void reset(iterator b, iterator e)
    {
      m_begin = b;
      m_end = e;
      m_it = m_begin;
    }

    bool has_next()
    {
      if(m_it!=m_end) {
        return true;
      }
      reset();
      return false;
    }

    iterator _iterate()
    {
      m_cur = m_it;
      return m_it++;
    }

    container& getContainer() { return *m_cont; }
    const container& getContainer() const { return *m_cont; }
    iterator begin() { return m_begin; }
    iterator end() { return m_end; }
    iterator m_current() { return m_it; }

  private:
    container *m_cont;
    iterator m_begin;
    iterator m_end;
    iterator m_it;
    iterator m_cur;
  };


  template <class Container, class ConstTag = non_const_tag>
  class ContainerPosition : public Position<Container, ConstTag>
  {
  public:
    typedef typename as_const<Container, ConstTag>::type container;
    typedef typename as_const<typename Container::value_type, ConstTag>::type value_type;
    ContainerPosition() {}
    ContainerPosition(container& c) { this->reset(c); }
    value_type& iterate() { return *this->_iterate(); }
  };


  template <class Container, class ConstTag = non_const_tag>
  class MapContainerPosition : public Position<Container, ConstTag>
  {
  public:
    typedef typename as_const<Container, ConstTag>::type container;
    typedef typename as_const<typename Container::mapped_type, ConstTag>::type value_type;
    MapContainerPosition() {}
    MapContainerPosition(container& c) { this->reset(c); }
    value_type& iterate() { return this->_iterate()->second; }
  };



  template<class ValueType, class ConstTag>
  class IteratorType : public IIterator<ValueType>
  {
  public:
    typedef IIterator<ValueType> Interface;
  };

  template<class ValueType>
  class IteratorType<ValueType, const_tag> : public IConstIterator<ValueType>
  {
  public:
    typedef IConstIterator<ValueType> Interface;
  };
} // impl 




// ContainerIterator 

template<
  class Container,
  class ConstTag = non_const_tag,
  class ValueType = typename Container::value_type,
  class Position = impl::ContainerPosition<Container, ConstTag>
>
class ContainerIterator : public impl::IteratorType<ValueType, ConstTag>
{
public:
  typedef ConstTag const_t;
  typedef typename impl::as_const<Container, ConstTag>::type container;
  typedef Position pos_t;
  typedef typename pos_t::iterator iterator;
  typedef typename pos_t::value_type value_type;

  void reset() { m_pos.reset(); }
  bool has_next() { return m_pos.has_next(); }
  value_type& iterate() { return m_pos.iterate(); }

  ContainerIterator() {}
  ContainerIterator(container& c) { reset(c); }
  void reset(container& c) { m_pos.reset(c); }
  void reset(iterator b, iterator e) { m_pos.reset(b, e); }
  iterator begin() { return m_pos.begin(); }
  iterator end() { return m_pos.end(); }
  iterator m_current() { return m_pos.m_current(); }

protected:
  pos_t m_pos;
};


// MapContainerIterator 

template<
  class Container,
  class ConstTag = non_const_tag,
  class ValueType = typename Container::mapped_type,
  class Position = impl::MapContainerPosition<Container, ConstTag>
>
class MapContainerIterator : public ContainerIterator<Container, ConstTag, ValueType, Position>
{
public:
  typedef typename impl::as_const<Container, ConstTag>::type container;

  MapContainerIterator() {}
  MapContainerIterator(container& c) { this->reset(c); }
};





// MultiContainerIterator 
// 同じタイプのコンテナを複数詰め込めるモノ 

template <
  class Container,
  class ConstTag = non_const_tag,
  class ValueType = typename Container::value_type,
  class Position = impl::ContainerPosition<Container, ConstTag>
>
class MultiContainerIterator : public impl::IteratorType<ValueType, ConstTag>
{
public:
  typedef ConstTag const_t;
  typedef typename impl::as_const<Container, ConstTag>::type container;
  typedef Position pos_t;
  typedef typename pos_t::value_type value_type;
  typedef std::vector<pos_t> pos_cont;
  typedef typename pos_cont::iterator iterator;

public:
  MultiContainerIterator() : m_current(0) {}

  void reset() {
    for(size_t i=0; i<m_poses.size(); ++i)
      m_poses[i].reset();
    m_current = 0;
  }

  bool has_next() {
    while(m_current<m_poses.size()) {
      if(m_poses[m_current].has_next())
        return true;
      ++m_current;
    }
    m_current = 0;
    return false;
  }

  value_type& iterate() {
    return m_poses[m_current].iterate();
  }


  void push_back(container& c) {
    m_poses.push_back(pos_t(c));
  }

  void push_back(const pos_t& p) {
    m_poses.push_back(p);
    m_poses.back().reset();
  }

  void clear() {
    m_poses.clear();
    m_current = 0;
  }

  size_t size() const { return m_poses.size(); }
  container& operator[](size_t i) { return m_poses[i].getContainer(); }
  iterator begin() { return m_poses.begin(); }
  iterator end() { return m_poses.end(); }

private:
  pos_cont m_poses;
  size_t m_current;
};



// 色んなタイプのコンテナを詰め込めるモノ 
template <
  class T,
  class ConstTag = non_const_tag
>
class MultiTypeIterator : public impl::IteratorType<T, ConstTag>
{
public:
  typedef typename impl::IteratorType<T, ConstTag>::Interface Interface;
  typedef T value_type;
  typedef boost::shared_ptr<Interface> i_ptr;
  typedef std::vector<i_ptr> i_ptr_cont;
  typedef typename i_ptr_cont::iterator iterator;

public:
  MultiTypeIterator() : m_current(0) {}

  void reset() {
    for(size_t i=0; i<m_its.size(); ++i)
      m_its[i]->reset();
    m_current = 0;
  }

  bool has_next() {
    while(m_current<m_its.size()) {
      if(m_its[m_current]->has_next())
        return true;
      ++m_current;
    }
    m_current = 0;
    return false;
  }

  value_type& iterate() {
    return m_its[m_current]->iterate();
  }


  void push_back(Interface *p) {
    m_its.push_back(i_ptr(p));
  }

  template<class Iterator>
  void push_back_iterator(const Iterator& p) {
    m_its.push_back(i_ptr(new Iterator(p)));
  }

  template<class Container>
  void push_back_container(Container& c) {
    m_its.push_back(i_ptr(new ContainerIterator<Container, ConstTag>(c)));
  }

  template<class Container>
  void push_back_map(Container& c) {
    m_its.push_back(i_ptr(new MapContainerIterator<Container, ConstTag>(c)));
  }

  void clear() {
    m_its.clear();
    m_current = 0;
  }

  size_t size() { return m_its.size(); }
  Interface& operator[](size_t i) { return *m_its[i]; }
  iterator begin() { return m_its.begin(); }
  iterator end() { return m_its.end(); }

private:
  i_ptr_cont m_its;
  size_t m_current;
};


} // namespace ist

#endif // IST_ITERATOR_H

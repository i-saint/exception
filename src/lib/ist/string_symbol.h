/*
  http://www.s34.co.jp/cpptechdoc/misc/stx/basic_symbol.html
*/

#include <set>
#include <string>

namespace stx {

  template<class Key, class Comp =std::less<Key> >
  class basic_symbol {
  public:
    typedef Key       key_type;
    typedef key_type* hash_type;

    basic_symbol()
      { k = &(*key_pool().insert(key_type()).first); }
    basic_symbol(const key_type& key)
      { k = &(*key_pool().insert(key).first); }
    basic_symbol(const basic_symbol& sym)
      : k(sym.k) {}

    basic_symbol& operator=(const basic_symbol& sym)
      { k = sym.k; return *this; }
    basic_symbol& operator=(const key_type& key)
      { k = &(*key_pool().insert(key).first); return *this; }

    const key_type& key() const  { return *k; }
    hash_type hash() const       { return hash_type(k); }
  
  private: 
    typedef std::set<key_type, Comp> pool_allocator;
    static pool_allocator& key_pool();
    const key_type* k;

  };

  template<class Key, class Comp>
  typename basic_symbol<Key,Comp>::pool_allocator& 
  basic_symbol<Key,Comp>::key_pool() {
    static pool_allocator pool;
    return pool;
  }

  template<class K, class C>
  inline bool operator==(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() == y.hash(); }

  template<class K, class C>
  inline bool operator!=(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() != y.hash(); }

  template<class K, class C>
  inline bool operator<(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() < y.hash(); }

  template<class K, class C>
  inline bool operator<=(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() <= y.hash(); }

  template<class K, class C>
  inline bool operator>(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() > y.hash(); }

  template<class K, class C>
  inline bool operator>=(const basic_symbol<K,C>& x, const basic_symbol<K,C>& y)
      { return x.hash() >= y.hash(); }

  typedef basic_symbol<std::string>  string_symbol;
  typedef basic_symbol<std::wstring> wstring_symbol;

}

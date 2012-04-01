#ifndef IST_OBJECT_H
#define IST_OBJECT_H

#include <stdexcept>
#include <boost/intrusive_ptr.hpp>
#include "ist_conf.h"

namespace ist {

class IST_CLASS Object
{
public:
  Object();
  virtual ~Object();

  void addRef();
  void release();
  size_t getRefCount();

#ifdef IST_BUILD_DSO
  void* operator new(size_t size);
  void operator delete(void *p);
#endif // IST_BUILD_DSO

private:
  size_t ref_count;
};

class IST_CLASS Error : public std::runtime_error
{
public:
  Error(const std::string& message) : std::runtime_error(message) {}
};

} // ist

inline void intrusive_ptr_add_ref(ist::Object *p) { p->addRef(); }
inline void intrusive_ptr_release(ist::Object *p) { p->release(); }

#endif

#include "ist_object.h"


namespace ist {

Object::Object() : ref_count(0)
{}

Object::~Object()
{}

void Object::addRef()
{
  ++ref_count;
}

void Object::release() {
  if(--ref_count==0)
    delete this;
}

size_t Object::getRefCount() {
  return ref_count;
}


#ifdef IST_BUILD_DSO
void* Object::operator new(size_t size)
{
  return malloc(size);
}

void Object::operator delete(void *p)
{
  return free(p);
}
#endif

}

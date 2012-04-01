#ifdef _WIN32
  #include <windows.h>
#else
  #include <dlfcn.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <cstdio>
#include <zlib.h>
#include "ist_sys.h"



#define IGZ_VERSION 100

namespace ist {

/*
  Directory
*/


bool IsFile(const std::string& path) {
#ifdef _WIN32
  DWORD ret = ::GetFileAttributes(path.c_str());
  return (ret!=(DWORD)-1) && !(ret & FILE_ATTRIBUTE_DIRECTORY);
#else
  struct stat st;
  return ::stat(path.c_str(), &st)==0 && (st.st_mode & S_IFREG);
#endif
}

bool IsDir(const std::string& path) {
#ifdef _WIN32
  DWORD ret = ::GetFileAttributes(path.c_str());
  return (ret!=(DWORD)-1) && (::GetFileAttributes(path.c_str())&FILE_ATTRIBUTE_DIRECTORY)!=0;
#else
  struct stat st;
  return ::stat(path.c_str(), &st)==0 && st.st_mode==S_IFDIR;
#endif
}

bool MakeDir(const std::string& path) {
#ifdef _WIN32
  return ::CreateDirectory(path.c_str(), NULL)==TRUE;
#else
  return ::mkdir(path.c_str(), 0777)==0;
#endif
}

bool MakeDeepDir(const std::string& path) {
  size_t num = 0;
  for(size_t i=0; i<=path.size(); ++i) {
    if(path[i]=='/' || i==path.size()) {
      std::string p(path.begin(), path.begin()+i);
      if(ist::MakeDir(p.c_str())) {
        ++num;
      }
    }
  }
  return num!=0;
}

bool RemoveDir(const std::string& path) {
#ifdef _WIN32
  return ::RemoveDirectory(path.c_str())==TRUE;
#else
  return ::rmdir(path.c_str())==0;
#endif
}

bool Remove(const std::string& path) {
  if(IsDir(path))
    return RemoveDir(path.c_str());
  else
    return std::remove(path.c_str())==0;
}

bool RemoveRecursive(const std::string& path) {
  bool res = false;
  if(IsDir(path)) {
    Dir dir(path);
    for(Dir::iterator p=dir.begin(); p!=dir.end(); ++p) {
      if(*p=="." || *p=="..")
        continue;
      std::string next = dir.getPath()+*p;
      res = RemoveRecursive(next.c_str());
    }
    res = RemoveDir(path);
  }
  else {
    res = std::remove(path.c_str())==0;
  }
  return res;
}

std::string GetCWD() {
  char buf[256];
#ifdef _WIN32
  ::GetCurrentDirectory(256, buf);
  for(size_t i=0; i<strlen(buf); ++i) {
    if(buf[i]=='\\')
      buf[i] = '/';
  }
  return buf;
#else
  ::getcwd(buf, 256);
  return buf;
#endif
}

bool SetCWD(const std::string& path) {
#ifdef _WIN32
  return ::SetCurrentDirectory(path.c_str())!=0;
#else
  return ::chdir(path.c_str())==0;
#endif
}


Dir::Dir() {}
Dir::Dir(const std::string& path) { open(path); }
size_t Dir::size() const { return m_files.size(); }
const std::string& Dir::operator[](size_t i) const { return m_files[i]; }
const std::string& Dir::getPath() const { return m_path; }

bool Dir::open(const std::string& path) {
  m_files.clear();
  m_path = path;
  if(m_path[m_path.size()-1]!='/')
    m_path+='/';

#ifdef _WIN32
  WIN32_FIND_DATA wfdata;
  HANDLE handle = ::FindFirstFile((m_path+"*").c_str(), &wfdata);
  if(handle!=INVALID_HANDLE_VALUE) {
    do {
      m_files.push_back(wfdata.cFileName);
    } while(::FindNextFile(handle, &wfdata));
    ::FindClose(handle);
  }
  else {
    return false;
  }
  return true;
#else
  DIR *dir = ::opendir(m_path.c_str());
  if(dir!=0) {
    dirent *dr;
    while((dr=::readdir(dir))!=0) {
      m_files.push_back(dr->d_name);
    }
    ::closedir(dir);
  }
  else {
    return false;
  }
  return true;
#endif
}

bool Dir::openRecursive(const std::string& path) {
  Dir dir(path);
  for(iterator p=dir.begin(); p!=dir.end(); ++p) {
    if(*p=="." || *p=="..")
      continue;
    std::string next = dir.getPath()+*p;
    if(IsDir(next.c_str()))
      openRecursive(next.c_str());
    else
      m_files.push_back(next);
  }
  return size()!=0;
}






/*
  DynamicSharedObject
*/

#ifdef _WIN32

DSO::DSO() :
  handle(NULL)
{}

DSO::DSO(const std::string& filename) :
  handle(NULL)
{
  load(filename);
}

DSO::~DSO()
{
  unload();
}

bool DSO::load(const std::string& _filename)
{
  unload();
  filename = _filename;
  handle = ::LoadLibrary(filename.c_str());
  if(!(*this))
    filename.clear();
  return !!(*this);
}

void DSO::unload()
{
  if(!!(*this)) {
    ::FreeLibrary(handle);
    filename.clear();
    handle = NULL;
  }
}

void* DSO::getFunction(const std::string& funcname) const
{
  if(!(*this))
    return 0;
  return (void*)::GetProcAddress(handle, funcname.c_str());
}

bool DSO::operator!() const
{
  return handle==NULL;
}

#else // _WIN32


DSO::DSO() : handle(0)
{}

DSO::DSO(const std::string& path) : handle(0)
{
  load(path);
}

DSO::~DSO()
{
  unload();
}

bool DSO::load(const std::string& path)
{
  unload();

  if(path[0]!='/') {
    char buf[128];
    filename = std::string(::getcwd(buf, 128))+'/'+path;
  }
  else
    filename = path;

  handle = ::dlopen(filename.c_str(), RTLD_NOW | RTLD_GLOBAL);

  if(!(*this))
    filename.clear();
  return !!(*this);
}

void DSO::unload()
{
  if(!!(*this)) {
    ::dlclose(handle);
    filename.clear();
    handle = 0;
  }
}

void* DSO::getFunction(const std::string& funcname) const
{
  if(!(*this))
    return 0;
  return ::dlsym(handle, funcname.c_str());
}

bool DSO::operator!() const
{
  return handle==0;
}
#endif // _WIN32



/*
  IGZ
*/

IGZCompresser::IGZCompresser(ist::bstream& s) : m_bf(s)
{
  char tag[10];
  memset(tag, 0, 10);
  sprintf(tag, "IGZ %d", IGZ_VERSION);
  m_bf.write(tag, 10);
}

IGZCompresser::~IGZCompresser() { }

bool IGZCompresser::addBuffer(const std::string& path, const ist::bbuffer& buf)
{
  unsigned long cmpsize = buf.size()*2+12;
  ist::bbuffer cmpbuf;
  cmpbuf.resize(cmpsize);
  compress((Bytef*)cmpbuf.ptr(), &cmpsize, (Bytef*)buf.ptr(), buf.size());

  m_bf << path << buf.size() << size_t(cmpsize);
  m_bf.write(cmpbuf.ptr(), cmpsize);

  return true;
}

bool IGZCompresser::addFile(const std::string& path)
{
  ist::bbuffer f;
  return f.load(path) && addBuffer(path, f);
}

size_t IGZCompresser::addDirectory(const std::string& path)
{
  size_t ret = 0;
  if(ist::IsDir(path)) {
    Dir dir(path);
    for(size_t i=0; i<dir.size(); ++i) {
      if(dir[i]!="." && dir[i]!="..") {
        ret+=addDirectory((dir.getPath()+dir[i]).c_str());
      }
    }
  }
  else {
    if(addFile(path)) {
      ++ret;
    }
  }

  return ret;
}



IGZExtracter::IGZExtracter(ist::bstream& s) : m_bf(s)
{
  size_t filesize = m_bf.size();

  char tag[10];
  int version;
  m_bf.read(tag, 10);
  if(sscanf(tag, "IGZ %d", &version)!=1) {
    return;
  }

  while(!m_bf.eof()) {
    std::string path;
    size_t size, cmpsize;
    m_bf >> path;
    m_files.push_back(path);
    m_entry[path] = m_bf.rtell();
    m_bf >> size >> cmpsize;
    if(m_bf.rtell()+cmpsize>=filesize) {
      break;
    }
    m_bf.rskip(cmpsize);
  }
  std::sort(m_files.begin(), m_files.end());
}

IGZExtracter::~IGZExtracter() { }

size_t IGZExtracter::getFileCount() const
{
  return m_files.size();
}

const std::string& IGZExtracter::getFileName(size_t i) const
{
  return m_files[i];
}

bool IGZExtracter::isExist(const std::string& filename) const
{
  entry_cont::const_iterator p = m_entry.find(filename);
  return p!=m_entry.end();
}

bool IGZExtracter::extractToBuffer(const std::string& path, ist::bbuffer& buf)
{
  entry_cont::iterator p = m_entry.find(path);
  if(p==m_entry.end()) {
    return false;
  }

  m_bf.rseek(p->second);

  size_t size, cmpsize;
  unsigned long _size;
  ist::bbuffer cmpbuf;

  m_bf >> size >> cmpsize;
  buf.resize(size);
  cmpbuf.resize(cmpsize);
  m_bf.read(cmpbuf.ptr(), cmpsize);

  _size = size;
  uncompress((Bytef*)buf.ptr(), &_size, (Bytef*)cmpbuf.ptr(), cmpsize);

  return true;
}

bool IGZExtracter::extractToFile(const std::string& path, const std::string& outdir)
{
  ist::bbuffer buf;
  if(extractToBuffer(path, buf)) {
    for(size_t i=0; i<path.size(); ++i) {
      if(path[i]=='/' || path[i]=='\\') {
        std::string p(path.begin(), path.begin()+i);
        ist::MakeDir(p.c_str());
      }
    }
    std::ofstream out((outdir+path).c_str(), std::ios::binary);
    if(!!out) {
      out.write(buf.ptr(), buf.size());
      return true;
    }
  }
  return false;
}

size_t IGZExtracter::extractAllFiles()
{
  size_t ret = 0;
  for(entry_cont::iterator p=m_entry.begin(); p!=m_entry.end(); ++p) {
    if(extractToFile(p->first)) {
      ++ret;
    }
  }
  return ret;
}



}


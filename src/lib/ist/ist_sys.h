#ifndef IST_SYS_H
#define IST_SYS_H

#ifdef _WIN32
  #include <windows.h>
#endif
#include <string>
#include <vector>
#include <map>
#include "ist_conf.h"
#include "bstream.h"


namespace ist {

/*
  Directory
*/
IST_EXPORT bool IsFile(const std::string& path);
IST_EXPORT bool IsDir(const std::string& path);
IST_EXPORT bool MakeDir(const std::string& path);
IST_EXPORT bool MakeDeepDir(const std::string& path); // 文字列から'/'を見つけてディレクトリ作成。 a/b/c でa/bとか。 
IST_EXPORT bool RemoveDir(const std::string& path); // 空のディレクトリのみを削除。 
IST_EXPORT bool Remove(const std::string& path); // ファイルもしくは空のディレクトリを削除。 
IST_EXPORT bool RemoveRecursive(const std::string& path); // ディレクトリ/ファイルを再帰的に完全消去。むやみに使っちゃダメ。 
IST_EXPORT std::string GetCWD();
IST_EXPORT bool SetCWD(const std::string& path);

class IST_CLASS Dir : public ist::Object
{
public:
  typedef std::vector<std::string> path_cont;
  typedef path_cont::iterator iterator;
  typedef path_cont::const_iterator const_iterator;

  Dir();
  explicit Dir(const std::string& path);
  bool open(const std::string& path);
  bool openRecursive(const std::string& path);

  size_t size() const;
  const std::string& operator[](size_t i) const;
  const std::string& getPath() const;

  iterator begin() { return m_files.begin(); }
  iterator end()   { return m_files.end(); }
  const_iterator begin() const { return m_files.begin(); }
  const_iterator end() const   { return m_files.end(); }

private:
  std::string m_path;
  path_cont m_files;
};


/*
  DynamicSharedObject
*/
class IST_CLASS DSO : public ist::Object
{
public:
  DSO();
  DSO(const std::string& filename);
  ~DSO();
  bool load(const std::string& filename);
  void unload();
  void* getFunction(const std::string& funcname) const;
  const std::string& getFilename() const;
  bool operator!() const;

private:
  std::string filename;
#ifdef _WIN32
  HMODULE handle;
#else
  void *handle;
#endif
};



/*
  IGZ
*/
class IST_CLASS IGZCompresser : public ist::Object
{
private:
  ist::bstream& m_bf;

public:
  IGZCompresser(ist::bstream& s);
  virtual ~IGZCompresser();

  virtual bool addBuffer(const std::string& path, const ist::bbuffer& out);
  virtual bool addFile(const std::string& path); 
  virtual size_t addDirectory(const std::string& path); // 指定したディレクトリ/ファイルを再帰的に圧縮。圧縮したファイルの数が返る。 
};

class IST_CLASS IGZExtracter : public ist::Object
{
private:
  typedef std::map<std::string, std::streampos> entry_cont;
  typedef std::vector<std::string> filename_cont;
  entry_cont m_entry;
  filename_cont m_files;
  ist::bstream& m_bf;

public:
  IGZExtracter(ist::bstream& s);
  virtual ~IGZExtracter();

  virtual size_t getFileCount() const;
  virtual const std::string& getFileName(size_t i) const;
  virtual bool isExist(const std::string& filename) const;

  virtual bool extractToBuffer(const std::string& filename, ist::bbuffer& out);
  virtual bool extractToFile(const std::string& filename, const std::string& outdir="");
  virtual size_t extractAllFiles();
};


} // namespace ist

#endif

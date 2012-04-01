#ifndef STRCNV_HPP_INCLUDED
#define STRCNV_HPP_INCLUDED

#include <locale>
#include <string>


namespace ist {


class CWConverter
{
public:
  static CWConverter& getDefault()
  {
    static CWConverter c;
    return c;
  }

  CWConverter(const char *l = "")
  {
    setLocale(l);
  }

  void setLocale(const char *l)
  {
    loc = l;
    ::setlocale(LC_ALL, l);
  }

  void MBS2WCS(std::wstring& dst, const std::string& src)
  {
    size_t len = mbstowcs(0, src.c_str(), 0)+1;
    if(len==(size_t)(-1)) {
      return;
    }
    wchar_t *buf(new wchar_t[len]);
    ::mbstowcs(buf, src.c_str(), len);
    dst = buf;
    delete[] buf;
  }

  void WCS2MBS(std::string& dst, const std::wstring& src)
  {
    size_t len = wcstombs(0, src.c_str(), 0)+1;
    if(len==(size_t)(-1)) {
      return;
    }
    char *buf(new char[len]);
    ::wcstombs(buf, src.c_str(), len);
    dst = buf;
    delete[] buf;
  }

  std::string loc;
};

}


inline std::wstring _L(const std::string& src)
{
  std::wstring dst;
  ist::CWConverter::getDefault().MBS2WCS(dst, src);
  return dst;
}

inline std::string _S(const std::wstring& src)
{
  std::string dst;
  ist::CWConverter::getDefault().WCS2MBS(dst, src);
  return dst;
}


#endif

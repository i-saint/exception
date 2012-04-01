#ifndef IST_NET_H
#define IST_NET_H

#define BOOST_DATE_TIME_NO_LIB
#include <istream>
#include <ostream>
#include <string>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "ist_conf.h"
#include "bstream.h"

namespace ist {

using boost::asio::ip::tcp;

class IST_CLASS HTTPRequest : public ist::Object
{
private:
  int m_code;
  boost::asio::streambuf m_data;
  boost::asio::io_service& m_io_service;


  bool send(const std::string& host, const std::string& path, boost::asio::streambuf& request)
  {
    tcp::resolver resolver(m_io_service);
    tcp::resolver::query query(host, "http");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    tcp::socket socket(m_io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while(error && endpoint_iterator != end) {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }
    if(error) {
      return false;
    }

    boost::asio::write(socket, request);

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, boost::regex("\r\n"));

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    response_stream >> m_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if(!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      return false;
    }
    if(m_code!=200)
    {
      return false;
    }

    boost::asio::read_until(socket, response, boost::regex("\r\n\r\n"));

    size_t length = 0;
    bool partial = false;
    {
      std::string l;
      while (std::getline(response_stream, l) && l!="\r") {
        for(size_t i=0; i<l.size(); ++i) {
          l[i] = tolower(l[i]);
        }
        if(length==0 && sscanf(l.c_str(), "content-length: %d", &length)==1) {
        }
      }

      if(length==0) {
        std::getline(response_stream, l);
        sscanf(l.c_str(), "%x", &length);
        partial = true;
      }
    }

    std::ostream content_stream(&m_data, std::ios::binary);
    if(partial) {
      boost::asio::read(socket, response, boost::asio::transfer_all(), error);
      std::istream in(&response);

      std::vector<char> buf;
      while(!in.eof() && length>0) {
        buf.resize(length);
        in.read(&buf[0], length);
        content_stream.write(&buf[0], length);

        std::string l;
        std::getline(in, l);
        l.clear();
        std::getline(in, l);
        if(sscanf(l.c_str(), "%x", &length)==1) {
        }
      }
    }
    else {
      while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
        content_stream << &response;
      }
    }

    if(error!=boost::asio::error::eof) {
      return false;
    }

    return true;
  }


public:
  HTTPRequest(boost::asio::io_service& ios) : m_io_service(ios), m_code(0)
  {}

  int getStatus() const { return m_code; }
  std::streambuf& getBuf() { return m_data; }
  char getchar() { return m_data.sbumpc(); }
  size_t size() { return m_data.size(); }
  bool eof() { return m_data.size()==0; }
  std::streamsize read(char *buf, std::streamsize size)
  {
    std::streamsize s = m_data.sgetn(buf, size);
    m_data.pubseekoff(size, std::ios_base::cur);
    return s;
  }

  bool get(const std::string& host, const std::string& path)
  {
    boost::asio::streambuf request;
    std::ostream rs(&request);
    rs << "GET " << path << " HTTP/1.1\r\n";
    rs << "Host: " << host << "\r\n";
    rs << "Accept: */*\r\n";
    rs << "Connection: Close\r\n\r\n";

    return send(host, path, request);
  }

  bool post(const std::string& host, const std::string& path, const std::string& query)
  {
    boost::asio::streambuf request;
    std::ostream rs(&request);
    rs << "POST " << path << " HTTP/1.1\r\n";
    rs << "Host: " << host << "\r\n";
    rs << "Accept: */*\r\n";
    rs << "Connection: Close\r\n";
    rs << "Content-Length: " << query.size() << "\r\n";
    rs << "\r\n";
    rs << query;

    return send(host, path, request);
  }

  // 苦肉の策のファイル送信メソッド 
  bool postFile(const std::string& host, const std::string& path,
    const std::string& fieldname, const std::string& remote_filename, const std::string& local_filename)
  {
    boost::asio::streambuf d;

    {
      char buf[512];
      std::ostream dout(&d);
      dout << "--aabbcc\r\n";
      sprintf(buf, "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", fieldname.c_str(), remote_filename.c_str());
      dout << buf;
      dout << "Content-Type: application/octet-stream\r\n\r\n";
      if(FILE *f = fopen(local_filename.c_str(), "rb")) {
        for(;;) {
          int len = fread(buf, 1, 512, f);
          if(len==0) {
            break;
          }
          dout.write(buf, len);
        }
        fclose(f);
      }
      dout << "--aabbcc--";
    }

    boost::asio::streambuf request;
    std::ostream rs(&request);
    rs << "POST " << path << " HTTP/1.1\r\n";
    rs << "Host: " << host << "\r\n";
    rs << "Accept: */*\r\n";
    rs << "Connection: Close\r\n";
    rs << "Content-Length: " << d.size() << "\r\n";
    rs << "Content-Type: multipart/form-data, boundary=aabbcc\r\n";
    rs << "\r\n";
    rs << &d;

    return send(host, path, request);
  }
};


class IST_CLASS HTTPRequestAsync : private ist::Object
{
friend class boost::thread;
private:
  typedef boost::shared_ptr<boost::thread> thread_ptr;
  thread_ptr m_thread;
  HTTPRequest m_req;
  bool m_complete;
  std::string m_method;
  std::string m_host;
  std::string m_path;
  std::string m_query;
  std::string m_fieldname;
  std::string m_remote_filename;
  std::string m_local_filename;

  void run()
  {
    m_thread.reset(new boost::thread(boost::ref(*this)));
  }


public:
  HTTPRequestAsync(boost::asio::io_service& ios) : m_req(ios), m_complete(false)
  {}

  ~HTTPRequestAsync()
  {
    if(m_thread) {
      m_thread->join();
    }
  }

  int getStatus() const { return m_req.getStatus(); }
  std::streambuf& getBuf() { return m_req.getBuf(); }
  char getchar() { return m_req.getchar(); }
  size_t size() { return m_req.size(); }
  bool eof() { return m_req.eof(); }
  std::streamsize read(char *buf, std::streamsize size) { return m_req.read(buf, size); }

  bool isComplete() { return m_complete; }

  void get(const std::string& host, const std::string& path)
  {
    m_method = "get";
    m_host = host;
    m_path = path;
    run();
  }

  void post(const std::string& host, const std::string& path, const std::string& query)
  {
    m_method = "post";
    m_host = host;
    m_path = path;
    m_query = query;
    run();
  }

  bool postFile(const std::string& host, const std::string& path,
    const std::string& fieldname, const std::string& remote_filename, const std::string& local_filename)
  {
    m_method = "postFile";
    m_host = host;
    m_path = path;
    m_fieldname = fieldname;
    m_remote_filename = remote_filename;
    m_local_filename = local_filename;
    run();
  }


  void operator()()
  {
    try {
      if(m_method=="get") {
        m_req.get(m_host, m_path);
      }
      else if(m_method=="post") {
        m_req.post(m_host, m_path, m_query);
      }
      else if(m_method=="postFile") {
        m_req.postFile(m_host, m_path, m_fieldname, m_remote_filename, m_local_filename);
      }
    }
    catch(...) {
    }
    m_complete = true;
  }
};

} // ist 

#endif

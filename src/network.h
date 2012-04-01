#ifndef network_h
#define network_h

#include <sstream>
#include <boost/lexical_cast.hpp>
#include "input.h"
#include "game.h"

namespace exception {

#ifdef EXCEPTION_ENABLE_NETPLAY 
  using boost::asio::ip::tcp;
  typedef shared_ptr<tcp::iostream> socket_ptr;
#endif // EXCEPTION_ENABLE_NETPLAY 

  void PushChatText(const string& t);
  size_t GetSessionID();


  class NetworkError : public std::runtime_error
  {
  typedef std::runtime_error Super;
  public:
    NetworkError(const string& message) : Super(message) {}
  };


  struct NMessage
  {
    enum {
      UNKNOWN = 0,
      QUERY,
      RESPONSE,
      ENTRY,
      ACCEPT,
      REJECT,
      CLOSE,
      START,
      END,
      PAUSE,
      RESUME,
      STATE,
      JOIN,
      LEAVE,
      CSTAT,
      INPUT,
      TEXT,
    };

    struct CStatField {
      size_t ping;
    };

    struct StartField {
      int mode;
      int stage;
      int seed;
      int delay;
    };

    struct InputField {
      int input;
    };

    struct EntryField {
      char name[16];
      size_t frame;
    };

    struct TextField {
      char text[64];
    };

    struct JoinField {
      size_t frame;
    };

    struct LeaveField {
      size_t frame;
    };

    struct PauseField {
      size_t frame;
    };

    struct ResumeField {
      size_t frame;
    };

    struct StateField {
      size_t frame;
    };



    int type;
    size_t cid;
    union {
      CStatField cstat;
      StartField start;
      InputField input;
      EntryField entry;
      TextField reject;
      TextField response;
      TextField text;
      TextField query;
      JoinField join;
      LeaveField leave;
      PauseField pause;
      ResumeField resume;
      StateField state;
    };
    boost::shared_ptr<ist::bbuffer> statep;

    NMessage() {}
    NMessage(ist::bstream& b) { deserialize(b); }

    void serialize(ist::bstream& b) const
    {
      b << type;
      switch(type) {
      case INPUT:    b << cid << input.input; break;
      case CSTAT:    b << cid << cstat.ping; break;
      case TEXT:     b << cid << text.text; break;
      case PAUSE:    b << pause.frame; break;
      case RESUME:   b << resume.frame; break;
      case START:    b << start.mode << start.stage << start.seed << start.delay; break;
      case END:      break;
      case CLOSE:    b << cid; break;
      case ACCEPT:   b << cid; break;
      case ENTRY:    b << cid << entry.name << entry.frame; break;
      case REJECT:   b << reject.text; break;
      case QUERY:    b << query.text; break;
      case RESPONSE: b << response.text; break;
      case JOIN:     b << cid << join.frame; break;
      case LEAVE:    b << cid << leave.frame; break;
      case STATE:    b << *statep << state.frame; break;
      default: throw NetworkError("protocol error"); break;
      }
    }

    void deserialize(ist::bstream& b)
    {
      b >> type;
      switch(type) {
      case INPUT:    b >> cid >> input.input; break;
      case CSTAT:    b >> cid >> cstat.ping; break;
      case TEXT:     b >> cid >> text.text; break;
      case PAUSE:    b >> pause.frame; break;
      case RESUME:   b >> resume.frame; break;
      case START:    b >> start.mode >> start.stage >> start.seed >> start.delay; break;
      case END:      break;
      case CLOSE:    b >> cid; break;
      case ACCEPT:   b >> cid; break;
      case ENTRY:    b >> cid >> entry.name >> entry.frame; break;
      case REJECT:   b >> reject.text; break;
      case QUERY:    b >> query.text; break;
      case RESPONSE: b >> response.text; break;
      case JOIN:     b >> cid >> join.frame; break;
      case LEAVE:    b >> cid >> leave.frame; break;
      case STATE:    statep.reset(new Deserializer()); b >> *statep >> state.frame; break;
      default: throw NetworkError("protocol error"); break;
      }
    }


    static NMessage Input(size_t cid, int stat)
    {
      NMessage t;
      t.type = INPUT;
      t.cid = cid;
      t.input.input = stat;
      return t;
    }

    static NMessage Start(int mode, int stage, int seed, int delay)
    {
      NMessage t;
      t.type = START;
      t.cid = 0;
      t.start.mode = mode;
      t.start.stage = stage;
      t.start.seed = seed;
      t.start.delay = delay;
      return t;
    }

    static NMessage End()
    {
      NMessage t;
      t.type = END;
      t.cid = 0;
      return t;
    }

    static NMessage Pause()
    {
      NMessage t;
      t.type = PAUSE;
      t.cid = 0;
      t.pause.frame = GetPast();
      return t;
    }

    static NMessage Resume()
    {
      NMessage t;
      t.type = RESUME;
      t.cid = 0;
      t.resume.frame = GetPast();
      return t;
    }

    static NMessage Close(size_t cid)
    {
      NMessage t;
      t.type = CLOSE;
      t.cid = cid;
      return t;
    }

    static NMessage ClientStatus(size_t cid, size_t ping)
    {
      NMessage t;
      t.type = CSTAT;
      t.cid = cid;
      t.cstat.ping = ping;
      return t;
    }

    static NMessage Text(size_t cid, string text)
    {
      if(text.size()>63) {
        text.resize(63);
      }
      for(size_t i=0; i<text.size(); ++i) {
        if(text[i]=='\'') { text[i]='"'; }
      }

      NMessage t;
      t.type = TEXT;
      t.cid = cid;
      strcpy(t.text.text, text.c_str());
      return t;
    }

    static NMessage Accept(size_t cid)
    {
      NMessage t;
      t.type = ACCEPT;
      t.cid = cid;
      return t;
    }

    static NMessage Reject(const string& text)
    {
      NMessage t;
      t.type = REJECT;
      strcpy(t.reject.text, text.c_str());
      return t;
    }

    static NMessage Entry(size_t cid, const string& name, size_t frame=0)
    {
      NMessage t;
      t.type = ENTRY;
      t.cid = cid;
      strcpy(t.entry.name, name.c_str());
      t.entry.frame = frame;
      return t;
    }

    static NMessage Query(const string& text)
    {
      NMessage t;
      t.type = QUERY;
      strcpy(t.query.text, text.c_str());
      return t;
    }

    static NMessage Response(const string& text)
    {
      NMessage t;
      t.type = RESPONSE;
      strcpy(t.response.text, text.c_str());
      return t;
    }

    static NMessage Join(size_t cid, size_t frame=0)
    {
      NMessage t;
      t.type = JOIN;
      t.cid = cid;
      t.join.frame = frame;
      return t;
    }

    static NMessage Leave(size_t cid, size_t frame=0)
    {
      NMessage t;
      t.type = LEAVE;
      t.cid = cid;
      t.leave.frame = frame;
      return t;
    }

    static NMessage State(ist::bbuffer *b, size_t frame)
    {
      NMessage t;
      t.type = STATE;
      t.state.frame = frame;
      t.statep.reset(b);
      return t;
    }
  };

  typedef std::vector<NMessage> nmessage_cont;

#ifdef EXCEPTION_ENABLE_NETPLAY
  inline void SendNetworkMessage(tcp::iostream& socket, const nmessage_cont& mes)
  {
    std::stringstream ss;
#ifdef EXCEPTION_TRIAL
    ss << "exception trial " << EXCEPTION_VERSION << "\n";
#else
    ss << "exception " << EXCEPTION_VERSION << "\n";
#endif
    ist::biostream bio(ss);
    bio << mes.size();
    for(size_t i=0; i<mes.size(); ++i) {
      mes[i].serialize(bio);
    }

    const string& s = ss.str();
    socket.write(s.c_str(), s.size());
  }

  inline void RecvNetworkMessage(tcp::iostream& ss, nmessage_cont& mes)
  {
    string header;
    int version;

    std::getline(ss, header);
#ifdef EXCEPTION_TRIAL
    if(sscanf(header.c_str(), "exception trial %d", &version)!=1) {
#else
    if(sscanf(header.c_str(), "exception %d", &version)!=1) {
#endif
      throw NetworkError("protocol error");
    }
    else if(version!=EXCEPTION_VERSION) {
      throw NetworkError("version not matched");
    }


    size_t size;
    ist::biostream bio(ss);
    bio >> size;
    for(size_t i=0; i<size; ++i) {
      mes.push_back(NMessage(bio));
    }
  }
#endif // EXCEPTION_ENABLE_NETPLAY 

  class Thread
  {
  private:
    typedef boost::shared_ptr<boost::thread> thread_ptr;
    thread_ptr m_thread;
    bool m_is_running;

  public:
    Thread() : m_is_running(false)
    {}

    virtual ~Thread()
    {}

    virtual void run()
    {
      m_thread.reset(new boost::thread(boost::ref(*this)));
    }

    void join()
    {
      if(m_thread) {
        m_thread->join();
      }
    }

    bool isRunning() const
    {
      return m_is_running;
    }

    void operator()()
    {
      m_is_running = true;
      exec();
      m_is_running = false;
    }

    virtual void exec()=0;
  };



  class IInputServer : public Thread
  {
  public:
    virtual void flush()=0;
  };

#ifdef EXCEPTION_ENABLE_NETPLAY
  class InputServer : public IInputServer
  {
  typedef IInputServer Super;
  public:

    class Client : public Thread
    {
    private:
      boost::mutex m_recv_mutex;
      boost::mutex m_send_mutex;
      nmessage_cont m_trecv_data;
      nmessage_cont m_recv_data;
      nmessage_cont m_send_data;
      nmessage_cont m_cstat;
      socket_ptr m_socket;
      const size_t m_cid;
      const string m_name;
      bool m_stopped;
      bool m_closed;
      size_t m_ping;
      int m_input_length;

    public:
      Client(socket_ptr s, size_t cid, const string& name) :
        m_socket(s), m_cid(cid), m_name(name), m_stopped(false), m_closed(false), m_ping(0)
      {}

      ~Client()
      {
        stop();
        join();
      }

      bool isClosed() { return m_closed; }
      size_t getID() { return m_cid; }
      const string& getName() { return m_name; }
      size_t getPing() { return m_ping; }

      void push(const NMessage& m) // sync 
      {
        boost::mutex::scoped_lock lock(m_send_mutex);
        m_send_data.push_back(m);
      }

      void push(const nmessage_cont& mc) // sync 
      {
        boost::mutex::scoped_lock lock(m_send_mutex);
        m_cstat.clear();
        for(size_t i=0; i<mc.size(); ++i) {
          if(mc[i].type==NMessage::CSTAT) {
            m_cstat.push_back(mc[i]);
          }
          else {
            m_send_data.push_back(mc[i]);
          }
        }
      }

      void pop(nmessage_cont& mc) // sync 
      {
        boost::mutex::scoped_lock lock(m_recv_mutex);
        mc.insert(mc.end(), m_recv_data.begin(), m_recv_data.end());
        m_recv_data.clear();
      }

      NMessage waitMessage(int type)
      {
        for(;;) {
          {
            boost::mutex::scoped_lock lock(m_recv_mutex);
            for(nmessage_cont::iterator p=m_recv_data.begin(); p!=m_recv_data.end(); ++p) {
              if(p->type==type) {
                NMessage tmp = *p;
                m_recv_data.erase(p);
                return tmp;
              }
            }
          }
          sgui::Sleep(5);
        }
      }

      void stop()
      {
        m_stopped = true;
      }

      void exec()
      {
        try {
          while(!m_stopped) {
            int t = sgui::GetTicks();

            recv();
            send();

            m_ping = sgui::GetTicks()-t;
            // ローカルだと凄い勢いで空回りするため、早すぎる場合少し待つ 
            if(m_ping < 1) {
              sgui::Sleep(1);
            }
          }
        }
        catch(...) {
        }
        m_closed = true;
        m_socket->close();
      }


    private:
      void send() // sync 
      {
        // メッセージが来るか1フレーム分の時間が経過するのを待つ 
        for(int i=0; i<16; ++i) {
          {
            boost::mutex::scoped_lock lock(m_send_mutex);
            if(!m_send_data.empty()) {
              break;
            }
          }
          sgui::Sleep(1);
        }
        boost::mutex::scoped_lock lock(m_send_mutex);
        m_send_data.insert(m_send_data.end(), m_cstat.begin(), m_cstat.end());
        SendNetworkMessage(*m_socket, m_send_data);
        m_send_data.clear();
      }

      bool handleMessage(const NMessage& m)
      {
        if(m.type==NMessage::CLOSE || m.type==NMessage::END) {
          stop();
          return true;
        }
        return false;
      }

      struct handler
      {
        Client *m_client;
        handler(Client *c) : m_client(c) {}
        bool operator()(const NMessage& m)
        {
          return m_client->handleMessage(m);
        }
      };
      friend struct handler;

      void recv() // sync 
      {
        RecvNetworkMessage(*m_socket, m_trecv_data);
        for(size_t i=0; i<m_trecv_data.size(); ++i) {
          m_trecv_data[i].cid = m_cid;
        }
        m_trecv_data.erase(
          std::remove_if(m_trecv_data.begin(), m_trecv_data.end(), handler(this)),
          m_trecv_data.end());

        {
          boost::mutex::scoped_lock lock(m_recv_mutex);
          m_recv_data.insert(m_recv_data.end(), m_trecv_data.begin(), m_trecv_data.end());
        }
        m_trecv_data.clear();
      }
    };


    class Appender : public Thread
    {
    public:
      ~Appender()
      {
        join();
      }

      void exec()
      {
        boost::asio::io_service ios;
        ist::HTTPRequest req(ios);

        char url[256];
        string path;
#ifdef EXCEPTION_DEBUG
        path = "/exception/d/server/";
#elif defined(EXCEPTION_TRIAL)
        path = "/exception/t/server/";
#else
        path = "/exception/r/server/";
#endif

        IConfig& c = *GetConfig();
        {
          boost::mutex::scoped_lock lock(c.getMutex());
          sprintf(url, "%s?cmd=add&port=%d&name=%s", path.c_str(), c.port, c.scorename.c_str());
        }
        req.get("i-saint.skr.jp", url);
      }
    };

    class Remover : public Thread
    {
    public:
      ~Remover()
      {
        join();
      }

      void exec()
      {
        boost::asio::io_service ios;
        ist::HTTPRequest req(ios);

        char url[256];
        string path;
#ifdef EXCEPTION_DEBUG
        path = "/exception/d/server/";
#elif defined(EXCEPTION_TRIAL)
        path = "/exception/t/server/";
#else
        path = "/exception/r/server/";
#endif
        sprintf(url, "%s?cmd=del", path.c_str());

        req.get("i-saint.skr.jp", url);
      }
    };

    class Flusher : public Thread
    {
    private:
      IInputServer *m_server;
      volatile bool m_stoped;

    public:
      Flusher(IInputServer& server) :
        m_server(&server), m_stoped(false)
      {}

      ~Flusher()
      {
        stop();
        join();
      }

      void stop()
      {
        m_stoped = true;
      }

      void exec()
      {
        while(!m_stoped) {
          int t = sgui::GetTicks();

          m_server->flush();

          // 空回り緩和 
          int past = sgui::GetTicks()-t;
          if(past<1) {
            sgui::Sleep(1);
          }
        }
      }
    };
    typedef shared_ptr<Thread> thread_ptr;
    typedef shared_ptr<Client> client_ptr;
    typedef std::map<int, client_ptr> client_cont;
    typedef shared_ptr<Flusher> flusher_ptr;


  private:
    boost::asio::io_service &m_io_service;
    boost::mutex m_mutex;
    bool m_stopped;
    client_cont m_clients;
    flusher_ptr m_flusher;
    thread_ptr m_appender;
    thread_ptr m_remover;
    nmessage_cont m_send_data;
    int m_delay;
    int m_idgen;

  public:
    InputServer(boost::asio::io_service& ios) :
      m_io_service(ios), m_stopped(false), m_delay(5), m_idgen(0)
    {
    }

    ~InputServer()
    {
      stop();
      join();

      closeClients();
      m_flusher.reset();
    }

    void stop()
    {
      if(m_stopped) {
        return;
      }
      m_stopped = true;

      if(m_appender && !m_appender->isRunning()) {
        m_remover.reset(new Remover());
        m_remover->run();
      }

      tcp::iostream socket("127.0.0.1", boost::lexical_cast<string>(GetConfig()->port));
      nmessage_cont sm;
      SendNetworkMessage(socket, sm);
    }

    void exec()
    {
      m_flusher.reset(new Flusher(*this));
      m_flusher->run();

      m_appender.reset(new Appender());
      m_appender->run();

      tcp::acceptor acceptor(m_io_service, tcp::endpoint(tcp::v4(), GetConfig()->port));
      while(!m_stopped) {
        socket_ptr s(new tcp::iostream());
        acceptor.accept(*s->rdbuf());
        s->rdbuf()->set_option(tcp::no_delay(true));
        if(!m_stopped) {
          accept(s);
        }
      }
    }

    void accept(socket_ptr&  socket)
    {
      nmessage_cont rm;
      nmessage_cont sm;

      try {
        RecvNetworkMessage(*socket, rm);
      }
      catch(const NetworkError& e) {
        sm.push_back(NMessage::Reject(e.what()));
        SendNetworkMessage(*socket, sm);
        return;
      }

      for(size_t i=0; i<rm.size(); ++i) {
        NMessage& m = rm[i];
        if(m.type==NMessage::ENTRY) { // 参加希望リクエストが来た場合 
          boost::mutex::scoped_lock lock(m_mutex); // sync 

          // 最大人数(4)超えてたらreject 
          if(m_clients.size()>=4) {
            sm.push_back(NMessage::Reject("too many players (max: 4players)"));
            continue;
          }

          // ID割り当て 
          int cid = ++m_idgen;
          client_ptr c(new Client(socket, cid, m.entry.name));
          m_clients[cid] = c;
          sm.push_back(NMessage::Accept(cid));

          // 途中参加の場合、ステートと途中参加メッセージを送信 
          if(IGame *game = GetGame()) {
            client_ptr local = m_clients[GetSessionID()];
            local->push(NMessage::Query("state"));
            NMessage m = local->waitMessage(NMessage::STATE);
            sm.push_back(m);

            m_send_data.push_back(NMessage::Entry(c->getID(), c->getName(), m.state.frame+m_delay));
            m_send_data.push_back(NMessage::Join(c->getID(), m.state.frame+m_delay));
          }
          else {
            // 他のプレイヤーの情報を送信 
            for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
              client_ptr& cli = p->second;
              sm.push_back(NMessage::Entry(cli->getID(), cli->getName()));
            }
            m_send_data.push_back(NMessage::Entry(c->getID(), c->getName()));
          }
          SendNetworkMessage(*socket, sm);
          c->run();
        }
        else if(m.type==NMessage::QUERY) {
          string query(m.query.text);
          if(query=="version") {
#ifdef EXCEPTION_TRIAL
            sm.push_back(NMessage::Response(string("exception trial ")+boost::lexical_cast<string>(EXCEPTION_VERSION)));
#else
            sm.push_back(NMessage::Response(string("exception ")+boost::lexical_cast<string>(EXCEPTION_VERSION)));
#endif
            SendNetworkMessage(*socket, sm);
          }
        }
      }
    }



    void flush() // sync 
    {
      boost::mutex::scoped_lock lock(m_mutex);

      // クライアントに溜まったメッセージを一まとめに 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->pop(m_send_data);
      }
      for(size_t a=0; a<m_send_data.size(); ++a) {
        NMessage& m = m_send_data[a];
        if(m.type==NMessage::START) {
          m_delay = m.start.delay + 1;
        //  stop();
        }
        else if(m.type==NMessage::PAUSE) {
          m.pause.frame+=m_delay;
        }
      }

      // クライアントの状態をメッセージ化 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); /**/) {
        client_ptr& c = p->second;
        int cid = p->first;
        if(c->isClosed()) { // コネクションが切れてたら消去 
          if(IGame *game = GetGame()) {
            if(cid==GetSessionID()) {
              m_send_data.push_back(NMessage::End());
            }
            else {
              m_send_data.push_back(NMessage::Leave(cid, game->getPast()+m_delay));
            }
          }
          m_send_data.push_back(NMessage::Close(cid));
          m_clients.erase(p++);
        }
        else {
          m_send_data.push_back(NMessage::ClientStatus(cid, c->getPing()));
          ++p;
        }
      }

      // 全メッセージ送信 
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->push(m_send_data);
      }
      m_send_data.clear();
    }

    void closeClients() // sync 
    {
      boost::mutex::scoped_lock lock(m_mutex);
      for(client_cont::iterator p=m_clients.begin(); p!=m_clients.end(); ++p) {
        p->second->stop();
      }
      m_clients.clear();
    }
  };
#endif // EXCEPTION_ENABLE_NETPLAY 



  class ISession : public RefCounter
  {
  public:
    virtual size_t getID() const=0;
    virtual size_t getPing() const=0;
    virtual const string& getName() const=0;
    virtual IInput& getInput()=0;
    virtual void serialize(Serializer& s) const=0;
  };

  class Session : public ISession
  {
  private:
    input_ptr m_input;
    size_t m_id;
    string m_name;
    size_t m_ping;
    size_t m_begin_frame;
    size_t m_join_frame;
    size_t m_leave_frame;

  public:
    Session(Deserializer& s)
    {
      m_input = new InputStream(s);
      s >> m_id >> m_name >> m_ping >> m_begin_frame >> m_join_frame >> m_leave_frame;
    }

    void serialize(Serializer& s) const
    {
      m_input->serialize(s);
      s << m_id << m_name << m_ping << m_begin_frame << m_join_frame << m_leave_frame;
    }

  public:
    Session(size_t id, const string& name, input_ptr input) :
      m_input(input), m_id(id), m_name(name), m_ping(0),
      m_begin_frame(0), m_join_frame(0), m_leave_frame(-1)
    {}

    Session(ist::gzbstream& s, int version) :
      m_id(0), m_ping(0), m_begin_frame(0), m_join_frame(0), m_leave_frame(-1)
    {
      s >> m_name;
      m_input = new InputStream(s);
      if(version>105) {
        s >> m_id >> m_begin_frame >> m_join_frame >> m_leave_frame;
      }
    }

    void write(ist::gzbstream& s) const
    {
      s << m_name;
      m_input->write(s);
      s << m_id << m_begin_frame << m_join_frame << m_leave_frame;
    }

    size_t getID() const { return m_id; }
    const string& getName() const { return m_name; }
    InputStream& getInput() { return *m_input; }

    size_t getPing() const       { return m_ping; }
    size_t getBeginFrame() const { return m_begin_frame; }
    size_t getJoinFrame() const  { return m_join_frame; }
    size_t getLeaveFrame() const { return m_leave_frame; }
    void setPing(size_t v)       { m_ping=v; }
    void setBeginFrame(size_t v) { m_begin_frame=v; }
    void setJoinFrame(size_t v)  { m_join_frame=v; }
    void setLeaveFrame(size_t v) { m_leave_frame=v; }

    void setID(size_t v) { m_id=v; }
  };
  typedef intrusive_ptr<Session> session_ptr;




  class IInputClient : public Thread
  {
  public:
    virtual void serialize(Serializer& s)=0;
    virtual void deserialize(Deserializer& s)=0;
    virtual void write(ist::gzbstream& s)=0;
    virtual size_t getSessionCount()=0;
    virtual session_ptr getSession(size_t i)=0;
    virtual session_ptr getSessionByID(size_t id)=0;
    virtual size_t getSessionID()=0;
    virtual int getDelay()=0;
    virtual void push(const NMessage& m)=0;
    virtual void flush()=0;
    virtual void sync()=0;
    virtual void update()=0;
  };

  class BaseInputClient : public IInputClient
  {
  protected:
    typedef std::map<size_t, session_ptr> session_cont;
    session_cont m_session;

  public:
    virtual void serialize(Serializer& s)
    {
      s << m_session.size();
      for(session_cont::const_iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->serialize(s);
      }
    }

    virtual void deserialize(Deserializer& s)
    {
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr s(new Session(s));
        m_session[s->getID()] = s;
      }
    }

    virtual void write(ist::gzbstream& s)
    {
      s << m_session.size();
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->write(s);
      }
    }

    virtual size_t getSessionCount()
    {
      return m_session.size();
    }

    virtual session_ptr getSession(size_t i)
    {
      if(i>=m_session.size()) {
        return session_ptr();
      }
      session_cont::iterator p = m_session.begin();
      std::advance(p, i);
      return p->second;
    }

    virtual session_ptr getSessionByID(size_t id)
    {
      session_cont::iterator p = m_session.find(id);
      if(p==m_session.end()) {
        return session_ptr();
      }
      return p->second;
    }

    virtual session_ptr findSession(size_t id)
    {
      session_cont::iterator p = m_session.find(id);
      return (p==m_session.end()) ? session_ptr() : p->second;
    }

    virtual size_t getSessionID()=0;
    virtual int getDelay() { return 0; }
    virtual void push(const NMessage& m)=0;
    virtual void flush() {}
    virtual void sync() {}

    virtual void update()
    {
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        p->second->getInput().update();
      }
    }

    virtual void exec() {}
  };



  class InputClientLocal : public BaseInputClient
  {
  public:
    InputClientLocal()
    {
      session_ptr s(new Session(0, GetConfig()->scorename, new InputStream()));
      m_session[s->getID()] = s;
    }

    size_t getSessionID()
    {
      return 0;
    }

    void push(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::INPUT) {
        getSessionByID(m.cid)->getInput().push(m.input.input);
      }
      else if(t==NMessage::PAUSE) {
        Pause();
      }
      else if(t==NMessage::RESUME){
        Resume();
      }
      else if(t==NMessage::END)   {
        FadeToTitle();
      }
    }
  };


  class InputClientReplay : public BaseInputClient
  {
  typedef BaseInputClient Super;
  private:
    session_cont m_join;
    size_t m_length;

  public:
    InputClientReplay(ist::gzbstream& s, int version) : m_length(0)
    {
      size_t num;
      s >> num;
      for(int i=0; i<num; ++i) {
        session_ptr s(new Session(s, version));
        if(version==105) {
          s->setID(i);
        }
        if(i==0) {
          m_length = s->getBeginFrame()+s->getInput().getLength();
        }
        if(s->getBeginFrame()==0) {
          m_session[s->getID()] = s;
        }
        else {
          m_join[s->getID()] = s;
        }
      }
    }

    InputClientReplay() : m_length(0)
    {}

    virtual void serialize(Serializer& s)
    {
      Super::serialize(s);
      s << m_join.size();
      for(session_cont::const_iterator p=m_join.begin(); p!=m_join.end(); ++p) {
        p->second->serialize(s);
      }
      s << m_length;
    }

    virtual void deserialize(Deserializer& s)
    {
      Super::deserialize(s);
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr s(new Session(s));
        m_join[s->getID()] = s;
      }
      s >> m_length;
    }

    size_t getLength() { return m_length; }
    size_t getSessionID() { return 0; }

    void push(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::PAUSE) {
        Pause();
      }
      else if(t==NMessage::RESUME){
        Resume();
      }
      else if(t==NMessage::END)   {
        FadeToTitle();
      }
    }

    void sync()
    {
      IGame *game = GetGame();
      if(!game) {
        return;
      }
      for(session_cont::iterator p=m_join.begin(); p!=m_join.end();/**/) {
        session_ptr s = p->second;
        if(s->getBeginFrame()==game->getPast()) {
          m_session[s->getID()] = s;
          m_join.erase(p++);
        }
        else {
          ++p;
        }
      }
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end();/**/) {
        session_ptr s = p->second;
        if(s->getJoinFrame()==game->getPast()) {
          game->join(s->getID());
        }
        if(s->getLeaveFrame()==game->getPast()) {
          game->leave(s->getID());
          m_session.erase(p++);
        }
        else {
          ++p;
        }
      }
    }
  };


#ifdef EXCEPTION_ENABLE_NETPLAY 
  IInputServer* GetInputServer();

  class InputClientIP : public BaseInputClient
  {
  typedef BaseInputClient Super;
  private:
    socket_ptr m_socket;
    boost::recursive_mutex m_recv_mutex;
    boost::mutex m_send_mutex;
    nmessage_cont m_trecv_data;
    nmessage_cont m_recv_data;
    nmessage_cont m_send_data;
    nmessage_cont m_start_data;
    session_cont m_closed;
    bool m_stopped;
    size_t m_cid;
    size_t m_delay;
    bool m_request_state;

  public:
    InputClientIP() : m_stopped(false), m_cid(0), m_delay(5), m_request_state(false)
    {}

    ~InputClientIP()
    {
      stop();
      join();
    }

    void serialize(Serializer& s)
    {
      Super::serialize(s);
      s << m_closed.size();
      for(session_cont::const_iterator p=m_closed.begin(); p!=m_closed.end(); ++p) {
        p->second->serialize(s);
      }
      s << m_delay;

      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        s << m_recv_data.size();
        for(size_t i=0; i<m_recv_data.size(); ++i) {
          m_recv_data[i].serialize(s);
        }
      }
    }

    void deserialize(Deserializer& s)
    {
      Super::deserialize(s);
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        session_ptr s(new Session(s));
        m_closed[s->getID()] = s;
      }
      s >> m_delay;

      boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_recv_data.push_back(NMessage(s));
      }
    }

    void write(ist::gzbstream& s)
    {
      session_cont tmp;
      tmp.insert(m_session.begin(), m_session.end());
      tmp.insert(m_closed.begin(), m_closed.end());

      s << tmp.size();
      for(session_cont::iterator p=tmp.begin(); p!=tmp.end(); ++p) {
        p->second->write(s);
      }
    }


    int getDelay() { return m_delay; }
    size_t getSessionID() { return m_cid; }

    virtual session_ptr getSessionByID(size_t id)
    {
      if(session_ptr s = Super::getSessionByID(id)) {
        return s;
      }
      session_cont::iterator p = m_closed.find(id);
      if(p==m_closed.end()) {
        return session_ptr();
      }
      return p->second;
    }

    void stop() // sync 
    {
      m_stopped = true;
    }

    void run()
    {
      for(size_t i=0; i<m_start_data.size(); ++i) {
        dispatch(m_start_data[i]);
      }
      m_start_data.clear();
      Super::run();
    }

    void connect(const string& host, ushort port)
    {
      m_socket.reset(new tcp::iostream(host, boost::lexical_cast<string>(port)));
      if(!*m_socket) {
        throw NetworkError("connection failed");
      }
      m_socket->rdbuf()->set_option(tcp::no_delay(true));

      {
        IConfig& conf = *GetConfig();
        boost::mutex::scoped_lock lock(conf.getMutex());

        nmessage_cont sm;
        sm.push_back(NMessage::Entry(0, conf.scorename));
        SendNetworkMessage(*m_socket, sm);
      }
      {
        RecvNetworkMessage(*m_socket, m_start_data);
        for(size_t i=0; i<m_start_data.size(); ++i) {
          NMessage& m = m_start_data[i];
          if(m.type==NMessage::ACCEPT) {
            m_cid = m.cid;
          }
          else if(m.type==NMessage::REJECT) {
            throw NetworkError(string("rejected: ")+m.reject.text);
          }
        }
      }
    }


    void push(const NMessage& t) // sync 
    {
      if(t.type==NMessage::INPUT && !findSession(getSessionID())) {
        return;
      }
      boost::mutex::scoped_lock lock(m_send_mutex);
      m_send_data.push_back(t);
    }


    struct handler
    {
      InputClientIP *client;
      handler(InputClientIP *c) : client(c) {}
      bool operator()(NMessage& m) {
        return client->dispatch(m);
      }
    };
    friend struct handler;

    void flush() // sync 
    {
      boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
      m_recv_data.erase(std::remove_if(m_recv_data.begin(), m_recv_data.end(), handler(this)), m_recv_data.end());

      if(m_request_state) {
        m_request_state = false;
        if(IGame *game = GetGame()) {
          Serializer *seri = new Serializer();
          game->serialize(*seri);
          push(NMessage::State(seri, game->getPast()));
        }
      }
    }


    bool needSync()
    {
      if(!GetGame()) {
        return false;
      }
      for(session_cont::iterator p=m_session.begin(); p!=m_session.end(); ++p) {
        InputStream& input = p->second->getInput();
        if(input.getIndex()>=input.getLength()) {
          return true;
        }
      }
      return false;
    }

    void sync()
    {
      flush();

      // 遅れてるpeerがいたら待つ 
      while(needSync() && isRunning()) {
        sgui::Sleep(1);
        flush();
      }
    }

  private:
    bool dispatch(const NMessage& m)
    {
      int t = m.type;
      if(t==NMessage::INPUT) {
        if(session_ptr s = findSession(m.cid)) {
          s->getInput().push(m.input.input);
        }
        else {
          throw Error("NMessage::INPUT");
        }
      }
      else if(t==NMessage::CSTAT) {
        if(session_ptr s = findSession(m.cid)) {
          s->setPing(m.cstat.ping);
        }
      }
      else if(t==NMessage::STATE) {
        LoadState(dynamic_cast<Deserializer&>(*m.statep));
      }
      else if(t==NMessage::ENTRY) {
        if(IGame *game = GetGame()) {
          if(m.entry.frame > game->getPast()) {
            return false;
          }
          else if(m.entry.frame < game->getPast()) {
            throw Error("NMessage::ENTRY");
          }
        }
        if(!findSession(m.cid)) {
          session_ptr s(new Session(m.cid, m.entry.name, new InputStream()));
          m_session[m.cid] = s;
          PushChatText(string("# ")+s->getName()+string(" join"));
          if(IGame *game = GetGame()) {
            s->setBeginFrame(m.entry.frame);
            s->setJoinFrame(-1);
            for(int j=0; j<m_delay; ++j) {
              s->getInput().push(0);
            }
          }
        }
      }
      else if(t==NMessage::JOIN) {
        if(IGame *game = GetGame()) {
          if(m.join.frame > game->getPast()) {
            return false;
          }
          else if(m.join.frame < game->getPast()) {
            throw Error("NMessage::JOIN");
          }
          else if(m.join.frame==game->getPast()) {
            if(session_ptr s=findSession(m.cid)) {
              s->setJoinFrame(m.join.frame);
              game->join(m.cid);
            }
          }
        }
      }
      else if(t==NMessage::LEAVE) {
        if(IGame *game = GetGame()) {
          if(m.leave.frame > game->getPast()) {
            return false;
          }
          else if(m.leave.frame < game->getPast()) {
            throw Error("NMessage::LEAVE");
          }
          else if(m.leave.frame==game->getPast()) {
            session_ptr s;
            session_cont::iterator i = m_closed.find(m.cid);
            if(i!=m_closed.end()) {
              s = i->second;
            }
            else {
              s = findSession(m.cid);
            }
            if(s) {
              game->leave(m.cid);
              i->second->setLeaveFrame(m.leave.frame);
            }
          }
        }
      }
      else if(t==NMessage::TEXT)  {
        if(session_ptr s = findSession(m.cid)) {
          PushChatText(string(s->getName())+": "+m.text.text);
        }
      }
      else if(t==NMessage::PAUSE) {
        if(IGame *game = GetGame()) {
          if(m.pause.frame > game->getPast()) {
            return false;
          }
          else if(m.pause.frame<=game->getPast()) {
            Pause();
          }
        }
      }
      else if(t==NMessage::RESUME){
        if(IGame *game = GetGame()) {
          if(m.resume.frame > game->getPast()) {
            return false;
          }
          else if(m.pause.frame<=game->getPast()) {
            Resume();
          }
        }
      }
      else if(t==NMessage::START) {
        GameOption opt;
        opt.mode = m.start.mode;
        opt.stage = m.start.stage;
        opt.seed = m.start.seed;
        m_delay = m.start.delay;
        for(session_cont::iterator i=m_session.begin(); i!=m_session.end(); ++i) {
          for(int j=0; j<m_delay; ++j) {
            i->second->getInput().push(0);
          }
        }
        FadeToGame(opt);
      }
      else if(t==NMessage::END)   {
        if(IGame *game = GetGame()) {
          FadeToTitle();
        }
        else {
          m_closed.clear();
          m_session.clear();
        }
      }
      else if(t==NMessage::CLOSE)   {
        if(session_ptr s = findSession(m.cid)) {
          PushChatText(string("# ")+s->getName()+" disconnected");
          m_session.erase(s->getID());
          if(GetGame()) {
            m_closed[s->getID()] = s;
          }
        }
      }
      else if(t==NMessage::QUERY)   {
        string query(m.query.text);
        if(query=="state") {
          m_request_state = true;
        }
        else {
          return false;
        }
      }
      return true;
    }


    void exec()
    {
      try {
        while(!m_stopped) {
          send();
          recv();
        }
      }
      catch(...) {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        m_recv_data.push_back(NMessage::Text(0, "# connection closed"));
        m_recv_data.push_back(NMessage::End());
      }
      m_socket->close();
    }

    void send() // sync 
    {
      boost::mutex::scoped_lock lock(m_send_mutex);
      SendNetworkMessage(*m_socket, m_send_data);
    //  printf("send() %d\n", m_send_data.size());
      m_send_data.clear();
    }

    void recv() // sync 
    {
    //  sgui::Sleep(40); // ping確認用 
      RecvNetworkMessage(*m_socket, m_trecv_data);
    //  printf("recv() %d\n", m_trecv_data.size());
      for(size_t i=0; i<m_trecv_data.size(); ++i) {
        NMessage& m = m_trecv_data[i];
        if(m.type==NMessage::END) {
          m_stopped = true;
        }
      }
      {
        boost::recursive_mutex::scoped_lock lock(m_recv_mutex);
        m_recv_data.insert(m_recv_data.end(), m_trecv_data.begin(), m_trecv_data.end());
      }
      m_trecv_data.clear();
    }
  };
#endif // EXCEPTION_ENABLE_NETPLAY 

  typedef shared_ptr<IInputServer> iserver_ptr;
  typedef shared_ptr<IInputClient> iclient_ptr;

  extern iserver_ptr g_iserver;
  extern iclient_ptr g_iclient;

  IInputClient* GetInputClient();
  void SendNMessage(const NMessage& t);
  bool IsLocalMode();
  bool IsServerMode();
  bool IsClientMode();
  bool IsReplayMode();
  size_t GetSessionCount();
  session_ptr GetSession(size_t i);
  session_ptr GetSessionByID(size_t i);

}
#endif

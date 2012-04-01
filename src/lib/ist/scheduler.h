#ifndef ist_scheduler
#define ist_scheduler

#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
  #define WINVER 0x0500
#endif

#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace ist {

class Task;
typedef boost::shared_ptr<Task> task_ptr;


namespace impl {
  class TaskThread;
  class TaskQueue;
}

class Task
{
friend class Scheduler;
friend class impl::TaskQueue;
friend class impl::TaskThread;
private:
  bool m_finished;
  boost::thread::id m_affinity;

  void initialize();
  void finalize();

public:
  Task();
  virtual ~Task();

  void setAffinity(boost::thread::id affinity);
  boost::thread::id getAffinity() const;
  bool isFinished() const;

  virtual void operator()()=0;
};


namespace impl {

  class TaskQueue
  {
  private:
    typedef std::list<task_ptr> task_cont;
    task_cont m_tasks;
    size_t m_num_waiting;
    boost::mutex m_suspender;
    boost::condition_variable m_cond;

  public:
    TaskQueue();
    size_t getNumWaiting() const;
    bool empty();
    void push(task_ptr t);

    template<class Iter>
    void push(Iter begin, Iter end)
    {
      boost::lock_guard<boost::mutex> lock(m_suspender);
      for(; begin!=end; ++begin) {
        task_ptr t = boost::static_pointer_cast<Task>(*begin);
        t->initialize();
        m_tasks.push_back(t);
      }

      notify();
    }

    task_ptr pop();
    task_ptr waitForNewTask();
    void notify();
  };



  class TaskThread
  {
  private:
    TaskQueue& m_task_queue;
    bool m_stop_flag;
    boost::shared_ptr<boost::thread> m_thread;
    boost::condition_variable m_cond;
    task_ptr m_current_task;

  public:
    TaskThread(TaskQueue& tq, int processor);
    ~TaskThread();
    void stop();
    boost::thread::id getID() const;
    void operator()();
  };
}


class Scheduler
{
private:
  typedef boost::shared_ptr<impl::TaskThread> thread_ptr;
  typedef std::vector<thread_ptr> thread_cont;
  impl::TaskQueue m_task_queue;
  thread_cont m_threads;
  static Scheduler *s_instance;

public:
  static Scheduler* instance();

  // Singleton。複数インスタンス作ろうとすると例外投げる。 
  // 引数は0以下の場合CPUの数に自動調整。 
  Scheduler(int num_thread=0);

  // 現在処理中のタスクの完了を待ってから停止する。 
  // (タスクキューが空になるのを待たない) 
  ~Scheduler();

  // 全タスクの完了を待つ。タスクキューが空ではない場合、呼び出し元スレッドもタスク処理に加わる。 
  // タスク内から呼ぶと永久停止するのでやっちゃダメ。 
  void waitForAll();

  // 指定タスクの完了を待つ。タスクキューが空ではない場合、呼び出し元スレッドもタスク処理に加わる。 
  void waitFor(task_ptr task);

  // 範囲指定バージョン 
  template<class Iter>
  void waitFor(Iter begin, Iter end)
  {
    while(true) {
      bool finished = true;
      for(Iter i=begin; i!=end; ++i) {
        if(!(*i)->isFinished()) {
          finished = false;
          break;
        }
      }

      if(finished) {
        break;
      }
      else if(task_ptr t = m_task_queue.pop()) {
        (*t)();
        t->finalize();
      }
      else {
        boost::this_thread::yield();
      }
    }
  }


  // タスクのスケジューリングを行う。 
  void schedule(task_ptr task);

  // 範囲指定バージョン 
  template<class Iter>
  void schedule(Iter begin, Iter end)
  {
    m_task_queue.push(begin, end);
  }


  size_t getThreadCount() const;
  boost::thread::id getThreadID(size_t i) const;


  // 以下デバッグ用。通常は使っちゃダメ。 
  impl::TaskQueue& getTaskQueue();
};

} // namespace ist
#endif // ist_scheduler 

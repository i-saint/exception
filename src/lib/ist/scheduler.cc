#include "scheduler.h"

namespace ist {


Task::Task() : m_finished(true)
{}

Task::~Task()
{}

void Task::initialize()
{
  m_finished = false;
}

void Task::finalize()
{
  m_finished = true;
}

void Task::setAffinity(boost::thread::id affinity)
{
  m_affinity = affinity;
}

boost::thread::id Task::getAffinity() const
{
  return m_affinity;
}

bool Task::isFinished() const
{
  return m_finished;
}


namespace impl {

  TaskQueue::TaskQueue() : m_num_waiting(0)
  {}

  size_t TaskQueue::getNumWaiting() const
  {
    return m_num_waiting;
  }

  bool TaskQueue::empty()
  {
    boost::unique_lock<boost::mutex> lock(m_suspender);
    return m_tasks.empty();
  }

  void TaskQueue::push(task_ptr t)
  {
    boost::lock_guard<boost::mutex> lock(m_suspender);

    t->initialize();
    m_tasks.push_back(t);

    notify();
  }

  task_ptr TaskQueue::pop()
  {
    boost::unique_lock<boost::mutex> lock(m_suspender);
    task_ptr t;
    for(task_cont::iterator i=m_tasks.begin(); i!=m_tasks.end(); ++i) {
      boost::thread::id affinity = (*i)->getAffinity();
      if(affinity==boost::thread::id() || affinity==boost::this_thread::get_id()) {
        t = *i;
        m_tasks.erase(i);
        break;
      }
    }
    return t;
  }

  task_ptr TaskQueue::waitForNewTask()
  {
    {
      boost::unique_lock<boost::mutex> lock(m_suspender);
      for(task_cont::iterator i=m_tasks.begin(); i!=m_tasks.end(); ++i) {
        boost::thread::id affinity = (*i)->getAffinity();
        if(affinity==boost::thread::id() || affinity==boost::this_thread::get_id()) {
          task_ptr t = *i;
          m_tasks.erase(i);
          return t;
        }
      }
      ++m_num_waiting;
      m_cond.wait(lock);
    }
    return pop();
  }

  void TaskQueue::notify()
  {
    m_num_waiting = 0;
    m_cond.notify_all();
  }



  TaskThread::TaskThread(TaskQueue& tq, int processor) :
    m_task_queue(tq), m_stop_flag(false)
  {
    m_thread.reset(new boost::thread(boost::ref(*this)));
#ifdef WIN32
    ::SetThreadAffinityMask(m_thread->native_handle(), 1<<processor);
#endif
  }

  TaskThread::~TaskThread()
  {
    m_thread->join();
  }

  void TaskThread::stop()
  {
    m_stop_flag = true;
  }

  boost::thread::id TaskThread::getID() const
  {
    return m_thread->get_id();
  }

  void TaskThread::operator()()
  {
    while(!m_stop_flag) {
      if(task_ptr t = m_task_queue.waitForNewTask()) {
        (*t)();
        t->finalize();
      }
    }
  }
} // namespace impl 



Scheduler* Scheduler::s_instance = 0;

Scheduler* Scheduler::instance()
{
  return s_instance;
}

Scheduler::Scheduler(int num_thread)
{
  if(s_instance!=0) {
    throw std::runtime_error("Scheduler::Scheduler()");
  }
  s_instance = this;

  int processors = 2;
#ifdef WIN32
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  processors = info.dwNumberOfProcessors;

  ::SetThreadAffinityMask(::GetCurrentThread(), 1);
#endif
  if(num_thread<=0) {
    num_thread = processors;
  }

  for(int i=1; i<num_thread; ++i) {
    thread_ptr t(new impl::TaskThread(m_task_queue, i%processors));
    m_threads.push_back(t);
  }
}

Scheduler::~Scheduler()
{
  for(size_t i=0; i<m_threads.size(); ++i) {
    m_threads[i]->stop();
  }
  m_task_queue.notify();
  m_threads.clear();

  s_instance = 0;
}

void Scheduler::waitForAll()
{
  while(m_task_queue.getNumWaiting()<m_threads.size() || !m_task_queue.empty()) {
    if(task_ptr t = m_task_queue.pop()) {
      (*t)();
      t->finalize();
    }
    else {
      boost::this_thread::yield();
    }
  }
}

void Scheduler::waitFor(task_ptr task)
{
  while(!task->isFinished()) {
    if(task_ptr t = m_task_queue.pop()) {
      (*t)();
      t->finalize();
    }
    else {
      boost::this_thread::yield();
    }
  }
}

void Scheduler::schedule(task_ptr task)
{
  m_task_queue.push(task);
}


size_t Scheduler::getThreadCount() const
{
  return m_threads.size();
}

boost::thread::id Scheduler::getThreadID(size_t i) const
{
  return m_threads[i]->getID();
}



} // namespace ist

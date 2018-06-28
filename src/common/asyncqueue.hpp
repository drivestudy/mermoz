#ifndef ASYNCQUEUE_H__
#define ASYNCQUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class async_queue
{
public:
  async_queue() {}
  ~async_queue() {}

  T& front();
  T pop_out();
  void pop();

  void push(const T& val);

  size_t size();
  bool empty();

private:
  size_t max_s;
  std::queue<T> queue;
  std::mutex mtx;
  std::condition_variable cond;
};

template<typename T>
T async_queue<T>::pop_out()
{
  std::unique_lock<std::mutex> mlock(mtx);
  while(queue.empty())
  {
    cond.wait(mlock);
  }
  T val = queue.front();
  queue.pop();
  cond.notify_one();
  return val;
}

template<typename T>
T& async_queue<T>::front()
{
  std::unique_lock<std::mutex> mlock(mtx);
  while(queue.empty())
  {
    cond.wait(mlock);
  }
  return queue.front();
}

template<typename T>
void async_queue<T>::pop()
{
  std::unique_lock<std::mutex> mlock(mtx);
  while(queue.empty())
  {
    cond.wait(mlock);
  }
  queue.pop();
  cond.notify_one();
}

template<typename T>
void async_queue<T>::push(const T& val)
{
  std::unique_lock<std::mutex> mlock(mtx);
  queue.push(val);
  cond.notify_one();
  mlock.unlock();
}

template<typename T>
size_t async_queue<T>::size()
{
  std::unique_lock<std::mutex> mlock(mtx);
  size_t s = queue.size();
  mlock.unlock();
  return s;
}

template<typename T>
bool async_queue<T>::empty()
{
  std::unique_lock<std::mutex> mlock(mtx);
  bool e = queue.empty();
  mlock.unlock();
  return e;
}

#endif // ASYNCQUEUE_H__

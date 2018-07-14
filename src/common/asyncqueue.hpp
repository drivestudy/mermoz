/*
 * MIT License
 * 
 * Copyright (c) 2018 Qwant Research 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * Author:
 * Noel Martin (n.martin@qwantresearch.com)
 *
 */

#ifndef MERMOZ_ASYNCQUEUE_H__
#define MERMOZ_ASYNCQUEUE_H__

#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace mermoz
{
namespace common
{

template<typename ValueType>
class AsyncQueue
{
public:
  AsyncQueue() {}
  ~AsyncQueue() {}

  ValueType& front();
  ValueType& back();

  void pop();
  void pop(ValueType& val);
  bool pop_for(ValueType& val, const int time_ms);

  void push(const ValueType& val);

  size_t size();
  bool empty();

private:
  std::mutex mutex;
  std::condition_variable cond;

  std::queue<ValueType> queue;
};

template<typename ValueType>
ValueType& AsyncQueue<ValueType>::front()
{
  std::unique_lock<std::mutex> mlock(mutex);

  while(queue.empty())
    cond.wait(mlock);

  return queue.front();
}

template<typename ValueType>
void AsyncQueue<ValueType>::pop()
{
  std::unique_lock<std::mutex> mlock(mutex);

  while(queue.empty())
    cond.wait(mlock);

  queue.pop();

  mlock.unlock();
  cond.notify_one();
}

template<typename ValueType>
void AsyncQueue<ValueType>::pop(ValueType& val)
{
  std::unique_lock<std::mutex> mlock(mutex);

  while(queue.empty())
    cond.wait(mlock);

  val = queue.front();
  queue.pop();

  mlock.unlock();
  cond.notify_one();
}

template<typename ValueType>
bool AsyncQueue<ValueType>::pop_for(ValueType& val, const int time_ms)
{
  using namespace std::chrono_literals;

  std::unique_lock<std::mutex> mlock(mutex);

  std::cv_status res;
  if(queue.empty())
    res = cond.wait_for(mlock, time_ms*1ms);

  if (res == std::cv_status::timeout)
    return false;

  val = queue.front();
  queue.pop();

  mlock.unlock();
  cond.notify_one();

  return true;
}

template<typename ValueType>
void AsyncQueue<ValueType>::push(const ValueType& val)
{
  std::unique_lock<std::mutex> mlock(mutex);

  queue.push(val);

  mlock.unlock();
  cond.notify_one();
}

template<typename ValueType>
size_t AsyncQueue<ValueType>::size()
{
  std::unique_lock<std::mutex> mlock(mutex);
  return queue.size();
}

template<typename ValueType>
bool AsyncQueue<ValueType>::empty()
{
  std::unique_lock<std::mutex> mlock(mutex);
  return queue.empty();
}

} // namespace common
} // namespace mermoz

#endif // MERMOZ_ASYNCQUEUE_H__

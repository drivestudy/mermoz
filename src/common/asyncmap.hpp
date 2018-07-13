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

#ifndef MERMOZ_ASYNCMAP_H__
#define MERMOZ_ASYNCMAP_H__

#include <map>
#include <utility>
#include <mutex>
#include <condition_variable>

namespace mermoz
{
namespace common
{

template<typename KeyType, typename MappedType>
class AsyncMap
{
public:
  using ValueType = std::pair<const KeyType, MappedType>;

  AsyncMap() {}
  ~AsyncMap() {}

  MappedType& operator[](const KeyType& key);

  std::pair<typename std::map<KeyType, MappedType>::iterator,bool> insert(const ValueType& val); 

  void erase(const KeyType& k);

  typename std::map<KeyType, MappedType>::iterator find(const KeyType& k);

  typename std::map<KeyType, MappedType>::iterator end();

  size_t size();

private:
  std::mutex mutex;
  std::condition_variable cond;

  std::map<KeyType, MappedType> map;
}; // class AsyncMap

template<typename KeyType, typename MappedType>
MappedType& AsyncMap<KeyType, MappedType>::operator[](const KeyType& key)
{
  return map[key];
}

template<typename KeyType, typename MappedType>
std::pair<typename std::map<KeyType, MappedType>::iterator,bool>
AsyncMap<KeyType, MappedType>::insert(const ValueType& val) 
{
  std::unique_lock<std::mutex> mlock(mutex);
  return map.insert(val);
}

template<typename KeyType, typename MappedType>
void AsyncMap<KeyType, MappedType>::erase(const KeyType& k)
{
  std::unique_lock<std::mutex> mlock(mutex);
  map.erase(k);
}

template<typename KeyType, typename MappedType>
typename std::map<KeyType, MappedType>::iterator
AsyncMap<KeyType, MappedType>::find(const KeyType& k)
{
  std::unique_lock<std::mutex> mlock(mutex);
  return map.find(k);
}

template<typename KeyType, typename MappedType>
typename std::map<KeyType, MappedType>::iterator
AsyncMap<KeyType, MappedType>::end()
{
  return map.end();
}

template<typename KeyType, typename MappedType>
size_t AsyncMap<KeyType, MappedType>::size()
{
  std::unique_lock<std::mutex> mlock(mutex);
  return map.size();
}

} // namespace common
} // namespace mermoz

#endif // MERMOZ_ASYNCMAP_H__

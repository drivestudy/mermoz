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

#include "common/memsec.hpp"
#include "common/logs.hpp"

namespace mermoz
{
namespace common
{

MemSec& MemSec::operator+=(uint64_t mem)
{
  if (cur_mem + mem > max_mem)
  {
    std::unique_lock<std::mutex> mlock(mtx);

    while (cur_mem + mem > max_mem)
      cond.wait(mlock);

    mlock.unlock();
  }

  cur_mem += mem;
}

MemSec& MemSec::operator-=(uint64_t mem)
{
  if (mem > cur_mem)
  {
    print_warning("Memory to remove supperior to the current memory");
    cur_mem = 0;
  }
  else
  {
    cur_mem -= mem;
  }

  cond.notify_one();
}

} // namespace common
} // namespace mermoz

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

#include "common/packer.hpp"

namespace mermoz
{
namespace common
{

void pack(std::string& pack, std::initializer_list<std::string*> args)
{
  if (args.size() == 0)
    return;

  size_t pos {0};
  for (auto& arg : args)
  {
    pos += arg->size() + sizeof(size_t);
  }

  pack.clear();
  pack.resize(pos);

  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size = arg->size();

    std::memcpy((void*)(&*itpack),
                (void*)(&arg_size),
                sizeof(size_t));

    itpack += sizeof(size_t);

    std::memcpy((void*)(&*itpack),
                (void*)(arg->data()),
                arg_size);

    itpack += arg_size;
  }
}

void unpack(std::string& pack, std::initializer_list<std::string*> args)
{
  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size;

    std::memcpy((void*)(&arg_size),
                (void*)(&*itpack),
                sizeof(size_t));

    itpack += sizeof(size_t);

    arg->clear();
    arg->resize(arg_size);

    std::memcpy((void*)(&*arg->begin()),
                (void*)(&*itpack),
                arg_size);

    itpack += arg_size;
  }
}

} // namespace spider
} // namespace mermoz

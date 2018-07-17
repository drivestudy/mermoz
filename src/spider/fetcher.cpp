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

#include <ctime>
#include "spider/fetcher.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace spider
{

void fetcher(mc::AsyncQueue<std::string>* url_queue,
             mc::AsyncQueue<std::string>* content_queue,
             std::string user_agent,
             std::atomic<uint64_t>* nfetched,
             mc::MemSec* mem_sec,
             bool* do_fetch)
{
  while (*do_fetch)
  {
    std::string url;
    url_queue->pop(url);
    (*mem_sec) -= url.size();

    std::string content;
#   ifdef MMZ_PROFILE
    long http_code = mc::http_fetch(url, content, 60L, user_agent);
#   else
    long http_code = mc::http_fetch(url, content, 10L, user_agent);
#   endif

    std::string http_code_string(std::to_string(http_code));

    std::string message;
    mc::pack(message, {&url, &content, &http_code_string});

    (*mem_sec) += message.size();
    content_queue->push(message);

    ++(*nfetched);
  }
}

} // namespace spider
} // namespace mermoz


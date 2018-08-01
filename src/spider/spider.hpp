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

#ifndef MERMOZ_SPIDER_H__
#define MERMOZ_SPIDER_H__

#include <atomic>

#include "tsafe/thread_safe_queue.h"

#include "spider/fetcher.hpp"
#include "spider/parser.hpp"

using TSQueueVector = std::vector<thread_safe::queue<std::string>>;

namespace mermoz
{

typedef struct SpiderSettings {
  unsigned int num_threads_fetchers;
  unsigned int num_threads_parsers;
  std::string user_agent;
  std::atomic<uint64_t>* nfetched;
  std::atomic<uint64_t>* nparsed;
  MemSec* mem_sec;
} SpiderSettings;


/*
 * General header definition of
 * the spider thread function
 */
void spider(bool* status, // defines if thread runs or not
            SpiderSettings* sset, // general settings
            TSQueueVector* url_queues, // incomming data
            TSQueueVector* content_queues); // outcomming data

} // namespace mermoz

#endif // MERMOZ_SPIDER_H__

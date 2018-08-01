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

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include "spider/spider.hpp"

namespace mermoz
{

void spider(bool* status, // defines if thread runs or not
            SpiderSettings* ssets, // general settings
            TSQueueVector* url_queues, // incomming data
            TSQueueVector* content_queues) // outcomming data
{
  TSQueueVector out_fetch(ssets->num_threads_fetchers);
  std::vector<std::thread> fetchers;

  for (unsigned int f_id = 0; f_id < ssets->num_threads_fetchers; f_id++) {
    fetchers.push_back(std::thread(fetcher, &url_queues->at(f_id), &out_fetch.at(f_id), ssets->user_agent, ssets->nfetched, ssets->mem_sec, status));
  }

  TSQueueVector in_parse(ssets->num_threads_parsers);
  std::vector<std::thread> parsers;

  for (unsigned int p_id = 0; p_id < ssets->num_threads_parsers; p_id++) {
    parsers.push_back(std::thread(parser, &in_parse.at(p_id), &content_queues->at(p_id), ssets->nparsed, ssets->mem_sec, status));
  }

  /*
   * Funnel function
   */
  unsigned int fetcher_id {0};
  unsigned int parser_id {0};
  while (*status) {
    if (!out_fetch.at(fetcher_id).empty()) {
      std::string tmp;
      out_fetch.at(fetcher_id).pop(tmp);
      in_parse.at(parser_id).push(tmp);

      parser_id++;
      if (parser_id >= ssets->num_threads_parsers) {
        parser_id = 0;
      }
    }

    fetcher_id++;
    if (fetcher_id >= ssets->num_threads_fetchers) {
      fetcher_id = 0;
    }
  }

  for (auto& t : fetchers)
    t.join();

  for (auto& t : parsers)
    t.join();
}

} // namespace mermoz

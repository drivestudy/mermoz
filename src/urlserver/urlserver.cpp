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

#include "urlserver/urlserver.hpp"

#include <curl/curl.h>
#include <ctime>
#include <csignal>

namespace mc = mermoz::common;

namespace mermoz
{
namespace urlserver
{

void urlserver(mermoz::common::AsyncQueue<std::string>* content_queue,
               mermoz::common::AsyncQueue<std::string>* url_queue,
               std::string user_agent,
               mc::MemSec* mem_sec,
               bool* status)
{
  std::signal(SIGPIPE, SIG_IGN);

  std::set<std::string> visited;
  std::set<std::string> to_visit;
  std::set<std::string> parsed_urls;

  std::map<std::string, urlfactory::Robots> robots;
  std::queue<std::string> robots_queue;
  const size_t robots_limit {100000};

  mermoz::common::AsyncQueue<std::string> allowed_queue;
  bool throwlist {false};

  std::thread t(dispatcher,
                url_queue,
                &allowed_queue,
                100U, // number stacks
                50U, // stack size
                &throwlist,
                status);
  t.detach();

  while (*status) {
    std::string content;
    bool res = content_queue->pop_for(content, 1000);

    // Very important verification todo !
    throwlist = !res && parsed_urls.empty();

    if (!content.empty() && res) {
      (*mem_sec) -= content.size();

      std::string url;
      std::string text;
      std::string links;
      std::string http_status;

      mc::unpack(content, {&url, &text, &links, &http_status});

      std::set<std::string>::iterator it;
      if ((it = to_visit.find(url)) != to_visit.end()) {
        to_visit.erase(it);
        (*mem_sec) -= url.size();
      }

      (*mem_sec) += url.size();
      visited.insert(url);

      std::string link;
      std::istringstream iss(links);

      while(!iss.eof()) {
        std::getline(iss, link);

        if (link.size() > 1) {
          if (visited.find(link) == visited.end()
              && to_visit.find(link) == to_visit.end()
              && parsed_urls.find(link) == parsed_urls.end()) {
            (*mem_sec) += link.size();
            parsed_urls.insert(link);
          }
        }
      }
    }

    // dispatching tasks
    for(auto purlit = parsed_urls.begin();
        purlit != parsed_urls.end();
       ) {
      urlfactory::UrlParser up(*purlit);

      std::map<std::string, urlfactory::Robots>::iterator mapit;

      if ((mapit = robots.find(up.get_host())) == robots.end()) {
        if (robots_queue.size() > robots_limit) {
          robots.erase(robots_queue.front());
          robots_queue.pop();
        }

        robots.emplace(up.get_host(),
                       urlfactory::Robots(up.get_url(true, true, false, false, false), "Qwantify", user_agent));
        robots[up.get_host()].async_init();
        robots_queue.push(up.get_host());

        purlit++;
      } else {
        if (mapit->second.good()) {
          if (mapit->second.is_allowed(up)
              && to_visit.find(*purlit) == to_visit.end()) {
            (*mem_sec) += 2*purlit->size();

            allowed_queue.push(*purlit);
            to_visit.insert(*purlit);
          }

          (*mem_sec) -= purlit->size();
          purlit = parsed_urls.erase(purlit);
        } else {
          purlit++;
        } // if (mapit->second.good())
      } // if (mapit == robots.end())
    } // for (auto& purlit : parsed_urls)
  } // while (*status)
}

void dispatcher(mermoz::common::AsyncQueue<std::string>* outurls_queue,
                mermoz::common::AsyncQueue<std::string>* allowed_queue,
                unsigned int num_stacks,
                unsigned int stack_size,
                bool* throwlist,
                bool* status)
{
  unsigned int stack_id {0};
  std::vector<std::queue<std::string>> stacks(num_stacks);

  while (*status) {
    std::string url;
    bool res = allowed_queue->pop_for(url, 100);

    if (res) {
      stacks[stack_id].push(url);

      if (stacks[stack_id].size() >= stack_size) {
        while (!stacks[stack_id].empty()) {
          outurls_queue->push(stacks[stack_id].front());
          stacks[stack_id].pop();
        }
      }
    } else if (*throwlist) {
      for (unsigned int sid = 0; sid < num_stacks; sid++) {
        while (!stacks[sid].empty()) {
          outurls_queue->push(stacks[sid].front());
          stacks[sid].pop();
        }
      }
    }

    stack_id+1 >= num_stacks ? stack_id = 0 : stack_id++;
  }
}

} // namespace urlserver
} // namespace mermoz

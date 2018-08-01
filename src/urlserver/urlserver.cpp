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

namespace mermoz
{

void urlserver(bool* status,
               UrlServerSettings* usets,
               TSQueueVector* content_queues,
               TSQueueVector* url_queues)
{
  std::signal(SIGPIPE, SIG_IGN);

  std::set<std::string> visited;
  std::set<std::string> to_visit;
  std::set<std::string> parsed_urls;

  std::map<std::string, urlfactory::Robots> robots;
  std::queue<std::string> robots_queue;
  const size_t robots_limit {100000};

  thread_safe::queue<std::string> allowed_queue;

  std::thread t(dispatcher,
                status,
                &allowed_queue,
                url_queues);
  t.detach();

  unsigned int parser_id {0};

  while (*status) {
    if (!content_queues->at(parser_id).empty()) {
      std::string content;
      content_queues->at(parser_id).pop(content);

      (*usets->mem_sec) -= content.size();

      std::string url;
      std::string eff_url;
      std::string text;
      std::string links;
      std::string http_status;

      unpack(content, {&url, &eff_url, &http_status, &text, &links});

      std::set<std::string>::iterator it;
      if ((it = to_visit.find(url)) != to_visit.end()) {
        to_visit.erase(it);
        (*usets->mem_sec) -= url.size();
      }

      (*usets->mem_sec) += url.size();
      visited.insert(url);

      std::string link;
      std::istringstream iss(links);

      while(!iss.eof()) {
        std::getline(iss, link);

        if (link.size() > 1) {
          if (visited.find(link) == visited.end()
              && to_visit.find(link) == to_visit.end()
              && parsed_urls.find(link) == parsed_urls.end()) {
            (*usets->mem_sec) += link.size();
            parsed_urls.insert(link);
          }
        }
      }
    }

    parser_id++;
    if (parser_id >= content_queues->size()) {
      parser_id = 0;
    }

    // dispatching tasks
    for(auto purlit = parsed_urls.begin();
        purlit != parsed_urls.end();) {
      urlfactory::UrlParser up(*purlit);

      std::map<std::string, urlfactory::Robots>::iterator mapit;

      if ((mapit = robots.find(up.get_host())) == robots.end()) {
        if (robots_queue.size() > robots_limit) {
          robots.erase(robots_queue.front());
          robots_queue.pop();
        }

        robots.emplace(up.get_host(),
                       urlfactory::Robots(up.get_url(true, true, false, false, false),
                                          "Qwantify",
                                          usets->user_agent)
                      );

        robots[up.get_host()].async_init();

        robots_queue.push(up.get_host());

        purlit++;
      } else {
        if (mapit->second.good()) {
          if (mapit->second.is_allowed(up)
              && to_visit.find(*purlit) == to_visit.end()) {
            (*usets->mem_sec) += 2*purlit->size();

            allowed_queue.push(*purlit);
            to_visit.insert(*purlit);
          }

          (*usets->mem_sec) -= purlit->size();
          purlit = parsed_urls.erase(purlit);
        } else {
          purlit++;
        } // if (mapit->second.good())
      } // if (mapit == robots.end())
    } // for (auto& purlit : parsed_urls)
  } // while (*status)
}

void dispatcher(bool* status,
                thread_safe::queue<std::string>* allowed_queue,
                TSQueueVector* url_queues)
{
  unsigned int fetcher_id {0};

  while (*status) {
    if (!allowed_queue->empty()) {
      std::string url;
      allowed_queue->pop(url);
      url_queues->at(fetcher_id).push(url);
    }

    fetcher_id++;
    if (fetcher_id >= url_queues->size()) {
      fetcher_id = 0;
    }
  }
}

} // namespace mermoz

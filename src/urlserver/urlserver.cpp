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
#include <list>
#include <map>

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

      if (url.compare(eff_url) != 0) {
        /*
         * One considers that URLs differs (redirection)
         * and this must be saved
         */
        visited.insert(eff_url);
      }

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
  std::map<std::string, std::pair<unsigned int, unsigned int>> history_map;
  std::queue<std::string> history_order;

  const unsigned long num_fetchers {url_queues->size()};
  const unsigned int max_fetch_per_site {10};
  unsigned int fetcher_id {0};
  unsigned int num_sent {0};

  auto hmapit = history_map.begin();

  while (*status) {
    if (num_sent%num_fetchers == 0) {
      for (hmapit = history_map.begin(); hmapit != history_map.end(); hmapit++) {
        if (num_sent - hmapit->second.first > num_fetchers) {
          // this means that more than 200 request where sent
          // while the last call, so we set it to zero
          hmapit->second.second = 0U;
        }
      }
    }

    if (!allowed_queue->empty()) {
      std::string url;
      allowed_queue->pop(url);
      urlfactory::UrlParser up(url);

      if ((hmapit = history_map.find(up.get_host())) != history_map.end()) {
        // the domain was found in the history
        if (hmapit->second.second < max_fetch_per_site) {
          // We are under the limit, ok to send it
          url_queues->at(fetcher_id).push(url);

          fetcher_id++;
          if (fetcher_id >=  num_fetchers) {
            fetcher_id = 0;
          }

          num_sent++;
          hmapit->second.second++;
        } else {
          // to many request, put it in the queue
          allowed_queue->push(url);
        }
      } else {
        // the domain was not found in the history
        if (history_order.size() > num_fetchers) {
          // the memory does not need to be greater 
          // than the number of fetchers !
          history_map.erase(history_order.front());
          history_order.pop();
        }
        history_map.emplace(up.get_host(), std::pair<unsigned int, unsigned int>(num_sent, 1U));
        url_queues->at(fetcher_id).push(url);
        num_sent++;

        fetcher_id++;
        if (fetcher_id >=  num_fetchers) {
          fetcher_id = 0;
        }
      }
    }
  }
}

} // namespace mermoz

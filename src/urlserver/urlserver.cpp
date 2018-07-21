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

  std::map<std::string, Robots> robots;

  while (*status)
  {
    std::string content;
    bool res = content_queue->pop_for(content, 1000);

    if (!content.empty() && res)
    {
      (*mem_sec) -= content.size();

      std::string url;
      std::string text;
      std::string links;
      std::string http_status;

      mc::unpack(content, {&url, &text, &links, &http_status});

      std::set<std::string>::iterator it;
      if ((it = to_visit.find(url)) != to_visit.end())
      {
        to_visit.erase(it);
        (*mem_sec) -= url.size();
      }

      (*mem_sec) += url.size();
      visited.insert(url);

      std::string link;
      std::istringstream iss(links);

      while(!iss.eof())
      {
        std::getline(iss, link);

        if (link.size() > 1)
        {
          if (visited.find(link) == visited.end()
              && to_visit.find(link) == to_visit.end()
              && parsed_urls.find(link) == parsed_urls.end())
          {
            (*mem_sec) += link.size();
            parsed_urls.insert(link);
          }
        }
      }
    }

    // dispatching tasks
    for(auto purlit = parsed_urls.begin();
        purlit != parsed_urls.end();)
    {
      mc::UrlParser up(*purlit);

      std::map<std::string, Robots>::iterator mapit; 

      if ((mapit = robots.find(up.domain)) == robots.end())
      {
        robots.emplace(up.domain, Robots(up.scheme + "://" + up.domain, "Qwantify", user_agent));
        robots[up.domain].async_init();
      }
      else
      {
        bool allowed {false};

        if (mapit->second.good())
        {
          allowed = mapit->second.is_allowed(up);

          if (allowed)
          {
            (*mem_sec) += 2*purlit->size();
            url_queue->push(*purlit);
            to_visit.insert(*purlit);
          }

          (*mem_sec) -= purlit->size();
          purlit = parsed_urls.erase(purlit);

          continue;
        }
      }

      purlit++;
    }
  }
}

} // namespace urlserver
} // namespace mermoz

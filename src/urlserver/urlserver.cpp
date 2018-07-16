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
  std::set<std::string> visited;
  std::set<std::string> to_visit;
  std::set<std::string> parsed_urls;

  mc::AsyncQueue<std::string> robots_to_fetch;
  mc::AsyncMap<std::string, Robots> robots;

  size_t counter {0};

  std::thread t(robot_manager, &robots_to_fetch, &robots, user_agent, status);
  t.detach();

  while (*status)
  {
    std::string content;
    bool res = content_queue->pop_for(content, 1000);
    (*mem_sec) -= content.size();

    if (res)
    {
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

      // parsing the incoming string
      std::string link;
      for (auto c : links)
      {
        if ((c == ' ' || c == ',') && !link.empty())
        {
          if (visited.find(link) == visited.end()
              && to_visit.find(link) == to_visit.end()
              && parsed_urls.find(link) == parsed_urls.end())
          {
            (*mem_sec) += link.size();
            parsed_urls.insert(link);
          }

          link.clear();
          continue;
        }

        link.push_back(c);
      }
    }

    // dispatching tasks
    for(auto it = parsed_urls.begin();
        it != parsed_urls.end();)
    {
      mc::UrlParser up(*it);

      if (robots.find(up.domain) == robots.end())
      {
        robots_to_fetch.push(up.domain);
        it++;
      }
      else
      {
        bool is_ok {false};
        try
        {
          is_ok = robots[up.domain].is_allowed(*it);
        }
        catch(...)
        {
          std::cerr << "Error for Robots check of " << *it << std::endl;
        }

        if (is_ok)
        {
          (*mem_sec) += 2*it->size();
          url_queue->push(*it);
          to_visit.insert(*it);
        }

        (*mem_sec) -= it->size();
        it = parsed_urls.erase(it);
      }
    }
  }
}

void robot_manager(mermoz::common::AsyncQueue<std::string>* robots_to_fetch,
                   mermoz::common::AsyncMap<std::string, Robots>* robots,
                   std::string user_agent,
                   bool* status)
{
  std::queue<std::string> ordered_domains;
  std::set<std::string> domains;

  while (&status)
  {
    std::string domain;
    robots_to_fetch->pop(domain);

    std::ostringstream oss;
    oss << "Number of robots saved: " << domain.size();
    mc::print_strong_log(oss.str());

    if (domains.find(domain) != domains.end())
    {
      continue;
    }
    else
    {
      ordered_domains.push(domain);
      domains.insert(domain);
    }

    if (domains.size() > 100000)
    {
      robots->erase(ordered_domains.front());
      domains.erase(ordered_domains.front());
      ordered_domains.pop();
    }

    robots->insert(std::pair<std::string, Robots>(domain,
                   Robots("http://" + domain, "Qwantify", user_agent)));
  }
}


} // namespace urlserver
} // namespace mermoz

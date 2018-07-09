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

namespace mc = mermoz::common;

namespace mermoz
{
namespace urlserver
{

void urlserver(mermoz::common::AsyncQueue<std::string>* content_queue,
               mermoz::common::AsyncQueue<std::string>* url_queue,
               bool* status)
{
  std::set<std::string> visited;
  std::map<std::string, Robots> robots;
  size_t counter {0};

  while (*status)
  {
    std::string content;
    std::cout << "wait..." << std::endl;
    content_queue->pop(content);

    std::string url;
    std::string text;
    std::string links;
    std::string http_status;
    mc::unpack(content, {&url, &text, &links, &http_status});
    std::cout << "counter " << counter++ << std::endl;
    std::cout << "treating " << url << std::endl;

    mc::UrlParser root(url);

    if (http_status.compare("200") != 0)
    {
      continue;
    }
    else
    {
      visited.insert(url);
    }

    std::vector<mc::UrlParser> outlinks;

    std::string link;
    for (auto c : links)
    {
      if ((c == ' ' || c == ',') && !link.empty())
      {
        mc::UrlParser up(link);
        if (up.scheme.empty() || up.authority.empty())
        {
          try
          {
            outlinks.push_back(up + root);
          }
          catch(...)
          {
            std::cout << "cannot add " << link << std::endl;
          }
        }
        else
          outlinks.push_back(up);

        link = {""};
        continue;
      }

      link.push_back(c);
    }

    for (auto& outlink : outlinks)
    {
      std::string clean_url = outlink.get_url(false, false);
      if (visited.find(clean_url) == visited.end())
      {
        if (robots.find(outlink.domain) == robots.end())
          robots.insert(std::pair<std::string, Robots>(outlink.scheme + "://" + outlink.domain, Robots(outlink.domain, "Qwantify")));

        bool is_allowed;

        try
        {
          is_allowed = robots[outlink.domain].is_allowed(clean_url);
        }
        catch (...)
        {
          std::cout << "err " << clean_url << std::endl;
        }

        if (is_allowed)
          url_queue->push(clean_url);
      }
    }
    std::cout << "new loop..." << std::endl;
  }
}

} // namespace urlserver
} // namespace mermoz

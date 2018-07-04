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

#include "common/robots.hpp"

namespace mermoz
{
namespace common
{

long Robots::fetch_robots()
{
  if (host.empty())
  {
    print_error("No host provided");
    return -1;
  }

  CURL* curl;
  CURLcode res;

  curl = curl_easy_init();
  long http_code = 0;

  if (curl)
  {
    std::string robots_url = host;
    robots_url.append("/robots.txt");

    curl_easy_setopt(curl, CURLOPT_URL, robots_url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);

    robots_file = "";
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &robots_file);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }

  return http_code;
}

size_t Robots::write_function (char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string* content =
    reinterpret_cast<std::string*>(userdata);

  size_t relsize = size*nmemb;

  content->append(ptr, relsize);

  return relsize;
}

bool Robots::parse_file()
{
  if (robots_file.empty())
  {
    print_error("Nothing to parse.");
    return false;
  }

  std::istringstream iss(robots_file);

  std::string key;

  bool read_settings {false};

  while (!iss.eof())
  {
    iss >> key;

    if (key[0] == '#')
    {
      iss.ignore(1024, '\n');
      continue;
    }

    if (key.compare("User-agent:") == 0 && !read_settings)
    {
      iss >> key;
      read_settings = key.compare("*") == 0 || key.compare(user_agent) == 0;
    }

    if (read_settings)
    {
      iss >> key;

      if (key.compare("Disallow:") == 0)
      {
        iss >> key;
        std::cout << "disallow: " << key << std::endl;
        walls.push_back(UrlParser(key));
        std::cout << "push" << std::endl;
        *walls.end() += up_host;
        std::cout << "add" << std::endl;
        std::cout << *walls.end() << std::endl;
        std::cout << "next..." << std::endl;
      }
      else if (key.compare("Allow:") == 0)
      {
        iss >> key;
        std::cout << "allow: " << key << std::endl;
        doors.push_back(UrlParser(key));
        std::cout << "push" << std::endl;
        *doors.end() += up_host;
        std::cout << "next..." << std::endl;
      }
      else if (key.compare("Crawl-delay:") == 0)
      {
        iss >> key;
        std::cout << "delay: " << key << std::endl;
        crawl_delay = std::max(crawl_delay, std::atoi(key.c_str()));
        std::cout << "next..." << std::endl;
      }
    }
  }
}

} // namespace common
} // namespace mermoz

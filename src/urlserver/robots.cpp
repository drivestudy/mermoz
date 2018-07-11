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

#include "urlserver/robots.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace urlserver
{

bool Robots::is_allowed(mc::UrlParser& up)
{
  /*
   * First we check Allow rules
   */
  for (auto& door : doors)
  {
    if (up >= door)
      return true;
  }

  /*
   * Then we check Disallow rules
   */
  for (auto& wall : walls)
  {
    if (up >= wall)
      return false;
  }

  return true;
}

bool Robots::is_allowed(std::string url)
{
  mc::UrlParser up(url);
  return is_allowed(up);
}

long Robots::fetch_robots()
{
  if (host.empty())
  {
    mc::print_error("No host provided");
    return -1;
  }

  std::string robots_url = host;
  robots_url.append("/robots.txt");

  long http_code = mc::http_fetch(robots_url, robots_file, 5L, user_agent_full);
  mc::print_log(robots_url);

  return http_code;
}

bool Robots::parse_file()
{
  if (robots_file.empty())
  {
    mc::print_error("Nothing to parse.");
    return false;
  }

  std::istringstream iss(robots_file);

  bool read_settings {false};
  bool has_generic {false};

  std::string line;

  while (!iss.eof())
  {
    std::getline(iss, line);

    if (line[0] == '#')
    {
      continue;
    }

    if (line.find("User-agent:") != std::string::npos ||
        line.find("User-Agent:") != std::string::npos)
    {
      if (line.find("*") != std::string::npos)
      {
        read_settings = !has_generic &&
          (walls.empty() || doors.empty());
      }
      else if (line.find(user_agent) != std::string::npos)
      {
        if (has_generic)
        {
          walls.empty();
          doors.empty();
          has_generic = false;
        }
        read_settings = true;
      }
      else
      {
        read_settings = false;
      }
    }

    if (read_settings)
    {
      size_t pos;

      if ((pos = line.find("Disallow:")) != std::string::npos)
      {
        if (line.size() > pos+9)
        {
          try
          {
            walls.push_back(mc::UrlParser(line.substr(pos+9)) + up_host);
          }
          catch(...)
          {
            std::cerr << "cannot add urls" << std::endl;
          }
        }
      }
      else if ((pos = line.find("Allow:")) != std::string::npos)
      {
        if (line.size() > pos+6)
        {
          try
          {
            doors.push_back(mc::UrlParser(line.substr(pos+6)) + up_host);
          }
          catch(...)
          {
            std::cerr << "cannot add urls" << std::endl;
          }
        }
      }
      else if ((pos = line.find("Crawl-delay:")) != std::string::npos)
      {
        if (line.size() > pos+12)
          crawl_delay = std::max(crawl_delay, std::atoi(line.substr(pos+12).c_str()));
      }
    }
  }

  return true;
}

} // namespace urlserver
} // namespace mermoz

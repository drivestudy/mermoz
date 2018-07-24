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

#include "robots.hpp"
#include "network.hpp"
#include "logs.hpp"

namespace urlfactory
{

bool Robots::is_allowed(UrlParser& up)
{
  if (!is_good)
    return false;
  else if (is_empty)
    return true;

  /*
   * First we check Allow rules
   */
  for (auto& door : doors)
    if (up >= door)
      return true;

  /*
   * Then we check Disallow rules
   */
  std::cout << up;
  for (auto& wall : walls)
  {
    std::cout << wall;
    if (up >= wall)
      return false;
  }

  return true;
}

bool Robots::is_allowed(std::string url)
{
  UrlParser up(url);
  return is_allowed(up);
}

void Robots::initialize(Robots* rbt)
{
  bool private_is_good {false};

  if (!rbt->host.empty())
  {
    std::string robotstxt;
    long http_code;

    rbt->fetch_robots(rbt, robotstxt, http_code);

    if (http_code >= 200 && http_code < 300)
    {
      private_is_good = true;

      if (!robotstxt.empty())
        rbt->parse_file(rbt, robotstxt);
    }
    else if (http_code >= 400 && http_code < 500)
    {
      // Why ? Because it means the 'robots.txt'
      // does not exists, so no rules are provided
      // and it is accepted case.
      private_is_good = true;
    }
    else
    {
      private_is_good = false;
    }

#   ifdef MMZ_VERBOZE
    std::ostringstream oss;
    if (private_is_good)
    {
      oss << "Valid \'robots\' rules: " << rbt->host;
      print_log(oss.str());
    }
    else
    {
      oss << "Invalid \'robots\' rules: " << rbt->host;
      print_warning(oss.str());
    }
#   endif

    rbt->is_good = private_is_good;
    rbt->is_empty = robotstxt.empty();
  }
}

void Robots::fetch_robots(Robots* rbt, std::string& robotstxt, long& http_code)
{
  std::string robots_url = rbt->host;
  robots_url.append("/robots.txt");

# ifdef MMZ_PROFILE
  http_code = http_fetch(robots_url, robotstxt, 60L, rbt->user_agent_full);
# else
  http_code = http_fetch(robots_url, robotstxt, 10L, rbt->user_agent_full);
# endif
}

void Robots::parse_file(Robots* rbt, std::string& robotstxt)
{
  std::istringstream iss(robotstxt);

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
          (rbt->walls.empty() || rbt->doors.empty());
      }
      else if (line.find(rbt->user_agent) != std::string::npos)
      {
        if (has_generic)
        {
          rbt->walls.empty();
          rbt->doors.empty();
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
          rbt->walls.push_back(UrlParser(line.substr(pos+9)) + rbt->up_host);
      }
      else if ((pos = line.find("Allow:")) != std::string::npos)
      {
        if (line.size() > pos+6)
          rbt->doors.push_back(UrlParser(line.substr(pos+6)) + rbt->up_host);
      }
      else if ((pos = line.find("Crawl-delay:")) != std::string::npos)
      {
        if (line.size() > pos+12)
          rbt->crawl_delay = std::max(rbt->crawl_delay, std::atoi(line.substr(pos+12).c_str()));
      }
    }
  }
}

} // namespace urlfactory

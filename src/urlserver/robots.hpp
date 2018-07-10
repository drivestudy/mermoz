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

#ifndef MERMOZ_ROBOTS_H__
#define MERMOZ_ROBOTS_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "common/common.hpp"

namespace mermoz
{
namespace urlserver
{

class Robots
{
public:
  Robots() : Robots("","","") {}

  Robots(std::string host, std::string user_agent, std::string user_agent_full) :
    host(host), user_agent(user_agent), user_agent_full(user_agent_full),
    up_host(mermoz::common::UrlParser(host)), crawl_delay(4)
  {
    if (host.empty())
      return;

    std::ostringstream oss;

    long err;
    if ((err = fetch_robots()) != CURLE_OK)
    {
      oss << "Could not fetch robots.txt for: " << host;
      oss << " HTTP_ERROR(" << err << ")";
      mermoz::common::print_error(oss.str());
      return;
    }

    if (!parse_file())
    {
      oss << "Could not parse robots.txt for: " << host;
      mermoz::common::print_error(oss.str());
      return;
    }
  }

  bool is_allowed(mermoz::common::UrlParser& up);
  bool is_allowed(std::string url);

private:
  const std::string host;
  const std::string user_agent;
  const std::string user_agent_full;
  int crawl_delay; // milliseconds

  std::string robots_file;

  mermoz::common::UrlParser up_host;
  std::vector<mermoz::common::UrlParser> walls;
  std::vector<mermoz::common::UrlParser> doors;

  long fetch_robots();

  bool parse_file();
}; // class Robots

} // namespace urlserver
} // namespace mermoz

#endif // MERMOZ_ROBOTS_H__

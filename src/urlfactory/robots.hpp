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

#ifndef URLFACTORY_ROBOTS_H__
#define URLFACTORY_ROBOTS_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include "urlparser.hpp"

namespace urlfactory
{

class Robots
{
public:
  Robots() : Robots("","","") {}

  Robots(std::string host,
         std::string user_agent,
         std::string user_agent_full) :
    is_tried(false),
    is_good(false),
    is_empty(false),
    host(host),
    user_agent(user_agent),
    user_agent_full(user_agent_full),
    crawl_delay(4),
    up_host(UrlParser(host)) {}

  bool good()
  {
    return is_good;
  }

  bool tried()
  {
    return is_tried;
  }

  bool empty()
  {
    return is_empty;
  }

  bool is_allowed(UrlParser& up);
  bool is_allowed(std::string url);

  void async_init()
  {
    std::thread t(initialize, this);
    t.detach();
  }

  void init()
  {
    initialize(this);
  }

private:
  bool is_tried;
  bool is_good;
  bool is_empty;

  const std::string host;
  const std::string user_agent;
  const std::string user_agent_full;
  int crawl_delay; // milliseconds

  UrlParser up_host;
  std::vector<UrlParser> walls;
  std::vector<UrlParser> doors;

  static void initialize(Robots* rbt);
  static void fetch_robots(Robots* rbt, std::string& robotstxt, long& http_code);
  static void parse_file(Robots* rbt, std::string& robotstxt);
}; // class Robots

} // namespace urlparser

#endif // URLPARSER_ROBOTS_H__

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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <initializer_list>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "common/common.hpp"
namespace mc = mermoz::common;

#include "spider/spider.hpp"
namespace ms = mermoz::spider;

int main (int argc, char** argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help", "displays this message")
  ("fetchers", po::value<int>(), "number of fetchers")
  ("parsers", po::value<int>(), "number of parsers")
  ("kafka-file", po::value<std::string>(), "config file for the Kafka consumer & producer")
  ;

  po::variables_map vmap;
  po::store(po::parse_command_line(argc, argv, desc), vmap);
  po::notify(vmap);

  if (vmap.count("help"))
  {
    std::cout << desc << std::endl;
    return 1;
  }

  bool status = true;

  mc::AsyncQueue<std::string> url_queue;
  mc::AsyncQueue<std::string> content_queue;
  mc::AsyncQueue<std::string> parsed_queue;

  std::vector<std::thread> fetchers;

  if (vmap.count("fetchers"))
  {
    for (int i = 0; i < vmap["fetchers"].as<int>(); i++)
    {
      fetchers.push_back(std::thread(ms::fetcher, &url_queue, &content_queue, &status));
    }
  }
  else
  {
    std::cout << "The number of fetchers must be declared" << std::endl;
    return 1;
  }

  std::vector<std::thread> parsers;

  if (vmap.count("parsers"))
  {
    for (int i = 0; i < vmap["parsers"].as<int>(); i++)
    {
      parsers.push_back(std::thread(ms::parser, &content_queue, &parsed_queue, &status));
    }
  }
  else
  {
    std::cout << "The number of parsers must be declared" << std::endl;
    return 1;
  }

  std::ifstream urlfile("urls.txt");
  while (urlfile.good())
  {
    std::string url;
    urlfile >> url;
    url_queue.push(url);
  }

  std::ofstream ofp ("parsed.txt");
  while (status)
  {
    std::string message;
    parsed_queue.pop(message);

    std::string url;
    std::string text;
    std::string links;

    mc::unpack(message, {&url, &text, &links});

    ofp << "URL : " << url << std::endl;
    ofp << "TEXT : " << text << std::endl;
    ofp << "LINKS : " << links << std::endl;
  }

  return 0;
}


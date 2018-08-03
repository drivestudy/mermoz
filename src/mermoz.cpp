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
#include <atomic>

#include <unistd.h>
#include <curl/curl.h>

#ifdef MMZ_PROFILE
#include <gperftools/heap-profiler.h>
#endif

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "tsafe/thread_safe_queue.h"

#include "common/common.hpp"
#include "spider/spider.hpp"
#include "urlserver/urlserver.hpp"

using namespace mermoz;

int main (int argc, char** argv)
{
# ifdef MMZ_PROFILE
  HeapProfilerStart("mmz");
# endif

  /*
   * Defining options for code
   */
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help", "displays this message")
  ("settings", po::value<std::string>(), "setting file for the whole Mermoz run")
  ("seeds", po::value<std::string>(), "list of seeds for the current run")
  ;

  po::variables_map vmap;
  po::store(po::parse_command_line(argc, argv, desc), vmap);
  po::notify(vmap);

  /*
   * If one asks '--help'
   */
  if (vmap.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  /*
   * Parsing the 'settings' file
   */
  size_t pos;
  std::string line;
  std::ifstream settingsfile(vmap["settings"].as<std::string>());

  std::string user_agent;
  unsigned int nfetchers {0};
  unsigned int nparsers {0};
  int max_ram {0};

  while(!settingsfile.eof()) {
    line.clear();
    std::getline(settingsfile, line);

    unsigned char c;
    while ((c = *(line.end()-1)) < 0x20 && !line.empty())
      line.pop_back();

    if ((pos = line.find("fetchers")) != std::string::npos)
      nfetchers = static_cast<unsigned int>(std::atoi(line.substr(pos + 9).c_str()));
    else if ((pos = line.find("parsers")) != std::string::npos)
      nparsers = static_cast<unsigned int>(std::atoi(line.substr(pos + 8).c_str()));
    else if ((pos = line.find("user-agent")) != std::string::npos)
      user_agent = line.substr(pos + 11);
    else if ((pos = line.find("max-ram")) != std::string::npos)
      max_ram = std::atoi(line.substr(pos + 8).c_str());
  }
  settingsfile.close();

  /*
   * One verifies that all the mandatory settings
   * where included
   */
  if (user_agent.empty() ||
      nfetchers == 0 ||
      nparsers == 0 ||
      max_ram == 0) {
    print_error("Wrong settings Mermoz cannot start");
  } else {
    print_strong_log("Staring of Mermoz with following settings:");

    std::ostringstream oss;

    oss << "Fetchers: " << nfetchers;
    print_strong_log(oss.str());

    oss.str("");
    oss << "Parsers: " << nparsers;
    print_strong_log(oss.str());

    oss.str("");
    oss << "User-agent: " << user_agent;
    print_strong_log(oss.str());

    oss.str("");
    oss << "Max-ram (GB): " << max_ram;
    print_strong_log(oss.str());
  }

  bool status = true;

  TSQueueVector url_queues(nfetchers);
  TSQueueVector content_queues(nparsers);
  MemSec mem_sec(max_ram * MemSec::GB);

  unsigned int queue_id {0};
  std::string link;
  std::ifstream seedfile(vmap["seeds"].as<std::string>());
  while (!seedfile.eof()) {
    link = {""};
    seedfile >> link;
    if (!link.empty()) {
      urlfactory::UrlParser up(link);
      std::string message, host;
      pack(message, {&host, &message});
      url_queues[queue_id].push(message);
      mem_sec += message.size();
      queue_id++;

      if (queue_id >= url_queues.size()) {
        queue_id = 0;
      }
    }
  }
  seedfile.close();

  /* Must initialize libcurl before any threads are started */
  curl_global_init(CURL_GLOBAL_ALL);

  /*
   * Settings for the UrlServer
   */
  UrlServerSettings uset = {
    user_agent,
    &mem_sec
  };

  std::thread userv(urlserver,
                    &status,
                    &uset,
                    &content_queues,
                    &url_queues);

  std::atomic<uint64_t> nfetched;
  nfetched = 0;

  std::atomic<uint64_t> nparsed;
  nparsed = 0;

  /*
   * Settings for the Spider
   */
  SpiderSettings sset = {
    nfetchers,
    nparsers,
    user_agent,
    &nfetched,
    &nparsed,
    &mem_sec,
  };

  std::thread spdr(spider,
                   &status,
                   &sset,
                   &url_queues,
                   &content_queues);

  std::ofstream ofp("log.out");

  ofp << "# time urls contents fetched parsed mem(MB)" << std::endl;

  while (status) {
    sleep(10);
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    unsigned long wait_url {0};
    for (auto& queue : url_queues) {
      wait_url += queue.size();
    }

    unsigned long wait_content {0};
    for (auto& queue : content_queues) {
      wait_content += queue.size();
    }

    ofp << tm.tm_hour*3600 + tm.tm_min*60 + tm.tm_sec << " ";
    ofp << wait_url << " ";
    ofp << wait_content << " ";
    ofp << nfetched << " ";
    ofp << nparsed << " ";
    ofp << mem_sec.get_mem()/(1UL << 20) << std::endl;
  }

# ifdef MMZ_PROFILE
  HeapProfilerStop();
# endif

  userv.join();
  spdr.join();

  return 0;
}


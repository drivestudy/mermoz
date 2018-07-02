#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <initializer_list>

#include <librdkafka/rdkafkacpp.h>

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
  mc::async_queue<std::string> url_queue;
  mc::async_queue<std::string> content_queue;
  mc::async_queue<std::string> parsed_queue;

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
    std::string message = parsed_queue.pop_out();

    std::string url;
    std::string text;
    std::string links;

    ms::unpack(message, {&url, &text, &links});

    ofp << "URL : " << url << std::endl;
    ofp << "TEXT : " << text << std::endl;
    ofp << "LINKS : " << links << std::endl;
  }

  return 0;
}

namespace mermoz
{
namespace spider
{

void pack(std::string& pack, std::initializer_list<std::string*> args)
{
  if (args.size() == 0)
    return;

  size_t pos {0};
  for (auto& arg : args)
  {
    pos += arg->size() + sizeof(size_t);
  }

  pack.clear();
  pack.resize(pos);

  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size = arg->size();

    std::memcpy((void*)(&itpack),
                (void*)(&arg_size),
                sizeof(size_t));

    itpack += sizeof(size_t);

    std::memcpy((void*)(&itpack),
                (void*)(arg->data()),
                arg_size);

    itpack += arg_size;
  }
}

void unpack(std::string& pack, std::initializer_list<std::string*> args)
{
  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size;

    std::memcpy((void*)(&arg_size),
                (void*)(&itpack),
                sizeof(size_t));

    itpack += sizeof(size_t);

    arg->clear();
    arg->resize(arg_size);

    std::memcpy((void*)(&*arg->begin()),
                (void*)(&itpack),
                arg_size);

    itpack += arg_size;
  }
}

} // namespace spider
} // namespace mermoz

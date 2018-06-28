#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <librdkafka/rdkafkacpp.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "common/common.hpp"
namespace mc = mermoz::common;

//#include "spider/spider.hpp"
//namespace ms = mermoz::spider;

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

  return 0;
}

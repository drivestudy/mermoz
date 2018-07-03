#include <iostream>
#include <string>
#include <cstring>

#include "common/urlparser.cpp"

namespace mc = mermoz::common;

int main (int argc, char** argv)
{
  std::string url_left;
  std::string url_right;

  mc::UrlParser up_left;
  mc::UrlParser up_right;

  if (argc == 2)
  {
    url_left = std::string(argv[1]);
    up_left.set_url(url_left);

    up_left.parse();
  }
  else if (argc == 4 && std::strcmp(argv[2], "+") == 0)
  {
    url_left = std::string(argv[1]);
    up_left.set_url(url_left);
    up_left.parse();

    url_right = std::string(argv[3]);
    up_right.set_url(url_right);
    up_right.parse();

    up_left += up_right;
  }
  else
  {
    std::cerr << "Unknown arguments. Exiting..." << std::endl;
    exit(-1);
  }

  std::cout << "scheme    " << up_left.scheme << std::endl;
  std::cout << "authority " << up_left.authority << std::endl;
  std::cout << "  user    " << up_left.user << std::endl;
  std::cout << "  pass    " << up_left.pass << std::endl;
  std::cout << "  port    " << up_left.port << std::endl;
  std::cout << "  domain  " << up_left.domain << std::endl;
  std::cout << "path      " << up_left.path << std::endl;
  for (auto& elem : up_left.path_tree)
  {
    std::cout << "   /      " << elem << std::endl;
  }
  std::cout << "query     " << up_left.query << std::endl;
  for (auto& elem : up_left.query_args)
  {
    std::cout << "    &     " << elem << std::endl;
  }
  std::cout << "fragment  " << up_left.fragment << std::endl;

  return 0;
}

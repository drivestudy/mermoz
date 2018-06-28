#include <iostream>
#include <string>

#include "common/urlparser.cpp"

namespace mc = mermoz::common;

int main (int argc, char** argv)
{
  std::string url(argv[1]);
  mc::UrlParser up(url);

  up.parse();

  std::cout << "scheme    " << up.scheme << std::endl;
  std::cout << "authority " << up.authority << std::endl;
  std::cout << "  user    " << up.user << std::endl;
  std::cout << "  pass    " << up.pass << std::endl;
  std::cout << "  port    " << up.port << std::endl;
  std::cout << "  domain  " << up.domain << std::endl;
  std::cout << "path      " << up.path << std::endl;
  for (int i = 0; i < up.path_tree.size(); i++)
  {
    std::cout << "  /       " << up.path_tree[i] << std::endl;
  }
  std::cout << "query     " << up.query << std::endl;
  for (int i = 0; i < up.query_args.size(); i++)
  {
    std::cout << "  &       " << up.query_args[i] << std::endl;
  }
  std::cout << "fragment  " << up.fragment << std::endl;

  return 0;
}

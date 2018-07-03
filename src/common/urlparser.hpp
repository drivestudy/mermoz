#ifndef MERMOZ_URLPARSER_H__
#define MERMOZ_URLPARSER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace mermoz
{
namespace common
{

class UrlParser
{
public:
  UrlParser () :
    url(""), do_parse(true), do_scheme(true), do_authority(false),
    do_path(false), do_query(false), do_fragment(false) {}

  UrlParser (std::string& url) :
    url(url), do_parse(true), do_scheme(true), do_authority(false),
    do_path(false), do_query(false), do_fragment(false) {}

  UrlParser& operator+=(UrlParser& rhs);

  void set_url(std::string& url) {this->url = url;}

  bool parse();

  std::string url;

  std::string scheme;

  std::string authority;
  std::string user;
  std::string pass;
  std::string domain;
  std::string port;

  std::string path;
  std::vector<std::string> path_tree;
  int depth;

  std::string query;
  std::vector<std::string> query_args;

  std::string fragment;

private:
  bool do_parse;

  bool do_scheme;
  bool do_authority;
  bool do_path;
  bool do_query;
  bool do_fragment;

  void parse_scheme(std::streambuf* sb);
  void parse_authority(std::streambuf* sb);
  void parse_path(std::streambuf* sb);
  void parse_query(std::streambuf* sb);
  void parse_fragment(std::streambuf* sb);
}; // class UrlParser

} // namespace common
} // namespace mermoz

#endif // MERMOZ_URLPARSER_H__

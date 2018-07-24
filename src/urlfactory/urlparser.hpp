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

#ifndef URLFACTORY_URLPARSER_H__
#define URLFACTORY_URLPARSER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <initializer_list>

namespace urlfactory
{

class UrlParser
{
public:
  UrlParser () : UrlParser("") {}

  UrlParser (std::string url) :
    url(url),
    inherit_scheme(false),
    inherit_auth(false),
    inherit_path(false),
    complete_url(false),
    is_file(false),
    has_final_slash(false),
    has_pattern(false),
    do_parse(true),
    do_scheme(false),
    do_authority(false),
    do_path(false), 
    do_query(false),
    do_fragment(false)
  {
    if (url.empty()) return;
    parse();
  }

  ~UrlParser() {}

  UrlParser& operator+=(UrlParser& rhs);
  UrlParser operator+(UrlParser& rhs);

  friend std::ostream& operator<<(std::ostream& os, const UrlParser& rhs);

  bool operator>(const UrlParser& rhs);
  bool operator>=(const UrlParser& rhs);

  bool operator<(const UrlParser& rhs);
  bool operator<=(const UrlParser& rhs);

  void set_url(std::string url)
  {
    this->url = url;
  }

  std::string get_url(bool get_query = true, bool get_fragment = true);

  bool parse();

  void exchange(std::string scheme1, std::string scheme2);

  bool valid_scheme(std::initializer_list<std::string> schemes);

  std::string url;

  bool inherit_scheme;
  bool inherit_auth;
  bool inherit_path;
  bool complete_url;

  bool is_file;
  bool has_final_slash;
  bool has_pattern;

  std::string scheme;

  std::string authority;
  std::string user;
  std::string pass;
  std::string domain;
  std::string port;

  std::string path;
  std::vector<std::string> path_tree;

  std::string query;
  std::vector<std::string> query_args;

  std::vector<std::string> patterns;

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

  bool match_pattern(const UrlParser& rhs);
}; // class UrlParser

} // namespace urlfactory

#endif // URLFACTORY_URLPARSER_H__

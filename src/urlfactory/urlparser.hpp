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

#include <string>
#include <vector>
#include <initializer_list>

namespace urlfactory
{

class UrlParser
{
public:
  UrlParser () : UrlParser ("") {}

  UrlParser (std::string url) :
    url(url), is_good(false)
  {
    parse();
  }

  void set_url (std::string url)
  {
    this->url = url;
  }

  void set_scheme (std::string scheme)
  {
    this->scheme = scheme;
  }

  void parse();

  std::string get_url(bool get_scheme = true,
                      bool get_auth = true,
                      bool get_path = true,
                      bool get_query = true,
                      bool get_frag = true);

  std::string get_host()
  {
    return host;
  }

  bool empty()
  {
    return url.empty();
  }

  bool good()
  {
    return is_good;
  }

  bool complete()
  {
    return is_complete;
  }

  bool has_scheme()
  {
    return !scheme.empty();
  }

  bool has_authority()
  {
    return !auth.empty();
  }

  bool valid_scheme(std::initializer_list<std::string> schemes);

  UrlParser& operator+=(UrlParser& rhs);
  UrlParser operator+(UrlParser& rhs);

  friend std::ostream& operator<<(std::ostream& os, const UrlParser& rhs);

  bool operator>=(UrlParser& rhs);
  bool operator>(UrlParser& rhs);

  bool operator<=(UrlParser& rhs)
  {
    return !(*this > rhs);
  }

  bool operator<(UrlParser& rhs)
  {
    return !(*this >= rhs);
  }

private:
  /*
   * Describes the state of parsing
   */
  enum {
    NO_PARSE,
    ERR_PARSE,
    PARSE_SCHEME,
    PARSE_AUTH,
    PARSE_PATH,
    PARSE_QUERY,
    PARSE_FRAG
  };

  std::string url; // The URL itself

  std::string scheme; // The extracted scheme

  /*
   * All the component located within AUTH
   */
  std::string auth; // Full authority
  std::string user; // Username
  std::string pass; // Password
  std::string host; // Hostname
  std::string port; // Port

  /*
   * Saves the PATH structure
   */
  std::string path; // The full path
  std::vector<std::string> path_tree; // Each component of the path
  std::string file_fomat; // If PATH refers to a file

  std::string query; // The query of URL
  std::vector<std::string> query_args; // Each args of the query

  std::string frag; // The fragment of URL

  bool is_good;

  bool is_complete;
  bool is_pattern;
  bool is_file;

  bool auth_less;

  bool rel_scheme;
  bool rel_auth;
  bool rel_path;

  int parse_scheme(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_auth(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_path(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_query(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_frag(const char* cstr, size_t& pos, const size_t pos_max);
}; // class UrlParser

} // urlfactory

#endif // URLFACTORY_URLPARSER_H__

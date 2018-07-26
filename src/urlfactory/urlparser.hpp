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

/*! \brief The URL parsing object
 *
 * By giving a string to the object, the class is able
 * to parse all the components of an URL
 * SCHEME://AUTHORITY/PATH/?QUERY#FRAGMENT
 */

class UrlParser
{
public:
  /*! Empty contructor */
  UrlParser () : UrlParser ("") {}

  /*! General constructor
   * \param url The URL you want to parse
   */
  UrlParser (std::string url) :
    url(url), is_good(false)
  {
    parse();
  }

  /*! Set the URL value
   *
   * This could be useful if you used an
   * empty constructor and you did not set this value
   *
   * \param url The URL you want to parse
   */
  void set_url (std::string url)
  {
    this->url = url;
  }

  /*! Set the SCHEME
   * \param scheme The SCHEME you want to add
   */
  bool set_scheme (std::string scheme)
  {
    if (rel_auth || rel_path) {
      /* 
       * If the URL is relative deeper than SCHEME,
       * adding it has not sense.
       */
      return false;
    } else {
      /* 
       * Great, you can give a SCHEME and maybe
       * delete its relative scheme properties
       */
      this->scheme = scheme;
      rel_scheme = false;
      return true;
    }
  }

  /*! Function for parsing URL */
  void parse();

  /*! Return the full or parts of the cleaned URL
   *
   * \param get_scheme Returns SCHEME
   * \param get_auth Returns AUTHORITY
   * \param get_path Returns PATH
   * \param get_query Returns QUERY
   * \param get_frag Return FRAG
   */
  std::string get_url(bool get_scheme = true,
                      bool get_auth = true,
                      bool get_path = true,
                      bool get_query = true,
                      bool get_frag = true);

  /*! Returns only the HOST value */
  std::string get_host()
  {
    return host;
  }

  /*! Returns true if a URL was defined */
  bool empty()
  {
    return url.empty();
  }

  /*! Returns true if the URL was parsed
   * and is not malformed
   */
  bool good()
  {
    return is_good;
  }

  /*! Returns true if the URL has 
   * no relative compontent (SCHEME, AUTH, or PATH)
   */
  bool complete()
  {
    return is_complete;
  }

  /*! Returns true if the URL has SCHEME */
  bool has_scheme()
  {
    return !scheme.empty();
  }

  /*! Returns true if the URL has AUTH */
  bool has_authority()
  {
    return !auth.empty();
  }

  /*! Verify that URL scheme matches with allowed schemes
   * \param schemes Allowed schemes to check
   */
  bool valid_scheme(std::initializer_list<std::string> schemes);

  /*! Verify that URL file format matches with allowed file formats
   *  Note: if 'is_file = false' the function returns 'true'
   * \param schemes Allowed file formats to check
   */
  bool valid_file(std::initializer_list<std::string> file_formats);

  /*! Add to the current UrlParser an RHS
   * \param rhs UrlParser Right hand side
   */
  UrlParser& operator+=(UrlParser& rhs);

  /*! Add two UrlParsers LHS + RHS
   * \param rhs UrlParser Right hand side
   */
  UrlParser operator+(UrlParser& rhs);

  /*! Comfy function to check how the parser analyzed the URL */
  friend std::ostream& operator<<(std::ostream& os, const UrlParser& rhs);

  /*! Checks if two URLs are equal, or match pattern
   * \param rhs UrlParser Right hand side
   */
  bool operator==(UrlParser& rhs);

  /*! Checks if LHS >= RHS, or match pattern
   * \param rhs UrlParser Right hand side
   */
  bool operator>=(UrlParser& rhs);

  /*! Checks if LHS > RHS, or match pattern
   * \param rhs UrlParser Right hand side
   */
  bool operator>(UrlParser& rhs);

  /* Checks if LHS <= RHS and does not match pattern */
  bool operator<=(UrlParser& rhs)
  {
    return !(*this > rhs);
  }

  /* Checks if LHS < RHS and does not match pattern */
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

  bool is_good; // saves if URL was parsed and wellformed

  bool is_complete; // is not a relative URL
  bool is_pattern; // has a '*' somewhere
  bool is_file; // has 'file.format' at the end of PATH

  bool auth_less; // special kind like 'mailto:', 'tel:', etc...

  bool rel_scheme; // relative scheme
  bool rel_auth; // relative authority
  bool rel_path; // relative path

  int parse_scheme(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_auth(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_path(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_query(const char* cstr, size_t& pos, const size_t pos_max);
  int parse_frag(const char* cstr, size_t& pos, const size_t pos_max);

  /*! Verify if URLs are matching
   *
   * \param lhs Left Hand Side URL
   * \param lhs_pos Left Hand Side URL iterator
   * \param rhs Right Hand Side URL
   * \param lhs_pos Right Hand Side URL iterator
   *
   * \return 1 Ending pattern match
   * \return 0 String and patterns are matching, and at least one of iterators reached limit
   * \return -1 Not equal at all
   */
  int url_matching(std::string& lhs, size_t& lhs_pos,
                   std::string& rhs, size_t& rhs_pos);
}; // class UrlParser

} // urlfactory

#endif // URLFACTORY_URLPARSER_H__

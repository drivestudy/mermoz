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

#include "urlparser.hpp"
#include "hexencode.hpp"
#include "ssanitize.hpp"
#include "logs.hpp"

namespace urlfactory
{

void UrlParser::parse()
{
  /*
   * Cleaning the incomming URL
   */
  trim_beg_end (url);

  /*
   * Initializing properties of a
   * possible relative URL
   */
  rel_scheme = {false};
  rel_auth = {false};
  rel_path = {false};

  /*
   * Special URL case, such as 'mailto:', 'tel:', etc...
   */
  auth_less = {false};

  /*
   * Initialize properities of the URL
   */
  is_complete = {false};
  is_pattern = {false};
  is_dir = {false};

  if (!url.empty()) {
    /*
     * Here it is all the needed data
     * structures for the URL parsing
     */
    const char* url_cstr = url.c_str();
    size_t url_pos {0};
    size_t url_size = url.size();

    /*
     * Conatins all the directives for parsing
     * the URL
     */
    int parse_status;

    char c {url_cstr[0]};

    if (c == '/') {
      /*
       * All URLs staring with '/' are at least
       * relative to the SCHEME definition
       */
      rel_scheme = true;

      if (url.size() > 1) {
        if (url[1] == '/') {
          /*
           * Relative scheme URL
           * Because the URL starts with '//'
           * One can go directly to the AUTH parsing
           */
          url_pos += 2;
          parse_status = PARSE_AUTH;
        } else {
          /*
           * Relative auth URL
           * It starts with '/[a-Z]' thus if refers to the website's ROOT
           * Let's switch to the PATH parsing
           */
          rel_auth = true;
          parse_status = PARSE_PATH;
        }
      } else {
        /*
         * Relative auth URL of SIZE = 1
         * Special case of an URL refering
         * to the website's ROOT
         */
        path = url;
        rel_auth = true;
        parse_status = NO_PARSE;
      }
    } else if (c == '.' || c == '*') {
      /*
       * Relative path URL
       * It can starts with '.' or '..'
       */
      parse_status = PARSE_PATH;

      /*
       * It corresponds to a path relative
       * pattern, usually defined within Robots.txt
       */
      is_pattern = c == '*';

      /*
       * If it is a relative URL,
       * thus by definition SCHEME and AUTH
       * ARE NOT defined !
       */
      rel_scheme = true;
      rel_auth = true;
      rel_path = true;
    } else if (c == '?') {
      /*
       * Query URL and path relative
       */
      parse_status = PARSE_QUERY;

      /*
       * If it is a relative URL,
       * thus by definition SCHEME and AUTH
       * ARE NOT defined !
       */
      rel_scheme = true;
      rel_auth = true;
      rel_path = true;
    } else if (c == '#') {
      /*
       * Fragment URL and path relative
       */
      parse_status = PARSE_QUERY;

      /*
       * If it is a relative URL,
       * thus by definition SCHEME and AUTH
       * ARE NOT defined !
       */
      rel_scheme = true;
      rel_auth = true;
      rel_path = true;
    } else {
      /*
       * The URL does not starts with special character
       * Thus it could be a complete one or just a path.
       */
      parse_status = PARSE_SCHEME;
    }

    while (parse_status != NO_PARSE && parse_status != ERR_PARSE) {
      if (parse_status == PARSE_SCHEME) {
        parse_status = parse_scheme(url_cstr, url_pos, url_size);
      } else if (parse_status == PARSE_AUTH) {
        parse_status = parse_auth(url_cstr, url_pos, url_size);
      } else if (parse_status == PARSE_PATH) {
        parse_status = parse_path(url_cstr, url_pos, url_size);
      } else if (parse_status == PARSE_QUERY) {
        parse_status = parse_query(url_cstr, url_pos, url_size);
      } else if (parse_status == PARSE_FRAG) {
        parse_status = parse_frag(url_cstr, url_pos, url_size);
      }
    } // while

    /*
     * Last verification about the URL quality
     */
    is_good = parse_status != ERR_PARSE;
    if (is_good) {
      // We replace the parsed URL by the cleaned one
      url = get_url();
      path = get_url(false, false, true, false, false);
      query = get_url(false, false, false, true, false);

      // We verify the value of 'is_complete'
      is_complete = !rel_scheme && !rel_auth && !rel_path;
    }
  } else {
    is_good = false;
  }
}

int UrlParser::parse_scheme(const char* cstr, size_t& pos, const size_t pos_max)
{
  /*
   * Here, one can parse SCHEME
   * or PATH... So data are not storred
   * directly to SCHEME memeber
   */
  std::string tmp_scheme;

  while(pos < pos_max) {
    if (cstr[pos] == '.'
        || cstr[pos] == '/'
        || cstr[pos] == '?'
        || cstr[pos] == '#') {
      /*
       * We previously excluded relative AUTH case
       * thus we are reading a PATH
       */
      break;
    } else if (cstr[pos] == ':') {
      /*
       * One detects the separator ':'
       * so we stop parsing scheme and we save it
       */
      scheme = tmp_scheme; // save

      pos++; // jump over ':'
      break;
    } else {
      tmp_scheme.push_back(cstr[pos]);
      pos++;
    }
  }

  if (scheme.empty()) {
    /*
     * No colon was found, in this case the parsed URL
     * may look like to:
     * - 'john', 'john/doe', 'john?doe' and 'john#doe'
     */
    pos = 0; // reset the position of re-parsing

    rel_scheme = true;
    rel_auth = true;
    rel_path = true;

    return PARSE_PATH;
  } else {
    /*
     * A colon delimiter was found
     * Some rules to check...
     */
    if (pos + 1 >= pos_max) {
      /*
       * Found scheme and the end of the string, ok. Exits !
       */
      return NO_PARSE;
    } else if (cstr[pos] == '/' && cstr[pos+1] == '/') {
      /*
       * After a separator, the authority is separated
       * is separated by '//'
       */
      pos += 2;
      return PARSE_AUTH;
    } else if (cstr[pos] == '/' && cstr[pos+1] != '/') {
      /*
       * This is not allowed. Sorry.
       */
      return ERR_PARSE;
    } else {
      /*
       * If after a scheme one not finds '//',
       * this means that we are reading a PATH
       * Source: https://tools.ietf.org/html/rfc3986#section-3
       */
      auth_less = true;
      return PARSE_PATH;
    }
  }
}

int UrlParser::parse_auth(const char* cstr, size_t& pos, const size_t pos_max)
{
  /*
   * Saving interal AUTH separator positions
   */
  long loc_pos {0}; // Position within AUTH substring
  long host_pos {0}; // Mandatory property of AUTH
  long colon_pos{-1}; // Optional separator, so -1 is a magic number

  /*
   * We arrived within AUTH,
   * thus options available are only:
   * - PATH or,
   * - QUERY or,
   * - FRAG.
   */
  int next_step {NO_PARSE};

  while (pos < pos_max) {
    if (cstr[pos] == '/') {
      /*
       * The '/' separator after a AUTH
       * is only reserved for PATH
       */
      next_step = PARSE_PATH;
      break;
    } else if (cstr[pos] == '?') {
      /*
       * The '?' separator after a AUTH
       * is only reserved for QUERY
       */
      next_step = PARSE_QUERY;
      break;
    } else if (cstr[pos] == '#') {
      /*
       * The '#' separator after a AUTH
       * is only reserved for FRAG
       */
      next_step = PARSE_FRAG;
      break;
    } else {
      auth.push_back(cstr[pos]);

      if (cstr[pos] == ':') {
        /*
         * We memorize the position of a colon separator
         * Whitin AUTH is could separate USER:PASS or
         * HOST:PORT
         */
        colon_pos = loc_pos;
      } else if (cstr[pos] == '@') {
        /*
         * We detect '@' which is the separator of a:
         * [USER[:PASS]@]HOST
         */
        if (colon_pos > 0) {
          /*
           * We previously memorized a colon
           * so a USER:PASS@HOST is defined
           */
          user = auth.substr(0, colon_pos);
          pass = auth.substr(colon_pos + 1, loc_pos - colon_pos - 1);
        } else {
          /*
           * No colon memorized, so only USER@HOST
           */
          user = auth.substr(0, loc_pos);
        }
        /*
         * Reset the saved colon position
         */
        colon_pos = -1;
        /*
         * If one finds '@' so the host position
         * is clearly defined
         */
        host_pos = loc_pos + 1;
      }

      loc_pos++;
      pos++;
    }
  }

  if (colon_pos > 0) {
    /*
     * A colon pos was memorized and not reseted by '@'
     * separator. So it is the definition of HOST:PORT
     */
    host = auth.substr(host_pos, colon_pos - host_pos);
    port = auth.substr(colon_pos + 1, loc_pos - colon_pos - 1);
  } else {
    /*
     * No colon, only [HOST]
     */
    host = auth.substr(host_pos, loc_pos - host_pos);
  }

  /*
   * Time to check the status what to do...
   */
  if (pos >= pos_max) {
    /*
     * The end of the string, ok. Exits !
     * We append a '/' to the path for possible upcomming '+'
     */
    return NO_PARSE;
  } else if (next_step != NO_PARSE) {
    /*
     * The story continues with PATH, QUERY or FRAG...
     */
    return next_step;
  } else {
    /*
     * Not the end of the URL and no separator ?!
     */
    return ERR_PARSE;
  }
}

int UrlParser::parse_path(const char* cstr, size_t& pos, const size_t pos_max)
{
  /*
   * We arrived within PATH,
   * thus options available are only:
   * - QUERY or,
   * - FRAG.
   */
  int next_step {NO_PARSE};

  /*
   * Saving interal PATH separator positions
   */
  long loc_pos {0}; // Position within PATH substring
  long slash_pos {-1}; // Optional separator, so -1 is a magic number

  /*
   * One can parse PATH for three reasons:
   * - after AUTH, thus URL is formed as AUTH/PATH,
   * - if URL starts with '/', URL looks like /PATH,
   * - and finaly all other relative pathes PATH = [a-Z]
   *
   * In order to add clean segments, we avoid the first slash
   */
  if (!rel_path && !auth_less) {
    if (cstr[pos] == '/') {
      // We increment only the reader
      pos++;

      if (pos >= pos_max) {
        /*
         * We verify that we did no reached
         * the end of the URL, if it's the
         * case that's ok !
         */
        is_dir = true;
        path.append("/");
        return NO_PARSE;
      }
    } else {
      return ERR_PARSE;
    }
  }

  while (pos < pos_max) {
    if (cstr[pos] == '?') {
      /*
       * The '?' separator after a PATH
       * is only reserved for QUERY
       */
      next_step = PARSE_QUERY;
      break;
    } else if (cstr[pos] == '#') {
      /*
       * The '#' separator after a PATH
       * is only reserved for FRAG
       */
      next_step = PARSE_FRAG;
      break;
    } else {
      /*
       * We verify that the PATH is not
       * a pattern
       */
      is_pattern = is_pattern || cstr[pos] == '*';

      if (cstr[pos] == '/') {
        if (path.back() != '/') {
          /*
           * We found a slash ! But we verify that the user
           * did not made the mistake:
           * 'example.com//john/doe' or 'example.com/john//doe'
           */

          if (!rel_path) {
            /*
             * We check that there's no stuff like:
             * 'example.com/john/./doe' or
             * 'example.com/john/../doe'
             */

            // Extracting the path to a temp variable
            std::string subpath = path.substr(slash_pos + 1, loc_pos - slash_pos - 1);

            if (subpath.compare(".") == 0 || subpath.compare("/.") == 0) {
              /*
               * Do not include this reference, but save the slash pos
               */
              slash_pos = loc_pos;
            } else if (subpath.compare("..") == 0 || subpath.compare("/..") == 0) {
              /*
               * It refers to a parent, let's erase the parent directory
               */
              if (!segments.empty()) {
                /*
                 * Before poping, verify that the vectory is not empty
                 * otherwise... !!! Segmentation fault !!!
                 */
                segments.pop_back();
              }
              slash_pos = loc_pos;
            } else {
              /*
               * Good, it is well formated
               */
              segments.push_back(subpath);
              slash_pos = loc_pos;
            }
          } else {
            /*
             * Relative path, no matter of './' or '../'
             */
            segments.push_back(path.substr(slash_pos + 1, loc_pos - slash_pos - 1));
            slash_pos = loc_pos;
          }
        } else {
          /*
           * We two slashes side by site. Skip it !
           */
          pos++;
          continue;
        }
      }

      path.push_back(cstr[pos]);

      loc_pos++;
      pos++;
    }
  }

  if (loc_pos - slash_pos - 1 > 0
      && static_cast<unsigned long>(slash_pos + 1) < path.size()) {
    /*
     * Include the last level found
     *
     * It could add an empty level is URL looks like
     * 'example.com/john/doe/'
     *                      ^ the last slash
     *
     * The second verification excludes
     * 'example.com/john/doe//'
     */
    segments.push_back(path.substr(slash_pos + 1, loc_pos - slash_pos - 1));
  }

  /*
   * If the URL refers to a directory, the path ends with a
   * slash
   */
  is_dir = path.back() == '/';

  if (!rel_path) {
    /*
     * After all the path parsing is
     * done we add the '/' root slash
     */
    path.insert(0, "/");
  }

  for (auto it = segments.begin();
       it != segments.end();
       it++) {
    /*
     * We perform a percentage encoding
     */
    if (!it->empty()) {
      std::string encoded;
      encode (*it, encoded);
      *it = encoded;
    }
  }

  /*
   * Time to check the status what to do...
   */
  if (pos >= pos_max) {
    /*
     * The end of the string, ok. Exits !
     */
    return NO_PARSE;
  } else if (next_step != NO_PARSE) {
    /*
     * The story continues with QUERY or FRAG...
     */
    return next_step;
  } else {
    /*
     * Not the end of the URL and no separator ?!
     */
    return ERR_PARSE;
  }
}

int UrlParser::parse_query(const char* cstr, size_t& pos, const size_t pos_max)
{
  /*
   * We delete the first char which is '?'
   */
  pos++;

  /*
   * We arrived within QUERY,
   * thus the only option available is:
   * - FRAG.
   */
  int next_step {NO_PARSE};

  /*
   * Saving interal PATH separator positions
   */
  long loc_pos {0}; // Position within QUERY substring
  long sep_pos {-1}; // Optional, a query can have only 1 arg

  while (pos < pos_max) {
    if (cstr[pos] == '#') {
      /*
       * The '#' separator after a QUERY
       * is only reserved for FRAG
       */
      next_step = PARSE_FRAG;
      break;
    } else {
      query.push_back(cstr[pos]);

      if (cstr[pos] == ';' || cstr[pos] == '&') {
        /*
         * Actually a query, could be formed:
         * 'example.com?fname=john&name=doe' or
         * 'example.com?fname=john;name=doe'
         */
        if (loc_pos - sep_pos - 1 > 0) {
          /*
           * Only save non-empty queries
           */
          arguments.push_back(query.substr(sep_pos + 1, loc_pos - sep_pos - 1));
        }
        sep_pos = loc_pos;
      }

      loc_pos++;
      pos++;
    }
  }

  if (loc_pos - sep_pos - 1 > 0) {
    /*
     * Only save non-empty queries
     */
    arguments.push_back(query.substr(sep_pos + 1, loc_pos - sep_pos - 1));
  }

  for (auto it = arguments.begin();
       it != arguments.end();
       it++) {
    /*
     * We perform a percentage encoding
     */
    if (!it->empty()) {
      std::string encoded;
      encode (*it, encoded);
      *it = encoded;
    }
  }

  /*
   * Time to check the status what to do...
   */
  if (pos >= pos_max) {
    /*
     * The end of the string, ok. Exits !
     */
    return NO_PARSE;
  } else if (next_step != NO_PARSE) {
    /*
     * The story continues with FRAG !
     */
    return next_step;
  } else {
    /*
     * Not the end of the URL and no separator ?!
     */
    return ERR_PARSE;
  }
}

int UrlParser::parse_frag(const char* cstr, size_t& pos, const size_t pos_max)
{
  /*
   * We delete the first char which is '#'
   */
  pos++;

  while (pos < pos_max) {
    frag.push_back(cstr[pos]);
    pos++;
  }

  /*
   * We perform a percentage encoding
   */
  if (!frag.empty()) {
    std::string encoded;
    encode (frag, encoded);
    frag = encoded;
  }

  /*
   * Time to check the status what to do...
   */
  if (pos >= pos_max) {
    /*
     * The end of the string, ok. Exits !
     */
    return NO_PARSE;
  } else {
    /*
     * Not the end of the URL and no separator ?!
     */
    return ERR_PARSE;
  }
}

std::string UrlParser::get_url(bool get_scheme, bool get_auth, bool get_path,
                               bool get_query, bool get_frag)
{
  std::string out_url;

  if (get_scheme && !rel_scheme) {
    /*
     * Scheme output
     * Without '//' because it depends if
     * it is followed by an AUTH or a PATH
     */
    out_url.append(scheme).append(":");
  }

  if (get_auth && !rel_auth && !auth.empty()) {
    /*
     * Authority output
     * Allawys starts with '//'
     */
    out_url.append("//").append(auth);
  }

  if (get_path) {
    /*
     * Reconstructing clean path
     */
    std::string out_path;

    if (!rel_path) {
      /*
       * If a path is not relative, it has to start with a '/'
       */
      out_path = "/";
    }

    /*
     * Let's add all the segments
     */
    for (auto& seg : segments) {
      out_path.append(seg).append("/");
    }

    if (!is_dir && !out_path.empty()) {
      out_path.pop_back();
    }

    out_url.append(out_path);
  }

  if (get_query) {
    if (!query.empty()) {
      out_url.append("?");

      for(auto& arg : arguments) {
        out_url.append(arg).append("&");
      }

      out_url.pop_back();
    }
  }

  if (get_frag && !frag.empty()) {
    out_url.append("#").append(frag);
  }

  return out_url;
}

UrlParser& UrlParser::operator+=(UrlParser& rhs)
{
  if (auth_less || rhs.auth_less) {
    /*
     * We are not handling AUTH-less URLs addition
     */
    return *this;
  } else if (is_complete && rhs.is_complete) {
    /*
     * One cannot decide what to add if
     * two URLs are already completed
     */
    return *this;
  } else {
    /*
     * At least one of the two URLs is incomplete,
     * by checking which one is relative to another
     * we will make the addition
     */

    if (rel_scheme && !rhs.rel_scheme) {
      /*
       * One checks if we have a relative SCHEME
       * and the RHS got one.
       */
      scheme = rhs.scheme;
      rel_scheme = false;
    }

    if (rel_auth && !rhs.rel_auth) {
      /*
       * One checks if we have a relative AUTH
       * and the RHS got one.
       */
      auth = rhs.auth;
      user = rhs.user;
      pass = rhs.pass;
      host = rhs.host;
      port = rhs.port;
      rel_auth = false;
    }

    if (rel_path && !rhs.rel_path) {
      /*
       * So we have a relative PATH
       * We will inherit BASE from the RHS
       */
      if (!rhs.is_dir) {
        /*
         * We verify that the RHS path does not refer
         * to a driectory
         *
         * 'sth' += '/john/doe' = '/john/sth'
         *                 ^^^ to remove
         */
        if (!rhs.segments.empty()) {
          segments.insert(segments.begin(),
                          rhs.segments.begin(),
                          rhs.segments.end() - 1);
        }
      } else {
        /*
         * We are refering to a directory
         *
         * 'sth' += '/john/doe/'
         * = '/john/doe/sth'
         */
        segments.insert(segments.begin(),
                        rhs.segments.begin(),
                        rhs.segments.end());
      }

      rel_path = false;
    } else if (!rel_path & rhs.rel_path) {
      /*
        * RHS has a relative path
        *
        * '/john/doe' += 'sth'
        * = '/john/sth'
        */
      if (!is_dir) {
        /*
          * We verify that the RHS path does not refer
          * to a driectory
          *
          * '/john/doe' += sth = '/john/sth'
          *        ^^^ to remove
          */
        if (!segments.empty()) {
          segments.pop_back(); // removes segment until '/'
        }
      }

      segments.insert(segments.end(),
                      rhs.segments.begin(),
                      rhs.segments.end());

      /*
        * Inherhit DIR properties from RHS
        */
      is_dir = rhs.is_dir;

      /*
        * RHS is only a relative PATH
        * and could be made of QUERY & FRAG
        * 'john/doe/sth?query#frag'
        */
      query = rhs.query;
      arguments = rhs.arguments;
      frag = rhs.frag;
    }

    if (!rel_path) {
      /*
       * We have to clean the URL
       */
      for (auto i = segments.begin();
           (i != segments.end()) && segments.size() > 0;) {
        if (i->empty()) {
          i = this->segments.erase(i);
        } else if (i->compare(".") == 0 || i->compare("/.") == 0) {
          i = segments.erase(i);
        } else if (i->compare("..") == 0) {
          i = this->segments.erase(i);
          if (i != segments.begin()) {
            i--;
            i = this->segments.erase(i);
          }
        } else {
          i++;
        }
      }
      path = get_url(false, false, true, false, false);
    }

    /*
     * Final recompute state of 'is_complete'
     */
    is_complete = !rel_scheme && !rel_auth && !rel_path;
  }

  /*
   * Finally we reset the saved URL
   */
  url = get_url();

  return *this;
}

UrlParser UrlParser::operator+(UrlParser& rhs)
{
  UrlParser lhs {*this};
  return lhs += rhs;
}

std::ostream& operator<<(std::ostream& os, const UrlParser& rhs)
{
  if (rhs.is_good) {
    os << "[URLPARSER] Results for:" << std::endl;
    os << " URL: " << rhs.url << std::endl << std::endl;

    os << " SCHEME    " << rhs.scheme << std::endl << std::endl;

    os << " AUTH      " << rhs.auth << std::endl;
    os << "   User    " << rhs.user << std::endl;
    os << "   Pass    " << rhs.pass << std::endl;
    os << "   Host    " << rhs.host << std::endl;
    os << "   Port    " << rhs.port << std::endl << std::endl;

    os << " PATH      " << rhs.path << std::endl;
    for (auto& elem : rhs.segments) {
      os << "   /       " << elem << std::endl;
    }

    os << " QUERY     " << rhs.query << std::endl;
    for (auto& elem : rhs.arguments) {
      os << "   &       " << elem << std::endl;
    }
    os << std::endl;

    os << " FRAG      " << rhs.frag << std::endl << std::endl;

    os << "Relativity of the URL:" << std::endl;
    if (rhs.rel_path) {
      os << "[SCHEME://AUTH/PARENT_PATH]PATH" << std::endl;
    } else if (rhs.rel_auth) {
      os << "[SCHEME://AUTH]/PATH" << std::endl;
    } else if (rhs.rel_scheme) {
      os << "[SCHEME:]//AUTH/PATH" << std::endl;
    } else if (rhs.auth_less) {
      os << "SCHEME:PATH" << std::endl;
    } else {
      os << "SCHEME://AUTH/PATH" << std::endl;
    }
  } else {
    os << "[URLPARSER] Malformed URL" << std::endl;
  }

  return os << std::endl;
}

bool UrlParser::operator==(UrlParser& rhs)
{
  if (is_pattern && rhs.is_pattern) {
    /*
     * Come on, how can I compare two patterns...
     */
    return true;
  }

  /*
   * We extract URL for comparison with same amout of
   * details
   */
  std::string lhs_url = get_url(!(rel_scheme || rhs.rel_scheme),
                                !(rel_auth || rhs.rel_auth));

  std::string rhs_url = rhs.get_url(!(rel_scheme || rhs.rel_scheme),
                                    !(rel_auth || rhs.rel_auth));

  /*
   * There is two iterators in case of PATTERNS
   */
  size_t lhs_pos {0};
  size_t lhs_limit = lhs_url.size();

  size_t rhs_pos {0};
  size_t rhs_limit = rhs_url.size();


  /*
   * Let's see if URLs are matching
   */
  int res = url_matching(lhs_url, lhs_pos,
                         rhs_url, rhs_pos);

  if (res == 1) {
    /*
     * This is a ending pattern case
     * (which says that LHS is at least equal to RHS)
     *
     * But here 'operator==(&)' returns TRUE
     * if a pattern is matched !
     * Because an pure equality between string and pattern
     * DOES NOT exist.
     */
    return true;
  } else if (res == 0) {
    /*
     * Everything worked fine, all chars and patterns (if getting one)
     * where correctly verified.
     *
     * But we are outside of a pattern,
     * and the equality means that :
     * - LHS_POS reached LHS_LIMIT,
     * - and RHS_POS reached RHS_LIMIT.
     */
    return lhs_pos == lhs_limit && rhs_pos == rhs_limit;
  } else {
    /*
     * The matching algorithm said
     * that outside of a pattern two chars DID NOT matched !
     */
    return false;
  }
}

bool UrlParser::operator>=(UrlParser& rhs)
{
  if (is_pattern && rhs.is_pattern) {
    /*
     * Come on, how can I compare two patterns...
     */
    return true;
  }

  /*
   * We extract URL for comparison with same amout of
   * details
   */
  std::string lhs_url = get_url(!(rel_scheme || rhs.rel_scheme),
                                !(rel_auth || rhs.rel_auth));

  std::string rhs_url = rhs.get_url(!(rel_scheme || rhs.rel_scheme),
                                    !(rel_auth || rhs.rel_auth));

  /*
   * There is two iterators in case of PATTERNS
   */
  size_t lhs_pos {0};
  size_t lhs_limit = lhs_url.size();

  size_t rhs_pos {0};
  size_t rhs_limit = rhs_url.size();

  /*
   * Let's see if URLs are matching
   */
  int res = url_matching(lhs_url, lhs_pos,
                         rhs_url, rhs_pos);

  if (res == 1) {
    /*
     * This is a ending pattern case
     * which says that LHS is at least equal to RHS
     */
    return true;
  } else if (res == -1) {
    /*
     * The matching algorithm said
     * that outside of a pattern two chars DID NOT matched !
     */
    return false;
  } else {
    /*
     * Everything worked fine, all chars and patterns (if getting one)
     * where correctly verified.
     *
     * But we are outside of a pattern, so the LHS_POS >= LHS_LIMIT
     */
    if (lhs_pos == lhs_limit && rhs_pos == rhs_limit) {
      /*
       * In this case, LHS and RHS reached their limit
       * we call it equality !
       */
      return true;
    } else if (lhs_pos < lhs_limit && rhs_pos == rhs_limit) {
      /*
       * Great, some terms are remaning within LHS
       */
      if (lhs_url[lhs_pos - 1] == '/' ||
          lhs_url[lhs_pos] == '/' ||
          lhs_url[lhs_pos - 1] == '?' ||
          lhs_url[lhs_pos] == '?') {
        /*
         * One found separator:
         * - '/p' > 'p'
         * - 'p/' > 'p'
         * - 'p?' > 'p'
         */
        return true;
      } else {
        /*
         * No separator, different directory:
         * 'p10' !(>=) 'p'
         */
        return false;
      }
    } else {
      /*
       * It seems that RHS reached the END,
       * but not LHS. Thus LHS < RHS
       */
      return false;
    }
  }
}

bool UrlParser::operator>(UrlParser& rhs)
{
  if (is_pattern && rhs.is_pattern) {
    /*
     * Come on, how can I compare two patterns...
     */
    return true;
  }

  /*
   * We extract URL for comparison with same amout of
   * details
   */
  std::string lhs_url = get_url(!(rel_scheme || rhs.rel_scheme),
                                !(rel_auth || rhs.rel_auth));

  std::string rhs_url = rhs.get_url(!(rel_scheme || rhs.rel_scheme),
                                    !(rel_auth || rhs.rel_auth));

  /*
   * There is two iterators in case of PATTERNS
   */
  size_t lhs_pos {0};
  size_t lhs_limit = lhs_url.size();

  size_t rhs_pos {0};
  size_t rhs_limit = rhs_url.size();

  /*
   * Let's see if URLs are matching
   */
  int res = url_matching(lhs_url, lhs_pos,
                         rhs_url, rhs_pos);

  if (res == 1) {
    /*
     * This is a ending pattern case
     * which says that LHS is at least equal to RHS
     *
     * So one has to check that LHS_POS < LHS_LIMIT
     */
    return lhs_pos < lhs_limit;
  } else if (res == -1) {
    /*
     * The matching algorithm said
     * that outside of a pattern two chars DID NOT matched !
     */
    return false;
  } else {
    /*
     * Everything worked fine, all chars and patterns (if getting one)
     * where correctly verified.
     *
     * But we are outside of a pattern, so the LHS_POS >= LHS_LIMIT
     */
    if (lhs_pos == lhs_limit && rhs_pos == rhs_limit) {
      /*
       * In this case, LHS and RHS reached their limit
       * they are equal, but not supperior to...
       */
      return false;
    } else if (lhs_pos < lhs_limit && rhs_pos == rhs_limit) {
      /*
       * Great, some terms are remaning within LHS
       */
      if (lhs_url[lhs_pos - 1] == '/' ||
          lhs_url[lhs_pos] == '/' ||  
          lhs_url[lhs_pos - 1] == '?' ||
          lhs_url[lhs_pos] == '?') {
        /*
         * One found separator:
         * - '/p' > 'p'
         * - 'p/' > 'p'
         * - 'p?' > 'p'
         */
        return true;
      } else {
        /*
         * No separator, different directory:
         * 'p10' !(>=) 'p'
         */
        return false;
      }
    } else {
      /*
       * It seems that RHS reached the END,
       * but not LHS. Thus LHS < RHS
       */
      return false;
    }
  }
}

int UrlParser::url_matching(std::string& lhs, size_t& lhs_pos,
                            std::string& rhs, size_t& rhs_pos)
{
  long pat_pos {-1}; // saving the last pattern position
  long prev_pat_pos {-1}; // saving the precedent pattern position

  size_t lhs_limit = lhs.size();
  size_t rhs_limit = rhs.size();

  while ((lhs_pos < lhs_limit) && (rhs_pos < rhs_limit)) {
    if (pat_pos >= 0) {
      /*
       * We are currently reading a pattern
       */
      if (lhs[lhs_pos] != rhs[rhs_pos]) {
        /*
         * The char read in LHS does not
         * match the restart char of RHS.
         * So, one continues reading of the pattern.
         */
        lhs_pos++;
      } else {
        /*
         * LHS char matched the restart char of RHS
         * We continue to read strings as common ones.
         */
        prev_pat_pos = pat_pos; // saving the '*' pos
        pat_pos = -1; // no pattern currently read

        /*
         * Normal reading, incrementing both
         */
        lhs_pos++;
        rhs_pos++;
      }
    } else {
      /*
       * We are reading a common string
       */
      if (rhs[rhs_pos] == '*') {
        /*
         * We found a pattern delimiter in RHS
         */
        pat_pos = rhs_pos; // saving the delimiter pos

        /*
         * We verify that we does not have a ending pattern:
         * 'example.com/sth*'
         */
        if (rhs_pos + 1 < rhs_limit) {
          rhs_pos++; // Let's move to the RHS restart char
        } else {
          /*
           * We reached the end of the pattern in RHS
           * Terms are perhaps remaining in LHS,
           * thus LHS is at least EQUAL to RHS.
           */
          return 1;
        }
      } else if (lhs[lhs_pos] != rhs[rhs_pos]) {
        /*
         * We found diffrent chars in LHS and RHS
         */
        if (prev_pat_pos >= 0) {
          /*
           * A pattern has been shown,
           * so it was a false positive restart
           */
          pat_pos = prev_pat_pos; // setting the '*' position
          rhs_pos = pat_pos + 1; // putting RHS to the restart char
          prev_pat_pos = -1; // reseting prev_pat_pos
        } else {
          /*
           * Not in a pattern,
           * clearly LHS != RHS
           */
          return -1;
        }
      } else {
        /*
         * We reading a common string
         * and char are equal, let's continue !
         */
        lhs_pos++;
        rhs_pos++;
      }
    }
  }

  /*
   * Return 0 means that:
   * - all pattern matched yet
   * - all chars in common string where equal
   */
  return 0;
}

bool UrlParser::valid_scheme(std::initializer_list<std::string> schemes)
{
  for (auto& s : schemes)
    if(s.compare(scheme) == 0)
      return true;

  return false;
}

} // namespace urlfactory

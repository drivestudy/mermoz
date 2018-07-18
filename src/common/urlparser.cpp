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

#include "urlparser.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace common
{

UrlParser& UrlParser::operator+=(UrlParser& rhs)
{
  if (complete_url && rhs.complete_url)
  {
    mc::print_warning ("Cannot add two complete URLs, return LHS");
    return *this;
  }
  else if (!complete_url && !rhs.complete_url)
  {
    mc::print_warning ("Cannot add two in-complete URLs, return LHS");
    return *this;
  }
  else if (!complete_url)
  {
    if (inherit_scheme)
    {
      scheme = rhs.scheme;
      inherit_scheme = false;
    }

    if (inherit_auth)
    {
      authority = rhs.authority;
      user = rhs.user;
      pass = rhs.pass;
      domain = rhs.domain;
      port = rhs.port;

      inherit_auth = false;
    }

    if (inherit_path)
    {
      path_tree.insert(path_tree.begin(), rhs.path_tree.begin(), rhs.path_tree.end());
      inherit_path = false;
    }
  }
  else if (!rhs.complete_url)
  {
    if (!rhs.inherit_scheme)
      scheme = rhs.scheme;

    if (!rhs.inherit_auth)
    {
      authority = rhs.authority;
      user = rhs.user;
      pass = rhs.pass;
      domain = rhs.domain;
      port = rhs.port;
    }

    if (rhs.inherit_path)
    {
      path_tree.insert(path_tree.end(), rhs.path_tree.begin(), rhs.path_tree.end());

      query = rhs.query;
      query_args = rhs.query_args;

      fragment = rhs.fragment;
    }
  }

  complete_url = !inherit_scheme && !inherit_auth && !inherit_path;

  // Cleanning up the path_tree vector
  std::vector<std::string>::iterator i = this->path_tree.begin();
  while (i != this->path_tree.end())
  {
    if (i->empty())
    {
      i = this->path_tree.erase(i);
    }
    else if (i->compare("..") == 0)
    {
      i = this->path_tree.erase(i);
      if (i != path_tree.begin())
      {
        i--;
        i = this->path_tree.erase(i);
      }
    }
    else
    {
      i++;
    }
  }

  // Constructing the full path
  this->path = "";
  for (auto& item : this->path_tree)
  {
    path.append(item);
    path.append("/");
  }
  // Removes the last "/"
  if (!this->path.empty())
    path.pop_back();

  return *this;
}

UrlParser UrlParser::operator+(UrlParser& rhs)
{
  UrlParser up(*this);
  up += rhs;
  return up;
}

std::ostream& operator<<(std::ostream& os, const UrlParser& rhs)
{
  os << "scheme    " << rhs.scheme << std::endl;
  os << "authority " << rhs.authority << std::endl;
  os << "  user    " << rhs.user << std::endl;
  os << "  pass    " << rhs.pass << std::endl;
  os << "  port    " << rhs.port << std::endl;
  os << "  domain  " << rhs.domain << std::endl;
  os << "path      " << rhs.path << std::endl;
  for (auto& elem : rhs.path_tree)
  {
    os << "  /       " << elem << std::endl;
  }
  os << "query     " << rhs.query << std::endl;
  for (auto& elem : rhs.query_args)
  {
    os << "  &       " << elem << std::endl;
  }
  os << "fragment  " << rhs.fragment << std::endl;
  os << "i_scheme  " << rhs.inherit_scheme << std::endl;
  os << "i_auth    " << rhs.inherit_auth << std::endl;
  os << "i_path    " << rhs.inherit_path << std::endl;
  os << "complete  " << rhs.complete_url << std::endl;

  return os;
}

bool UrlParser::operator>(const UrlParser& rhs)
{
  if (authority.empty() || rhs.authority.empty())
  {
    throw std::invalid_argument("Cannot compare URLs without authority");
  }

  if (domain.compare(rhs.domain) != 0)
  {
    return false;
  }
  else
  {
    if (path_tree.size() <= rhs.path_tree.size())
      return false;

    auto it_lhs = path_tree.begin();
    auto it_rhs = rhs.path_tree.begin();

    bool is_diff {false};

    while (it_lhs != path_tree.end() &&
           it_rhs != rhs.path_tree.end())
    {
      if (it_lhs->empty() ^ it_rhs->empty())
      {
        it_lhs--;
        it_rhs--;
        break;
      }

      if (it_lhs->compare(*it_rhs) != 0)
      {
        is_diff = true;
        break;
      }
      else
      {
        it_lhs++;
        it_rhs++;
      }
    }

    return !is_diff && (it_lhs < path_tree.end());
  }
}

bool UrlParser::operator>=(const UrlParser& rhs)
{
  if (authority.empty() || rhs.authority.empty())
  {
    throw std::invalid_argument("Cannot compare URLs without authority");
  }

  if (domain.compare(rhs.domain) != 0)
  {
    return false;
  }
  else
  {
    if (path_tree.size() < rhs.path_tree.size())
      return false;

    auto it_lhs = path_tree.begin();
    auto it_rhs = rhs.path_tree.begin();

    bool is_diff {false};

    while (it_lhs != path_tree.end() &&
           it_rhs != rhs.path_tree.end())
    {
      if (it_lhs->empty() ^ it_rhs->empty())
      {
        it_lhs--;
        it_rhs--;
        break;
      }

      if (it_lhs->compare(*it_rhs) != 0)
      {
        is_diff = true;
        break;
      }
      else
      {
        it_lhs++;
        it_rhs++;
      }
    }

    return !is_diff;
  }
}

bool UrlParser::operator<(const UrlParser& rhs)
{
  return !(*this >= rhs);
}

bool UrlParser::operator<=(const UrlParser& rhs)
{
  return !(*this > rhs);
}

std::string UrlParser::get_url(bool get_query, bool get_fragment)
{
  std::string out_url;

  if (!scheme.empty())
    out_url.append(scheme).append("://");

  if (!authority.empty())
    out_url.append(authority).append("/");

  for (auto& elem : path_tree)
    if (!elem.empty() || elem.compare(" ") == 0)
      out_url.append(elem).append("/");

  if(!out_url.empty())
    out_url.pop_back(); // remove last '/'

  if (get_query)
  {
    if (!query.empty() && !query_args.empty())
    {
      out_url.append("?");
      for (auto& elem : query_args)
        if (!elem.empty())
          out_url.append(elem).append("&");

      out_url.pop_back(); // remove last '&'
    }
  }

  if (get_fragment)
    if (!fragment.empty())
      out_url.append("#").append(fragment);

  return out_url;
}

void UrlParser::exchange(std::string scheme1, std::string scheme2)
{
  if (scheme.compare(scheme1) == 0)
    scheme = scheme2;
  else if (scheme.compare(scheme2) == 0)
    scheme = scheme1;
  else
    print_warning("Switch-scheme not handled by UrlParser");
}

bool UrlParser::valid_scheme(std::initializer_list<std::string> schemes)
{
  for (auto& s : schemes)
    if(s.compare(scheme) == 0)
      return true;

  return false;
}

bool UrlParser::parse()
{
  // sanitize the URL
  size_t pos {0};
  for (unsigned char c : url)
  {
    if (c <= 0x20 || c >= 0x7f)
      pos++;
    else
      break;
  }

  url = url.substr(pos);

  // trim invalid chars
  unsigned char c;
  while (((c = *(url.end()-1)) < 0x20) && !url.empty())
    url.pop_back();


  if (url[0] == '/')
  {
    inherit_scheme = true;

    if (url[1] == '/')
      do_authority = true;
    else
    {
      inherit_auth = true;
      do_path = true;
    }
  }
  else if (url[0] == '.' || url[0] == '*')
  {
    inherit_scheme = true;
    inherit_auth = true;
    inherit_path = true;

    do_path = true;
  }
  else
    do_scheme = true;

  std::stringstream ss(url);
  std::streambuf* sb = ss.rdbuf();

  while (do_parse)
  {
    if (do_scheme)
      parse_scheme(sb);
    else if (do_authority)
      parse_authority(sb);
    else if (do_path)
      parse_path(sb);
    else if (do_query)
      parse_query(sb);
    else if (do_fragment)
      parse_fragment(sb);
    else
      return false;
  }

  complete_url = !inherit_scheme && !inherit_auth && !inherit_path;

  return true;
}

void UrlParser::parse_scheme(std::streambuf* sb)
{
  do_scheme = false;

  char c;
  int nslash {0};
  bool has_separator {false};

  while ((c = sb->sbumpc()) != EOF)
  {
    if (c != ':')
    {
      scheme.push_back(c);
    }
    else
    {
      has_separator = true;
      while ((c = sb->sbumpc()) == '/') { nslash++; }
      sb->sungetc();
      break;
    }
  }

  do_parse = c != EOF;

  if (!has_separator)
  {
    // case of relative pathes
    sb->pubseekoff(0, std::ios_base::beg);
    scheme = "";

    inherit_scheme = true;
    inherit_auth = true;
    inherit_path = true;

    do_parse = true;
    do_path = true;
  }
  else
  {
    nslash == 2 ? do_authority = true : do_path = true;
  }
}

void UrlParser::parse_authority(std::streambuf* sb)
{
  do_authority = false;

  int pos{0};
  int colon_pos{-1};
  int domain_limit {0};

  char c;

  while ((c = sb->sbumpc()) == '/') {}
  sb->sungetc();

  while ((c = sb->sbumpc()) != EOF)
  {
    if (c == '/')
    {
      do_path = true;
      break;
    }
    else if (c == '?')
    {
      do_query = true;
      break;
    }
    else if (c == '#')
    {
      do_fragment = true;
      break;
    }
    else
    {
      authority.push_back(c);

      if (c == ':')
      {
        colon_pos = pos;
      }
      else if (c == '@')
      {
        if (colon_pos > 0)
        {
          user = authority.substr(0, colon_pos);
          pass = authority.substr(colon_pos + 1, pos - colon_pos - 1);
        }
        else
        {
          user = authority.substr(0, pos);
        }
        colon_pos = -1;
        domain_limit = pos + 1;
      }

      pos++;
    }
  }

  do_parse = c != EOF;

  if (authority.compare(".") == 0)
  {
    authority = "";
  }
  else if (authority.compare("..") == 0)
  {
    authority = "";
    sb->pubseekoff(0, std::ios_base::beg);
    do_parse = true;
    do_path = true;
  }

  if (colon_pos > 0)
  {
    domain = authority.substr(domain_limit, colon_pos - domain_limit);
    port = authority.substr(colon_pos + 1, pos - colon_pos - 1);
  }
  else
  {
    domain = authority.substr(domain_limit, pos - domain_limit);
  }
}

void UrlParser::parse_path(std::streambuf* sb)
{
  do_path = false;

  int pos {0};
  int slash_pos {-1};
  char c;

  while ((c = sb->sbumpc()) == '/') {}
  sb->sungetc();

  while ((c = sb->sbumpc()) != EOF)
  {
    if (c == '?')
    {
      do_query = true;
      break;
    }
    else if (c == '#')
    {
      do_fragment = true;
      break;
    }
    else
    {
      path.push_back(c);

      if (c == '/' && path.size() > 1)
      {
        path_tree.push_back(path.substr(slash_pos + 1, pos - slash_pos - 1));
        slash_pos = pos;
      }
      else if (c == '/' && path.size() == 1)
      {
        path.pop_back();
      }

      pos++;
    }
  }

  path_tree.push_back(path.substr(slash_pos + 1, pos - slash_pos - 1));

  if (path.empty())
  {
    path_tree.clear();
  }

  do_parse = c != EOF;
}

void UrlParser::parse_query(std::streambuf* sb)
{
  do_query = false;

  int pos {0};
  int sep_pos {-1};
  char c;

  while ((c = sb->sbumpc()) != EOF)
  {
    if (c == '#')
    {
      do_fragment = true;
      break;
    }
    else
    {
      query.push_back(c);

      if (c == ';' || c == '&')
      {
        query_args.push_back(query.substr(sep_pos + 1, pos - sep_pos - 1));
        sep_pos = pos;
      }

      pos++;
    }
  }

  query_args.push_back(query.substr(sep_pos + 1, pos - sep_pos - 1));

  do_parse = c != EOF;
}

void UrlParser::parse_fragment(std::streambuf* sb)
{
  do_fragment = false;

  char c;

  while ((c = sb->sbumpc()) != EOF)
  {
    fragment.push_back(c);
  }
}

} // namespace common
} // namespace mermoz

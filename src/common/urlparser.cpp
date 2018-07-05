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

namespace mermoz
{
namespace common
{

bool UrlParser::parse()
{
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

  return true;
}

UrlParser& UrlParser::operator+=(UrlParser& rhs)
{
  // Scheme-less case
  if (this->scheme.empty() && !this->authority.empty())
  {
    this->scheme = rhs.scheme;
    return *this;
  }

  // Relative path case
  if (this->scheme.empty() && this->authority.empty())
  {
    // L.H.S. has a relative path
    this->scheme = rhs.scheme;
    this->authority = rhs.authority;
    this->user = rhs.user;
    this->pass = rhs.pass;
    this->port = rhs.port;

    this->path_tree.insert(this->path_tree.begin(), rhs.path_tree.begin(), rhs.path_tree.end());
  }
  else if (rhs.scheme.empty() && rhs.authority.empty())
  {
    // R.H.S. has a relative path
    this->path_tree.insert(this->path_tree.end(), rhs.path_tree.begin(), rhs.path_tree.end());

    this->query = rhs.query;
    this->query_args = rhs.query_args;

    this->fragment = rhs.fragment;
  }
  else
  {
    throw std::invalid_argument("Both UrlParsers have scheme and/or authority");
  }

  // Cleanning up the path_tree vector
  std::vector<std::string>::iterator i = this->path_tree.begin();
  while (i != this->path_tree.end())
  {
    if (i->empty())
    {
      i = this->path_tree.erase(i);
    }
    else if (*i == "..")
    {
      i = this->path_tree.erase(i);
      i--;
      i = this->path_tree.erase(i);
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

std::ostream& operator<<(std::ostream& os, UrlParser& rhs)
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

  return os;
}

void UrlParser::parse_scheme(std::streambuf* sb)
{
  do_scheme = false;

  char c;

  if ((c = sb->sbumpc()) == '/')
  {
    if ((c = sb->sbumpc()) == '/')
    {
      sb->pubseekoff(0, std::ios_base::beg);
      do_authority = true;
      return;
    }

    sb->pubseekoff(0, std::ios_base::beg);
    do_path = true;
    return;
  }
  sb->sungetc();

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
      while ((c = sb->sbumpc()) == '/') {}
      sb->sungetc();
      break;
    }
  }

  do_parse = c != EOF;

  if (!has_separator && !do_parse)
  {
    // case of relative pathes
    sb->pubseekoff(0, std::ios_base::beg);
    scheme = "";
    do_parse = true;
    do_path = true;
  }
  else
  {
    do_authority = true;
  }
}

void UrlParser::parse_authority(std::streambuf* sb)
{
  do_authority = false;

  int pos{0};
  int semicolon_pos{-1};
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
        semicolon_pos = pos;
      }
      else if (c == '@')
      {
        if (semicolon_pos > 0)
        {
          user = authority.substr(0, semicolon_pos);
          pass = authority.substr(semicolon_pos + 1, pos - semicolon_pos - 1);
        }
        else
        {
          user = authority.substr(0, pos);
        }
        semicolon_pos = -1;
        domain_limit = pos + 1;
      }

      pos++;
    }
  }

  if (semicolon_pos > 0)
  {
    domain = authority.substr(domain_limit, semicolon_pos - domain_limit);
    port = authority.substr(semicolon_pos + 1, pos - semicolon_pos - 1);
  }
  else
  {
    domain = authority.substr(domain_limit, pos - domain_limit);
  }

  do_parse = c != EOF;
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

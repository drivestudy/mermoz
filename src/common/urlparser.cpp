#include "urlparser.hpp"

namespace mermoz
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

void UrlParser::parse_scheme(std::streambuf* sb)
{
  do_scheme = false;

  char c;
  while ((c = sb->sbumpc()) != EOF)
  {
    if (c != ':')
      scheme.push_back(c);
    else
    {
      while ((c = sb->sbumpc()) == '/') {}
      sb->sungetc();
      break;
    }
  }

  do_authority = true;
  do_parse = c != EOF;
}

void UrlParser::parse_authority(std::streambuf* sb)
{
  do_authority = false;

  int pos{0};
  int semicolon_pos{-1};
  int domain_limit {0};

  char c;

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

  port = 80;
  if (semicolon_pos > 0)
  {
    domain = authority.substr(domain_limit, semicolon_pos - domain_limit);
    port = atoi(authority.substr(semicolon_pos + 1, pos - semicolon_pos - 1).c_str());
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

      if (c == '/')
      {
        path_tree.push_back(path.substr(slash_pos + 1, pos - slash_pos - 1));
        slash_pos = pos;
      }

      pos++;
    }
  }

  path_tree.push_back(path.substr(slash_pos + 1, pos - slash_pos - 1));

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
        std::cout << "separator" << std::endl;
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

} // namespace mermoz

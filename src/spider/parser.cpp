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

#include "spider/parser.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace spider
{

void parser(mermoz::common::AsyncQueue<std::string>* content_queue,
            mermoz::common::AsyncQueue<std::string>* parsed_queue,
            bool* status)
{
  while (*status)
  {
    std::string message;
    content_queue->pop(message);

    std::string url;
    std::string content;
    std::string http_status;
    mc::unpack(message, {&url, &content, &http_status});
    std::cout << "parsing " << url << std::endl;

    GumboOutput* output = gumbo_parse(content.c_str());

    std::string text = get_text(output->root);
    text_cleaner(text);

    std::string links = get_links(output->root);

    mc::pack(message, {&url, &text, &links, &http_status});

    parsed_queue->push(message);

    gumbo_destroy_output(&kGumboDefaultOptions, output);
  }
}

std::string get_text(GumboNode* node)
{
  if (node->type == GUMBO_NODE_TEXT)
  {
    return std::string(node->v.text.text);
  }
  else if (node->type == GUMBO_NODE_ELEMENT &&
           node->v.element.tag != GUMBO_TAG_SCRIPT &&
           node->v.element.tag != GUMBO_TAG_STYLE)
  {
    std::string fulltext("");
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
      const std::string text = get_text(static_cast<GumboNode*>(children->data[i]));
      if (i != 0 && !text.empty())
      {
        fulltext.append(" ");
      }
      fulltext.append(text);
    }
    return fulltext;
  }
  else
  {
    return "";
  }
}

std::string get_links(GumboNode* node)
{
  if (node->type != GUMBO_NODE_ELEMENT)
  {
    return "";
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
  {
    return std::string(href->value);
  }

  std::string links("");

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i)
  {
    std::string link = get_links(static_cast<GumboNode*>(children->data[i]));
    if (i != 0 && !link.empty())
    {
      while (*link.begin() == ',' && !link.empty())
      {
        link.erase(link.begin());
      }
      links.append(",");
    }
    links.append(link);
  }

  return links;
}

void text_cleaner (std::string& s)
{
  auto it = s.begin();

  while (it != s.end())
  {
    if (*it == '\t')
      *it = ' ';

    if (*it == ' ' && *(it+1) == ' ')
    {
      it = s.erase(it);
    }
    else if (*it == ' ' && *(it+1) == '\n')
    {
      it = s.erase(it);
    }
    else if (*it == '\n' && *(it+1) == ' ')
    {
      it = s.erase(it+1);
    }
    else if (*it == '\n' && *(it+1) == '\n')
    {
      it = s.erase(it+1);
    }
    else
    {
      it++;
    }
  }
}

} // namespace spider
} // namespace mermoz

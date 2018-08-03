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
 * Parsing routines are highly inspired from the 'gumbo-parser' examples:
 * - located at: https://github.com/google/gumbo-parser/tree/master/examples,
 * - and originaly written by: jdtang@google.com (Jonathan Tang).
 *
 */

#include "spider/parser.hpp"

namespace mermoz
{

void parser(thread_safe::queue<std::string>* content_queue,
            thread_safe::queue<std::string>* parsed_queue,
            std::atomic<uint64_t>* nparsed,
            MemSec* mem_sec,
            bool* status)
{
  while (*status)
  {
    std::string message;
    content_queue->pop(message);
    (*mem_sec) -= message.size();

    std::string url;
    std::string eff_url;
    std::string content;
    std::string http_status;
    unpack(message, {&url, &eff_url, &http_status, &content});

    message.clear();
    long http_code = atoi(http_status.c_str());

    if (http_code >= 200 && http_code < 300)
    {
      GumboOutput* output = gumbo_parse(content.c_str());

      std::map<std::string, std::string> page_properties =
        get_page_properties(output->root);

      std::string text = get_text(output->root);
      text_cleaner(text);

      std::string raw_links = get_links(output->root);
      std::string formated_urls;

      std::string base;
      auto mapit = page_properties.end();

      if ((mapit = page_properties.find("base")) != page_properties.end()) {
        urlfactory::UrlParser up_base(mapit->second);
        if (up_base.complete()) {
          base = up_base.get_url();
        } else {
          urlfactory::UrlParser up_eff(eff_url);
          up_base += up_eff;
          base = up_base.get_url();
        }
      } else {
        base = eff_url;
      }

      url_formating(base, raw_links, formated_urls);
      raw_links.clear();

      /**
       * For now on we just test the exploration
       * mechanism, clear text is deleted
       */
      text.clear();
      /**
       * To remove if you need data for indexing
       */

      pack(message, {&url, &eff_url, &http_status, &text, &formated_urls});

      gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
    else
    {
      std::string text, links;
      pack(message, {&url, &eff_url, &http_status, &text, &links});
    }

    (*mem_sec) += message.size();
    parsed_queue->push(message);

    ++(*nparsed);
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

std::map<std::string, std::string> get_page_properties(GumboNode* root)
{
  std::map<std::string, std::string> page_properties;

  if (!(root->type == GUMBO_NODE_ELEMENT)
      || !(root->v.element.children.length >= 2))
    return page_properties;

  const GumboVector* root_children = &root->v.element.children;
  GumboNode* head = nullptr;
  for (unsigned int i = 0; i < root_children->length; ++i)
  {
    GumboNode* child = static_cast<GumboNode*>(root_children->data[i]);
    if (child->type == GUMBO_NODE_ELEMENT &&
        child->v.element.tag == GUMBO_TAG_HEAD)
    {
      head = child;
      break;
    }
  }

  if (head == nullptr)
    return page_properties;

  GumboVector* head_children = &head->v.element.children;
  for (unsigned int i = 0; i < head_children->length; ++i)
  {
    GumboNode* child = static_cast<GumboNode*>(head_children->data[i]);
    if (child->type == GUMBO_NODE_ELEMENT)
    {
      if(child->v.element.tag == GUMBO_TAG_TITLE)
      {
        if (child->v.element.children.length == 1)
        {
          GumboNode* title_text = static_cast<GumboNode*>(child->v.element.children.data[0]);
          page_properties.emplace("title", title_text->v.text.text);
        }
      }
      else if (child->v.element.tag == GUMBO_TAG_BASE)
      {
        GumboAttribute* href;
        if ((href = gumbo_get_attribute(&child->v.element.attributes, "href")) != nullptr)
          page_properties.emplace("base", href->value);
      }
      else if (child->v.element.tag == GUMBO_TAG_META)
      {
        GumboAttribute* name;
        if ((name = gumbo_get_attribute(&child->v.element.attributes, "name")) != nullptr)
        {
          if (std::strcmp(name->value, "description") == 0)
          {
            GumboAttribute* content;
            if ((content = gumbo_get_attribute(&child->v.element.attributes, "content")) != nullptr)
              page_properties.emplace("description", content->value);
          }
        }
      }
    }
  }

  return page_properties;
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
      while (*link.begin() == '\n' && !link.empty())
      {
        link.erase(link.begin());
      }
      links.append("\n");
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

void url_formating(std::string& base, std::string& raw_urls, std::string& formated_urls)
{
  formated_urls.clear();

  if (raw_urls.empty())
    return;

  urlfactory::UrlParser baseup(base);

  std::istringstream iss(raw_urls);
  std::string link;

  while (!iss.eof())
  {
    link.clear();
    std::getline(iss, link);

    if (link.size() > 1)
    {
      if (link.find("javascript") == std::string::npos
          && link.find("mailto") == std::string::npos
          && link.find(",") == std::string::npos
          && link.find("404") == std::string::npos
          && link.find("download") == std::string::npos)
      {
        urlfactory::UrlParser up(link);

        if (!up.complete())
          up += baseup;

        if (up.valid_scheme({"http", "https"})) {
          /*
           * Do not follow links with fragment, it is the same page...
           */
          formated_urls.append(up.get_url(true, true, true, true, false)).append("\n");
        }
      }
    }
    else
    {
      continue;
    }
  }

  if (!formated_urls.empty())
    formated_urls.pop_back(); // removes last \n
}

} // namespace mermoz

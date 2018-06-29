#include "spider/parser.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace spider
{

void parser(mermoz::common::async_queue<std::string>* content_queue,
            mermoz::common::async_queue<std::string>* parsed_queue,
            bool* status)
{
  while (*status)
  {
    GumboOutput* output = gumbo_parse(content_queue->pop_out().c_str());

    std::string result ("{text: \'");
    result.append(get_text(output->root));

    result.append("\', links: \'");
    result.append(get_links(output->root));
    result.append("\'},\n");

    parsed_queue->push(result);

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
    const std::string link = get_links(static_cast<GumboNode*>(children->data[i]));
    if (i != 0 && !link.empty())
    {
      links.append(",");
    }
    links.append(link);
  }

  return links;
}

} // namespace spider
} // namespace mermoz

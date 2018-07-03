#ifndef MERMOZ_PARSER_H__
#define MERMOZ_PARSER_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "gumbo.h"

#include "common/common.hpp"

namespace mermoz
{
namespace spider
{

void parser(mermoz::common::async_queue<std::string>* content_queue,
             mermoz::common::async_queue<std::string>* parsed_queue,
             bool* status);

std::string get_text(GumboNode* node);

std::string get_links(GumboNode* node);

} // namespace spider
} // namespace mermoz

#endif // MERMOZ_PARSER_H__

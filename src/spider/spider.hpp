#ifndef MERMOZ_SPIDER_H__
#define MERMOZ_SPIDER_H__

#include "spider/fetcher.hpp"
#include "spider/parser.hpp"

namespace mermoz
{
namespace spider
{

void pack(std::string& pack, std::initializer_list<std::string*> args);
void unpack(std::string& pack, std::initializer_list<std::string*> args);

} // namespace spider
} // namespace mermoz

#endif // MERMOZ_SPIDER_H__

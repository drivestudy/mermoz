#ifndef MERMOZ_PACKER_H__
#define MERMOZ_PACKER_H__

#include <string>
#include <cstring>
#include <initializer_list>

namespace mermoz
{
namespace common
{

void pack(std::string& pack, std::initializer_list<std::string*> args);
void unpack(std::string& pack, std::initializer_list<std::string*> args);

} // namespace common
} // namespace mermoz

#endif // MERMOZ_PACKER_H__

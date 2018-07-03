#include "common/packer.hpp"

namespace mermoz
{
namespace common
{

void pack(std::string& pack, std::initializer_list<std::string*> args)
{
  if (args.size() == 0)
    return;

  size_t pos {0};
  for (auto& arg : args)
  {
    pos += arg->size() + sizeof(size_t);
  }

  pack.clear();
  pack.resize(pos);

  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size = arg->size();

    std::memcpy((void*)(&*itpack),
                (void*)(&arg_size),
                sizeof(size_t));

    itpack += sizeof(size_t);

    std::memcpy((void*)(&*itpack),
                (void*)(arg->data()),
                arg_size);

    itpack += arg_size;
  }
}

void unpack(std::string& pack, std::initializer_list<std::string*> args)
{
  std::string::iterator itpack = pack.begin();

  for (auto& arg : args)
  {
    size_t arg_size;

    std::memcpy((void*)(&arg_size),
                (void*)(&*itpack),
                sizeof(size_t));

    itpack += sizeof(size_t);

    arg->clear();
    arg->resize(arg_size);

    std::memcpy((void*)(&*arg->begin()),
                (void*)(&*itpack),
                arg_size);

    itpack += arg_size;
  }
}

} // namespace spider
} // namespace mermoz

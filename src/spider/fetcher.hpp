#ifndef MERMOZ_FETCHER_H__
#define MERMOZ_FETCHER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>

#include <curl/curl.h>

#include "common/common.hpp"
#include "spider/spider.hpp"

namespace mermoz
{
namespace spider
{

void fetcher(mermoz::common::async_queue<std::string>* url_queue,
             mermoz::common::async_queue<std::string>* content_queue,
             bool* do_fetch);

size_t write_function (char* ptr, size_t size, size_t nmemb, void* userdata);

} // namespace spider
} // namespace mermoz

#endif // MERMOZ_FETCHER_H__

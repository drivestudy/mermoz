#include "spider/fetcher.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace spider
{

void fetcher(mc::async_queue<std::string>* url_queue,
             mc::async_queue<std::string>* content_queue,
             bool* do_fetch)
{
  CURL* curl;
  CURLcode res;

  while (do_fetch)
  {
    curl = curl_easy_init();

    if (curl)
    {
      std::string url(url_queue->pop_out());
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, content_queue);

      res = curl_easy_perform(curl);

      curl_easy_cleanup(curl);
    }
  }
}

size_t write_function (char* ptr, size_t size, size_t nmemb, void* userdata)
{
  mc::async_queue<std::string>* content_queue =
    reinterpret_cast<mc::async_queue<std::string>*>(userdata);

  content_queue->push(ptr);

  return size;
}

} // namespace spider
} // namespace mermoz


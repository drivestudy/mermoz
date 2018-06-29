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
  while (*do_fetch)
  {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
      std::string content;

      std::string url(url_queue->pop_out());
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

      res = curl_easy_perform(curl);

      content_queue->push(content);

      curl_easy_cleanup(curl);
    }
  }
}

size_t write_function (char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string* content =
    reinterpret_cast<std::string*>(userdata);

  content->append(ptr);

  return size*nmemb;
}

} // namespace spider
} // namespace mermoz


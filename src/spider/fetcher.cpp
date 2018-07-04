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
      std::string url(url_queue->pop_out());
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
      
      std::string content;
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

      long http_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      res = curl_easy_perform(curl);

      std::string message;
      mc::pack(message, {&url, &content});

      content_queue->push(message);

      curl_easy_cleanup(curl);
    }
  }
}

size_t write_function (char* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string* content =
    reinterpret_cast<std::string*>(userdata);

  size_t relsize = size*nmemb;

  content->append(ptr, relsize);

  return relsize;
}

} // namespace spider
} // namespace mermoz


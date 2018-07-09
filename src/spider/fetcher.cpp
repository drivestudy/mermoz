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
 */

#include "spider/fetcher.hpp"

namespace mc = mermoz::common;

namespace mermoz
{
namespace spider
{

void fetcher(mc::AsyncQueue<std::string>* url_queue,
             mc::AsyncQueue<std::string>* content_queue,
             std::string user_agent,
             bool* do_fetch)
{
  while (*do_fetch)
  {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
      std::string url;
      url_queue->pop(url);
      std::cout << "fecthing " << url << std::endl;

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);

      curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());

      std::string content;
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

      long http_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      res = curl_easy_perform(curl);

      std::string http_status(http_code == CURLE_OK ? "200" : std::to_string(http_code));
      std::string message;
      mc::pack(message, {&url, &content, &http_status});

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


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

#include "common/httpfetch.hpp"

#ifdef MMZ_PROFILE
#include <gperftools/heap-profiler.h>
#endif

namespace mermoz
{
namespace common
{

long http_fetch(std::string& url,
                std::string& content,
                long time_out,
                const std::string user_agent)
{
# ifdef MMZ_PROFILE
  HeapProfilerDump("fetching");
# endif

  UrlParser up(url);

  if (up.scheme.empty())
    up.scheme = "http";

  url = up.get_url();
  long res = curl_wraper(url, content, time_out, user_agent);

  if (res < 200 || res >= 300)
  {
    std::ostringstream oss;
    oss << "Exchange scheme [HTTP/HTTPS] for " << url;

    up.exchange("https", "http");
    url = up.get_url();

    content.clear();
    res = curl_wraper(url, content, time_out, user_agent);

    print_strong_log(oss.str());
  }

  return res;
}

long curl_wraper(std::string& url,
                 std::string& content,
                 long time_out,
                 const std::string user_agent)
{
  CURL* curl;
  CURLcode res;
  long http_code = -1;

  curl = curl_easy_init();

  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, time_out);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

    res = curl_easy_perform(curl);

    char *ct = NULL;
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

    if (ct)
      if (std::string(ct).find("text") == std::string::npos)
        content.clear();

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  }

  curl_easy_cleanup(curl);

  return http_code;
}

size_t write_function (char* ptr,
                       size_t size,
                       size_t nmemb,
                       void* userdata)
{
  std::string* content =
    reinterpret_cast<std::string*>(userdata);

  size_t relsize = size*nmemb;

  content->append(ptr, relsize);

  return relsize;
}


} // namespace common
} // namespace mermoz

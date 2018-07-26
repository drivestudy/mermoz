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

#include <cstring>
#include <vector>
#include <curl/curl.h>

#include "urlfactory/urlfactory.hpp"
#include "common/logs.hpp"

namespace mermoz
{
namespace common
{

long http_fetch(std::string& url,
                std::string& content,
                long time_out,
                const std::string user_agent)
{
  urlfactory::UrlParser up(url);

  if (!up.has_scheme())
    up.set_scheme("http");

  url = up.get_url();

  std::string eff_url;
  long res = curl_wraper(url, eff_url, content, time_out, user_agent);

  if (!(res >= 200 && res < 300)) {
    std::ostringstream oss;
    oss << url << " (" << res << ")";
    print_warning(oss.str());
  }

  return res;
}

long curl_wraper(std::string& url,
                 std::string& eff_url,
                 std::string& content,
                 long time_out,
                 const std::string user_agent)
{
  // Basic CURL initalizer
  CURL* curl = curl_easy_init();

  // Saving HTTP error codes CODE \in [100; 600[
  // or libCURL error codes CODE \in [0; 100[
  long http_code {-1};

  if (curl) {
    /*
     * Define CURL options
     */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // gives the URL
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str()); // sets user-agent

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirections (HTTP 3xx errors)
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L); // avoid infinite redirs by limiting to 5

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, time_out); // defines timeout

    /*
     * Define function for saving page content
     */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

    /*
     * Let's fetch the URL with the previously
     * defined options
     */
    CURLcode res = curl_easy_perform(curl);

    /*
     * We check if 'curl_easy_perform' went wrong or not
     */
    if (res == CURLE_OK) {
      // Extracts the HTTP RESPONSE CODE
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

      // Extracts CONTENT_TYPE
      char *ct = NULL;
      res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

      if (ct) {
        // ct is not NULL
        if (std::string(ct).find("text") == std::string::npos) {
          // For now on, we only manage 'text' & 'text/html'
          content.clear();
        }
      }

      // Extracts EFFECTIVE_URL, if REDIRS
      char *eff = NULL;
      res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &eff);

      if (eff) {
        // eff_url is not NULL
        eff_url = {eff};
      }
    } else {
      /*
       * 'curl_easy_perform' went wrong
       * res != CURL_OK
       */
      http_code = res;
    }

    /*
     * Mandatory after curl was INIT and not equal to NULL
     */
    curl_easy_cleanup(curl);
  }

  return http_code;
}

size_t write_function (char* ptr,
                       size_t size,
                       size_t nmemb,
                       void* userdata)
{
  std::string* content = reinterpret_cast<std::string*>(userdata);

  size_t relsize = size*nmemb;
  content->append(ptr, relsize);

  return relsize;
}

} // namespace common
} // namespace mermoz

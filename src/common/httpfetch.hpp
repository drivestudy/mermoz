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

#ifndef MERMOZ_HTTPFETCH_H__
#define MERMOZ_HTTPFETCH_H__

#include <string>
#include <cstring>
#include <vector>
#include <curl/curl.h>

#include "urlfactory/urlfactory.hpp"

namespace mermoz
{
namespace common
{

long http_fetch(std::string& url,
                std::string& eff_url,
                std::string& content,
                long time_out,
                const std::string user_agent);

long curl_wraper(std::string& url,
                 std::string& eff_url,
                 std::string& content,
                 long time_out,
                 const std::string user_agent);

size_t write_function (char* ptr,
                       size_t size,
                       size_t nmemb,
                       void* userdata);

} // namespace common
} // namespace mermoz

#endif // MERMOZ_HTTPFETCH_H__

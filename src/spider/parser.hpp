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

#ifndef MERMOZ_PARSER_H__
#define MERMOZ_PARSER_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <atomic>

#include "gumbo.h"

#include "common/common.hpp"

namespace mermoz
{
namespace spider
{

void parser(mermoz::common::AsyncQueue<std::string>* content_queue,
            mermoz::common::AsyncQueue<std::string>* parsed_queue,
            std::atomic<uint64_t>* nparsed,
            bool* status);

std::string get_text(GumboNode* node);

std::string get_links(GumboNode* node);

void text_cleaner(std::string& s);

} // namespace spider
} // namespace mermoz

#endif // MERMOZ_PARSER_H__

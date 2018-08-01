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

#ifndef MERMOZ_LOGS_H__
#define MERMOZ_LOGS_H__

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <ctime>
#include <iomanip>

namespace mermoz
{

void print_log(std::string message);
void print_strong_log(std::string message);
void print_warning(std::string message);
void print_error(std::string message);

} // namespace mermoz

#endif // MERMOZ_LOGS_H__

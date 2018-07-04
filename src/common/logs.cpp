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

#include "common/logs.hpp"

namespace mermoz
{
namespace common
{

void print_log(std::string message)
{
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::cout << "\033[1m [" << std::put_time(&tm, "%T") << "] [MERMOZ] -\033[0m " << message << std::endl;
}

void print_strong_log(std::string message)
{
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::cout << "\033[1;32m [" << std::put_time(&tm, "%T") << "] [MERMOZ] -\033[0m \033[92m" << message << "\033[0m" << std::endl;
}

void print_warning(std::string message)
{
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::cout << "\033[1;33m [" << std::put_time(&tm, "%T") << "] [MERMOZ] -\033[0m \033[93m" << message << "\033[0m" << std::endl;
}

void print_error(std::string message)
{
  std::time_t t = std::time(nullptr);
  std::tm tm = *std::localtime(&t);

  std::cout << "\033[1;31m [" << std::put_time(&tm, "%T") << "] [MERMOZ] -\033[0m \033[91m" << message << "\033[0m" << std::endl;
}

} // namespace common
} // namespace mermoz

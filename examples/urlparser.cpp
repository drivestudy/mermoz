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

#include <iostream>
#include <string>
#include <cstring>

#include "common/urlparser.hpp"

namespace mc = mermoz::common;

int main (int argc, char** argv)
{
  std::string url_left;
  std::string url_right;

  mc::UrlParser up_left;
  mc::UrlParser up_right;

  if (argc == 2)
  {
    up_left.set_url(argv[1]);
    up_left.parse();
  }
  else if (argc == 4 && std::strcmp(argv[2], "lt") == 0)
  {
    up_left.set_url(argv[1]);
    up_left.parse();

    up_right.set_url(argv[3]);
    up_right.parse();

    std::cout << (up_left < up_right ? "true" : "false") << std::endl;

    return 0;
  }
  else if (argc == 4 && std::strcmp(argv[2], "leq") == 0)
  {
    up_left.set_url(argv[1]);
    up_left.parse();

    up_right.set_url(argv[3]);
    up_right.parse();

    std::cout << (up_left <= up_right ? "true" : "false") << std::endl;

    return 0;
  }
  else if (argc == 4 && std::strcmp(argv[2], "gt") == 0)
  {
    up_left.set_url(argv[1]);
    up_left.parse();

    up_right.set_url(argv[3]);
    up_right.parse();

    std::cout << (up_left > up_right ? "true" : "false") << std::endl;

    return 0;
  }
  else if (argc == 4 && std::strcmp(argv[2], "geq") == 0)
  {
    up_left.set_url(argv[1]);
    up_left.parse();

    up_right.set_url(argv[3]);
    up_right.parse();

    std::cout << (up_left >= up_right ? "true" : "false") << std::endl;

    return 0;
  }
  else if (argc == 4 && std::strcmp(argv[2], "+") == 0)
  {
    up_left.set_url(argv[1]);
    up_left.parse();

    up_right.set_url(argv[3]);
    up_right.parse();

    up_left += up_right;
  }
  else
  {
    std::cerr << "Unknown arguments. Exiting..." << std::endl;
    exit(-1);
  }

  std::cout << up_left;

  return 0;
}

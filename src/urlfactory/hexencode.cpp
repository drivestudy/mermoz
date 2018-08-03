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
#include <sstream>
#include <iomanip>

#include "hexencode.hpp"

namespace urlfactory
{
 
void encode (std::string& instring, std::string& outstring)
{
  std::ostringstream oss;
  unsigned int remains {0};

  for (unsigned char c : instring) {
    if (remains > 0) {
      oss << "%" << std::hex << static_cast<int>(c);
      remains--;
    } else if ((c >= 0x30 && c <= 0x39) || // 0 -> 9
               (c >= 0x41 && c <= 0x5a) || // A -> Z
               (c >= 0x61 && c <= 0x7a) || // a -> z
               c == '%' || // do not convert percents !!!
               c == '.') { // neither dots
      /*
       * Allowed without percent encoding
       */
      oss << c;
    } else {
      /*
       * Forbidden without percent encoding
       */
      if (c > 0x7f && c <= 0xdf) {
        remains = 1;
      } else if (c > 0xdf && c <= 0xef) {
        remains = 2;
      } else if (c > 0xef) {
        remains = 3;
      }
      
      oss << "%" << std::hex << static_cast<int>(c);
    }
  }

  outstring = oss.str();
}

} // namespace urlfactory

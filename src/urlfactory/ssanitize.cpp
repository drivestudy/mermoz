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

#include "ssanitize.hpp"

namespace urlfactory
{

/*
 * Removes all the special characters
 * at the begining of the string.
 */
void trim_beg (std::string& s)
{
  size_t pos_max {s.size()};
  size_t pos;

  for (pos = 0;
       ((unsigned char) s[pos]) <= 0x20
       && pos < pos_max;
       pos++) {}

  if (pos != 0) {
    s = s.substr(pos);
  }
}

/*
 * Removes all the special characters
 * at the end of the string.
 */
void trim_end (std::string& s)
{
  size_t pos_max {s.size()};
  size_t pos;

  for (pos = pos_max - 1;
       ((unsigned char) s[pos]) <= 0x20
       && pos > 0;
       pos--) {}

  if (pos != pos_max - 1) {
    s = s.substr(0, pos);
  }
}

/*
 * Removes all the special characters
 * both at the begining and the end of the string.
 */
void trim_beg_end (std::string& s)
{
  trim_beg (s);
  trim_end (s);
}

} // namespace urlfactory

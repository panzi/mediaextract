/* Copyright (c) 2015 Mathias Panzenböck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MEDIAEXTRACT_BINK_H__
#define MEDIAEXTRACT_BINK_H__

#include "mediaextract.h"

#define BINK_MAGIC1 MAGIC("BIKb")
#define BINK_MAGIC2 MAGIC("BIKd")
#define BINK_MAGIC3 MAGIC("BIKf")
#define BINK_MAGIC4 MAGIC("BIKg")
#define BINK_MAGIC5 MAGIC("BIKh")
#define BINK_MAGIC6 MAGIC("BIKi")

#define IS_BINK_MAGIC(magic) \
	((magic) == BINK_MAGIC1 || \
	 (magic) == BINK_MAGIC2 || \
	 (magic) == BINK_MAGIC3 || \
	 (magic) == BINK_MAGIC4 || \
	 (magic) == BINK_MAGIC5 || \
	 (magic) == BINK_MAGIC6)

#define BINK_HEADER_SIZE 44

int bink_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_BINK_H__ */

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

#ifndef MEDIAEXTRACT_ID3_H__
#define MEDIAEXTRACT_ID3_H__
#pragma once

#include "mediaextract.h"

#define ID3_MASK    MAGIC("\xff\xff\xff\x00")
#define ID3v1_MAGIC MAGIC("TAG\0")
#define ID3v2_MAGIC MAGIC("ID3\0")

#define IS_ID3v1_MAGIC(hdr) ((MAGIC(hdr) & ID3_MASK) == ID3v1_MAGIC)
#define IS_ID3v2_MAGIC(hdr) ((MAGIC(hdr) & ID3_MASK) == ID3v2_MAGIC)
#define ID3v1_TAG_SIZE 128
#define ID3v2_HEADER_SIZE 10
#define ID3v2_FOOTER_SIZE 10

int id3v1_istag(const uint8_t *data, size_t input_len, size_t *lengthptr);
int id3v2_istag(const uint8_t *data, size_t input_len, int appended, size_t *lengthptr);

#endif /* MEDIAEXTRACT_ID3_H__ */

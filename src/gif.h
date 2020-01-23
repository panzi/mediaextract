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

#ifndef MEDIAEXTRACT_GIF_H__
#define MEDIAEXTRACT_GIF_H__
#pragma once

#include "mediaextract.h"

#define GIF87a_SIGNATURE "GIF87a"
#define GIF89a_SIGNATURE "GIF89a"
#define GIF_MAGIC MAGIC(GIF87a_SIGNATURE)
#define GIF_SIGNATURE_SIZE 6

#define GIF_HEADER_SIZE      13
#define GIF_IMAGE_BLOCK_SIZE 11
#define GIF_MIN_SIZE         14

#define GIF_IMAGE_BLOCK ','
#define GIF_EXT         '!'
#define GIF_TRAILER     ';'

#define GIF_CT_FLAG     0x80

int gif_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_GIF_H__ */

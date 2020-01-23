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

#ifndef MEDIAEXTRACT_MPG123_H__
#define MEDIAEXTRACT_MPG123_H__
#pragma once

#include "mediaextract.h"

#define IS_MPG123_MAGIC(hdr)  \
	(  ( ((uint8_t)(hdr)[0] & 0xFF) == 0xFF) \
	&& ( ((uint8_t)(hdr)[1] & 0xE0) == 0xE0)   /* 3 sync bits  */ \
	&& ( ((uint8_t)(hdr)[1] & 0x18) != 0x08)   /* Version rsvd */ \
	&& ( ((uint8_t)(hdr)[1] & 0x06) != 0x00)   /* Layer rsvd   */ \
	&& ( ((uint8_t)(hdr)[2] & 0xF0) != 0xF0))  /* Bitrate rsvd */

#define MPG123_HEADER_SIZE 4

struct mpg123_info {
	uint16_t frame_size;
	uint8_t  version;
	uint8_t  layer;
};

int mpg123_isframe(const uint8_t *data, size_t input_len, struct mpg123_info *info);

#endif /* MEDIAEXTRACT_MPG123_H__ */

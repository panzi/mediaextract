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

#ifndef MEDIAEXTRACT_MOD_H__
#define MEDIAEXTRACT_MOD_H__
#pragma once

#include "mediaextract.h"

#define MOD_4CH_MAGIC1 MAGIC("M.K.")
#define MOD_4CH_MAGIC2 MAGIC("M!K!")
#define MOD_4CH_MAGIC3 MAGIC("M&K!")
#define MOD_4CH_MAGIC4 MAGIC("N.T.")
#define MOD_4CH_MAGIC5 MAGIC("FLT4")

#define MOD_8CH_MAGIC1 MAGIC("FLT8")
#define MOD_8CH_MAGIC2 MAGIC("CD81")
#define MOD_8CH_MAGIC3 MAGIC("OKTA")

#define IS_MOD_TDZX_MAGIC(magic) \
	(magic[0] == 'T' && \
	 magic[1] == 'D' && \
	 magic[2] == 'Z' && \
	 magic[3] >  '0' && magic[3] <= '9')

#define IS_MOD_XCHN_MAGIC(magic) \
	(magic[0] >  '0' && magic[0] <= '9' && \
	 magic[1] == 'C' && \
	 magic[2] == 'H' && \
	 magic[3] == 'N')

#define IS_MOD_XXCH_MAGIC(magic) \
	(magic[0] >= '0' && magic[0] <= '9' && \
	 magic[1] >= '0' && magic[1] <= '9' && \
	 magic[2] == 'C' && magic[3] == 'H')

#define IS_MOD_XXCN_MAGIC(magic) \
	(magic[0] >= '0' && magic[0] <= '9' && \
	 magic[1] >= '0' && magic[1] <= '9' && \
	 magic[2] == 'C' && magic[3] == 'N')

#define IS_MOD_MAGIC(magic) \
	(IS_MOD_4CH_MAGIC(magic)  || \
	 IS_MOD_8CH_MAGIC(magic)  || \
	 IS_MOD_XCHN_MAGIC(magic) || \
	 IS_MOD_XXCH_MAGIC(magic) || \
	 IS_MOD_XXCN_MAGIC(magic) || \
	 IS_MOD_TDZX_MAGIC(magic))

#define IS_MOD_4CH_MAGIC(magic) \
	((MAGIC(magic) == MOD_4CH_MAGIC1) || \
	 (MAGIC(magic) == MOD_4CH_MAGIC2) || \
	 (MAGIC(magic) == MOD_4CH_MAGIC3) || \
	 (MAGIC(magic) == MOD_4CH_MAGIC4) || \
	 (MAGIC(magic) == MOD_4CH_MAGIC5))

#define IS_MOD_8CH_MAGIC(magic) \
	((MAGIC(magic) == MOD_8CH_MAGIC1) || \
	 (MAGIC(magic) == MOD_8CH_MAGIC2) || \
	 (MAGIC(magic) == MOD_8CH_MAGIC3))

#define MOD_MAGIC_OFFSET 1080

int mod_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_MOD_H__ */

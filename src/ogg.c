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

#include "ogg.h"

/* see: http://www.xiph.org/ogg/doc/framing.html */

int ogg_ispage(const uint8_t *data, size_t input_len, struct ogg_info *pageinfo)
{
	size_t nsegs, length, i;
	const uint8_t *segs = data + OGG_HEADER_SIZE;

	/* full header available? */
	if (input_len < OGG_HEADER_SIZE)
		return 0;

	/* capture pattern */
	if (MAGIC(data) != OGG_MAGIC)
		return 0;

	/* stream structure version */
	if (data[4] != 0x00)
		return 0;

	/* valid header type flags */
	if ((data[5] & ~7) != 0x00)
		return 0;
	
	nsegs  = data[26];
	length = OGG_HEADER_SIZE + nsegs;

	/* segment sizes fully available? */
	if (input_len < length)
		return 0;

	for (i = 0; i < nsegs; ++ i)
	{
		length += segs[i];
	}

	/* segments fully available? */
	if (input_len < length)
		return 0;
	
	if (pageinfo)
	{
		pageinfo->length = length;
		pageinfo->pageno = le32toh(*(const uint32_t *)(data + 18));
	}

	/* I think we can reasonably assume it is a real page now */
	return 1;
}

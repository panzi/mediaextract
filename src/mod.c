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

#include "mod.h"

int mod_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	size_t length = MOD_MAGIC_OFFSET + 4;
	size_t channels = 0;
	size_t patterns = 0;

	if (input_len < length || !probalby_mod_text(data, 20))
		return 0;

	for (const unsigned char *ptr = data + 20, *sample_end = ptr + 31*30;
		ptr < sample_end; ptr += 30)
	{
		length += (size_t)be16toh(*(const uint16_t*)(ptr + 22)) << 1;
	}

	uint8_t song_length = data[950];
	if (song_length > 0x80)
		return 0;

	for (const unsigned char *ptr = data + 952, *pattern_table_end = ptr + 128;
		ptr < pattern_table_end; ++ ptr)
	{
		if (*ptr > patterns) patterns = *ptr;
	}
	++ patterns;

	const unsigned char *magic = data + MOD_MAGIC_OFFSET;

	if      (IS_MOD_4CH_MAGIC(magic))  channels =  4;
	else if (IS_MOD_8CH_MAGIC(magic))  channels =  8;
	else if (IS_MOD_XCHN_MAGIC(magic))
	{
		channels = magic[0] - '0';
	}
	else if (IS_MOD_XXCH_MAGIC(magic) || IS_MOD_XXCN_MAGIC(magic))
	{
		channels = (magic[0] - '0') * 10 + (magic[1] - '0');
	}
	else if (IS_MOD_TDZX_MAGIC(magic))
	{
		channels = magic[3] - '0';
	}
	else
		return 0;

	length += patterns * channels * 64 * 4;
	if (input_len < length)
		return 0;
	
	if (lengthptr) *lengthptr = length;

	return 1;
}

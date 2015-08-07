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

#include <stdint.h>

#include "aiff.h"
#include "xmidi.h"

int xmidi_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	size_t length = 0;

	if (input_len < XMIDI_MIN_SIZE)
		return 0;

	if (MAGIC(data) == FORM_MAGIC) {
		length = be32toh(*(const uint32_t *)(data + 4));
		if (SIZE_MAX - 8 < length || MAGIC(data + 8) != XDIR_MAGIC)
			return 0;
		length += 8;
	}

	if (input_len < length || input_len - length < XMIDI_MIN_SIZE)
		return 0;

	if (MAGIC(data + length) != CAT_MAGIC || MAGIC(data + length + 8) != XMID_MAGIC)
		return 0;
	
	size_t cat_length = be32toh(*(const uint32_t *)(data + length + 4));

	if (SIZE_MAX - 8 < cat_length)
		return 0;

	cat_length += 8;

	if (SIZE_MAX - length < cat_length)
		return 0;

	length += cat_length;

	if (info)
	{
		info->length = length;
		info->ext    = "xmid";
	}

	return 1;
}

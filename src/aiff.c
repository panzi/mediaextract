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

#include "aiff.h"

int aiff_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	size_t length;
	const char *ext;
	uint32_t format;

	if (input_len < AIFF_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != FORM_MAGIC)
		return 0;

	length = be32toh(*(const uint32_t *)(data + 4)) + 8;

	if (input_len < length)
		return 0;

	format = MAGIC(data + 8);
	if (format == AIFF_MAGIC)
		ext = "aiff";
	else if (format == AIFC_MAGIC)
		ext = "aifc";
	else
		return 0;

	if (info)
	{
		info->length = length;
		info->ext    = ext;
	}

	return 1;
}

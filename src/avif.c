/* Copyright (c) 2024 Mathias Panzenböck
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

#include "avif.h"

#include <string.h>
#include <stdint.h>

#define READ_U32_BE(PTR) (((PTR)[0] << 24) | ((PTR)[1] << 16) | ((PTR)[2] << 8) | (PTR)[3])

enum BoxType {
	META = 1 << 0,
	MOOV = 1 << 1,
	MDAT = 1 << 2,
};

int avif_isfile(const uint8_t *data, size_t input_len, file_format formats, struct file_info *info_ptr)
{
	if (input_len <= AVIF_HEADER_SIZE)
		return 0;

	uint32_t box_size = READ_U32_BE(data);
	const char *ext = NULL;

	if (box_size < AVIF_HEADER_SIZE || memcmp(data + 4, "ftyp", 4) != 0)
		return 0;

	if ((formats & AVIF) && memcmp(data + 8, "avif", 4) == 0)
	{
		ext = "avif";
	}
	else if ((formats & HEIF) && memcmp(data + 8, "heic", 4) == 0)
	{
		ext = "heif";
	}
	else
	{
		return 0;
	}

	int boxes = 0;
	size_t length = (size_t) box_size;

	for (;;)
	{
		const uint8_t *ptr = data + length;
		box_size = READ_U32_BE(ptr);

		if (box_size < 8 || (size_t) box_size > SIZE_MAX - length)
		{
			if (boxes & (MOOV | MDAT))
				break;

			return 0;
		}

		ptr += 4;

		if (memcmp(ptr, "meta", 4) == 0)
		{
			boxes |= META;
		}
		else if (memcmp(ptr, "mdat", 4) == 0)
		{
			boxes |= MDAT;
		}
		else if (memcmp(ptr, "moov", 4) == 0)
		{
			boxes |= MOOV;
		}
		else if (boxes & (MOOV | MDAT))
		{
			break;
		}
		else{
			return 0;
		}

		length += (size_t) box_size;
	}

	if (info_ptr)
	{
		info_ptr->length = length;
		info_ptr->ext    = ext;
	}

	return 1;
}

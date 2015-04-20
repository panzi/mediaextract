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

#include "gif.h"

// See: http://www.onicos.com/staff/iz/formats/gif.html

#pragma pack(push, 1)
struct gif_header {
	uint8_t  signature[GIF_SIGNATURE_SIZE];
	uint16_t screenwidth;
	uint16_t screenheight;
	uint8_t  flags;
	uint8_t  bgcolorindex;
	uint8_t  pxaspect;
};

struct gif_image_block {
	uint8_t  mark;
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	uint8_t  flags;
};
#pragma pack(pop)

int gif_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < GIF_MIN_SIZE || (
		memcmp(data, GIF87a_SIGNATURE, GIF_SIGNATURE_SIZE) != 0 &&
		memcmp(data, GIF89a_SIGNATURE, GIF_SIGNATURE_SIZE) != 0))
		return 0;
	
	const struct gif_header *header = (const struct gif_header *)data;
	size_t length = GIF_HEADER_SIZE;

	if (header->flags & GIF_CT_FLAG)
		length += (2 << (header->flags & 7)) * 3;

	for (;;)
	{
		if (length > input_len - 1)
			return 0;

		unsigned int marker = data[length];

		if (marker == GIF_IMAGE_BLOCK)
		{
			if (length > input_len - GIF_IMAGE_BLOCK_SIZE)
				return 0;

			const struct gif_image_block *image = (struct gif_image_block *)(data + length);
			length += GIF_IMAGE_BLOCK_SIZE;

			if (image->flags & GIF_CT_FLAG)
			{
				size_t tblsize = (2 << (image->flags & 15)) * 3;

				if (SIZE_MAX - tblsize < length)
					return 0;

				length += tblsize;

				if (length > input_len)
					return 0;
			}
		}
		else if (marker == GIF_EXT)
		{
			if (length > input_len - 2)
				return 0;
			length += 2;
		}
		else if (marker == GIF_TRAILER)
		{
			++ length;
			break;
		}
		else
		{
			return 0;
		}

		// parse data blocks until terminator (block of size 0)
		for (;;)
		{
			size_t size = data[length];

			if (size == 0)
			{
				if (length == SIZE_MAX)
					return 0;

				++ length;
				break;
			}
			else if (SIZE_MAX - (size + 1) < length)
				return 0;
			

			length += size + 1;

			if (length > input_len)
				return 0;
		}
	}

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

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

#include "smk.h"

struct smk_header {
	uint32_t signature;
	uint32_t width;
	uint32_t height;
	uint32_t frames;
	uint32_t framerate;
	uint32_t flags;
	uint32_t audiosize[7];
	uint32_t treessize;
	uint32_t mmapsize;
	uint32_t mclrsize;
	uint32_t fullsize;
	uint32_t typesize;
	uint32_t audiorate[7];
	uint32_t dummy;
};

int smk_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	const struct smk_header *header = (const struct smk_header *)data;

	if (input_len < SMK_HEADER_SIZE || !IS_SMK_MAGIC(header->signature))
		return 0;

	size_t   frames    = le32toh(header->frames);
	uint32_t flags     = le32toh(header->flags);
	size_t   treessize = le32toh(header->treessize);
	size_t   length    = SMK_HEADER_SIZE + treessize + 5 * frames;

	if (flags > 7 || input_len < length)
		return 0;

	const uint32_t *end = (const uint32_t*)(data + SMK_HEADER_SIZE) + frames;
	for (const uint32_t *ptr = (const uint32_t*)(data + SMK_HEADER_SIZE);
		ptr < end; ++ ptr)
	{
		size_t framesize = *ptr;

		if (SIZE_MAX - framesize < length)
			return 0;

		length += framesize;
	}

	// assume now it is a smk file and if the length doesn't fit that it is truncated
	if (length > input_len)
		length = input_len;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

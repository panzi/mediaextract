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

#include "bink.h"

int bink_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < BINK_HEADER_SIZE)
		return 0;

	uint32_t magic = MAGIC(data);
	if (!IS_BINK_MAGIC(magic))
		return 0;
	
	size_t length      = le32toh(*(uint32_t *)(data +  4)) + 8;
	size_t frame_count = le32toh(*(uint32_t *)(data +  8));
	size_t width       = le32toh(*(uint32_t *)(data + 20));
	size_t height      = le32toh(*(uint32_t *)(data + 24));
	size_t fps_num     = le32toh(*(uint32_t *)(data + 28));
	size_t fps_den     = le32toh(*(uint32_t *)(data + 32));

	if (input_len < length ||
		frame_count == 0 ||
		width  > 7680 ||
		height > 4800 ||
		fps_num == 0  ||
		fps_den == 0)
	{
		return 0;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

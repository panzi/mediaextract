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

#include "id3.h"

int id3v1_istag(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (ID3v1_TAG_SIZE < input_len)
		return 0;
	
	if (!IS_ID3v1_MAGIC(data))
		return 0;
	
	if (lengthptr) *lengthptr = ID3v1_TAG_SIZE;

	return 1;
}

int id3v2_istag(const uint8_t *data, size_t input_len, int appended, size_t *lengthptr)
{
	if (input_len < ID3v2_HEADER_SIZE)
		return 0;
	
	if (!IS_ID3v2_MAGIC(data))
		return 0;
	
	uint8_t major = data[3];
	uint8_t minor = data[4];
	uint8_t flags = data[5];
	size_t  size  = ((data[6] & 0x7f) << 21)
	               | ((data[7] & 0x7f) << 14)
	               | ((data[8] & 0x7f) <<  7)
	               |  (data[9] & 0x7f);
	if (major == 0xff || major < 2 || minor == 0xff || *(uint32_t *)(data + 6) & 0x80808080)
		return 0;

	size_t length = size + ID3v2_HEADER_SIZE;

	/* has footer? */
	if (flags & 0x10)
		length += ID3v2_FOOTER_SIZE;
	else if (major >= 4 && appended)
		return 0;
	
	if (input_len < length)
		return 0;

	if (lengthptr) *lengthptr = length;

	return 1;
}

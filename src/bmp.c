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

#include "bmp.h"

// see: http://en.wikipedia.org/wiki/BMP_file_format

#pragma pack(push, 1)
struct bmp_header {
	uint16_t magic;
	uint32_t filesize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t dataoffset;
	
	// DIB header
	uint32_t headersize;
	int32_t  width;
	int32_t  height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t datasize;
	uint32_t hres;
	uint32_t vres;
	uint32_t palettecolors;
	uint32_t importantcolors;
};
#pragma pack(pop)

int bmp_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < (BMP_HEADER_SIZE + DIB_HEADER_SIZE) || !IS_BMP_MAGIC(data))
		return 0;

	const struct bmp_header *header = (const struct bmp_header *)data;

	size_t   filesize      = le32toh(header->filesize);
	uint16_t reserved1     = le16toh(header->reserved1);
	uint16_t reserved2     = le16toh(header->reserved2);
	size_t   dataoffset    = le32toh(header->dataoffset);
	size_t   headersize    = le32toh(header->headersize);
	int32_t  width         = le32toh(header->width);
	int32_t  height        = le32toh(header->height);
	uint16_t planes        = le16toh(header->planes);
	uint16_t bpp           = le16toh(header->bpp);
	uint32_t compression   = le32toh(header->compression);
	size_t   datasize      = le32toh(header->datasize);
	size_t   palettecolors = le32toh(header->palettecolors);
	size_t   colortblsize  = 4 * palettecolors;

	// integer overflow?
	if (SIZE_MAX / 4 < palettecolors)
		return 0;

	// sanity of all sizes and offsets
	if (filesize < (BMP_HEADER_SIZE + DIB_HEADER_SIZE) || dataoffset >= filesize || datasize >= filesize ||
		headersize >= filesize || headersize < DIB_HEADER_SIZE || colortblsize >= filesize ||
		dataoffset < (BMP_HEADER_SIZE + headersize + colortblsize) ||
		filesize - datasize < dataoffset)
		return 0;

	// legal range of values?
	if (width <= 0 || height == 0 || planes != 1 || bpp == 0 ||
		reserved1 != 0 || reserved2 != 0 || compression > 6)
		return 0;

	if (filesize > input_len)
		return 0;

	if (lengthptr)
		*lengthptr = filesize;
	
	return 1;
}

#include "png.h"

// See: http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

#define IS_PNG_MAGIC_CHAR(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_PNG_CHUNK_MAGIC(m) \
	(IS_PNG_MAGIC_CHAR((m)[0]) && \
	 IS_PNG_MAGIC_CHAR((m)[1]) && \
	 IS_PNG_MAGIC_CHAR((m)[2]) && \
	 IS_PNG_MAGIC_CHAR((m)[3]))

#pragma pack(push, 1)
struct png_chunk_header {
	uint32_t size;
	uint32_t magic;
};

struct png_ihdr_chunk {
	uint32_t size;
	uint32_t magic;
	uint32_t width;
	uint32_t height;
	uint8_t  bitdepth;
	uint8_t  colortype;
	uint8_t  compression;
	uint8_t  filter;
	uint8_t  interlace;
	uint32_t crc;
};

struct png_iend_header {
	uint32_t size;
	uint32_t magic;
	uint32_t crc;
};
#pragma pack(pop)

int png_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < PNG_MIN_SIZE || memcmp(data, PNG_SIGNATURE, PNG_SIGNATURE_SIZE) != 0)
		return 0;
	
	const struct png_ihdr_chunk *ihdr = (const struct png_ihdr_chunk *)(data + PNG_SIGNATURE_SIZE);

	uint32_t size   = be32toh(ihdr->size);
	uint32_t width  = be32toh(ihdr->width);
	uint32_t height = be32toh(ihdr->height);
	uint8_t  bitdepth    = ihdr->bitdepth;
	uint8_t  colortype   = ihdr->colortype;
	uint8_t  compression = ihdr->compression;
	uint8_t  filter      = ihdr->filter;
	uint8_t  interlace   = ihdr->interlace;

	if (ihdr->magic != PNG_IHDR_MAGIC || size != 13)
		return 0;
	
	if (width > INT32_MAX || height > INT32_MAX)
		return 0;
	
	if (bitdepth != 1 && bitdepth != 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
		return 0;
	
	if (colortype != 0 && colortype != 2 && colortype != 3 && colortype != 4 && colortype != 6)
		return 0;

	if (compression != 0 || filter != 0)
		return 0;
	
	if (interlace != 0 && interlace != 1)
		return 0;
	
	size_t length = PNG_HEADER_SIZE;

	for (;;) {
		const struct png_chunk_header *chunk = (const struct png_chunk_header *)(data + length);

		if (input_len - 12 < length)
			return 0;

		size_t chunksize = be32toh(chunk->size);
		union {
			uint32_t number;
			uint8_t  chars[4];
		} magic = { .number = chunk->magic };

		if (chunksize > INT32_MAX || SIZE_MAX - (chunksize + 12) < length)
			return 0;

		if (!IS_PNG_CHUNK_MAGIC(magic.chars))
			return 0;

		length += chunksize + 12;

		if (chunk->magic == PNG_IEND_MAGIC)
		{
			uint32_t crc = be32toh(((const struct png_iend_header *)chunk)->crc);
			if (chunksize != 0 || crc != 0xAE426082)
				return 0;

			break;
		}
	}

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

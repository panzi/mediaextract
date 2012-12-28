#include "id3.h"

int id3v1_istag(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	if ((intptr_t)end <= ID3v1_TAG_SIZE || end - ID3v1_TAG_SIZE < start)
		return 0;
	
	if (!IS_ID3v1_MAGIC(start))
		return 0;
	
	if (lengthptr) *lengthptr = ID3v1_TAG_SIZE;

	return 1;
}

int id3v2_istag(const unsigned char *start, const unsigned char *end, int appended, size_t *lengthptr)
{
	size_t input_len = (size_t)(end - start);
	if (input_len < ID3v2_HEADER_SIZE)
		return 0;
	
	if (!IS_ID3v2_MAGIC(start))
		return 0;
	
	uint8_t  major = start[3];
	uint8_t  minor = start[4];
	uint8_t  flags = start[5];
	uint32_t size  = ((start[6] & 0x7f) << 21)
	               | ((start[7] & 0x7f) << 14)
	               | ((start[8] & 0x7f) <<  7)
	               |  (start[9] & 0x7f);
	if (major == 0xff || major < 2 || minor == 0xff || *(uint32_t *)(start + 6) & 0x80808080)
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

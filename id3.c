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
	
	uint8_t  major = data[3];
	uint8_t  minor = data[4];
	uint8_t  flags = data[5];
	uint32_t size  = ((data[6] & 0x7f) << 21)
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

#include "wave.h"

int wave_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t input_len = (size_t)(end - start);
	size_t length;

	if (input_len < WAVE_HEADER_SIZE)
		return 0;

	if (MAGIC(start) != RIFF_MAGIC)
		return 0;

	length = le32toh(*(const uint32_t *)(start + 4)) + 8;

	if (input_len < length)
		return 0;

	if (MAGIC(start + 8) != WAVE_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

int aiff_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t input_len = (size_t)(end - start);
	size_t length;
	int16_t format;

	if (input_len < WAVE_HEADER_SIZE)
		return 0;

	if (MAGIC(start) != FORM_MAGIC)
		return 0;

	length = be32toh(*(const uint32_t *)(start + 4)) + 8;

	if (input_len < length)
		return 0;

	format = MAGIC(start + 8);
	if (format != AIFF_MAGIC && format != AIFC_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

#include "wave.h"

int wave_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t length;

	if (end <= (unsigned char *)WAVE_HEADER_SIZE || end - WAVE_HEADER_SIZE < start)
		return 0;

	if (*(const int32_t *)start != RIFF_MAGIC)
		return 0;

	length = le32toh(*(const uint32_t *)(start + 4)) + 8;

	if (end <= (unsigned char *)length || end - length < start)
		return 0;

	if (*(const uint32_t *)(start + 8) != WAVE_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

int aiff_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t length;
	int16_t format;

	if (end <= (unsigned char *)WAVE_HEADER_SIZE || end - WAVE_HEADER_SIZE < start)
		return 0;

	if (*(const int32_t *)start != FORM_MAGIC)
		return 0;

	length = be32toh(*(const uint32_t *)(start + 4)) + 8;

	if (end <= (unsigned char *)length || end - length < start)
		return 0;

	format = *(const uint32_t *)(start + 8);
	if (format != AIFF_MAGIC && format != AIFC_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

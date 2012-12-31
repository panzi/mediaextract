#include "wave.h"

int wave_ischunk(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	size_t length;

	if (input_len < WAVE_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != RIFF_MAGIC)
		return 0;

	length = le32toh(*(const uint32_t *)(data + 4)) + 8;

	if (input_len < length)
		return 0;

	if (MAGIC(data + 8) != WAVE_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

int aiff_ischunk(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	size_t length;
	uint32_t format;

	if (input_len < WAVE_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != FORM_MAGIC)
		return 0;

	length = be32toh(*(const uint32_t *)(data + 4)) + 8;

	if (input_len < length)
		return 0;

	format = MAGIC(data + 8);
	if (format != AIFF_MAGIC && format != AIFC_MAGIC)
		return 0;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

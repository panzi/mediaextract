#include "aiff.h"

int aiff_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	size_t length;
	const char *ext;
	uint32_t format;

	if (input_len < AIFF_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != FORM_MAGIC)
		return 0;

	length = be32toh(*(const uint32_t *)(data + 4)) + 8;

	if (input_len < length)
		return 0;

	format = MAGIC(data + 8);
	if (format == AIFF_MAGIC)
		ext = "aiff";
	else if (format == AIFC_MAGIC)
		ext = "aifc";
	else
		return 0;

	if (info)
	{
		info->length = length;
		info->ext    = ext;
	}

	return 1;
}

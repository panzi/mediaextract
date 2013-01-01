#include "s3m.h"

int s3m_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < S3M_HEADER_SIZE || !probalby_mod_text(data, 28))
		return 0;

	uint8_t mark = data[28];
	uint8_t type = data[29];

	if (mark != 0x1A || type != 16)
		return 0;

	if (MAGIC(data + S3M_MAGIC_OFFSET) != S3M_MAGIC)
		return 0;
	
	uint16_t orders   = le16toh(*(uint16_t *)(data + 32));
	uint16_t samples  = le16toh(*(uint16_t *)(data + 34));
	uint16_t patterns = le16toh(*(uint16_t *)(data + 36));
	size_t   length   = S3M_HEADER_SIZE + orders + (samples << 1) + (patterns << 1);

	if (input_len < length)
		return 0;

#define UPDATE_LENGTH(len) \
	{ \
		size_t _len = (len); \
		if (_len > length) \
		{ \
			length = _len; \
			if (input_len < length) \
				return 0; \
		} \
	}

	/* scan samples */
	for (const uint16_t *para = (const uint16_t *)(data + S3M_HEADER_SIZE + orders),
		*para_end = para + samples;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le16toh(*para) << 4;
		const unsigned char *ptr = data + off;

		UPDATE_LENGTH(off + S3M_SAMPLE_HEADER_SIZE);

		uint8_t type = *ptr;
		if (MAGIC(ptr + 76) != S3M_SAMPLE_MAGIC)
			continue; // skip unknown samples

		if (type == 1)
		{
			size_t data_length = le32toh(*(const uint32_t*)(ptr + 16));
			size_t data_off = (ptr[14] | (ptr[15] << 8) | (ptr[13] << 16)) << 4;
			size_t data_end = data_off + data_length;

			// there are some S3M files out there with truncated samples:
			if (data_end > input_len) data_end = input_len;
			if (data_end > length)    length   = data_end;
		}
	}

	/* scan patterns */
	for (const uint16_t *para = (const uint16_t *)(data + S3M_HEADER_SIZE + orders + (samples << 1)),
		*para_end = para + patterns;
		para < para_end; ++ para)
	{
		size_t off = le16toh(*para) << 4;
		const unsigned char *ptr = data + off;

		UPDATE_LENGTH(off + 2);

		size_t pattern_length = (size_t)le16toh(*(const uint16_t*)ptr) + 2;
		size_t pattern_end    = off + pattern_length;

		// there are some S3M files out there with truncated patterns:
		if (pattern_end > input_len) pattern_end = input_len;
		if (pattern_end > length)    length      = pattern_end;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

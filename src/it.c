#include "it.h"

int it_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < IT_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != IT_MAGIC || !probalby_mod_text(data + 4, 26))
		return 0;

	uint16_t orders      = le16toh(*(uint16_t *)(data + 0x20));
	uint16_t instruments = le16toh(*(uint16_t *)(data + 0x22));
	uint16_t samples     = le16toh(*(uint16_t *)(data + 0x24));
	uint16_t patterns    = le16toh(*(uint16_t *)(data + 0x26));
	uint16_t version     = le16toh(*(uint16_t *)(data + 0x28));
	uint16_t compat_vers = le16toh(*(uint16_t *)(data + 0x2A));
	uint16_t special     = le16toh(*(uint16_t *)(data + 0x2E));

	size_t length = IT_HEADER_SIZE + orders + instruments * 4 + samples * 4 + patterns * 4;

	if (input_len < length)
		return 0;

	if (version == 0x0888 && compat_vers == 0x0888)
	{
		// this seems to be ok
	}
	else if (((version < 0x0200 || version >= 0x0300) && (version < 0x1020 || version > 0x1050)) ||
		compat_vers < 0x0100 || compat_vers > 0x0217)
	{
		return 0;
	}

	if (special & 1)
	{
		size_t message_length = le16toh(*(uint16_t *)(data + 0x36));
		size_t message_start  = le32toh(*(uint32_t *)(data + 0x38));

		if ((size_t)(-1) - message_length < message_start)
			return 0;

		size_t message_end = message_start + message_length;
		if (message_start < input_len && message_end > length)
		{
			/* truncated messages are ok */
			length = message_end < input_len ? message_end : input_len;
		}
	}

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

	/* scan instruments */
	for (const uint32_t *para = (const uint32_t *)(data + IT_HEADER_SIZE + orders),
		*para_end = para + instruments;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le32toh(*para);

		UPDATE_LENGTH(off + IT_INSTRUMENT_SIZE);
	}

	/* scan samples */
	for (const uint32_t *para = (const uint32_t *)(data + IT_HEADER_SIZE + orders + instruments * 4),
		*para_end = para + samples;
		para < para_end; ++ para)
	{
		size_t off = (size_t)le32toh(*para);
		const unsigned char *ptr = data + off;

		UPDATE_LENGTH(off + IT_SAMPLE_HEADER_SIZE);
		
		if (MAGIC(ptr) != IT_SAMPLE_MAGIC)
			continue;

		size_t sample_length  = le32toh(*(uint32_t *)(ptr + 0x30));
		size_t sample_pointer = le32toh(*(uint32_t *)(ptr + 0x48));

		if (sample_length && sample_pointer)
		{
			size_t sample_end = sample_pointer + sample_length;

			// there are some IT files out there with truncated samples:
			if (sample_end > input_len) sample_end = input_len;
			if (sample_end > length)    length     = sample_end;
		}
	}
	
	/* scan patterns */
	for (const uint32_t *para = (const uint32_t *)(data + IT_HEADER_SIZE + orders + instruments * 4 + samples * 4),
		*para_end = para + patterns;
		para < para_end; ++ para)
	{
		size_t off = le32toh(*para);
		const unsigned char *ptr = data + off;

		UPDATE_LENGTH(off + IT_PATTERN_HEADER_SIZE);

		size_t pattern_length = le32toh(*(uint16_t *)ptr);
		size_t pattern_end    = off + IT_PATTERN_HEADER_SIZE + pattern_length;

		// there are some IT files out there with truncated patterns:
		if (pattern_end > input_len) pattern_end = input_len;
		if (pattern_end > length)    length      = pattern_end;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

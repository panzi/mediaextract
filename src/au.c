#include "au.h"

struct au_header {
	uint32_t magic;
	uint32_t data_offset;
	uint32_t data_size;
	uint32_t encoding;
	uint32_t sample_rate;
	uint32_t channels;
};

int au_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < AU_HEADER_SIZE || MAGIC(data) != AU_MAGIC)
		return 0;

	const struct au_header *header = (const struct au_header *)data;

	size_t   data_offset = be32toh(header->data_offset);
	size_t   data_size   = be32toh(header->data_size);
	uint32_t encoding    = be32toh(header->encoding);
	uint32_t channels    = be32toh(header->channels);

	if (data_offset % 8 != 0 ||
		encoding <  1  ||
		encoding > 27  ||
		channels  == 0 ||
		data_size == 0 ||
		data_size == 0xffffffff)
		return 0;

	if (SIZE_MAX - data_offset < data_size)
		return 0;

	size_t length = data_offset + data_size;

	// I'm pretty sure it's a truncated AU file when this happens
	if (length > input_len)
		length = input_len;

	if (lengthptr)
		*lengthptr = length;

	return 1;
}

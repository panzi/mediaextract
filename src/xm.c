#include "xm.h"

#pragma pack(push, 1)
struct xm_header {
	char     id[XM_ID_SIZE];
	char     name[20];
	uint8_t  x1A;
	char     tracker_name[20];
	uint16_t version;
	uint32_t header_size;
	uint16_t song_length;
	uint16_t restart_position;
	uint16_t channels;
	uint16_t patterns;
	uint16_t intruments;
	uint16_t flags;
	uint16_t tempo;
	uint16_t bpm;
	uint8_t  pattern_order_table[];
};

struct xm_pattern_header {
	uint32_t header_size;
	uint8_t  packing_type;
	uint16_t rows;
	uint16_t data_size;
};

struct xm_instrument_header {
	uint32_t header_size;
	char     name[22];
	uint8_t  type;
	uint16_t samples;
	uint32_t sample_header_size;
	/* ... */
};

struct xm_sample_header {
	uint32_t size;
	uint32_t loop_start;
	uint32_t loop_end;
	/* ... */
};
#pragma pack(pop)

int xm_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < XM_HEADER_SIZE)
		return 0;
	
	const struct xm_header *header = (const struct xm_header *)data;

	// x1A should be 0x1A but can be 0x00 in stripped XM files
	// I don't really support stripped XM files because many other things are 0 in them too (e.g. the file magic),
	// but sometimes this value is 0x00 even if the file is not stripped. And once I saw als 0x20 here in an otherwise
	// working file.
	if (memcmp(header->id, XM_ID, XM_ID_SIZE) != 0 || (header->x1A != 0x1A && header->x1A != 0x00 && header->x1A != 0x20))
		return 0;

	uint16_t version     = le16toh(header->version);
	uint32_t header_size = le32toh(header->header_size);
	uint16_t patterns    = le16toh(header->patterns);
	uint16_t intruments  = le16toh(header->intruments);

	size_t length = 60 + header_size;

	if (length < XM_HEADER_SIZE || input_len < length || version < 0x0104)
		return 0;

	// At this point I can be fairly sure that this is a XM file,
	// so it now input_len is not big enough the file is really truncated.
	
	// scan patterns
	for (size_t i = 0; i < patterns; ++ i)
	{
		const struct xm_pattern_header *pattern_header =
			(const struct xm_pattern_header *)(data + length);

		if (length > input_len - 9)
		{
			if (lengthptr) *lengthptr = input_len;
			return 1;
		}

		uint32_t pattern_header_size = le32toh(pattern_header->header_size);
		uint16_t pattern_data_size   = le16toh(pattern_header->data_size);

		if ((size_t)(-1) - pattern_header_size < length)
			return 0;

		length += pattern_header_size;
		
		if ((size_t)(-1) - pattern_data_size < length)
			return 0;

		length += pattern_data_size;

		if (length >= input_len)
		{
			if (lengthptr) *lengthptr = input_len;
			return 1;
		}
	}
	
	// scan instruments
	for (size_t i = 0; i < intruments; ++ i)
	{
		const struct xm_instrument_header *instrument_header =
			(const struct xm_instrument_header *)(data + length);

		if (length > input_len - 4)
		{
			if (lengthptr) *lengthptr = input_len;
			return 1;
		}

		uint32_t instrument_header_size = le32toh(instrument_header->header_size);
		uint16_t instrument_samples     = instrument_header_size >= 29 ?
			le16toh(instrument_header->samples) : 0;
		
		if ((size_t)(-1) - instrument_header_size < length)
		{
			return 0;
		}

		length += instrument_header_size;

		if (length >= input_len)
		{
			if (lengthptr) *lengthptr = input_len;
			return 1;
		}

		// scan samples
		if (instrument_samples > 0 && instrument_header_size >= 33)
		{
			size_t data_size = 0;
			uint32_t sample_header_size = le32toh(instrument_header->sample_header_size);

			if (sample_header_size == 0)
			{
				instrument_samples = 0;
			}
			else if (sample_header_size < 4 || sample_header_size > input_len)
			{
				return 0;
			}

			for (size_t j = 0; j < instrument_samples; ++ j)
			{
				const struct xm_sample_header *sample_header =
					(const struct xm_sample_header *)(data + length);

				if (length >= input_len - sample_header_size)
				{
					if (lengthptr) *lengthptr = input_len;
					return 1;
				}

				length += sample_header_size;

				uint32_t sample_size = le32toh(sample_header->size);

				if ((size_t)(-1) - sample_size < data_size)
					return 0;

				data_size += sample_size;
			}

			if ((size_t)(-1) - data_size < length)
				return 0;

			length += data_size;

			if (length > input_len)
			{
				if (lengthptr) *lengthptr = input_len;
				return 1;
			}
		}
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

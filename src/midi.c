#include "midi.h"

int midi_isheader(const uint8_t *data, size_t input_len, size_t *lengthptr, size_t *tracksptr)
{
	if (input_len < MIDI_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != MIDI_MAGIC)
		return 0;

	uint32_t chunk_size  = be32toh(*(const uint32_t *)(data +  4));
	uint16_t format_type = be16toh(*(const uint16_t *)(data +  8));
	uint16_t tracks      = be16toh(*(const uint16_t *)(data + 10));

	if (chunk_size != 6
	 || (format_type != 0 && format_type != 1 && format_type != 2)
	 || tracks == 0
	 || (format_type == 0 && tracks > 1))
		return 0;
	
	if (lengthptr) *lengthptr = MIDI_HEADER_SIZE;
	if (tracksptr) *tracksptr = tracks;

	return 1;
}

int midi_istrack(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len <= MIDI_TRACK_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != MIDI_TRACK_MAGIC)
		return 0;

	size_t chunk_size = be32toh(*(const uint32_t *)(data + 4));
	size_t length = MIDI_TRACK_HEADER_SIZE + chunk_size;

	if (input_len < length)
		return 0;

	if (lengthptr) *lengthptr = length;

	return 1;
}

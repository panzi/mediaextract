#include "midi.h"

int midi_isheader(const unsigned char *start, const unsigned char *end, size_t *lengthptr, size_t *tracksptr)
{
	if ((intptr_t)end <= MIDI_HEADER_SIZE || end - MIDI_HEADER_SIZE < start)
		return 0;

	if (*(const int32_t *)start != MIDI_MAGIC)
		return 0;

	uint32_t chunk_size  = be32toh(*(const uint32_t *)(start +  4));
	uint16_t format_type = be16toh(*(const uint16_t *)(start +  8));
	uint16_t tracks      = be16toh(*(const uint16_t *)(start + 10));

	if (chunk_size != 6
	 || (format_type != 0 && format_type != 1 && format_type != 2)
	 || tracks == 0
	 || (format_type == 0 && tracks > 1))
		return 0;
	
	if (lengthptr) *lengthptr = MIDI_HEADER_SIZE;
	if (tracksptr) *tracksptr = tracks;

	return 1;
}

int midi_istrack(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	if ((intptr_t)end <= MIDI_TRACK_HEADER_SIZE || end - MIDI_TRACK_HEADER_SIZE < start)
		return 0;

	if (*(const int32_t *)start != MIDI_TRACK_MAGIC)
		return 0;

	uint32_t chunk_size = be32toh(*(const uint32_t *)(start + 4));
	size_t length = MIDI_TRACK_HEADER_SIZE + chunk_size;

	if ((intptr_t)end <= length || end - length < start)
		return 0;

	if (lengthptr) *lengthptr = length;

	return 1;
}

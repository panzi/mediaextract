#ifndef AUDIOEXTRACT_MIDI_H__
#define AUDIOEXTRACT_MIDI_H__

#include "audioextract.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#	define MIDI_MAGIC       0x6468544D /* "dhTM" */
#	define MIDI_TRACK_MAGIC 0x6B72544D /* "krTM" */

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define MIDI_MAGIC       0x4D546864 /* "MThd" */
#	define MIDI_TRACK_MAGIC 0x4D54726B /* "MTrk" */

#else

#	error unsupported endian

#endif

#define MIDI_HEADER_SIZE 14
#define MIDI_TRACK_HEADER_SIZE 8

int midi_isheader(const unsigned char *start, const unsigned char *end, size_t *lengthptr, size_t *tracksptr);
int midi_istrack(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_MIDI_H__ */

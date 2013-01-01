#ifndef AUDIOEXTRACT_MIDI_H__
#define AUDIOEXTRACT_MIDI_H__

#include "audioextract.h"

#define MIDI_MAGIC       MAGIC("MThd")
#define MIDI_TRACK_MAGIC MAGIC("MTrk")

#define MIDI_HEADER_SIZE 14
#define MIDI_TRACK_HEADER_SIZE 8

int midi_isheader(const uint8_t *data, size_t input_len, size_t *lengthptr, size_t *tracksptr);
int midi_istrack(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_MIDI_H__ */

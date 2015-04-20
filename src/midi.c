/* Copyright (c) 2015 Mathias Panzenböck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

#include "mpeg.h"

/* MPEG frame length calculation code from:
 * http://www.hydrogenaudio.org/forums/index.php?showtopic=85125
 */

// MPEG versions - use [version]
const uint8_t mpeg_versions[4] = { 25, 0, 2, 1 };

// Layers - use [layer]
const uint8_t mpeg_layers[4] = { 0, 3, 2, 1 };

// Bitrates - use [version][layer][bitrate]
const uint16_t mpeg_bitrates[4][4][16] = {
	{ // Version 2.5
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
		{ 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
		{ 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
		{ 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  // Layer 1
	},
	{ // Reserved
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Invalid
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }  // Invalid
	},
	{ // Version 2
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
		{ 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 3
		{ 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }, // Layer 2
		{ 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  // Layer 1
	},
	{ // Version 1
		{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, // Reserved
		{ 0,  32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 }, // Layer 3
		{ 0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 }, // Layer 2
		{ 0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }, // Layer 1
	}
};

// Sample rates - use [version][srate]
const uint16_t mpeg_srates[4][4] = {
	{ 11025, 12000,  8000, 0 }, // MPEG 2.5
	{     0,     0,     0, 0 }, // Reserved
	{ 22050, 24000, 16000, 0 }, // MPEG 2
	{ 44100, 48000, 32000, 0 }  // MPEG 1
};

// Samples per frame - use [version][layer]
const uint16_t mpeg_frame_samples[4][4] = {
//    Rsvd     3     2     1  < Layer  v Version
	{    0,  576, 1152,  384 }, //       2.5
	{    0,    0,    0,    0 }, //       Reserved
	{    0,  576, 1152,  384 }, //       2
	{    0, 1152, 1152,  384 }  //       1
};

// Slot size (MPEG unit of measurement) - use [layer]
const uint8_t mpeg_slot_size[4] = { 0, 1, 1, 4 }; // Rsvd, 3, 2, 1

int mpeg_isframe(const unsigned char *start, const unsigned char *end, struct mpeg_info *info)
{
	size_t input_len = (size_t)(end - start);
	if (input_len < MPEG_HEADER_SIZE)
		return 0;
	
	// Quick validity check
	if (!IS_MPEG_MAGIC(start))
		return 0;
	
	// Data to be extracted from the header
	uint8_t   ver = (start[1] & 0x18) >> 3;   // Version index
	uint8_t   lyr = (start[1] & 0x06) >> 1;   // Layer index
	uint8_t   pad = (start[2] & 0x02) >> 1;   // Padding? 0/1
	uint8_t   brx = (start[2] & 0xf0) >> 4;   // Bitrate index
	uint8_t   srx = (start[2] & 0x0c) >> 2;   // SampRate index
	
	// Lookup real values of these fields
	uint32_t  bitrate   = mpeg_bitrates[ver][lyr][brx] * 1000;
	uint32_t  samprate  = mpeg_srates[ver][srx];
	uint16_t  samples   = mpeg_frame_samples[ver][lyr];
	uint8_t   slot_size = mpeg_slot_size[lyr];

	if (bitrate == 0 || samprate == 0 || samples == 0)
		return 0;
	
	// In-between calculations
	// Frame sizes are truncated integers
	uint16_t  bps       = samples / 8;
	uint16_t  fsize     = (uint16_t)(((uint32_t)bps * bitrate) / samprate);
	if (pad)  fsize    += slot_size;

	if (input_len < fsize)
		return 0;
	
	if (info)
	{
		info->frame_size = fsize;
		info->version    = mpeg_versions[ver];
		info->layer      = mpeg_layers[lyr];
	}

	return 1;
}

#ifndef AUDIOEXTRACT_MPEG_H__
#define AUDIOEXTRACT_MPEG_H__

#include "audioextract.h"

#define IS_MPEG_MAGIC(hdr)  \
	(  ( ((uint8_t)(hdr)[0] & 0xFF) == 0xFF) \
	&& ( ((uint8_t)(hdr)[1] & 0xE0) == 0xE0)   /* 3 sync bits  */ \
	&& ( ((uint8_t)(hdr)[1] & 0x18) != 0x08)   /* Version rsvd */ \
	&& ( ((uint8_t)(hdr)[1] & 0x06) != 0x00)   /* Layer rsvd   */ \
	&& ( ((uint8_t)(hdr)[2] & 0xF0) != 0xF0))  /* Bitrate rsvd */

#define MPEG_HEADER_SIZE 4

struct mpeg_info {
	uint16_t frame_size;
	uint8_t  version;
	uint8_t  layer;
};

int mpeg_isframe(const unsigned char *start, const unsigned char *end, struct mpeg_info *info);

#endif /* AUDIOEXTRACT_MPEG_H__ */

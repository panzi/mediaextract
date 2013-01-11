#ifndef MEDIAEXTRACT_MPG123_H__
#define MEDIAEXTRACT_MPG123_H__

#include "mediaextract.h"

#define IS_MPG123_MAGIC(hdr)  \
	(  ( ((uint8_t)(hdr)[0] & 0xFF) == 0xFF) \
	&& ( ((uint8_t)(hdr)[1] & 0xE0) == 0xE0)   /* 3 sync bits  */ \
	&& ( ((uint8_t)(hdr)[1] & 0x18) != 0x08)   /* Version rsvd */ \
	&& ( ((uint8_t)(hdr)[1] & 0x06) != 0x00)   /* Layer rsvd   */ \
	&& ( ((uint8_t)(hdr)[2] & 0xF0) != 0xF0))  /* Bitrate rsvd */

#define MPG123_HEADER_SIZE 4

struct mpg123_info {
	uint16_t frame_size;
	uint8_t  version;
	uint8_t  layer;
};

int mpg123_isframe(const uint8_t *data, size_t input_len, struct mpg123_info *info);

#endif /* MEDIAEXTRACT_MPG123_H__ */

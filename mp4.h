#ifndef AUDIOEXTRACT_MP4_H__
#define AUDIOEXTRACT_MP4_H__

#include "audioextract.h"

#define MP4_MAGIC MAGIC("ftyp")
#define MP4_MAGIC_OFFSET  4
#define MP4_HEADER_SIZE  16

struct mp4_info {
	size_t length;
	const char *ext;
};

int mp4_isfile(const uint8_t *data, size_t input_len, struct mp4_info *info);

#endif /* AUDIOEXTRACT_MP4_H__ */

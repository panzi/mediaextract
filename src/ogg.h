#ifndef AUDIOEXTRACT_OGG_H__
#define AUDIOEXTRACT_OGG_H__

#include "audioextract.h"

#define OGG_MAGIC MAGIC("OggS")
#define OGG_HEADER_SIZE 27

struct ogg_info {
	size_t   length;
	uint32_t pageno;
};

int ogg_ispage(const uint8_t *data, size_t input_len, struct ogg_info *pageinfo);

#endif /* AUDIOEXTRACT_OGG_H__ */

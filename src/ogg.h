#ifndef MEDIAEXTRACT_OGG_H__
#define MEDIAEXTRACT_OGG_H__

#include "mediaextract.h"

#define OGG_MAGIC MAGIC("OggS")
#define OGG_HEADER_SIZE 27

struct ogg_info {
	size_t   length;
	uint32_t pageno;
};

int ogg_ispage(const uint8_t *data, size_t input_len, struct ogg_info *pageinfo);

#endif /* MEDIAEXTRACT_OGG_H__ */

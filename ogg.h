#ifndef AUDIOEXTRACT_OGG_H__
#define AUDIOEXTRACT_OGG_H__

#include "audioextract.h"

#define OGG_MAGIC MAGIC("OggS")
#define OGG_HEADER_SIZE 27
#define ogg_isinitial(data) ((data)[5] & 2)

int ogg_ispage(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_OGG_H__ */

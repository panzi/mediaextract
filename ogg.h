#ifndef AUDIOEXTRACT_OGG_H__
#define AUDIOEXTRACT_OGG_H__

#include "audioextract.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#	define OGG_MAGIC  0x5367674f /* "SggO" */

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define OGG_MAGIC  0x5367674f /* "OggS" */

#else

#	error unsupported endian

#endif

#define OGG_HEADER_SIZE 27
#define ogg_isinitial(data) ((data)[5] & 2)

int ogg_ispage(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_OGG_H__ */

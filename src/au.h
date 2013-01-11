#ifndef MEDIAEXTRACT_AU_H__
#define MEDIAEXTRACT_AU_H__

#include "mediaextract.h"

#define AU_MAGIC MAGIC(".snd")

#define AU_HEADER_SIZE 24

int au_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_AU_H__ */

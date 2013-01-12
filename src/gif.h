#ifndef MEDIAEXTRACT_GIF_H__
#define MEDIAEXTRACT_GIF_H__

#include "mediaextract.h"

#define GIF87a_SIGNATURE "GIF87a"
#define GIF89a_SIGNATURE "GIF89a"
#define GIF_MAGIC MAGIC(GIF87a_SIGNATURE)
#define GIF_SIGNATURE_SIZE 6

#define GIF_HEADER_SIZE      13
#define GIF_IMAGE_BLOCK_SIZE 11
#define GIF_MIN_SIZE         14

#define GIF_IMAGE_BLOCK ','
#define GIF_EXT         '!'
#define GIF_TRAILER     ';'

#define GIF_CT_FLAG     0x80

int gif_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_GIF_H__ */

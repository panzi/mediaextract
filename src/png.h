#ifndef MEDIAEXTRACT_PNG_H__
#define MEDIAEXTRACT_PNG_H__

#include "mediaextract.h"

#define PNG_SIGNATURE "\x89PNG\r\n\x1a\n"
#define PNG_MAGIC MAGIC(PNG_SIGNATURE)
#define PNG_SIGNATURE_SIZE 8

#define PNG_IHDR_MAGIC MAGIC("IHDR")
#define PNG_IEND_MAGIC MAGIC("IEND")

#define PNG_HEADER_SIZE 33
#define PNG_MIN_SIZE    45

int png_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_PNG_H__ */

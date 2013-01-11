#ifndef MEDIAEXTRACT_BMP_H__
#define MEDIAEXTRACT_BMP_H__

#include "mediaextract.h"

#define IS_BMP_MAGIC(data) ((*(const uint16_t *)(data)) == (*(const uint16_t *)("BM")))

#define BMP_HEADER_SIZE 14
#define DIB_HEADER_SIZE 40

int bmp_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_BMP_H__ */

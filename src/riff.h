#ifndef MEDIAEXTRACT_RIFF_H__
#define MEDIAEXTRACT_RIFF_H__

#include "mediaextract.h"

#define RIFF_MAGIC      MAGIC("RIFF")
#define RIFF_LIST_MAGIC MAGIC("LIST")

#define RIFF_HEADER_SIZE 12

int riff_isfile(const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* MEDIAEXTRACT_RIFF_H__ */

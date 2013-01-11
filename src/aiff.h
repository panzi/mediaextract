#ifndef MEDIAEXTRACT_AIFF_H__
#define MEDIAEXTRACT_AIFF_H__

#include "mediaextract.h"

#define FORM_MAGIC MAGIC("FORM")
#define AIFF_MAGIC MAGIC("AIFF")
#define AIFC_MAGIC MAGIC("AIFC")

#define AIFF_HEADER_SIZE 12

int aiff_isfile(const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* MEDIAEXTRACT_AIFF_H__ */

#ifndef AUDIOEXTRACT_AIFF_H__
#define AUDIOEXTRACT_AIFF_H__

#include "audioextract.h"

#define FORM_MAGIC MAGIC("FORM")
#define AIFF_MAGIC MAGIC("AIFF")
#define AIFC_MAGIC MAGIC("AIFC")

#define AIFF_HEADER_SIZE 12

int aiff_isfile(const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* AUDIOEXTRACT_AIFF_H__ */

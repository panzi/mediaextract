#ifndef AUDIOEXTRACT_RIFF_H__
#define AUDIOEXTRACT_RIFF_H__

#include "audioextract.h"

#define RIFF_MAGIC MAGIC("RIFF")

#define RIFF_HEADER_SIZE 12

int riff_isfile(const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* AUDIOEXTRACT_RIFF_H__ */

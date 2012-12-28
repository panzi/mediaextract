#ifndef AUDIOEXTRACT_WAVE_H__
#define AUDIOEXTRACT_WAVE_H__

#include "audioextract.h"

#define RIFF_MAGIC MAGIC("RIFF")
#define WAVE_MAGIC MAGIC("WAVE")

#define FORM_MAGIC MAGIC("FORM")
#define AIFF_MAGIC MAGIC("AIFF")
#define AIFC_MAGIC MAGIC("AIFC")

#define WAVE_HEADER_SIZE 8

int wave_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr);
int aiff_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_WAVE_H__ */

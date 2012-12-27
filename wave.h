#ifndef AUDIOEXTRACT_WAVE_H__
#define AUDIOEXTRACT_WAVE_H__

#include "audioextract.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#	define RIFF_MAGIC 0x46464952 /* "RIFF" (reversed) */
#	define WAVE_MAGIC 0x45564157 /* "WAVE" (reversed) */

#	define FORM_MAGIC 0x4d524f46 /* "FORM" (reversed) */
#	define AIFF_MAGIC 0x46464941 /* "AIFF" (reversed) */
#	define AIFC_MAGIC 0x43464941 /* "AIFC" (reversed) */

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define RIFF_MAGIC 0x46464952 /* "RIFF" */
#	define WAVE_MAGIC 0x57415645 /* "WAVE" */

#	define FORM_MAGIC 0x464f524d /* "FORM" */
#	define AIFF_MAGIC 0x41494646 /* "AIFF" */
#	define AIFC_MAGIC 0x41494643 /* "AIFC" */

#else

#	error unsupported endian

#endif

#define WAVE_HEADER_SIZE 8

int wave_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr);
int aiff_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /*  AUDIOEXTRACT_WAVE_H__ */

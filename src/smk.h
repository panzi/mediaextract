#ifndef AUDIOEXTRACT_SMK_H__
#define AUDIOEXTRACT_SMK_H__

#include "audioextract.h"

#define SMK2_MAGIC MAGIC("SMK2")
#define SMK4_MAGIC MAGIC("SMK4")

#define IS_SMK_MAGIC(magic) ((magic) == SMK2_MAGIC || (magic) == SMK4_MAGIC)

#define SMK_HEADER_SIZE 104

int smk_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* AUDIOEXTRACT_SMK_H__ */

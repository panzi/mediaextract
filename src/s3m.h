#ifndef AUDIOEXTRACT_S3M_H__
#define AUDIOEXTRACT_S3M_H__

#include "audioextract.h"

#define S3M_MAGIC        MAGIC("SCRM")
#define S3M_SAMPLE_MAGIC MAGIC("SCRS")
#define S3M_MAGIC_OFFSET 44
#define S3M_HEADER_SIZE  96
#define S3M_SAMPLE_HEADER_SIZE 80

int s3m_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* AUDIOEXTRACT_S3M_H__ */

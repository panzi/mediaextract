#ifndef AUDIOEXTRACT_XM_H__
#define AUDIOEXTRACT_XM_H__

#include "audioextract.h"

#define XM_ID          "Extended Module: "
#define XM_ID_SIZE     17
#define XM_MAGIC       MAGIC(XM_ID)
#define XM_HEADER_SIZE 80

int xm_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* AUDIOEXTRACT_XM_H__ */

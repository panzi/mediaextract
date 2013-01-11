#ifndef MEDIAEXTRACT_BINK_H__
#define MEDIAEXTRACT_BINK_H__

#include "mediaextract.h"

#define BINK_MAGIC1 MAGIC("BIKb")
#define BINK_MAGIC2 MAGIC("BIKd")
#define BINK_MAGIC3 MAGIC("BIKf")
#define BINK_MAGIC4 MAGIC("BIKg")
#define BINK_MAGIC5 MAGIC("BIKh")
#define BINK_MAGIC6 MAGIC("BIKi")

#define IS_BINK_MAGIC(magic) \
	((magic) == BINK_MAGIC1 || \
	 (magic) == BINK_MAGIC2 || \
	 (magic) == BINK_MAGIC3 || \
	 (magic) == BINK_MAGIC4 || \
	 (magic) == BINK_MAGIC5 || \
	 (magic) == BINK_MAGIC6)

#define BINK_HEADER_SIZE 44

int bink_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_BINK_H__ */

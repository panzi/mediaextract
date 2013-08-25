#ifndef MEDIAEXTRACT_TEXT_H__
#define MEDIAEXTRACT_TEXT_H__

#include "mediaextract.h"

#define IS_NON_PRINTABLE_CODEPOINT(CP) \
	((CP) <= 0x8 || (CP) == 0xB || ((CP) >= 0xE && (CP) <= 0x1F) || (CP) == 0x7F)

int ascii_isfile(const uint8_t *data, size_t input_len, struct file_info *info);
int utf8_isfile (const uint8_t *data, size_t input_len, struct file_info *info);

#endif /* MEDIAEXTRACT_TEXT_H__ */

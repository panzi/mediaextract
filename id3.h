#ifndef AUDIOEXTRACT_ID3_H__
#define AUDIOEXTRACT_ID3_H__

#include "audioextract.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN

#	define ID3_MASK    (uint32_t)0x00FFFFFF
#	define ID3v1_MAGIC (uint32_t)0x00474154 /* "\0GAT" */
#	define ID3v2_MAGIC (uint32_t)0x00334449 /* "\03DI" */

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define ID3_MASK    (uint32_t)0xFFFFFF00
#	define ID3v1_MAGIC (uint32_t)0x54414700 /* "TAG\0" */
#	define ID3v2_MAGIC (uint32_t)0x49443300 /* "ID3\0" */

#else

#	error unsupported endian

#endif

#define IS_ID3v1_MAGIC(hdr) ((*(uint32_t *)(hdr) & ID3_MASK) == ID3v1_MAGIC)
#define IS_ID3v2_MAGIC(hdr) ((*(uint32_t *)(hdr) & ID3_MASK) == ID3v2_MAGIC)
#define ID3v1_TAG_SIZE 128
#define ID3v2_HEADER_SIZE 10
#define ID3v2_FOOTER_SIZE 10

int id3v1_istag(const unsigned char *start, const unsigned char *end, size_t *lengthptr);
int id3v2_istag(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /* AUDIOEXTRACT_ID3_H__ */

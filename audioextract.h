#ifndef AUDIOEXTRACT_H__
#define AUDIOEXTRACT_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#	define __WINDOWS__

#endif

#if defined(__linux__)

#	include <endian.h>

#elif defined(__APPLE__)

#	include <libkern/OSByteOrder.h>

#	define htobe16 OSSwapHostToBigInt16
#	define htole16 OSSwapHostToLittleInt16
#	define be16toh OSSwapBigToHostInt16
#	define le16toh OSSwapLittleToHostInt16
 
#	define htobe32 OSSwapHostToBigInt32
#	define htole32 OSSwapHostToLittleInt32
#	define be32toh OSSwapBigToHostInt32
#	define le32toh OSSwapLittleToHostInt32
 
#	define htobe64 OSSwapHostToBigInt64
#	define htole64 OSSwapHostToLittleInt64
#	define be64toh OSSwapBigToHostInt64
#	define le64toh OSSwapLittleToHostInt64

#elif defined(__OpenBSD__)

#	include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)

#	include <sys/endian.h>

#	define be16toh betoh16
#	define le16toh letoh16

#	define be32toh betoh32
#	define le32toh letoh32

#	define be64toh betoh64
#	define le64toh letoh64

#elif defined(__WINDOWS__)

/* assume little-endian for Windows
 * currently only Windows on the XBox 360 uses big-endian */

#	include <arpa/inet.h>

#	define htobe16 htons
#	define htole16(x) (x)
#	define be16toh ntohs
#	define le16toh(x) (x)
 
#	define htobe32 htonl
#	define htole32(x) (x)
#	define be32toh ntohl
#	define le32toh(x) (x)
 
#	define htobe64 htonll
#	define htole64(x) (x)
#	define be64toh ntohll
#	define le64toh(x) (x)

#else

#	error "platform not supported"

#endif

#define MAGIC(STR) (*(const uint32_t*)(STR))

struct file_info {
	size_t length;
	const char *ext;
};

int probalby_mod_text(const uint8_t *str, size_t length);

#endif /* AUDIOEXTRACT_H__ */

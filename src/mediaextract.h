/* Copyright (c) 2015 Mathias Panzenböck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MEDIAEXTRACT_H__
#define MEDIAEXTRACT_H__
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#	define __WINDOWS__

#endif

#if defined(__linux__) || defined(__CYGWIN__)

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

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

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

#	include <winsock2.h>
#	include <sys/param.h>

#	if BYTE_ORDER == LITTLE_ENDIAN

#		define htobe16 htons
#		define htole16(x) (x)
#		define be16toh ntohs
#		define le16toh(x) (x)
 
#		define htobe32 htonl
#		define htole32(x) (x)
#		define be32toh ntohl
#		define le32toh(x) (x)
 
#		define htobe64 htonll
#		define htole64(x) (x)
#		define be64toh ntohll
#		define le64toh(x) (x)

#	elif BYTE_ORDER == BIG_ENDIAN

		/* that would be xbox 360 */
#		define htobe16(x) (x)
#		define htole16(x) __builtin_bswap16(x)
#		define be16toh(x) (x)
#		define le16toh(x) __builtin_bswap16(x)
 
#		define htobe32(x) (x)
#		define htole32(x) __builtin_bswap32(x)
#		define be32toh(x) (x)
#		define le32toh(x) __builtin_bswap32(x)
 
#		define htobe64(x) (x)
#		define htole64(x) __builtin_bswap64(x)
#		define be64toh(x) (x)
#		define le64toh(x) __builtin_bswap64(x)

#	else

#		error byte order not supported

#	endif

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#else

#	error platform not supported

#endif

#ifdef __WINDOWS__

#	define PATH_SEP '\\'
#	define PATH_SEP_STR "\\"

#else

#	define PATH_SEP '/'
#	define PATH_SEP_STR "/"

#endif

#define _CMAGIC(C1,C2,C3,C4) (uint32_t)(((C1) << 24) | ((C2) << 16) | ((C3) << 8) | (C4))

#if __BYTE_ORDER == __LITTLE_ENDIAN

#	define CMAGIC(C1,C2,C3,C4) _CMAGIC(C4,C3,C2,C1)

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define CMAGIC(C1,C2,C3,C4) _CMAGIC(C1,C2,C3,C4)

#elif __BYTE_ORDER == __PDP_ENDIAN

#	define CMAGIC(C1,C2,C3,C4) _CMAGIC(C3,C4,C1,C2)

#else

#	error unsupported byte order

#endif

#if 0
#define MAGIC(STR) (*(const uint32_t*)(STR))
#else

#define _MAGIC(B) CMAGIC((B)[0], (B)[1], (B)[2], (B)[3])
#define MAGIC(S)  _MAGIC((const uint8_t *)(S))

#endif

#define MEDIAEXTRACT_VERSION "1.2.0"

typedef uint64_t file_format;

#define NONE     ((uint64_t)      0)
#define OGG      ((uint64_t)1 <<  0)
#define RIFF     ((uint64_t)1 <<  1)
#define AIFF     ((uint64_t)1 <<  2)
#define MPG123   ((uint64_t)1 <<  3)
#define ID3v2    ((uint64_t)1 <<  4)
#define MP4      ((uint64_t)1 <<  5)
#define MIDI     ((uint64_t)1 <<  6)
#define XMIDI    ((uint64_t)1 <<  7)
#define MOD      ((uint64_t)1 <<  8)
#define S3M      ((uint64_t)1 <<  9)
#define IT       ((uint64_t)1 << 10)
#define XM       ((uint64_t)1 << 11)
#define ASF      ((uint64_t)1 << 12)
#define BINK     ((uint64_t)1 << 13)
#define AU       ((uint64_t)1 << 14)
#define SMK      ((uint64_t)1 << 15)
#define BMP      ((uint64_t)1 << 16)
#define PNG      ((uint64_t)1 << 17)
#define JPEG     ((uint64_t)1 << 18)
#define GIF      ((uint64_t)1 << 19)
#define MPEG1    ((uint64_t)1 << 20)
#define MPEGPS   ((uint64_t)1 << 21)
#define MPEGVS   ((uint64_t)1 << 22) // TODO
#define MPEGTS   ((uint64_t)1 << 23)
#define ASCII    ((uint64_t)1 << 24)
#define UTF_8    ((uint64_t)1 << 25)
#define UTF_16LE ((uint64_t)1 << 26)
#define UTF_16BE ((uint64_t)1 << 27)
#define UTF_32LE ((uint64_t)1 << 28)
#define UTF_32BE ((uint64_t)1 << 29)
#define AVIF     ((uint64_t)1 << 30)
#define HEIF     ((uint64_t)1 << 31)

// TODO: look into these formats:
// VTF, PSD, XCF, ICO, OpenEXR, PCX, JPEG 2000, TIFF

struct file_info {
	size_t length;
	const char *ext;
};

struct extract_options {
	const char *filepath;
	const char *outdir;
	const char *filename;
	size_t   minsize;
	size_t   maxsize;
	uint64_t offset;
	size_t   length;
	size_t   index;
	file_format formats;
	bool quiet;
	bool simulate;
};

int probably_mod_text(const uint8_t *str, size_t length);
int extract(const struct extract_options *options,
            size_t *numfilesptr, size_t *sumsizeptr);
int do_extract(const uint8_t *filedata, size_t filesize,
               const struct extract_options *options,
               size_t *numfilesptr, size_t *sumsizeptr);
int write_data(const char *filename, const uint8_t *data, size_t length);

#endif /* MEDIAEXTRACT_H__ */

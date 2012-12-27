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
#include <arpa/inet.h>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)
#define __WINDOWS__
#endif

#ifndef __WINDOWS__
#include <endian.h>
#endif

#ifndef __BYTE_ORDER
#	ifndef __WINDOWS__
#		error cannot detect byte order
#	else
		/* assume little endian byte order on windows */
#		define __LITTLE_ENDIAN 1234
#		define __BIG_ENDIAN 4321
#		define __PDP_ENDIAN 3412
#		define __BYTE_ORDER __LITTLE_ENDIAN
#	endif
#elif __BYTE_ORDER != __LITTLE_ENDIAN && __BYTE_ORDER != __BIG_ENDIAN
#	error unsupported byte order
#endif

#endif /*  AUDIOEXTRACT_H__ */

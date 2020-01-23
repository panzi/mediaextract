/* Copyright (c) 2020 Mathias Panzenböck
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

#include "formatstring.h"

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

void formatstring_error(const char *format, size_t index, const char *message) {
	fprintf(stderr, "illegal format string:%zu: %s\n\n\t%s\n\t", index + 1, message, format);
	while (index > 0) {
		fprintf(stderr, "-");
		-- index;
	}
	fprintf(stderr, "^\n");
}

#define IS_VAR_NAME(PTR, LEN, NAME) (strncmp((PTR), (NAME), MIN(sizeof (NAME), LEN)) == 0)

#define WRITE_STR(STR, LEN) \
	{ \
	const size_t COPY_STR_rem = endptr > bufptr ? endptr - bufptr : 0; \
	const size_t COPY_STR_cpylen = MIN((LEN), COPY_STR_rem); \
	memcpy(bufptr, (STR), COPY_STR_cpylen); \
	bufptr += (LEN); \
	}

#define HEX_CHARS "0123456789ABCDEF"

#define WRITE_HEX(NUM) \
	{ \
	size_t WRITE_HEX_size = sizeof(NUM); \
	size_t WRITE_HEX_rem  = (NUM); \
	while (WRITE_HEX_size) { \
		-- WRITE_HEX_size; \
		char *WRITE_HEX_outptr = bufptr + 2 * WRITE_HEX_size; \
		if (WRITE_HEX_outptr < endptr) { \
			*WRITE_HEX_outptr = HEX_CHARS[(WRITE_HEX_rem >> 4) & 0xF]; \
		} \
		++ WRITE_HEX_outptr; \
		if (WRITE_HEX_outptr < endptr) { \
			*WRITE_HEX_outptr = HEX_CHARS[WRITE_HEX_rem & 0xF]; \
		} \
		WRITE_HEX_rem >>= 8; \
	} \
	bufptr += 2 * sizeof(NUM); \
	}

#define WRITE_CHAR(CH) \
	if (bufptr < endptr) { \
		*bufptr = (CH); \
	} \
	++ bufptr;

#define WRITE_DEC(NUM) \
	if ((NUM) == 0) { \
		WRITE_CHAR('0'); \
	} else { \
		size_t WRITE_DEC_isize = 0; \
		size_t WRITE_DEC_n = (NUM); \
		while (WRITE_DEC_n) { \
			WRITE_DEC_n /= 10; \
			++ WRITE_DEC_isize; \
		} \
		WRITE_DEC_n = (NUM); \
		size_t WRITE_DEC_index = WRITE_DEC_isize; \
		while (WRITE_DEC_n) { \
			-- WRITE_DEC_index; \
			const size_t WRITE_DEC_v = WRITE_DEC_n % 10; \
			char *WRITE_DEC_outptr = bufptr + WRITE_DEC_index; \
			if (WRITE_DEC_outptr < endptr) { \
				*WRITE_DEC_outptr = '0' + WRITE_DEC_v; \
			} \
			WRITE_DEC_n /= 10; \
		} \
		bufptr += WRITE_DEC_isize; \
	}

ssize_t formatstring(char *buffer, size_t buffer_size, const char *format, const char *filename,
                     size_t index, size_t offset, size_t size, const char *ext) {
	assert(buffer_size == 0 || buffer != NULL);
	assert(format != NULL);
	assert(filename != NULL);
	assert(ext != NULL);

	const char *fmtptr = format;
	char       *bufptr = buffer_size == 0 ? NULL : buffer;
	const char *endptr = bufptr + buffer_size;
	const size_t filename_len = strlen(filename);
	const size_t ext_len      = strlen(ext);

	while (*fmtptr) {
		const char *nextptr = fmtptr;
		while (*nextptr && *nextptr != '{' && *nextptr != '}') {
			++ nextptr;
		}

		if (nextptr != fmtptr) {
			const size_t len = nextptr - fmtptr;
			WRITE_STR(fmtptr, len);
			fmtptr = nextptr;
		}

		if (*fmtptr == '}') {
			++ fmtptr;
			if (*fmtptr == '}') {
				WRITE_CHAR('}');
				++ fmtptr;
			} else {
				formatstring_error(format, fmtptr - format, "expected '}'");
				errno = EINVAL;
				return -1;
			}
		} else if (*fmtptr == '{') {
			++ fmtptr;
			if (*fmtptr == '{') {
				WRITE_CHAR('{');
				++ fmtptr;
			} else {
				nextptr = fmtptr;
				while (*nextptr && *nextptr != '}') {
					++ nextptr;
				}

				if (fmtptr == nextptr) {
					formatstring_error(format, fmtptr - format, "expected a variable name");
					errno = EINVAL;
					return -1;
				}

				const size_t len = nextptr - fmtptr;
				if (IS_VAR_NAME(fmtptr, len, "filename")) {
					WRITE_STR(filename, filename_len);
				} else if (IS_VAR_NAME(fmtptr, len, "ext")) {
					WRITE_STR(ext, ext_len);
				} else if (IS_VAR_NAME(fmtptr, len, "offset")) {
					WRITE_HEX(offset);
				} else if (IS_VAR_NAME(fmtptr, len, "index")) {
					WRITE_DEC(index);
				} else if (IS_VAR_NAME(fmtptr, len, "size")) {
					WRITE_DEC(size);
				} else {
					formatstring_error(format, fmtptr - format, "unknown variable name");
					errno = EINVAL;
					return -1;
				}
				
				fmtptr = nextptr + 1;
			}
		}
	}

	if (bufptr < endptr) {
		*bufptr = 0;
	} else if (buffer_size > 0) {
		buffer[buffer_size - 1] = 0;
	}
	++ bufptr;
	
	const size_t strsize = bufptr - buffer;

	if (strsize > SSIZE_MAX) {
		formatstring_error(format, fmtptr - format, "string too long");
		errno = ERANGE;
		return -1;
	}

	return (ssize_t) strsize;
}

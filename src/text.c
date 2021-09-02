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

#ifndef MEDIAEXTRACT_RECODE_BIN
#include <strings.h>
#endif

#include "text.h"

typedef uint32_t codepoint_t;

static const uint8_t *decode_utf8_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf16le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf16be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf32le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf32be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);

#ifndef MEDIAEXTRACT_RECODE_BIN
static int text_isfile(const uint8_t *data, size_t input_len, struct file_info *info,
	const uint8_t *(*decode_codepoint)(const uint8_t *str, size_t size, codepoint_t *cpptr),
	const char *ext, const char *cr_ext, const char *crlf_ext, const char *lf_ext);
#endif

const uint8_t *decode_utf8_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size == 0) {
		return NULL;
	}

	uint8_t byte1 = str[0];

	codepoint_t cp = 0;

	if (byte1 < 0x80) {
		cp = byte1;
		str += 1;
	}
	else if (byte1 < 0xBF) {
		// unexpected continuation or overlong 2-byte sequence
		return NULL;
	}
	else if (byte1 < 0xE0) {
		if (size < 2) {
			// unexpected end of file
			return NULL;
		}

		uint8_t byte2 = str[1];

		if ((byte2 & 0xC0) != 0x80) {
			// illegal byte sequence
			return NULL;
		}

		// ensure promotion to codepoint_t (uint32_t) and not to a signed integer
		cp = (codepoint_t)(byte1 & 0x1F) << 6 |
		     (codepoint_t)(byte2 & 0x3F);

		str += 2;
	}
	else if (byte1 < 0xF0) {
		if (size < 3) {
			// unexpected end of file
			return NULL;
		}

		uint8_t byte2 = str[1];
		uint8_t byte3 = str[2];

		if ((byte1 == 0xE0 && byte2 < 0xA0) ||
		    (byte2 & 0xC0) != 0x80 ||
		    (byte3 & 0xC0) != 0x80) {
			// illegal byte sequence
			return NULL;
		}

		cp = (codepoint_t)(byte1 & 0x0F) << 12 |
		     (codepoint_t)(byte2 & 0x3F) <<  6 |
		     (codepoint_t)(byte3 & 0x3F);

		str += 3;
	}
	else if (byte1 < 0xF8) {
		if (size < 4) {
			// unexpected end of file
			return NULL;
		}

		uint8_t byte2 = str[1];
		uint8_t byte3 = str[2];
		uint8_t byte4 = str[3];

		if ((byte1 == 0xF0 && byte2 < 0x90) || (byte1 == 0xF4 && byte2 >= 0x90) ||
		    (byte2 & 0xC0) != 0x80 ||
		    (byte3 & 0xC0) != 0x80 ||
		    (byte4 & 0xC0) != 0x80) {
			// illegal byte sequence
			return NULL;
		}

		cp = (codepoint_t)(byte1 & 0x07) << 18 |
		     (codepoint_t)(byte2 & 0x3F) << 12 |
		     (codepoint_t)(byte3 & 0x3F) <<  6 |
		     (codepoint_t)(byte4 & 0x3F);

		if (cp > CODEPOINT_MAX) {
			return NULL;
		}

		str += 4;
	}
	else {
		// illegal byte sequence
		return NULL;
	}

	if (cpptr) *cpptr = cp;

	return str;
}

const uint8_t *decode_utf16le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size < 2) {
		return NULL;
	}

	uint16_t w1 = (str[1] << 8) | str[0];
	codepoint_t cp = 0;

	str += 2;
	switch (w1 & 0xfc00) {
		case 0xd800:
			if (size < 4) {
				return NULL;
			}
			else {
				uint16_t w2 = (str[1] << 8) | str[0];

				if ((w2 & 0xfc00) != 0xdc00) {
					return NULL;
				}

				codepoint_t hi = w1 & 0x3ff;
				codepoint_t lo = w2 & 0x3ff;

				cp = (hi << 10) | lo | 0x10000;

				if (cp > CODEPOINT_MAX) {
					return NULL;
				}
			}
			break;

		case 0x0000:
			cp = w1;
			break;

		default:
			return NULL;
	}

	if (cpptr) *cpptr = cp;

	return str;
}

const uint8_t *decode_utf16be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size < 2) {
		return NULL;
	}

	uint16_t w1 = (str[0] << 8) | str[1];
	codepoint_t cp = 0;

	str += 2;
	switch (w1 & 0xfc00) {
		case 0xd800:
			if (size < 4) {
				return NULL;
			}
			else {
				uint16_t w2 = (str[0] << 8) | str[1];

				if ((w2 & 0xfc00) != 0xdc00) {
					return NULL;
				}

				codepoint_t hi = w1 & 0x3ff;
				codepoint_t lo = w2 & 0x3ff;

				cp = (hi << 10) | lo | 0x10000;

				if (cp > CODEPOINT_MAX) {
					return NULL;
				}

				str += 2;
			}
			break;

		case 0x0000:
			cp = w1;
			break;

		default:
			return NULL;
	}

	if (cpptr) *cpptr = cp;

	return str;
}

const uint8_t *decode_utf32le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size < 4) {
		return NULL;
	}

	codepoint_t cp = (((((str[3] << 8) | str[2]) << 8) | str[1]) << 8) | str[0];

	if (cp > CODEPOINT_MAX) {
		return NULL;
	}

	str += 4;

	if (cpptr) *cpptr = cp;

	return str;
}

const uint8_t *decode_utf32be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size < 4) {
		return NULL;
	}

	codepoint_t cp = (((((str[0] << 8) | str[1]) << 8) | str[2]) << 8) | str[3];

	if (cp > CODEPOINT_MAX) {
		return NULL;
	}

	str += 4;

	if (cpptr) *cpptr = cp;

	return str;
}

const uint8_t *decode_latin1_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size == 0) {
		return NULL;
	}

	if (cpptr) *cpptr = str[0];

	str += 1;

	return str;
}

#ifndef MEDIAEXTRACT_RECODE_BIN

int ascii_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	int cr   = 0;
	int crlf = 0;
	int lf   = 0;
	const uint8_t *ptr = data;
	const uint8_t *end = data + input_len;

	while (ptr < end) {
		uint8_t ch = *ptr;

		if (IS_NON_PRINTABLE_CODEPOINT(ch) || ch > ASCII_MAX) {
			break;
		}

		++ ptr;

		if (ch == '\r') {
			if (ptr < end && *ptr == '\n') {
				++ ptr;
				crlf = 1;
			}
			else {
				cr = 1;
			}
		}
		else if (ch == '\n') {
			lf = 1;
		}
	}

	size_t length = ptr - data;

	if (length < 2) {
		return 0;
	}

	if (info) {
		info->length = length;

		if ((cr + crlf + lf) != 1) {
			info->ext = "ascii.txt";
		}
		else if (cr) {
			info->ext = "mac.ascii.txt";
		}
		else if (crlf) {
			info->ext = "dos.ascii.txt";
		}
		else {
			info->ext = "unix.ascii.txt";
		}
	}

	return 1;
}

int utf8_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	int cr   = 0;
	int crlf = 0;
	int lf   = 0;
	int non_ascii = 0;
	codepoint_t cp = 0;
	const uint8_t *ptr = data;
	const uint8_t *end = data + input_len;

	while (ptr < end) {
		const uint8_t *next = decode_utf8_codepoint(ptr, end - ptr, &cp);

		if (!next || IS_NON_PRINTABLE_CODEPOINT(cp)) {
			break;
		}

		ptr = next;

		if (cp >= 0x80) {
			non_ascii = 1;
		}
		else if (cp == '\r') {
			if (ptr < end && *ptr == '\n') {
				++ ptr;
				crlf = 1;
			}
			else {
				cr = 1;
			}
		}
		else if (cp == '\n') {
			lf = 1;
		}
	}

	size_t length = ptr - data;

	if (length < 2) {
		return 0;
	}

	if (info) {
		info->length = length;

		if ((cr + crlf + lf) != 1) {
			info->ext = non_ascii ? "utf8.txt" : "ascii.txt";
		}
		else if (cr) {
			info->ext = non_ascii ? "mac.utf8.txt" : "mac.ascii.txt";
		}
		else if (crlf) {
			info->ext = non_ascii ? "dos.utf8.txt" : "dos.ascii.txt";
		}
		else {
			info->ext = non_ascii ? "unix.utf8.txt" : "unix.ascii.txt";
		}
	}

	return 1;
}

int text_isfile(const uint8_t *data, size_t input_len, struct file_info *info,
	const uint8_t *(*decode_codepoint)(const uint8_t *str, size_t size, codepoint_t *cpptr),
	const char *ext, const char *cr_ext, const char *crlf_ext, const char *lf_ext) {
	int cr   = 0;
	int crlf = 0;
	int lf   = 0;
	codepoint_t cp = 0;
	const uint8_t *ptr = data;
	const uint8_t *end = data + input_len;

	while (ptr < end) {
		const uint8_t *next = decode_codepoint(ptr, end - ptr, &cp);

		if (!next || IS_NON_PRINTABLE_CODEPOINT(cp)) {
			break;
		}

		ptr = next;

		if (cp == '\r') {
			if (ptr < end && (next = decode_codepoint(ptr, end - ptr, &cp)) && cp == '\n') {
				ptr  = next;
				crlf = 1;
			}
			else {
				cr = 1;
			}
		}
		else if (cp == '\n') {
			lf = 1;
		}
	}

	size_t length = ptr - data;

	if (length < 2) {
		return 0;
	}

	if (info) {
		info->length = length;

		if ((cr + crlf + lf) != 1) {
			info->ext = ext;
		}
		else if (cr) {
			info->ext = cr_ext;
		}
		else if (crlf) {
			info->ext = crlf_ext;
		}
		else {
			info->ext = lf_ext;
		}
	}

	return 1;
}

int utf16le_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	return text_isfile(data, input_len, info, decode_utf16le_codepoint, "utf16le.txt", "mac.utf16le.txt", "dos.utf16le.txt", "unix.utf16le.txt");
}

int utf16be_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	return text_isfile(data, input_len, info, decode_utf16be_codepoint, "utf16be.txt", "mac.utf16be.txt", "dos.utf16be.txt", "unix.utf16be.txt");
}

int utf32le_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	return text_isfile(data, input_len, info, decode_utf32le_codepoint, "utf32le.txt", "mac.utf32le.txt", "dos.utf32le.txt", "unix.utf32le.txt");
}

int utf32be_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	return text_isfile(data, input_len, info, decode_utf32be_codepoint, "utf32be.txt", "mac.utf32be.txt", "dos.utf32be.txt", "unix.utf32be.txt");
}

#else

size_t encode_utf8_codepoint(codepoint_t codepoint, uint8_t *buf, size_t size) {
	if (codepoint > CODEPOINT_MAX) {
		return 0;
	}

	if (codepoint >= 0x10000) {
		if (size > 0) buf[0] =  (codepoint >> 18)         | 0xF0;
		if (size > 1) buf[1] = ((codepoint >> 12) & 0x3F) | 0x80;
		if (size > 2) buf[2] = ((codepoint >>  6) & 0x3F) | 0x80;
		if (size > 3) buf[3] =  (codepoint        & 0x3F) | 0x80;

		return 4;
	}
	else if (codepoint >= 0x800) {
		if (size > 0) buf[0] =  (codepoint >> 12)         | 0xE0;
		if (size > 1) buf[1] = ((codepoint >>  6) & 0x3F) | 0x80;
		if (size > 2) buf[2] =  (codepoint        & 0x3F) | 0x80;

		return 3;
	}
	else if (codepoint >= 0x80) {
		if (size > 0) buf[0] = (codepoint >> 6)   | 0xC0;
		if (size > 1) buf[1] = (codepoint & 0x3F) | 0x80;

		return 2;
	}
	else {
		if (size > 0) buf[0] = codepoint;

		return 1;
	}
}

size_t encode_latin1_codepoint(codepoint_t codepoint, uint8_t *buf, size_t size) {
	if (codepoint > LATIN1_MAX) {
		return 0;
	}

	if (size > 0) {
		buf[0] = codepoint;
	}

	return 1;
}

enum convert_status {
	CONVERT_OK           = 0,
	CONVERT_DECODE_ERROR = 1,
	CONVERT_ENCODE_ERROR = 2,
	CONVERT_READ_ERROR   = 3,
	CONVERT_WRITE_ERROR  = 4
};

#define MAX_ENCODED_BYTES 4

enum convert_status convert (
	FILE *input, FILE *output,
	const uint8_t *(*decode_codepoint)(const uint8_t *str, size_t size, codepoint_t *cpptr),
	size_t (*encode_codepoint)(codepoint_t codepoint, uint8_t *buf, size_t size),
	size_t *lineptr, size_t *colptr, size_t *byteptr) {
	size_t line = 1, col = 1, byte = 0, off = 0;
	codepoint_t cp = 0;
	uint8_t inbuf[BUFSIZ];
	uint8_t outbuf[MAX_ENCODED_BYTES];

	for (;;) {
		size_t count = off + fread(inbuf + off, 1, BUFSIZ - off, input);
		const uint8_t *stop = inbuf + (count < BUFSIZ ? count : BUFSIZ - MAX_ENCODED_BYTES);
		const uint8_t *end  = inbuf + BUFSIZ;
		const uint8_t *ptr  = inbuf;

		while (ptr < stop) {
			const uint8_t *next = decode_codepoint(ptr, end - ptr, &cp);

			if (!next) {
				if (lineptr) *lineptr = line;
				if (colptr)  *colptr  = col;
				if (byteptr) *byteptr = byte;

				return CONVERT_DECODE_ERROR;
			}

			size_t outcount = encode_codepoint(cp, outbuf, MAX_ENCODED_BYTES);

			if (outcount == 0 || outcount > MAX_ENCODED_BYTES) {
				if (lineptr) *lineptr = line;
				if (colptr)  *colptr  = col;
				if (byteptr) *byteptr = byte;

				return CONVERT_ENCODE_ERROR;
			}

			if (fwrite(outbuf, 1, outcount, output) < outcount) {
				if (lineptr) *lineptr = line;
				if (colptr)  *colptr  = col;
				if (byteptr) *byteptr = byte;

				return CONVERT_WRITE_ERROR;
			}

			byte += next - ptr;
			if (cp == '\n') {
				++ line;
				col = 1;
			}
			else {
				++ col;
			}

			ptr = next;
		}

		if (count < BUFSIZ) {
			if (ferror(input)) {
				if (lineptr) *lineptr = line;
				if (colptr)  *colptr  = col;
				if (byteptr) *byteptr = byte;

				return CONVERT_READ_ERROR;
			}

			break;
		}

		off = inbuf + BUFSIZ - ptr;
		memmove(inbuf, ptr, off);
	}
	
	if (lineptr) *lineptr = line;
	if (colptr)  *colptr  = col;
	if (byteptr) *byteptr = byte;

	return CONVERT_OK;
}

void print_convert_error(const char *filename, size_t errline, size_t errcol, size_t errbyte, enum convert_status status) {
	switch (status) {
	case CONVERT_DECODE_ERROR:
		fprintf(stderr, "%s:%zd:%zd: illegal byte sequence at offset %zd\n", filename, errline, errcol, errbyte);
		break;

	case CONVERT_ENCODE_ERROR:
		fprintf(stderr, "%s:%zd:%zd: could not encode character at offset %zd\n", filename, errline, errcol, errbyte);
		break;

	case CONVERT_READ_ERROR:
		fprintf(stderr, "%s:%zd:%zd: read error at offset %zd\n", filename, errline, errcol, errbyte);
		break;

	case CONVERT_WRITE_ERROR:
		fprintf(stderr, "%s:%zd:%zd: write error at offset %zd\n", filename, errline, errcol, errbyte);
		break;

	case CONVERT_OK:
		break;
	}
}

int main (int argc, char *argv[]) {
	const uint8_t *(*decode_codepoint)(const uint8_t *str, size_t size, codepoint_t *cpptr) = NULL;
	size_t (*encode_codepoint)(codepoint_t codepoint, uint8_t *buf, size_t size) = NULL;

	if (argc < 3) {
		fprintf(stderr, "usage: recode <input-encoding> <output-encoding> [file]...\n");
		return EXIT_FAILURE;
	}

	const char *encoding = argv[1];
	if (strcasecmp(encoding,"utf-8") == 0 || strcasecmp(encoding,"utf8") == 0) {
		decode_codepoint = decode_utf8_codepoint;
	}
	else if (strcasecmp(encoding,"utf-16le") == 0 || strcasecmp(encoding,"utf16le") == 0) {
		decode_codepoint = decode_utf16le_codepoint;
	}
	else if (strcasecmp(encoding,"utf-16be") == 0 || strcasecmp(encoding,"utf16be") == 0) {
		decode_codepoint = decode_utf16be_codepoint;
	}
	else if (strcasecmp(encoding,"utf-32le") == 0 || strcasecmp(encoding,"utf32le") == 0) {
		decode_codepoint = decode_utf32le_codepoint;
	}
	else if (strcasecmp(encoding,"utf-32be") == 0 || strcasecmp(encoding,"utf32be") == 0) {
		decode_codepoint = decode_utf32be_codepoint;
	}
	else if (strcasecmp(encoding,"latin-1") == 0 || strcasecmp(encoding,"latin1") == 0 ||
	         strcasecmp(encoding,"iso-8859-1") == 0) {
		decode_codepoint = decode_latin1_codepoint;
	}
	else {
		fprintf(stderr, "usage: input encoding not supported: %s\n", encoding);
		return EXIT_FAILURE;
	}

	encoding = argv[2];
	if (strcasecmp(encoding,"utf-8") == 0 || strcasecmp(encoding,"utf8") == 0) {
		encode_codepoint = encode_utf8_codepoint;
	}
	else if (strcasecmp(encoding,"latin-1") == 0 || strcasecmp(encoding,"latin1") == 0 ||
	         strcasecmp(encoding,"iso-8859-1") == 0) {
		encode_codepoint = encode_latin1_codepoint;
	}
	else {
		fprintf(stderr, "usage: output encoding not supported: %s\n", encoding);
		return EXIT_FAILURE;
	}

	size_t errline = 0, errcol = 0, errbyte = 0;

	if (argc > 3) {
		for (int i = 3; i < argc; ++ i) {
			const char *filename = argv[i];
			FILE *input = fopen(filename, "rb");

			if (!input) {
				perror(filename);
				return EXIT_FAILURE;
			}

			enum convert_status status = convert(input, stdout, decode_codepoint, encode_codepoint, &errline, &errcol, &errbyte);
			fclose(input);

			if (status != CONVERT_OK) {
				fflush(stdout);
				print_convert_error(filename, errline, errcol, errbyte, status);
				return EXIT_FAILURE;
			}
		}
	}
	else {
		enum convert_status status = convert(stdin, stdout, decode_codepoint, encode_codepoint, &errline, &errcol, &errbyte);

		if (status != CONVERT_OK) {
			fflush(stdout);
			print_convert_error("<stdin>", errline, errcol, errbyte, status);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

#endif

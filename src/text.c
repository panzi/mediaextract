#ifndef MEDIAEXTRACT_TO_UTF8_BIN
#include <strings.h>
#endif

#include "text.h"

typedef uint32_t codepoint_t;

static const uint8_t *decode_utf8_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf16le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf16be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf32le_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);
static const uint8_t *decode_utf32be_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);

static int text_isfile(const uint8_t *data, size_t input_len, struct file_info *info,
	const uint8_t *(*decode_codepoint)(const uint8_t *str, size_t size, codepoint_t *cpptr),
	const char *ext, const char *cr_ext, const char *crlf_ext, const char *lf_ext);

const uint8_t *decode_utf8_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr) {
	if (size == 0) {
		return NULL;
	}

	uint8_t byte = *str;

	if (byte >= 0xfc) {
		return NULL;
	}

	codepoint_t cp = 0;

#define decode_next_byte \
		byte = *str; \
		if ((byte & 0xc0) != 0x80) { \
			return NULL; \
		} \
		cp <<= 6; \
		cp |= byte & 0x3f; \
		++ str;

	++ str;
	if ((byte & 0xfe) == 0xfc) {
		if (size < 6) {
			return NULL;
		}

		cp = byte & 0x01;

		decode_next_byte;
		decode_next_byte;
		decode_next_byte;
		decode_next_byte;
		decode_next_byte;

		if (cp > CODEPOINT_MAX) {
			return NULL;
		}
	}
	else if ((byte & 0xfc) == 0xf8) {
		if (size < 5) {
			return NULL;
		}

		cp = byte & 0x03;

		decode_next_byte;
		decode_next_byte;
		decode_next_byte;
		decode_next_byte;
	}
	else if ((byte & 0xf8) == 0xf0) {
		if (size < 4) {
			return NULL;
		}

		cp = byte & 0x07;

		decode_next_byte;
		decode_next_byte;
		decode_next_byte;
	}
	else if ((byte & 0xf0) == 0xe0) {
		if (size < 3) {
			return NULL;
		}

		cp = byte & 0x0f;

		decode_next_byte;
		decode_next_byte;
	}
	else if ((byte & 0xe0) == 0xc0) {
		if (size < 2) {
			return NULL;
		}

		cp = byte & 0x1f;

		decode_next_byte;
	}
	else {
		cp = byte;
	}

#undef decode_next_byte

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

#ifndef MEDIAEXTRACT_TO_UTF8_BIN

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

	size_t i;

#define encode_first_byte(prefix) \
		if (size > i) { \
			buf[i] = prefix | codepoint; \
		}

#define encode_prev_byte \
		if (size > i) { \
			buf[i] = 0x80 | (codepoint & 0x3f); \
		} \
		codepoint >>= 6; \
		-- i;

	if (codepoint > 0x3FFFFFF) {
		i = 5;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_first_byte(0xfc);
		return 6;
	}
	else if (codepoint > 0x1FFFFF) {
		i = 4;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_first_byte(0xf8);
		return 5;
	}
	else if (codepoint > 0xFFFF) {
		i = 3;
		encode_prev_byte;
		encode_prev_byte;
		encode_prev_byte;
		encode_first_byte(0xf0);
		return 4;
	}
	else if (codepoint > 0x07FF) {
		i = 2;
		encode_prev_byte;
		encode_prev_byte;
		encode_first_byte(0xe0);
		return 3;
	}
	else if (codepoint > 0x007F) {
		i = 1;
		encode_prev_byte;
		encode_first_byte(0xc0);
		return 2;
	}
	else {
		if (size > 0) {
			buf[0] = codepoint;
		}
		return 1;
	}

#undef encode_first_byte
#undef encode_next_byte
}

enum convert_status {
	CONVERT_OK           = 0,
	CONVERT_DECODE_ERROR = 1,
	CONVERT_ENCODE_ERROR = 2,
	CONVERT_READ_ERROR   = 3,
	CONVERT_WRITE_ERROR  = 4
};

#define MAX_ENCODED_BYTES 6

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

			byte += next - ptr;
			if (cp == '\n') {
				++ line;
				col = 1;
			}
			else {
				++ col;
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

	if (argc < 2) {
		fprintf(stderr, "usage: to-utf8 <input-encoding> [file]...\n");
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
	else {
		fprintf(stderr, "usage: encoding not supported: %s\n", encoding);
		return EXIT_FAILURE;
	}

	size_t errline = 0, errcol = 0, errbyte = 0;

	if (argc > 2) {
		for (size_t i = 2; i < argc; ++ i) {
			const char *filename = argv[i];
			FILE *input = fopen(filename, "rb");

			if (!input) {
				perror(filename);
				return EXIT_FAILURE;
			}

			enum convert_status status = convert(input, stdout, decode_codepoint, encode_utf8_codepoint, &errline, &errcol, &errbyte);
			fclose(input);

			if (status != CONVERT_OK) {
				fflush(stdout);
				print_convert_error(filename, errline, errcol, errbyte, status);
				return EXIT_FAILURE;
			}
		}
	}
	else {
		enum convert_status status = convert(stdin, stdout, decode_codepoint, encode_utf8_codepoint, &errline, &errcol, &errbyte);

		if (status != CONVERT_OK) {
			fflush(stdout);
			print_convert_error("<stdin>", errline, errcol, errbyte, status);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

#endif

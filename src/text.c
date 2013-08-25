#include "text.h"

typedef uint32_t codepoint_t;

static const uint8_t *decode_utf8_codepoint(const uint8_t *str, size_t size, codepoint_t *cpptr);

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

int ascii_isfile(const uint8_t *data, size_t input_len, struct file_info *info) {
	int cr   = 0;
	int crlf = 0;
	int lf   = 0;
	const uint8_t *ptr = data;
	const uint8_t *end = data + input_len;

	while (ptr < end) {
		uint8_t ch = *ptr;

		if (IS_NON_PRINTABLE_CODEPOINT(ch) || ch >= 0x80) {
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

#include "riff.h"

/* see: http://www.johnloomis.org/cpe102/asgn/asgn1/riff.html */

struct fourcc {
	const char *fourcc;
	const char *ext;
};

static const struct fourcc fourccs[] = {
	{ "WAVE", "wav" },
	{ "AVI ", "avi" },
	{ "ACON", "ani" },
	{ "RMID", "rmi" },
	{ "PAL ", "pal" },
	{ "CPPO", "riff" }, /* don't know the correct extension */
	{ "RDIB", "rdi" },
	{ "RMMP", "mmm" },
	{ 0, 0 }
};

int riff_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	if (input_len < RIFF_HEADER_SIZE)
		return 0;

	if (MAGIC(data) != RIFF_MAGIC)
		return 0;

	size_t length = le32toh(*(const uint32_t *)(data + 4)) + 8;

	if (input_len < length)
		return 0;

	uint32_t magic = MAGIC(data + 8);
	const char *ext = NULL;
	for (const struct fourcc *fourcc = fourccs; fourcc->fourcc; ++ fourcc)
	{
		if (MAGIC(fourcc->fourcc) == magic)
		{
			ext = fourcc->ext;
			break;
		}
	}

	if (!ext)
		return 0;

	if (info)
	{
		info->length = length;
		info->ext    = ext;
	}

	return 1;
}

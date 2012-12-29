#include "mod.h"

int mod_isfile(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t input_len = (size_t)(end - start);
	size_t length = MOD_MAGIC_OFFSET + 4;
	size_t channels = 0;
	size_t patterns = 0;

	if (input_len < length)
		return 0;

	for (const unsigned char *ptr = start + 20, *sample_end = ptr + 31*30;
		ptr < sample_end; ptr += 30)
	{
		length += (size_t)be16toh(*(const uint16_t*)(ptr + 22)) << 1;
	}

	uint8_t song_length = start[950];
	if (song_length > 0x80)
		return 0;

	for (const unsigned char *ptr = start + 952, *pattern_table_end = ptr + 128;
		ptr < pattern_table_end; ++ ptr)
	{
		if (*ptr > patterns) patterns = *ptr;
	}
	++ patterns;

	uint32_t magic = MAGIC(start + MOD_MAGIC_OFFSET);

	if      (IS_MOD_4CH_MAGIC(magic))  channels =  4;
	else if (IS_MOD_5CH_MAGIC(magic))  channels =  5;
	else if (IS_MOD_6CH_MAGIC(magic))  channels =  6;
	else if (IS_MOD_7CH_MAGIC(magic))  channels =  7;
	else if (IS_MOD_8CH_MAGIC(magic))  channels =  8;
	else if (IS_MOD_16CH_MAGIC(magic)) channels = 16;
	else if (IS_MOD_32CH_MAGIC(magic)) channels = 32;
	else
		return 0;

	length += patterns * channels * 64 * 4;
	if (input_len < length)
		return 0;
	
	if (lengthptr) *lengthptr = length;

	return 1;
}

#include "bink.h"

int bink_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < BINK_HEADER_SIZE)
		return 0;

	uint32_t magic = MAGIC(data);
	if (!IS_BINK_MAGIC(magic))
		return 0;
	
	size_t length      = le32toh(*(uint32_t *)(data +  4)) + 8;
	size_t frame_count = le32toh(*(uint32_t *)(data +  8));
	size_t width       = le32toh(*(uint32_t *)(data + 20));
	size_t height      = le32toh(*(uint32_t *)(data + 24));
	size_t fps_num     = le32toh(*(uint32_t *)(data + 28));
	size_t fps_den     = le32toh(*(uint32_t *)(data + 32));

	if (input_len < length ||
		frame_count == 0 ||
		width  > 7680 ||
		height > 4800 ||
		fps_num == 0  ||
		fps_den == 0)
	{
		return 0;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

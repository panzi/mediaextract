#include "bink.h"

int bink_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	if (input_len < BINK_HEADER_SIZE)
		return 0;

	uint32_t magic = MAGIC(data);
	if (!IS_BINK_MAGIC(magic))
		return 0;
	
	size_t   length      = le16toh(*(uint32_t *)(data +  4)) + 8;
	uint32_t frame_count = le16toh(*(uint32_t *)(data +  8));
	uint32_t width       = le16toh(*(uint32_t *)(data + 20));
	uint32_t height      = le16toh(*(uint32_t *)(data + 24));
	uint32_t fps_num     = le16toh(*(uint32_t *)(data + 28));
	uint32_t fps_den     = le16toh(*(uint32_t *)(data + 32));

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

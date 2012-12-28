#include "ogg.h"

int ogg_ispage(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	unsigned char nsegs;
	size_t length, i, input_len = (size_t)(end - start);
	const unsigned char *segs = start + OGG_HEADER_SIZE;

	/* full header available? */
	if (input_len < OGG_HEADER_SIZE)
		return 0;

	/* capture pattern */
	if (*(const int32_t *)start != OGG_MAGIC)
		return 0;

	/* stream structure version */
	if (start[4] != 0x00)
		return 0;

	/* header type flag */
	if ((start[5] & ~7) != 0x00)
		return 0;
	
	nsegs = start[26];
	length = OGG_HEADER_SIZE + nsegs;

	/* segment sizes fully available? */
	if (input_len < length)
		return 0;

	for (i = 0; i < nsegs; ++ i)
	{
		length += segs[i];
	}

	/* segments fully available? */
	if (input_len < length)
		return 0;
	
	if (lengthptr)
		*lengthptr = length;

	/* I think we can reasonably assume it is a real page now */
	return 1;
}

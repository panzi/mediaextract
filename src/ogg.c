#include "ogg.h"

/* see: http://www.xiph.org/ogg/doc/framing.html */

int ogg_ispage(const uint8_t *data, size_t input_len, struct ogg_info *pageinfo)
{
	uint8_t nsegs;
	size_t length, i;
	const uint8_t *segs = data + OGG_HEADER_SIZE;

	/* full header available? */
	if (input_len < OGG_HEADER_SIZE)
		return 0;

	/* capture pattern */
	if (MAGIC(data) != OGG_MAGIC)
		return 0;

	/* stream structure version */
	if (data[4] != 0x00)
		return 0;

	/* valid header type flags */
	if ((data[5] & ~7) != 0x00)
		return 0;
	
	nsegs  = data[26];
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
	
	if (pageinfo)
	{
		pageinfo->length = length;
		pageinfo->pageno = le32toh(*(const uint32_t *)(data + 18));
	}

	/* I think we can reasonably assume it is a real page now */
	return 1;
}

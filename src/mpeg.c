#include "mpeg.h"

// See: http://www.andrewduncan.ws/MPEG/MPEG-2_Picts.html
//      http://lostcode.wikidot.com/transportstream
//      http://en.wikipedia.org/wiki/MPEG_transport_stream
//      http://dmr.ath.cx/notes/mpeg1.html

static size_t mpeg_ispacket(const uint8_t *data, size_t input_len)
{
	if (input_len < 6)
		return 0;
	
	uint32_t magic = MAGIC(data);
	if (magic < MPEG_PKG_MIN_MAGIC || magic > MPEG_PKG_MAX_MAGIC)
		return 0;
	
	size_t length = 6 + ((data[4] << 8) | data[5]);

	if (length <= 6 || length > input_len)
		return 0;
	
	// truncated file?
	if (length > input_len)
		return input_len;

	return length;
}

static size_t mpeg_ispack(const uint8_t *data, size_t input_len, enum fileformat *format)
{
	if (input_len < 12 || MAGIC(data) != MPEG_MAGIC)
		return 0;

	size_t length = 12;
	
	switch (data[4] >> 6)
	{
		case 0: // MPEG 1
			*format = MPEG1;
			break;

		case 1: // MPEG 2
			if (input_len < 14)
				return 0;

			// stuffing
			length = 14 + (data[13] & 0x3);

			if (length > input_len)
				return 0;

			if (!(data[4] & 0x4) || !(data[ 6] & 0x4) || !(data[8] & 0x4) ||
				!(data[9] & 0x1) || !(data[12] & 0x3))
				return 0;
			*format = MPEGPS;
			break;

		default:
			return 0;
	}

	const uint8_t *ptr = data + length;
	const uint8_t *end = data + input_len - 6;

	while (ptr < end)
	{
		size_t pkglen = mpeg_ispacket(ptr, input_len - length);

		if (pkglen == 0)
			break;

		length += pkglen;
		ptr    += pkglen;
	}

	return length;
}

static size_t mpeg_ispacks(const uint8_t *data, size_t input_len, int formats)
{
	enum fileformat format = NONE;
	size_t length = mpeg_ispack(data, input_len, &format);
	
	if (length == 0 || (format & formats) == 0)
		return 0;

	while (length < input_len)
	{
		// ignore up to 64 nil bytes
		size_t maxskip = input_len - length;
		if (maxskip > 66) maxskip  = length + 66;
		else              maxskip += length;

		size_t i = length;
		while (i < maxskip && data[i] == 0)
			++ i;

		// there are 2 nil bytes at the start of the magic
		if ((i - length) < 2)
			break;
		length = i - 2;

		enum fileformat nextformat = NONE;
		size_t nextlen = mpeg_ispack(data + length, input_len - length, &nextformat);

		if (nextlen == 0 || nextformat != format)
			break;

		length += nextlen;
	}

	if (length <= input_len - 4 && MAGIC(data + length) == MPEG_END_MAGIC)
		length += 4;

	return length;
}

#if 0
// TODO: find out how to determine the sizes of all the parts of a MPEG VS
static size_t mpeg_isvshdr(const uint8_t *data, size_t input_len)
{
	if (input_len < 12 || MAGIC(data) != MPEG_VS_HDR_MAGIC)
		return 0;

	size_t width   = (data[4] <<  4) | (data[5] >> 4);
	size_t height  = (data[5] <<  4) |  data[6];
	size_t bitrate = (data[8] << 10) | (data[9] << 2) | (data[10] >> 6);

	if (!width || !height || !bitrate || !(data[10] & 0x20))
		return 0;
	
	if (data[11] & 0x2)
	{
		if (input_len < 76)
			return 0;

		if (data[75] & 0x1)
		{
			if (input_len < 140)
				return 0;
			
			return 140;
		}

		return 76;
	}

	return 12;
}

static size_t mpeg_isvs(const uint8_t *data, size_t input_len)
{
	size_t length = mpeg_isvshdr(data, input_length);

	if (!length)
		return 0;

	// TODO

	return length;
}
#endif

static size_t mpeg_istspack(const uint8_t *data, size_t input_len)
{
	// There are versions with 192 and 208 bytes that prepend/append
	// a 4 byte timecode or 20 byte for error correction. Because
	// I don't have any example files and don't know how to detect
	// these variations I haven't implemented them for now. See:
	// http://lostcode.wikidot.com/transportstream
	if (input_len < 188 || data[0] != MPEG_TS_SYNC_BYTE || !(data[3] & 0x30))
		return 0;

	if (data[3] & 0x20)
	{
		size_t hdrlen = data[4];

		if (hdrlen == 0 || hdrlen > 183)
			return 0;
	}

	return 188;
}

static size_t mpeg_ists(const uint8_t *data, size_t input_len)
{
	size_t length = 0;
	
	while (length < input_len)
	{
		size_t pkglen = mpeg_istspack(data + length, input_len - length);

		if (!pkglen)
			break;

		length += pkglen;
	}

	return length;
}

int mpeg_isfile(const uint8_t *data, size_t input_len, int formats, size_t *lengthptr)
{
	size_t length = 0;

	if (formats & (MPEG1 | MPEGPS))
		length = mpeg_ispacks(data, input_len, formats);

//	if (!length && formats & MPEGVS)
//		length = mpeg_isvs(data, input_len);
	
	if (!length && formats & MPEGTS)
		length = mpeg_ists(data, input_len);
	
	if (length)
	{
		if (lengthptr)
			*lengthptr = length;

		return 1;
	}

	return 0;
}

/* Copyright (c) 2020 Mathias Panzenböck
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

#include "dds.h"
#include "byteorder.h"

#include <string.h>

#pragma pack(push, 1)
typedef struct {
	/*   0 */ uint32_t size;
	/*   4 */ uint32_t flags;
	/*   8 */ char     four_cc[4];
	/*  12 */ uint32_t rgb_bit_count;
	/*  16 */ uint32_t r_bit_mask;
	/*  20 */ uint32_t g_bit_mask;
	/*  24 */ uint32_t b_bit_mask;
	/*  28 */ uint32_t a_bit_mask;
} DDS_PIXELFORMAT;

typedef struct {
	/*   0 */ uint32_t        size;
	/*   4 */ uint32_t        flags;
	/*   8 */ uint32_t        height;
	/*  12 */ uint32_t        width;
	/*  16 */ uint32_t        pitch_or_linear_size;
	/*  20 */ uint32_t        depth;
	/*  24 */ uint32_t        mip_map_count;
	/*  28 */ uint32_t        reserved1[11];
	/*  72 */ DDS_PIXELFORMAT ddspf;
	/* 104 */ uint32_t        caps;
	/* 108 */ uint32_t        caps2;
	/* 112 */ uint32_t        caps3;
	/* 116 */ uint32_t        caps4;
	/* 120 */ uint32_t        reserved2;
} DDS_HEADER;

typedef struct {
	/*   0 */ uint32_t dxgi_format;
	/*   4 */ uint32_t resource_dimension;
	/*   8 */ uint32_t misc_flag;
	/*  12 */ uint32_t array_size;
	/*  16 */ uint32_t misc_flags2;
} DDS_HEADER_DXT10;
#pragma pack(pop)

#define GET_LE32(PTR) ((uint32_t)(PTR)[0] | ((uint32_t)(PTR)[1] << 8) | ((uint32_t)(PTR)[2] << 16) | ((uint32_t)(PTR)[3] << 24))

#define DXGI_FORMAT_MAX 119
#define RESOURCE_DIMENSIONS_MAX 4

int dds_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr) {
	if (input_len < sizeof(DDS_HEADER) + 4)
		return 0;

	if (memcmp(data, "DDS ", 4) != 0) {
		return 0;
	}

	const uint8_t *hdrptr = data + 4;

#define HDR_FIELD_LE32(name) .name = GET_LE32(hdrptr + offsetof(DDS_HEADER, name))
#define HDR_FIELD_BYTE(name) .name = hdrptr[offsetof(DDS_HEADER, name)]

	const DDS_HEADER header = {
		HDR_FIELD_LE32(size),
		HDR_FIELD_LE32(flags),
		HDR_FIELD_LE32(height),
		HDR_FIELD_LE32(width),
		HDR_FIELD_LE32(pitch_or_linear_size),
		HDR_FIELD_LE32(depth),
		HDR_FIELD_LE32(mip_map_count),

		HDR_FIELD_LE32(reserved1[0]),
		HDR_FIELD_LE32(reserved1[1]),
		HDR_FIELD_LE32(reserved1[2]),
		HDR_FIELD_LE32(reserved1[3]),
		HDR_FIELD_LE32(reserved1[4]),
		HDR_FIELD_LE32(reserved1[5]),
		HDR_FIELD_LE32(reserved1[6]),
		HDR_FIELD_LE32(reserved1[7]),
		HDR_FIELD_LE32(reserved1[8]),
		HDR_FIELD_LE32(reserved1[9]),
		HDR_FIELD_LE32(reserved1[10]),

		HDR_FIELD_LE32(ddspf.size),
		HDR_FIELD_LE32(ddspf.flags),

		HDR_FIELD_BYTE(ddspf.four_cc[0]),
		HDR_FIELD_BYTE(ddspf.four_cc[1]),
		HDR_FIELD_BYTE(ddspf.four_cc[2]),
		HDR_FIELD_BYTE(ddspf.four_cc[3]),

		HDR_FIELD_LE32(ddspf.rgb_bit_count),
		HDR_FIELD_LE32(ddspf.r_bit_mask),
		HDR_FIELD_LE32(ddspf.g_bit_mask),
		HDR_FIELD_LE32(ddspf.b_bit_mask),
		HDR_FIELD_LE32(ddspf.a_bit_mask),

		HDR_FIELD_LE32(caps),
		HDR_FIELD_LE32(caps2),
		HDR_FIELD_LE32(caps3),
		HDR_FIELD_LE32(caps4),
		HDR_FIELD_LE32(reserved2),
	};

	if (header.height == 0 || header.width == 0 || header.mip_map_count == 0) {
		return 0;
	}

	size_t size = sizeof(DDS_HEADER) + 4;
	size_t array_size = 1;
	size_t block_size;

	if (memcmp(header.ddspf.four_cc, "DX10", 4) == 0) {
		if (input_len < sizeof(DDS_HEADER) + 4 + sizeof(DDS_HEADER_DXT10))
			return 0;

#define DXT10_FIELD_LE32(name) .name = GET_LE32(hdrptr + sizeof(DDS_HEADER) + offsetof(DDS_HEADER_DXT10, name))

		const DDS_HEADER_DXT10 header10 = {
			DXT10_FIELD_LE32(dxgi_format),
			DXT10_FIELD_LE32(resource_dimension),
			DXT10_FIELD_LE32(misc_flag),
			DXT10_FIELD_LE32(array_size),
			DXT10_FIELD_LE32(misc_flags2),
		};

		if (
				header10.dxgi_format > DXGI_FORMAT_MAX ||
				header10.resource_dimension > RESOURCE_DIMENSIONS_MAX ||
				header10.array_size == 0) {
			return 0;
		}

		size += sizeof(DDS_HEADER_DXT10);
		array_size = header10.array_size;

		// XXX: I'm not sure about this. The docs only explicitly mention DXT1-5.
		// SEE: https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-file-layout-for-textures
		block_size = 16;
	} else if (memcmp(header.ddspf.four_cc, "DXT1", 4) == 0) {
		block_size = 8;
	} else if (
			memcmp(header.ddspf.four_cc, "DXT", 3) != 0 &&
			(header.ddspf.four_cc[3] < '1' || header.ddspf.four_cc[3] > '5')) {
		return 0;
	} else {
		block_size = 16;
	}

	// TODO: more sanity checks
	// TODO: uncompressed textures!?!
	// TODO: cube and other types
	// TODO: depth!?!

	for (uint32_t array_index = 0; array_index < array_size; ++ array_index) {
		uint32_t mipmap_width  = header.width;
		uint32_t mipmap_height = header.height;

		for (uint32_t mipmap_index = 0; mipmap_index < header.mip_map_count; ++ mipmap_index) {
			const uint32_t w = (mipmap_width  + 3) / 4;
			const uint32_t h = (mipmap_height + 3) / 4;
			const size_t mipmap_size = (w < 1 ? 1 : w) * (h < 1 ? 1 : h) * block_size;

			size += mipmap_size;

			mipmap_width  /= 4;
			mipmap_height /= 4;
		}
	}

	if (lengthptr) {
		*lengthptr = size;
	}

	return 1;
}
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

#include "asf.h"

// See: http://msdn.microsoft.com/en-us/library/bb643323.aspx
//      http://www.microsoft.com/en-us/download/details.aspx?id=14995

enum asf_file_property_flag {
	ASF_BROADCAST_FLAG = 1,
	ASF_SEEKABLE_FLAG  = 2
};

struct asf_object {
	uint8_t  object_id[ASF_GUID_SIZE];
	uint64_t object_size;
};

struct asf_header {
	uint8_t  object_id[ASF_GUID_SIZE];
	uint64_t object_size;
	uint32_t header_object_count;
	uint8_t  reserved1;
	uint8_t  reserved2;
};

struct asf_file_properties {
	uint8_t  object_id[ASF_GUID_SIZE];
	uint64_t object_size;
	uint8_t  file_id[ASF_GUID_SIZE];
	uint64_t file_size;
	uint64_t creation_date;
	uint64_t data_packets_count;
	uint64_t play_duration;
	uint64_t send_duration;
	uint64_t preroll;
	uint32_t flags;
	uint32_t min_data_packet_size;
	uint32_t max_data_packet_size;
	uint32_t max_bitrate;
};

const char *asf_body_object_guids[] = {
	ASF_Data_Object_GUID,
	ASF_Simple_Index_Object_GUID,
	ASF_Index_Object_GUID,
	ASF_Media_Object_Index_Object_GUID,
	ASF_Timecode_Index_Object_GUID,
	NULL
};

static int asf_isbody(const struct asf_object *object)
{
	for (const char **guid = asf_body_object_guids; *guid; ++ guid)
	{
		if (memcmp(object->object_id, *guid, ASF_GUID_SIZE) == 0)
			return 1;
	}

	return 0;
}

int asf_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr)
{
	const struct asf_header *header = (const struct asf_header *)data;

	if (input_len < ASF_HEADER_SIZE || memcmp(header->object_id, ASF_Header_Object_GUID, ASF_GUID_SIZE) != 0)
		return 0;

	uint64_t header_object_size  = le64toh(header->object_size);
	size_t   header_object_count = le32toh(header->header_object_count);

	if (header_object_size < 30 || input_len < header_object_size)
		return 0;
	
	const uint8_t *ptr = data + 30;
	const struct asf_file_properties *file_properties = NULL;
	for (size_t i = 0; i < header_object_count; ++ i)
	{
		const struct asf_object *object = (const struct asf_object *)ptr;
		size_t off = (size_t)(ptr - data);

		if ((size_t)(-1) - ASF_OBJECT_SIZE < off)
			return 0;

		if (off + ASF_OBJECT_SIZE > header_object_size)
			return 0;

		if (memcmp(object->object_id, ASF_File_Properties_Object_GUID, ASF_GUID_SIZE) == 0)
		{
			file_properties = (const struct asf_file_properties *)object;
		}

		uint64_t object_size = le64toh(object->object_size);

		if ((size_t)(-1) - object_size < off)
			return 0;

		if (off + object_size > header_object_size)
			return 0;

		ptr += object_size;
	}

	size_t length = 0;
	if (file_properties == NULL || file_properties->flags & ASF_BROADCAST_FLAG)
	{
		/* Need to parse toplevel objects to detect file size.
		 * If there are objects with unknown object IDs this will
		 * fail (truncate the file). */
		for (;;)
		{
			size_t off = (size_t)(ptr - data);

			if ((size_t)(-1) - ASF_OBJECT_SIZE < off)
				break;

			if (off + ASF_OBJECT_SIZE > input_len)
				break;

			const struct asf_object *object = (const struct asf_object *)ptr;

			if (!asf_isbody(object))
				break;

			uint64_t object_size = le64toh(object->object_size);

			if ((size_t)(-1) - object_size < off)
				break;

			if (off + object_size > header_object_size)
				break;

			ptr += object_size;
		}
		length = (size_t)(ptr - data);
	}
	else
	{
		length = le64toh(file_properties->file_size);

		if (length < header_object_size)
			return 0;
	}

	if (lengthptr) *lengthptr = length;

	return 1;
}

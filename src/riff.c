#include "riff.h"

/* see: http://www.johnloomis.org/cpe102/asgn/asgn1/riff.html */

struct riff_chunk_spec {
	uint32_t type;
	const struct riff_chunk_spec *body;
	int required;
};

struct riff_file_spec {
	struct riff_chunk_spec spec;
	const char *ext;
};

static const struct riff_chunk_spec riff_empty_body[] = {
	{ 0, 0, 0 }
};

/* WAVE */
static const struct riff_chunk_spec riff_wav_body[] = {
	{ CMAGIC('f','m','t',' '), 0, 1 },
	{ 0, 0, 0 }
};

/* AVI */
// TODO: AVI 2.0? only makes sense for files > 4 GB
// http://www.the-labs.com/Video/odmlff2-avidef.pdf
static const struct riff_chunk_spec riff_avi_hdrl_body[] = {
	{ CMAGIC('a','v','i','h'), 0, 1 },
	{ 0, 0, 0 }
};

static const struct riff_chunk_spec riff_avi_body[] = {
	{ CMAGIC('h','d','r','l'), 0, 1 },
	{ CMAGIC('m','o','v','i'), 0, 1 },
	{ 0, 0, 0 }
};

/* ACON */
static const struct riff_chunk_spec riff_ani_fram_body[] = {
	{ CMAGIC('i','c','o','n'), 0, 1 },
	{ 0, 0, 0 }
};

static const struct riff_chunk_spec riff_ani_body[] = {
	{ CMAGIC('I','N','F','O'), riff_empty_body, 0 },
	{ CMAGIC('a','n','i','h'), 0, 1 },
	{ CMAGIC('f','r','a','m'), riff_ani_fram_body, 1 },
	{ 0, 0, 0 }
};

/* PAL */
static const struct riff_chunk_spec riff_pal_body[] = {
	{ CMAGIC('d','a','t','a'), 0, 1 },
	{ 0, 0, 0 }
};

static const struct riff_file_spec riff_file_specs[] = {
	{ { CMAGIC('W','A','V','E'), riff_wav_body,   1 }, "wav" },
	{ { CMAGIC('A','V','I',' '), riff_avi_body,   1 }, "avi" },
	{ { CMAGIC('A','C','O','N'), riff_ani_body,   1 }, "ani" },
	{ { CMAGIC('R','M','I','D'), riff_empty_body, 1 }, "rmi" },
	{ { CMAGIC('P','A','L',' '), riff_pal_body,   1 }, "pal" },
	{ { CMAGIC('R','D','I','B'), riff_empty_body, 1 }, "rdi" },
	{ { CMAGIC('R','M','M','P'), riff_empty_body, 1 }, "mmm" },
	{ { 0, 0, 0 }, 0 }
};

const uint8_t *riff_match(const uint8_t *data, size_t size,
	const struct riff_chunk_spec *spec, uint32_t LIST)
{
	if (spec->body)
	{
		if (size < 12 || MAGIC(data) != LIST)
			return NULL;

		uint32_t chunk_size = le32toh(*(const uint32_t *)(data + 4));
		uint32_t type = MAGIC(data + 8);

		if (type != spec->type)
			return NULL;

		if (chunk_size > size - 8)
			return NULL;

		// match sub chunks
		size_t count = 0;
		for (const struct riff_chunk_spec *subspec = spec->body; subspec->type; ++ subspec)
			++ count;

		uint32_t *counts = (uint32_t *)calloc(count, sizeof(uint32_t));

		if (!counts) {
			perror("parsing RIFF structure");
			return NULL;
		}

		const uint8_t *end = data + chunk_size + 8;
		for (const uint8_t *subdata = data + 12; subdata < end;)
		{
			size_t subsize = (size_t)(end - subdata);

			if (subsize < 8)
				break;

			size_t subchunk_size = le32toh(*(const uint32_t *)(subdata + 4));

			if (subchunk_size > subsize - 8)
			{
				free(counts);
				return NULL;
			}

			for (size_t i = 0;; ++ i)
			{
				const struct riff_chunk_spec *subspec = spec->body + i;

				if (!subspec->type)
					break;

				if (riff_match(subdata, subsize, subspec, RIFF_LIST_MAGIC))
				{
					++ counts[i];
					break;
				}
			}

			subdata += subchunk_size + 8;
		}
		
		for (size_t i = 0;; ++ i)
		{
			const struct riff_chunk_spec *subspec = spec->body + i;

			if (!subspec->type)
				break;

			if (subspec->required && counts[i] == 0)
			{
				free(counts);
				return NULL;
			}
		}

		free(counts);

#if 0
		// If the sub chunk matching code above delivers to many false positives this code could be used
		// instead. This code is not really correct, because sub chunks could theretically occure in any
		// order, but most of the time they don't.

		const uint8_t *end = data + size;
		const uint8_t *subdata = data + 12;
		for (const struct riff_chunk_spec *subspec = spec->body; subspec->type; ++ subspec)
		{
			const uint8_t *subend = riff_match(subdata, (size_t)(end - subdata), subspec, RIFF_LIST_MAGIC);
			if (!subend && subspec->required) return NULL;
			else subdata = subend;
		}
#endif

		return data + (chunk_size + 8);
	}
	else
	{
		if (size < 8 || MAGIC(data) != spec->type)
			return NULL;
	
		uint32_t chunk_size = le32toh(*(const uint32_t *)(data + 4));

		if (chunk_size > size - 8)
			return NULL;

		return data + (chunk_size + 8);
	}
}

int riff_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	for (const struct riff_file_spec *file_spec = riff_file_specs; file_spec->spec.type; ++ file_spec)
	{
		const uint8_t *end = riff_match(data, input_len, &(file_spec->spec), RIFF_MAGIC);
		if (end) {
			if (info)
			{
				info->length = (size_t)(end - data);
				info->ext    = file_spec->ext;
			}
			return 1;
		}
	}

	return 0;
}

#include "riff.h"

/* see: http://www.johnloomis.org/cpe102/asgn/asgn1/riff.html */
// possible TODO: AVI 2.0, RIFX (big endian), RF64, BWF

#define CHUNK_SPEC(metatype,c1,c2,c3,c4,body,required) { (metatype), CMAGIC(c1,c2,c3,c4), (body), (required) }
#define LIST(c1,c2,c3,c4,body,required) CHUNK_SPEC(CMAGIC('L','I','S','T'),c1,c2,c3,c4,body,required)
#define FORM(c1,c2,c3,c4,body,required) CHUNK_SPEC(CMAGIC('R','I','F','F'),c1,c2,c3,c4,body,required)
#define RIFF(c1,c2,c3,c4,body)          FORM(c1,c2,c3,c4,body,1)
#define CHUNK(c1,c2,c3,c4,required)     CHUNK_SPEC(0,c1,c2,c3,c4,0,required)
#define END { 0, 0, 0, 0 }
#define BODY(...) { __VA_ARGS__, END }

struct riff_chunk_spec {
	uint32_t metatype;
	uint32_t type;
	const struct riff_chunk_spec *body;
	int required;
};

struct riff_file_spec {
	struct riff_chunk_spec spec;
	const char *ext;
};

static const struct riff_chunk_spec riff_empty_body[] = { END };

/* WAVE */
static const struct riff_chunk_spec riff_wav_body[] = BODY(
	CHUNK('f','m','t',' ', 1)
);

/* AVI */
// TODO: AVI 2.0? only makes sense for files > 4 GB
// http://www.the-labs.com/Video/odmlff2-avidef.pdf
static const struct riff_chunk_spec riff_avi_hdrl_body[] = BODY(
	CHUNK('a','v','i','h', 1)
);

static const struct riff_chunk_spec riff_avi_body[] = BODY(
	CHUNK('h','d','r','l', 1),
	CHUNK('m','o','v','i', 1)
);

/* ACON */
static const struct riff_chunk_spec riff_ani_fram_body[] = BODY(
	CHUNK('i','c','o','n', 1)
);

static const struct riff_chunk_spec riff_ani_body[] = BODY(
	LIST( 'I','N','F','O', 0,                  0),
	CHUNK('a','n','i','h',                     1),
	LIST( 'f','r','a','m', riff_ani_fram_body, 1)
);

/* PAL */
static const struct riff_chunk_spec riff_pal_body[] = BODY(
	CHUNK('d','a','t','a', 1)
);

/* Direct Music Audiopath (AUD) */
static const struct riff_chunk_spec riff_aud_body[] = BODY(
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	LIST( 'D','M','T','G', 0, 0)
);

/* Direct Music Band */
static const struct riff_chunk_spec riff_dmbd_body[] = BODY(
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	LIST( 'l','b','i','l', 0, 1)
);

/* Direct Music Chrodmap */
static const struct riff_chunk_spec riff_dmpr_body[] = BODY(
	CHUNK('p','e','r','h',    1),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	CHUNK('c','h','d','t',    1),
	LIST( 'c','h','p','l', 0, 1),
	LIST( 'c','m','a','p', 0, 1),
	LIST( 's','p','s','q', 0, 1)
);

/* Direct Music Container */
static const struct riff_chunk_spec riff_dmcn_body[] = BODY(
	CHUNK('c','o','n','h',    1),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	LIST( 'c','o','s','l', 0, 1)
);

/* DirectSound Buffer Configuration */
static const struct riff_chunk_spec riff_dsbc_body[] = BODY(
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	CHUNK('d','s','b','d',    1),
	CHUNK('d','s','i','d',    0),
	CHUNK('d','s','3','d',    0),
	LIST( 'f','x','l','s', 0, 0)
);

/* DirectSound Effects */
static const struct riff_chunk_spec riff_dsfx_body[] = BODY(
	CHUNK('f','x','h','r', 1),
	CHUNK('d','a','t','a', 0)
);

/* Direct Music Script */
static const struct riff_chunk_spec riff_dmsc_body[] = BODY(
	CHUNK('s','c','h','d',    1),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	CHUNK('s','c','v','e',    1),
	FORM( 'D','M','C','N', 0, 1),
	CHUNK('s','c','l','a',    1),
	CHUNK('s','c','s','r',    0),
	LIST( 'D','M','R','F', 0, 0)
);

/* Direct Music Segment (SGT) */
static const struct riff_chunk_spec riff_sgt_body[] = BODY(
	CHUNK('s','e','g','h',    1),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	FORM( 'D','M','C','N', 0, 0),
	LIST( 't','r','k','l', 0, 1),
	FORM( 'D','M','T','G', 0, 0),
	FORM( 'D','M','A','P', 0, 0)
);

/* Direct Music Style (STY) */
static const struct riff_chunk_spec riff_sty_body[] = BODY(
	CHUNK('s','t','y','h',    1),
	CHUNK('g','u','i','d',    1),
	LIST( 'U','N','F','O', 0, 0),
	CHUNK('v','e','r','s',    0),
	LIST( 'p','a','r','t', 0, 1),
	LIST( 'p','t','t','n', 0, 1),
	FORM( 'D','M','B','D', 0, 1),
	LIST( 'p','r','r','f', 0, 0)
);

/* Direct Music Tool */
static const struct riff_chunk_spec riff_dmtl_body[] = BODY(
	CHUNK('t','o','l','h', 1),
	CHUNK('d','a','t','a', 0)
);

/* Direct Music Toolgraph */
static const struct riff_chunk_spec riff_dmtg_body[] = BODY(
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	LIST( 't','o','l','l', 0, 1)
);

/* Direct Music Toolgraph */
static const struct riff_chunk_spec riff_dmtk_body[] = BODY(
	CHUNK('t','r','k','h',    1),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	CHUNK('d','a','t','a',    0)
);

/* Direct Music Band Track */
static const struct riff_chunk_spec riff_dmbt_body[] = BODY(
	CHUNK('b','d','t','h',    0),
	CHUNK('g','u','i','d',    0),
	CHUNK('v','e','r','s',    0),
	LIST( 'U','N','F','O', 0, 0),
	LIST( 'l','b','d','l', 0, 1)
);

/* Direct Music Pattern Track */
static const struct riff_chunk_spec riff_dmpt_body[] = BODY(
	CHUNK('s','t','y','h',    1),
	LIST( 'p','t','t','n', 0, 1)
);

static const struct riff_file_spec riff_file_specs[] = {
	{ RIFF('W','A','V','E', riff_wav_body ), "wav"  },
	{ RIFF('A','V','I',' ', riff_avi_body ), "avi"  },
	{ RIFF('A','C','O','N', riff_ani_body ), "ani"  },
	{ RIFF('R','M','I','D', 0             ), "rmi"  },
	{ RIFF('P','A','L',' ', riff_pal_body ), "pal"  },
	{ RIFF('R','D','I','B', 0             ), "rdi"  },
	{ RIFF('R','M','M','P', 0             ), "mmm"  },
	{ RIFF('D','M','A','P', riff_aud_body ), "aud"  },
	{ RIFF('D','M','B','D', riff_dmbd_body), "riff" },
	{ RIFF('D','M','P','R', riff_dmpr_body), "cdm"  },
	{ RIFF('D','M','C','N', riff_dmcn_body), "riff" },
	{ RIFF('D','S','B','C', riff_dsbc_body), "riff" },
	{ RIFF('D','S','F','X', riff_dsfx_body), "riff" },
	{ RIFF('D','M','S','C', riff_dmsc_body), "riff" },
	{ RIFF('D','M','S','G', riff_sgt_body ), "sgt"  },
	{ RIFF('D','M','S','T', riff_sty_body ), "sty"  },
	{ RIFF('D','M','T','L', riff_dmtl_body), "riff" },
	{ RIFF('D','M','T','G', riff_dmtg_body), "riff" },
	{ RIFF('D','M','T','K', riff_dmtk_body), "riff" },
	{ RIFF('D','M','B','T', riff_dmbt_body), "riff" },
	{ RIFF('D','M','P','T', riff_dmpt_body), "riff" },

	{ END, 0 }
};

const uint8_t *riff_match(const uint8_t *data, size_t size,
	const struct riff_chunk_spec *spec)
{
	if (spec->metatype)
	{
		if (size < 12 || MAGIC(data) != spec->metatype)
			return NULL;

		uint32_t chunk_size = le32toh(*(const uint32_t *)(data + 4));
		uint32_t type = MAGIC(data + 8);

		if (type != spec->type)
			return NULL;

		if (chunk_size > size - 8)
			return NULL;

		// match sub chunks
		const struct riff_chunk_spec *body = spec->body;
		uint32_t *counts = NULL;
		if (body)
		{
			size_t count = 0;
			for (const struct riff_chunk_spec *subspec = body; subspec->type; ++ subspec)
				++ count;

			if (count > 0)
			{
				counts = (uint32_t *)calloc(count, sizeof(uint32_t));
				
				if (!counts) {
					perror("parsing RIFF structure");
					return NULL;
				}
			}
		}
		else
		{
			body = riff_empty_body;
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
				const struct riff_chunk_spec *subspec = body + i;

				if (!subspec->type)
					break;

				if (riff_match(subdata, subsize, subspec))
				{
					++ counts[i];
					break;
				}
			}

			subdata += subchunk_size + 8;
		}
		
		for (size_t i = 0;; ++ i)
		{
			const struct riff_chunk_spec *subspec = body + i;

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
		for (const struct riff_chunk_spec *subspec = body; subspec->type; ++ subspec)
		{
			const uint8_t *subend = riff_match(subdata, (size_t)(end - subdata), subspec);
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
		const uint8_t *end = riff_match(data, input_len, &(file_spec->spec));
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

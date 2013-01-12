#include "mp4.h"

struct ftyp {
	uint32_t brand;
	const char *ext;
};

static const struct ftyp mp4_ftyps[] = {
	{ CMAGIC('3','g','2','a'), "3g2" },
	{ CMAGIC('3','g','2','b'), "3g2" },
	{ CMAGIC('3','g','2','c'), "3g2" },
	{ CMAGIC('3','g','e','6'), "3gp" },
	{ CMAGIC('3','g','e','7'), "3gp" },
	{ CMAGIC('3','g','g','6'), "3gp" },
	{ CMAGIC('3','g','p','1'), "3gp" },
	{ CMAGIC('3','g','p','2'), "3gp" },
	{ CMAGIC('3','g','p','3'), "3gp" },
	{ CMAGIC('3','g','p','4'), "3gp" },
	{ CMAGIC('3','g','p','5'), "3gp" },
	{ CMAGIC('3','g','p','6'), "3gp" },
	{ CMAGIC('3','g','p','6'), "3gp" },
	{ CMAGIC('3','g','p','6'), "3gp" },
	{ CMAGIC('3','g','s','7'), "3gp" },
	{ CMAGIC('a','v','c','1'), 0 },
	{ CMAGIC('C','A','E','P'), 0 },
	{ CMAGIC('c','a','q','v'), 0 },
	{ CMAGIC('C','D','e','s'), 0 },
	{ CMAGIC('d','a','0','a'), 0 },
	{ CMAGIC('d','a','0','b'), 0 },
	{ CMAGIC('d','a','1','a'), 0 },
	{ CMAGIC('d','a','1','b'), 0 },
	{ CMAGIC('d','a','2','a'), 0 },
	{ CMAGIC('d','a','2','b'), 0 },
	{ CMAGIC('d','a','3','a'), 0 },
	{ CMAGIC('d','a','3','b'), 0 },
	{ CMAGIC('d','m','b','1'), 0 },
	{ CMAGIC('d','m','p','f'), 0 },
	{ CMAGIC('d','r','c','1'), "mp4" },
	{ CMAGIC('d','v','1','a'), 0 },
	{ CMAGIC('d','v','1','b'), 0 },
	{ CMAGIC('d','v','2','a'), 0 },
	{ CMAGIC('d','v','2','b'), 0 },
	{ CMAGIC('d','v','3','a'), 0 },
	{ CMAGIC('d','v','3','b'), 0 },
	{ CMAGIC('d','v','r','1'), "dvb" },
	{ CMAGIC('d','v','t','1'), "dvb" },
	{ CMAGIC('F','4','V',' '), "f4v" },
	{ CMAGIC('F','4','P',' '), "f4p" },
	{ CMAGIC('F','4','A',' '), "f4a" },
	{ CMAGIC('F','4','B',' '), "f4b" },
	{ CMAGIC('i','s','c','2'), 0 },
	{ CMAGIC('i','s','o','2'), "mp4" },
	{ CMAGIC('i','s','o','m'), "mp4" },
	{ CMAGIC('J','P','2',' '), "jp2" },
	{ CMAGIC('J','P','2','0'), "jp2" },
	{ CMAGIC('j','p','m',' '), "jpm" },
	{ CMAGIC('j','p','x',' '), "jpx" },
	{ CMAGIC('K','D','D','I'), "3gp" },
	{ CMAGIC('M','4','A',' '), "m4a" },
	{ CMAGIC('M','4','B',' '), "m4b" },
	{ CMAGIC('M','4','P',' '), "m4p" },
	{ CMAGIC('M','4','V',' '), "m4v" },
	{ CMAGIC('M','4','V','H'), "m4v" },
	{ CMAGIC('M','4','V','P'), "m4v" },
	{ CMAGIC('m','j','2','s'), "jp2" },
	{ CMAGIC('m','j','p','2'), "jp2" },
	{ CMAGIC('m','m','p','4'), "mp4" },
	{ CMAGIC('m','p','2','1'), 0 },
	{ CMAGIC('m','p','4','1'), "mp4" },
	{ CMAGIC('m','p','4','2'), "mp4" },
	{ CMAGIC('m','p','7','1'), "mp4" },
	{ CMAGIC('M','P','P','I'), 0 },
	{ CMAGIC('m','q','t',' '), "mqv" },
	{ CMAGIC('M','S','N','V'), "mp4" },
	{ CMAGIC('N','D','A','S'), 0 },
	{ CMAGIC('N','D','S','C'), "mp4" },
	{ CMAGIC('N','D','S','H'), "mp4" },
	{ CMAGIC('N','D','S','M'), "mp4" },
	{ CMAGIC('N','D','S','P'), "mp4" },
	{ CMAGIC('N','D','S','S'), "mp4" },
	{ CMAGIC('N','D','X','C'), "mp4" },
	{ CMAGIC('N','D','X','H'), "mp4" },
	{ CMAGIC('N','D','X','M'), "mp4" },
	{ CMAGIC('N','D','X','P'), "mp4" },
	{ CMAGIC('N','D','X','S'), "mp4" },
	{ CMAGIC('o','d','c','f'), 0 },
	{ CMAGIC('o','p','f','2'), 0 },
	{ CMAGIC('o','p','x','2'), 0 },
	{ CMAGIC('p','a','n','a'), 0 },
	{ CMAGIC('q','t',' ',' '), "mov" },
	{ CMAGIC('R','O','S','S'), 0 },
	{ CMAGIC('s','d','v',' '), 0 },
	{ CMAGIC('s','s','c','1'), 0 },
	{ CMAGIC('s','s','c','2'), 0 },
	{ 0                      , 0 }
};

static const uint32_t mp4_bodyatom_types[] = {
	CMAGIC('m','o','o','v'),
	CMAGIC('m','d','a','t'),
	CMAGIC('f','r','e','e'),
	CMAGIC('s','k','i','p'),
	CMAGIC('w','i','d','e'),
	CMAGIC('p','n','o','t'),
	0
};

struct mp4_atom_head {
	uint32_t size;
	uint32_t type;
};

struct mp4_type_atom {
	uint32_t size;
	uint32_t type;
	uint32_t major_brand;
	uint32_t minor_version;
	uint32_t compatible_brands[];
};

static const char *mp4_find_ext(uint32_t brand)
{
	for (const struct ftyp *ftyp = mp4_ftyps; ftyp->brand; ++ ftyp)
	{
		if (ftyp->brand == brand)
		{
			if (!ftyp->ext) return "mp4";
			return ftyp->ext;
		}
	}
	return NULL;
}

static int mp4_isbodyatom_type(uint32_t type)
{
	for (const uint32_t *atom = mp4_bodyatom_types; *atom; ++ atom)
	{
		if (*atom == type)
		{
			return 1;
		}
	}
	return 0;
}

int mp4_isfile(const uint8_t *data, size_t input_len, struct file_info *info)
{
	const char *ext = 0;
	const struct mp4_type_atom *type = (struct mp4_type_atom *)data;

	if (input_len < MP4_HEADER_SIZE || type->type != MP4_MAGIC)
		return 0;

	size_t length = be32toh(type->size);

	if (length < MP4_HEADER_SIZE || input_len < length)
		return 0;

	ext = mp4_find_ext(type->major_brand);

	if (!ext)
	{
		for (const uint32_t *brand = type->compatible_brands,
			*end = (const uint32_t*)(data + length);
			brand < end; ++ brand)
		{
			if (*brand)
			{
				ext = mp4_find_ext(*brand);
				if (ext) break;
			}
		}
	}

	if (!ext)
		return 0;

	/* Now I'm fairly sure it is a mp4 file, so if it now ends
	 * prematurely I assume it is a truncated mp4 file. */
	while (length < input_len - 8)
	{
		const struct mp4_atom_head *head = (const struct mp4_atom_head *)(data + length);
		size_t size = be32toh(head->size);
		if (size < 8 || !mp4_isbodyatom_type(head->type) || (size_t)(-1) - size < length)
			break;
		length += size;
		if (length > input_len) length = input_len;
	}

	if (info)
	{
		info->length = length;
		info->ext    = ext;
	}

	return 1;
}

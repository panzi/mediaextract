#ifndef AUDIOEXTRACT_MOD_H__
#define AUDIOEXTRACT_MOD_H__

#include "audioextract.h"

#define MOD_4CH_MAGIC1 MAGIC("M.K.")
#define MOD_4CH_MAGIC2 MAGIC("M!K!")
#define MOD_4CH_MAGIC3 MAGIC("M&K!")
#define MOD_4CH_MAGIC4 MAGIC("N.T.")
#define MOD_4CH_MAGIC5 MAGIC("FLT4")
#define MOD_4CH_MAGIC6 MAGIC("4CHN")
#define MOD_4CH_MAGIC7 MAGIC("TDZ4")

#define MOD_5CH_MAGIC1 MAGIC("TDZ5") /* not sure if this exists */

#define MOD_6CH_MAGIC1 MAGIC("6CHN")
#define MOD_6CH_MAGIC2 MAGIC("TDZ6")

#define MOD_7CH_MAGIC1 MAGIC("TDZ7") /* not sure if this exists */

#define MOD_8CH_MAGIC1 MAGIC("8CHN")
#define MOD_8CH_MAGIC2 MAGIC("TDZ8")
#define MOD_8CH_MAGIC3 MAGIC("FLT8")
#define MOD_8CH_MAGIC4 MAGIC("CD81")
#define MOD_8CH_MAGIC5 MAGIC("OKTA")

#define MOD_16CH_MAGIC1 MAGIC("16CH")
#define MOD_16CH_MAGIC2 MAGIC("16CN")

#define MOD_32CH_MAGIC1 MAGIC("32CH")
#define MOD_32CH_MAGIC2 MAGIC("32CN")

#define IS_MOD_MAGIC(magic) \
	(IS_MOD_4CH_MAGIC(magic)  || \
	 IS_MOD_5CH_MAGIC(magic)  || \
	 IS_MOD_6CH_MAGIC(magic)  || \
	 IS_MOD_7CH_MAGIC(magic)  || \
	 IS_MOD_8CH_MAGIC(magic)  || \
	 IS_MOD_16CH_MAGIC(magic) || \
	 IS_MOD_32CH_MAGIC(magic))

#define IS_MOD_4CH_MAGIC(magic) \
	(((magic) == MOD_4CH_MAGIC1) || \
	 ((magic) == MOD_4CH_MAGIC2) || \
	 ((magic) == MOD_4CH_MAGIC3) || \
	 ((magic) == MOD_4CH_MAGIC4) || \
	 ((magic) == MOD_4CH_MAGIC5) || \
	 ((magic) == MOD_4CH_MAGIC6) || \
	 ((magic) == MOD_4CH_MAGIC7))

#define IS_MOD_5CH_MAGIC(magic) \
	((magic) == MOD_5CH_MAGIC1)

#define IS_MOD_6CH_MAGIC(magic) \
	(((magic) == MOD_6CH_MAGIC1) || \
	 ((magic) == MOD_6CH_MAGIC2))

#define IS_MOD_7CH_MAGIC(magic) \
	((magic) == MOD_7CH_MAGIC1)

#define IS_MOD_8CH_MAGIC(magic) \
	(((magic) == MOD_8CH_MAGIC1) || \
	 ((magic) == MOD_8CH_MAGIC2) || \
	 ((magic) == MOD_8CH_MAGIC3) || \
	 ((magic) == MOD_8CH_MAGIC4) || \
	 ((magic) == MOD_8CH_MAGIC5))

#define IS_MOD_16CH_MAGIC(magic) \
	(((magic) == MOD_16CH_MAGIC1) || \
	 ((magic) == MOD_16CH_MAGIC2))

#define IS_MOD_32CH_MAGIC(magic) \
	(((magic) == MOD_32CH_MAGIC1) || \
	 ((magic) == MOD_32CH_MAGIC2))

#define MOD_MAGIC_OFFSET 1080

int mod_isfile(const unsigned char *start, const unsigned char *end, size_t *lengthptr);

#endif /* AUDIOEXTRACT_MOD_H__ */

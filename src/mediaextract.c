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

/*
 * mediaextract
 *
 * Author: Mathaias Panzenböck
 * This is derived from oggextract:
 * http://ner.mine.nu/oggextract/
 *
 * Original author of oggextract: Adrian Keet
 */

#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mediaextract.h"
#include "formatstring.h"
#include "riff.h"
#include "aiff.h"
#include "ogg.h"
#include "mpg123.h"
#include "mp4.h"
#include "id3.h"
#include "midi.h"
#include "xmidi.h"
#include "mod.h"
#include "s3m.h"
#include "xm.h"
#include "it.h"
#include "asf.h"
#include "bink.h"
#include "au.h"
#include "smk.h"
#include "bmp.h"
#include "png.h"
#include "jpg.h"
#include "gif.h"
#include "mpeg.h"
#include "text.h"

#if defined(__WINDOWS__) && !defined(__CYGWIN__)
#	ifdef _WIN64
#		define PRIzu PRIu64
#		define PRIzx PRIx64
#	else
#		define PRIzu PRIu32
#		define PRIzx PRIx32
#	endif
#else
#	define PRIzu "zu"
#	define PRIzx "zx"
#endif

#define SEE_HELP "See --help for usage information.\n"

#define TRACKER_FORMATS (MOD   | S3M    | IT     | XM)
#define AUDIO_FORMATS   (OGG   | RIFF   | AIFF   | MPG123 | MP4 | ID3v2  | MIDI   | XMIDI  | MOD | S3M | IT | XM | ASF | AU)
#define VIDEO_FORMATS   (MP4   | RIFF   | ASF    | BINK   | SMK | MPEGPS | MPEGVS | MPEGTS)
#define MPEG_FORMATS    (MPEG1 | MPEGPS | MPEGVS | ID3v2)
#define IMAGE_FORMATS   (BMP   | PNG    | JPEG   | GIF)
#define TEXT_FORMATS    (UTF_8 | UTF_16LE | UTF_16BE | UTF_32LE | UTF_32BE)
#define ALL_FORMATS     (OGG   | RIFF   | AIFF   | MPG123 | MP4 | ID3v2  | MIDI   | XMIDI  | MOD | S3M | IT | XM | ASF | BINK | AU | SMK | BMP | PNG | JPEG | GIF | MPEG1 | MPEGPS | MPEGVS | MPEGTS | TEXT_FORMATS)
#define DEFAULT_FORMATS (OGG   | RIFF   | AIFF   |          MP4 | ID3v2  | MIDI   | XMIDI  |       S3M | IT | XM | ASF | BINK | AU | SMK | BMP | PNG | JPEG | GIF | MPEG1 | MPEGPS | MPEGVS)

static int usage(int argc, char **argv);
static const char *basename(const char *path);
static int parse_formats(const char *sformats, int *formats);
static int parse_size_p(const char *str, uint64_t *size);
static int parse_size(const char *str, size_t *size);
static int parse_offset(const char *str, uint64_t *size);
static const char *format_size(uint64_t in, double *out);

static int usage(int argc, char **argv)
{
	const char *progname = argc <= 0 ? "mediaextract" : argv[0];
	double default_length = 0;
	double default_size   = 0;
	const char *length_unit = format_size((SIZE_MAX>>1), &default_length);
	const char *size_unit   = format_size(SIZE_MAX,      &default_size);

	fprintf(stderr,
		"mediaextract - extracts media files that are embedded within other files\n"
		"\n"
		"Usage:\n"
		"  %s [option...] <filename> [<filename> ...]\n"
		"\n"
		"Options:\n"
		"  -h, --help             Print this help message.\n"
		"  -q, --quiet            Do not print status messages.\n"
		"  -s, --simulate         Don't write any output files.\n"
		"  -o, --output=DIR       Directory where extracted files should be written. (default: \".\")\n"
		"  -a, --filename=FORMAT  Format string for the file names.\n"
		"                         (default: \"{filename}_{offset}.{ext}\")\n"
		"\n"
		"                         Supported variables:\n"
		"                           filename  Filename of the extracted archive.\n"
		"                           offset    Offset within the archive in hexadecimal.\n"
		"                           index     0-based index of the extracted file in decimal.\n"
		"                           size      Size of the extracted file in decimal.\n"
		"                           ext       Extension associated with the filetype of the\n"
		"                                     extracted file.\n"
		"\n"
		"  -i, --offset=OFFSET    Start processing at byte OFFSET. (default: 0)\n"
		"  -n, --length=LENGTH    Only process LENGTH bytes.\n"
		"                         (default and maximum: %g %s)\n",
		progname, default_length, length_unit);

#if !defined(__LP64__) && !defined(_WIN64)

	fprintf(stderr,
		"\n"
		"                         NOTE: This program is compiled as a 32bit binary. This means\n"
		"                         the maximum amount of bytes that can be processed at once are\n"
		"                         limited to 2 GB. The rest of bigger files will be ignored. You\n"
		"                         need to run this program several times with different offsets\n"
		"                         to process such a file whole.\n"
		"\n"
		"                         This also means that extracted files can never be larger than\n"
		"                         2 GB.\n"
		"\n");

#endif

	fprintf(stderr,
		"  -m, --min-size=SIZE    Minumum size of extracted files (skip smaller). (default: 0)\n"
		"  -x, --max-size=SIZE    Maximum size of extracted files (skip larger).\n"
		"                         (default and maximum: %g %s)\n"
		"\n"
		"                         The last character of OFFSET, LENGTH and SIZE may be one of the\n"
		"                         following:\n"
		"                           B (or none)  for Bytes\n"
		"                           k            for Kilobytes (units of 1024 Bytes)\n"
		"                           M            for Megabytes (units of 1024 Kilobytes)\n"
		"                           G            for Gigabytes (units of 1024 Megabytes)\n"
		"                           T            for Terabytes (units of 1024 Gigabytes)\n"
		"                           P            for Petabytes (units of 1024 Terabytes)\n"
		"                           E            for Exabytes  (units of 1024 Petabytes)\n"
		"\n"
		"                         The special value \"max\" selects the maximum alowed value.\n"
		"\n",
		default_size, size_unit);

	fprintf(stderr,
		"  -f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.\n"
		"\n"
		"                         Supported formats:\n"
		"                           all      all supported formats\n"
		"                           default  the default set of formats (AIFF, ASF, AU, BINK, BMP,\n"
		"                                    GIF, ID3v2, IT, JPEG, MPEG 1, MPEG PS, MIDI, MP4, Ogg,\n"
		"                                    PNG, RIFF, S3M, SMK, XM, XMIDI)\n"
		"                           audio    all audio files (AIFF, ASF, AU, ID3v2, IT, MIDI, MP4,\n"
		"                                    Ogg, RIFF, S3M, XM, XMIDI)\n"
		"                           text     all text files (ASCII, UTF-8, UTF-16LE, UTF-16BE,\n"
		"                                    UTF-32LE, UTF-32BE)\n"
		"                           image    all image files (BMP, PNG, JPEG, GIF)\n"
		"                           mpeg     all safe mpeg files (MPEG 1, MPEG PS, ID3v2)\n"
		"                           tracker  all tracker files (MOD, S3M, IT, XM)\n"
		"                           video    all video files (ASF, BINK, MP4, RIFF, SMK)\n"
		"\n"
		"                           aiff     big-endian (Apple) wave files\n"
		"                           ascii    7-bit ASCII files (only printable characters)\n"
		"                           asf      Advanced Systems Format files (also WMA and WMV)\n"
		"                           au       Sun Microsystems audio file format (.au or .snd)\n"
		"                           bink     BINK files\n"
		"                           bmp      Windows Bitmap files\n"
		"                           gif      Graphics Interchange Format files\n"
		"                           id3v2    MPEG layer 1/2/3 files with ID3v2 tags\n"
		"                           it       ImpulseTracker files\n"
		"                           jpeg     JPEG Interchange Format files\n"
		"                           midi     MIDI files\n"
		"                           mod      Noisetracker/Soundtracker/Protracker Module files\n"
		"                           mpg123   MPEG layer 1/2/3 files (MP1, MP2, MP3)\n"
		"                           mpeg1    MPEG 1 System Streams\n"
		"                           mpegps   MPEG 2 Program Streams\n"
		"                           mpegts   MPEG 2 Transport Streams\n"
		"                           mp4      MP4 files (M4A, M4V, 3GPP etc.)\n"
		"                           ogg      Ogg files (Vorbis, Opus, Theora, etc.)\n"
		"                           png      Portable Network Graphics files\n"
		"                           riff     Resource Interchange File Format files (ANI, AVI, MMM,\n"
		"                                    PAL, RDI, RMI, SGT, STY, WAV, WEBP and more)\n"
		"                           s3m      ScreamTracker III files\n"
		"                           smk      Smaker files\n"
		"                           utf-8    7-bit ASCII and UTF-8 files (only printable code points)\n"
		"                           utf-16be big-endian UTF-16 files (only printable code points)\n"
		"                           utf-16le little-endian UTF-16 files (only printable code points)\n"
		"                           utf-32be big-endian UTF-32 files (only printable code points)\n"
		"                           utf-32le little-endian UTF-32 files (only printable code points)\n"
		"                           xm       Extended Module files\n"
		"                           xmidi    XMIDI files\n"
		"\n");

	fprintf(stderr,
		"                         WARNING: Because MP1/2/3 files do not have a nice file magic, using\n"
		"                         the 'mpg123' format may cause *a lot* of false positives. Nowadays\n"
		"                         MP3 files usually have an ID3v2 tag at the start, so using the\n"
		"                         'id3v2' format is the better option anyway.\n"
		"\n"
		"                         The detection accuracy of MOD files is not much better and of MPEG TS\n"
		"                         it is even worse and thus the 'mpg123', 'mpegts' and 'mod' formats\n"
		"                         are per default disabled.\n"
		"\n"
		"                         NOTE: When using only the 'mpg123' format but not 'id3v2' any ID3v2\n"
		"                         tag will be stripped. ID3v1 tags will still be kept.\n"
		"\n"
		"                         NOTE: The 'text' format might detect too much bogus text in UTF-16 or\n"
		"                         UTF-32 encodings. I recommend to use 'utf-8' or 'ascii' instead, if\n"
		"                         you can.\n"
		"\n"
		"                         If '-' is written before a format name the format will be\n"
		"                         removed from the set of formats to extract. E.g. extract\n"
		"                         everything except tracker files:\n"
		"\n"
		"                           %s --formats=all,-tracker data.bin\n"
		"\n",
		progname);
	return 255;
}

int probably_mod_text(const uint8_t *str, size_t length)
{
	size_t non_ascii = 0;
	for (const uint8_t *end = str + length; str < end; ++ str)
	{
		uint8_t c = *str;

		if (c > 0 && c < ' ')
			return 0;

		if (c > '~' && c < 0xFF)
			++ non_ascii;
	}

	return length / 2 > non_ascii;
}

const char *basename(const char *path)
{
	const char *ptr = strrchr(path, '/');
#if defined(__WINDOWS__) || defined(__CYGWIN__)
	/* Windows supports both / and \ */
	const char *ptr2 = strrchr(path, '\\');
	if (ptr2 > ptr)
		ptr = ptr2;
#endif
	return ptr ? ptr + 1 : path;
}

int write_file(const uint8_t *data, size_t length, const struct extract_options *options,
               const char *filename, size_t index, size_t offset, const char *ext, char *pathbuf, size_t pathbuflen)
{
	double sz = 0;
	const char *sz_unit = NULL;

	size_t outdir_len = strlen(options->outdir);
	assert(pathbuflen > 0 && pathbuflen - 1 > outdir_len);
	memcpy(pathbuf, options->outdir, outdir_len);
	pathbuf[outdir_len] = PATH_SEP;

	size_t pathbuf_used = outdir_len + 1;
	ssize_t filename_len = formatstring(pathbuf + pathbuf_used, pathbuflen - pathbuf_used, options->filename, filename, index, offset, length, ext);
	assert(filename_len >= 0 && (size_t)filename_len <= pathbuflen);

	if (length < options->minsize)
	{
		if (!options->quiet)
		{
			sz_unit = format_size(length, &sz);
			fprintf(stderr, "Skipped too small (%g %s) %s\n", sz, sz_unit, pathbuf);
		}

		return 0;
	}
	else if (length > options->maxsize)
	{
		if (!options->quiet)
		{
			sz_unit = format_size(length, &sz);
			fprintf(stderr, "Skipped too large (%g %s) %s\n", sz, sz_unit, pathbuf);
		}

		return 0;
	}

	double slice_size = 0;
	const char *slice_unit = format_size(length, &slice_size);
	printf("Writing %g %s to %s\n", slice_size, slice_unit, pathbuf);

	if (options->simulate)
		return 1;

	return write_data(pathbuf, data, length);
}

int do_extract(const uint8_t *filedata, size_t filesize, const struct extract_options *options, size_t *numfilesptr, size_t *sumsizeptr)
{
	const uint8_t *ptr = NULL, *end = NULL;
	enum fileformat format = NONE;

	size_t sumsize = 0;
	size_t length = 0;
	int success = 1;
	int formats = options->formats;
	char *outfilename = NULL;

	size_t numfiles = 0;
	const char *filename = basename(options->filepath);
	// max. ext length is 16 characters
	ssize_t formatted_len = formatstring(NULL, 0, options->filename, filename, SIZE_MAX, SIZE_MAX, SIZE_MAX, "0123456789ABCDEF");
	if (formatted_len < 0) {
		goto error;
	}
	size_t output_len = strlen(options->outdir);
	if (output_len > SIZE_MAX - 2 || output_len + 2 > SIZE_MAX - (size_t)formatted_len) {
		fprintf(stderr, "error: output filename too long\n");
		goto error;
	}
	size_t namelen = output_len + 1 + (size_t)formatted_len + 1;

	struct mpg123_info mpg123;
	struct ogg_info ogg;
	struct file_info info = {0, 0};
	size_t count = 0; // e.g. for tracks count in midi
	const uint8_t *audio_start = NULL;
	size_t input_len = 0;

	outfilename = malloc(namelen);
	if (outfilename == NULL)
	{
		perror(options->filepath);
		goto error;
	}

	if (!options->quiet)
	{
		double slice_size = 0;
		const char *slice_unit = format_size(filesize, &slice_size);
		printf("Extracting 0x%08"PRIx64" ... 0x%08"PRIx64" (%g %s) from %s\n",
			options->offset,
			options->offset + filesize,
			slice_size, slice_unit,
			options->filepath);
	}

#define WRITE_FILE(data, length, ext) \
	if (write_file((data), length, options, filename, numfiles, (size_t)((data) - filedata), (ext), outfilename, namelen)) \
	{ \
		++ numfiles; \
		sumsize += length; \
	}
	
	ptr = filedata;
	end = filedata + filesize;
	for (input_len = filesize; input_len >= 4; input_len = (size_t)(end - ptr))
	{
		uint32_t magic = MAGIC(ptr);
		
		if (formats & OGG && magic == OGG_MAGIC && ogg_ispage(ptr, input_len, &ogg))
		{
			uint32_t pageno = ogg.pageno;
			audio_start = ptr;

			for (;;)
			{
				ptr += ogg.length;
				
				if (!ogg_ispage(ptr, (size_t)(end - ptr), &ogg) || ogg.pageno <= pageno)
					break;

				pageno = ogg.pageno;
			}

			WRITE_FILE(audio_start, ptr - audio_start, "ogg");
			continue;
		}

		if (formats & RIFF && magic == RIFF_MAGIC && riff_isfile(ptr, input_len, &info))
		{
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}
		
		if (formats & AIFF && magic == FORM_MAGIC && aiff_isfile(ptr, input_len, &info))
		{
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}
		
		if (formats & MIDI && magic == MIDI_MAGIC && midi_isheader(ptr, input_len, &length, &count))
		{
			audio_start = ptr;
			do {
				ptr += length;
			} while (count-- > 0 && midi_istrack(ptr, (size_t)(end - ptr), &length));

			if (count != 0 && !(options->quiet))
			{
				fprintf(stderr, "warning: midi file misses %"PRIzu" tracks\n", count);
			}

			WRITE_FILE(audio_start, ptr - audio_start, "mid");
			continue;
		}
		
		if (formats & XMIDI && magic == FORM_MAGIC && xmidi_isfile(ptr, input_len, &info))
		{
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}
		
		format = NONE;
		if (formats & ID3v2 && IS_ID3v2_MAGIC(ptr) && id3v2_istag(ptr, input_len, 0, &length))
		{
			format = ID3v2;
		}

		if (formats & MPG123 && IS_MPG123_MAGIC(ptr))
		{
			format = MPG123;
			length = 0;
		}

		if (format & (ID3v2 | MPG123) && mpg123_isframe(ptr + length, input_len - length, &mpg123))
		{
			uint8_t version = mpg123.version;
			uint8_t layer   = mpg123.layer;

			audio_start = ptr;
			ptr += length;

			do {
				ptr += mpg123.frame_size;
			} while (mpg123_isframe(ptr, (size_t)(end - ptr), &mpg123)
			      && mpg123.version == version
			      && mpg123.layer   == layer);
					
			if (id3v1_istag(ptr, (size_t)(end - ptr), &length))
			{
				ptr += length;
			}

			if (formats & ID3v2 && id3v2_istag(ptr, (size_t)(end - ptr), 1, &length))
			{
				ptr += length;
			}
					
			WRITE_FILE(audio_start, ptr - audio_start,
				layer == 1 ? "mp1" :
				layer == 2 ? "mp2" :
				layer == 3 ? "mp3" :
				             "mpg");
			continue;
		}
		
		if (formats & IT && magic == IT_MAGIC && it_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "it");
			ptr += length;
			continue;
		}

		if (formats & XM && magic == XM_MAGIC && xm_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "xm");
			ptr += length;
			continue;
		}

		if (formats & ASF && magic == ASF_MAGIC && asf_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "asf");
			ptr += length;
			continue;
		}

		if (formats & AU && magic == AU_MAGIC && au_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "au");
			ptr += length;
			continue;
		}

		if (formats & PNG && magic == PNG_MAGIC && png_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "png");
			ptr += length;
			continue;
		}

		if (formats & GIF && magic == GIF_MAGIC && gif_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "gif");
			ptr += length;
			continue;
		}

		if (formats & (MPEG1 | MPEGPS | MPEGVS) && IS_MPEG_MAGIC(magic) && mpeg_isfile(ptr, input_len, formats, &length))
		{
			WRITE_FILE(ptr, length, "mpg");
			ptr += length;
			continue;
		}

		if (formats & MPEGTS && IS_MPEG_TS_MAGIC(ptr) && mpeg_isfile(ptr, input_len, formats, &length))
		{
			WRITE_FILE(ptr, length, "mpg");
			ptr += length;
			continue;
		}

		if (formats & JPEG && IS_JPG_MAGIC(magic) && jpg_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "jpg");
			ptr += length;
			continue;
		}

		if (formats & BINK && IS_BINK_MAGIC(magic) && bink_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "bik");
			ptr += length;
			continue;
		}

		if (formats & BMP && IS_BMP_MAGIC(ptr) && bmp_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "bmp");
			ptr += length;
			continue;
		}

		if (formats & SMK && IS_SMK_MAGIC(magic) && smk_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "smk");
			ptr += length;
			continue;
		}

		if (formats & MP4 && input_len > MP4_HEADER_SIZE &&
			MAGIC(ptr + MP4_MAGIC_OFFSET) == MP4_MAGIC &&
			mp4_isfile(ptr, input_len, &info))
		{
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		if (formats & S3M && input_len > S3M_MAGIC_OFFSET + 4 &&
			MAGIC(ptr + S3M_MAGIC_OFFSET) == S3M_MAGIC &&
			s3m_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "s3m");
			ptr += length;
			continue;
		}

		if (formats & MOD && input_len > MOD_MAGIC_OFFSET + 4)
		{
			const uint8_t *modmagic = ptr + MOD_MAGIC_OFFSET;
			if (IS_MOD_MAGIC(modmagic) && mod_isfile(ptr, input_len, &length))
			{
				WRITE_FILE(ptr, length, "mod");
				ptr += length;
				continue;
			}
		}

		if (formats & UTF_32LE && utf32le_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		if (formats & UTF_32BE && utf32be_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		if (formats & UTF_16LE && utf16le_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		if (formats & UTF_16BE && utf16be_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		if (formats & UTF_8 && utf8_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}
		else if (formats & ASCII && ascii_isfile(ptr, input_len, &info)) {
			WRITE_FILE(ptr, info.length, info.ext);
			ptr += info.length;
			continue;
		}

		++ ptr;
	}

	goto cleanup;

error:
	success = 0;

cleanup:
	if (outfilename)
		free(outfilename);

	if (numfilesptr)
		*numfilesptr = numfiles;
	
	if (sumsizeptr)
		*sumsizeptr = sumsize;

	return success;
}

int parse_formats(const char *sformats, int *formats)
{
	unsigned int parsed = NONE;
	const char *start = sformats;
	const char *end = strchr(start,',');

	while (*start)
	{
		if (!end)
			end = sformats + strlen(sformats);

		size_t len = (size_t)(end - start);
		unsigned int mask = NONE;
		int remove = *start == '-';

		if (remove)
		{
			++ start;
			-- len;
		}

		if (strncasecmp("ogg", start, len) == 0)
		{
			mask = OGG;
		}
		else if (strncasecmp("riff", start, len) == 0)
		{
			mask = RIFF;
		}
		else if (strncasecmp("aiff", start, len) == 0)
		{
			mask = AIFF;
		}
		else if (strncasecmp("mpg123", start, len) == 0)
		{
			mask = MPG123;
		}
		else if (strncasecmp("mp4", start, len) == 0)
		{
			mask = MP4;
		}
		else if (strncasecmp("id3v2", start, len) == 0)
		{
			mask = ID3v2;
		}
		else if (strncasecmp("midi", start, len) == 0)
		{
			mask = MIDI;
		}
		else if (strncasecmp("xmidi", start, len) == 0)
		{
			mask = XMIDI;
		}
		else if (strncasecmp("mod", start, len) == 0)
		{
			mask = MOD;
		}
		else if (strncasecmp("s3m", start, len) == 0)
		{
			mask = S3M;
		}
		else if (strncasecmp("it", start, len) == 0)
		{
			mask = IT;
		}
		else if (strncasecmp("xm", start, len) == 0)
		{
			mask = XM;
		}
		else if (strncasecmp("asf", start, len) == 0)
		{
			mask = ASF;
		}
		else if (strncasecmp("bink", start, len) == 0)
		{
			mask = BINK;
		}
		else if (strncasecmp("smk", start, len) == 0)
		{
			mask = SMK;
		}
		else if (strncasecmp("bmp", start, len) == 0)
		{
			mask = BMP;
		}
		else if (strncasecmp("png", start, len) == 0)
		{
			mask = PNG;
		}
		else if (strncasecmp("jpeg", start, len) == 0)
		{
			mask = JPEG;
		}
		else if (strncasecmp("mpeg1", start, len) == 0)
		{
			mask = MPEG1;
		}
		else if (strncasecmp("mpegps", start, len) == 0)
		{
			mask = MPEGPS;
		}
		else if (strncasecmp("mpegvs", start, len) == 0)
		{
			mask = MPEGVS;
		}
		else if (strncasecmp("mpegts", start, len) == 0)
		{
			mask = MPEGTS;
		}
		else if (strncasecmp("mpeg", start, len) == 0)
		{
			mask = MPEG_FORMATS;
		}
		else if (strncasecmp("gif", start, len) == 0)
		{
			mask = GIF;
		}
		else if (strncasecmp("ascii", start, len) == 0)
		{
			mask = ASCII;
		}
		else if (strncasecmp("utf8", start, len) == 0 || strncasecmp("utf-8", start, len) == 0)
		{
			mask = UTF_8;
		}
		else if (strncasecmp("utf16le", start, len) == 0 || strncasecmp("utf-16le", start, len) == 0)
		{
			mask = UTF_16LE;
		}
		else if (strncasecmp("utf16be", start, len) == 0 || strncasecmp("utf-16be", start, len) == 0)
		{
			mask = UTF_16BE;
		}
		else if (strncasecmp("utf32le", start, len) == 0 || strncasecmp("utf-32le", start, len) == 0)
		{
			mask = UTF_32LE;
		}
		else if (strncasecmp("utf32be", start, len) == 0 || strncasecmp("utf-32be", start, len) == 0)
		{
			mask = UTF_32BE;
		}
		else if (strncasecmp("audio", start, len) == 0)
		{
			mask = AUDIO_FORMATS;
		}
		else if (strncasecmp("image", start, len) == 0)
		{
			mask = IMAGE_FORMATS;
		}
		else if (strncasecmp("tracker", start, len) == 0)
		{
			mask = TRACKER_FORMATS;
		}
		else if (strncasecmp("video", start, len) == 0)
		{
			mask = VIDEO_FORMATS;
		}
		else if (strncasecmp("text", start, len) == 0)
		{
			mask = TEXT_FORMATS;
		}
		else if (strncasecmp("all", start, len) == 0)
		{
			mask = ALL_FORMATS;
		}
		else if (strncasecmp("default", start, len) == 0)
		{
			mask = DEFAULT_FORMATS;
		}
		else if (len != 0)
		{
			fprintf(stderr, "Unknown format: \"");
			fwrite(start, len, 1, stderr);
			fprintf(stderr, "\"\n"SEE_HELP);
			return 0;
		}

		if (remove) parsed &= ~mask;
		else        parsed |= mask;

		if (!*end)
			break;

		start = end + 1;
		end   = strchr(start, ',');
	}

	if (formats)
		*formats = parsed;

	return 1;
}

int parse_size_p(const char *str, uint64_t *size)
{
	char sizeunit = 'B';
	char *endptr = NULL;
	uint64_t sz = strtoull(str, &endptr, 10);

	if (endptr == str)
	{
		errno = EINVAL;
		return 0;
	}

	// skip whitespace
	while (isspace(*endptr)) ++ endptr;

	sizeunit = *endptr;
	// if something follows
	if (sizeunit)
	{
		const char *suffix = ++ endptr;
		
		// find end of word
		while (*endptr && !isspace(*endptr)) ++ endptr;

		// allow suffix "B" and "iB" for units other than "B"
		if ((sizeunit == 'B' && endptr != suffix) || (
			strncasecmp("B",  suffix, endptr - suffix) != 0 &&
		    strncasecmp("iB", suffix, endptr - suffix) != 0))
		{
			errno = EINVAL;
			return 0;
		}

		// skip whitespace
		while (isspace(*endptr)) ++ endptr;

		// but now there may not be anything more
		if (*endptr)
		{
			errno = EINVAL;
			return 0;
		}
	}

	switch (toupper(sizeunit))
	{
		case '\0':
		case 'B':
			break;

		case 'K':
			if (UINT64_MAX / 1024ll < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll;
			break;

		case 'M':
			if (UINT64_MAX / (1024ll * 1024ll) < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll * 1024ll;
			break;

		case 'G':
			if (UINT64_MAX / (1024ll * 1024ll * 1024ll) < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll * 1024ll * 1024ll;
			break;

		case 'T':
			if (UINT64_MAX / (1024ll * 1024ll * 1024ll * 1024ll) < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll * 1024ll * 1024ll * 1024ll;
			break;

		case 'P':
			if (UINT64_MAX / (1024ll * 1024ll * 1024ll * 1024ll * 1024ll) < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll * 1024ll * 1024ll * 1024ll * 1024ll;
			break;

		case 'E':
			if (UINT64_MAX / (1024ll * 1024ll * 1024ll * 1024ll * 1024ll * 1024ll) < sz)
			{
				errno = ERANGE;
				return 0;
			}
			sz *= 1024ll * 1024ll * 1024ll * 1024ll * 1024ll * 1024ll;
			break;

		default:
			errno = EINVAL;
			return 0;
	}

	if (size) *size = sz;

	return 1;
}

int parse_size(const char *str, size_t *size)
{
	uint64_t sz = 0;
	if (strcasecmp("max", str) == 0)
	{
		if (size) *size = SIZE_MAX;
		return 1;
	}
	else if (parse_size_p(str, &sz))
	{
		/* might be bigger than max. size_t on 32bit plattforms */
		if (sz <= SIZE_MAX) {
			if (size) *size = (size_t)sz;
			return 1;
		}
		errno = ERANGE;
	}
	return 0;
}

int parse_offset(const char *str, uint64_t *size)
{
	uint64_t sz = 0;
	if (strcasecmp("max", str) == 0)
	{
		if (size) *size = INT64_MAX;
		return 1;
	}
	else if (parse_size_p(str, &sz))
	{
		/* offset is always signed 64bit (because of compiler flags) */
		if (sz <= INT64_MAX) {
			if (size) *size = sz;
			return 1;
		}
		errno = ERANGE;
	}
	return 0;
}

const char *format_size(uint64_t in, double *out)
{
	double size = 0;
	const char *unit = "B";

	if (in >= 1024ll * 1024ll * 1024ll * 1024ll * 1024ll * 1024ll)
	{
		size = (double)in / (double)(1024ll * 1024ll * 1024ll * 1024ll * 1024ll * 1024ll);
		unit = "EB";
	}
	else if (in >= 1024ll * 1024ll * 1024ll * 1024ll * 1024ll)
	{
		size = (double)in / (double)(1024ll * 1024ll * 1024ll * 1024ll * 1024ll);
		unit = "PB";
	}
	else if (in >= 1024ll * 1024ll * 1024ll * 1024ll)
	{
		size = (double)in / (double)(1024ll * 1024ll * 1024ll * 1024ll);
		unit = "TB";
	}
	else if (in >= 1024ll * 1024ll * 1024ll)
	{
		size = (double)in / (double)(1024ll * 1024ll * 1024ll);
		unit = "GB";
	}
	else if (in >= 1024ll * 1024ll)
	{
		size = (double)in / (double)(1024ll * 1024ll);
		unit = "MB";
	}
	else if (in >= 1024ll)
	{
		size = (double)in / (double)1024ll;
		unit = "kB";
	}
	else
	{
		size = (double)in;
	}

	if (out) *out = size;
	return unit;
}

const struct option long_options[] = {
	{"formats",  required_argument, 0, 'f' },
	{"output",   required_argument, 0, 'o' },
	{"filename", required_argument, 0, 'a' },
	{"help",     no_argument,       0, 'h' },
	{"quiet",    no_argument,       0, 'q' },
	{"min-size", required_argument, 0, 'm' },
	{"max-size", required_argument, 0, 'x' },
	{"length",   required_argument, 0, 'n' },
	{"offset",   required_argument, 0, 'i' },
	{"simulate", no_argument,       0, 's' },
	{0,          0,                 0,  0  }
};

int main(int argc, char **argv)
{
	struct extract_options options = {
		.filepath = NULL,
		.outdir   = ".",
		.filename = "{filename}_{offset}.{ext}",
		.minsize  = 0,
		.maxsize  = SIZE_MAX,
		.length   = (SIZE_MAX>>1),
		.formats  = DEFAULT_FORMATS,
		.quiet    = false,
		.simulate = false
	};
	int i = 0, opt = 0;
	size_t failures = 0;
	size_t sumnumfiles = 0;
	size_t numfiles = 0;
	size_t size = 0;
	size_t sumsize = 0;
	struct stat st;

	while ((opt = getopt_long(argc, argv, "f:o:a:hqm:x:n:i:s", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'f':
				if (!parse_formats(optarg, &options.formats))
					return 255;
				else if (options.formats == 0)
				{
					fprintf(stderr, "error: No formats specified.\n"SEE_HELP);
					return 255;
				}
				break;

			case 'o':
				options.outdir = optarg;
				break;

			case 'a':
				options.filename = optarg;
				break;

			case 'h':
				return usage(argc, argv);

			case 'q':
				options.quiet = 1;
				break;

			case 'x':
			case 'm':
			case 'n':
				if (!parse_size(optarg, &size))
				{
					perror(optarg);
					fprintf(stderr, SEE_HELP);
					return 255;
				}

				if (opt == 'm')
					options.minsize = size;
				else if (opt == 'x')
					options.maxsize = size;
				else
					options.length  = size;
				break;

			case 'i':
				if (!parse_offset(optarg, &(options.offset)))
				{
					perror(optarg);
					fprintf(stderr, SEE_HELP);
					return 255;
				}
				break;

			case 's':
				options.simulate = 1;
				break;

			default:
				fprintf(stderr, SEE_HELP);
				return 255;
		}
	}

	if (optind >= argc)
	{
		fprintf(stderr, "error: Not enough arguments.\n"SEE_HELP);
		return 1;
	}

	if (options.length == 0)
	{
		if (!options.quiet)
			printf("Nothing to extract for 0-length range.\n");
		return 0;
	}

	if (stat(options.outdir, &st) != 0) {
		perror(options.outdir);
		return 1;
	}

	if (!S_ISDIR(st.st_mode)) {
		fprintf(stderr, "%s: %s\n", options.outdir, strerror(ENOTDIR));
		return 1;
	}

	for (i = optind; i < argc; ++ i)
	{
		options.filepath = argv[i];
		numfiles = 0;
		size = 0;
		if (extract(&options, &numfiles, &size))
		{
			sumnumfiles += numfiles;
			sumsize += size;
		}
		else {
			fprintf(stderr, "Error processing file: %s\n", options.filepath);
			failures += 1;
		}
	}

	double sz = 0;
	const char *sz_unit = format_size(sumsize, &sz);
	if (sumnumfiles == 1)
		printf("Extracted 1 file of %g %s size.\n", sz, sz_unit);
	else
		printf("Extracted %"PRIzu" files of %g %s size.\n", sumnumfiles, sz, sz_unit);

	if (failures > 0)
	{
		fprintf(stderr, "%"PRIzu" error(s) during extraction.\n", failures);
		return 1;
	}
	return 0;
}

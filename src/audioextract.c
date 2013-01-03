/*
 * audioextract
 *
 * Author: Mathaias Panzenböck
 * This is derived from oggextract:
 * http://ner.mine.nu/oggextract/
 *
 * Original author of oggextract: Adrian Keet
 */

#include <getopt.h>
#include <strings.h>

#include "audioextract.h"
#include "riff.h"
#include "aiff.h"
#include "ogg.h"
#include "mpg123.h"
#include "mp4.h"
#include "id3.h"
#include "midi.h"
#include "mod.h"
#include "s3m.h"
#include "xm.h"
#include "it.h"
#include "asf.h"
#include "bink.h"

#if defined(__WINDOWS__) && !defined(__CYGWIN__)
#	ifdef _WIN64
#		define ZU_FMT "%l64u"
#		define EXTRACTED_FILE_FMT "%s%c%s_%08l64x.%s"
#		pragma GCC diagnostic ignored "-Wformat"
#		pragma GCC diagnostic ignored "-Wformat-extra-args"
#	else
#		define ZU_FMT "%u"
#		define EXTRACTED_FILE_FMT "%s%c%s_%08x.%s"
#	endif
#else
#	define ZU_FMT "%zu"
#	define EXTRACTED_FILE_FMT "%s%c%s_%08zx.%s"
#endif

enum fileformat {
	NONE   =    0,
	OGG    =    1,
	RIFF   =    2,
	AIFF   =    4,
	MPG123 =    8,
	ID3v2  =   16,
	MP4    =   32,
	MIDI   =   64,
	MOD    =  128,
	S3M    =  256,
	IT     =  512,
	XM     = 1024,
	ASF    = 2048,
	BINK   = 4096
};

#define ALL_FORMATS     (OGG | RIFF | AIFF | MPG123 | MP4 | ID3v2 | MIDI | MOD | S3M | IT | XM | ASF | BINK)
#define DEFAULT_FORMATS (OGG | RIFF | AIFF |          MP4 | ID3v2 | MIDI |       S3M | IT | XM | ASF | BINK)
#define TRACKER_FORMATS (MOD | S3M  | IT   | XM)

int usage(int argc, char **argv)
{
	const char *progname = argc <= 0 ? "audioextract" : argv[0]; 
	fprintf(stderr,
		"audioextract - extracts audio files that are embedded within other files\n"
		"\n"
		"Usage:\n"
		"  %s [option...] <filename> [<filename> ...]\n"
		"\n"
		"Options:\n"
		"  -h, --help             Print this help message.\n"
		"  -q, --quiet            Do not print status messages.\n"
		"  -f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.\n"
		"\n"
		"                         Supported formats:\n"
		"                           all      all supported formats\n"
		"                           default  the default set of formats (AIFF, ASF, BINK, ID3v2, IT,\n"
		"                                    MIDI, MP4, Ogg, RIFF, S3M, XM)\n"
		"                           aiff     big-endian (Apple) wave files\n"
		"                           asf      Advanced Systems Format files (also WMA and WMV)\n"
		"                           bink     BINK files\n"
		"                           id3v2    MPEG layer 1/2/3 files with ID3v2 tags\n"
		"                           it       ImpulseTracker files\n"
		"                           midi     MIDI files\n"
		"                           mod      FastTracker files\n"
		"                           mpg123   MPEG layer 1/2/3 files (MP1, MP2, MP3)\n"
		"                           mp4      MP4 files (M4A, M4V, 3GPP etc.)\n"
		"                           ogg      Ogg files (Vorbis, Opus, Theora, etc.)\n"
		"                           riff     Resource Interchange File Format files (ANI, AVI, MMM,\n"
		"                                    PAL, RDI, RMI, WAV)\n"
		"                           s3m      ScreamTracker III files\n"
		"                           xm       Extended Module files\n"
		"                           tracker  all tracker files (MOD, S3M, IT, XM)\n"
		"\n"
		"                         WARNING: Because MP1/2/3 files do not have a nice file magic, using\n"
		"                         the 'mpg123' format may cause *a lot* of false positives. Nowadays\n"
		"                         MP3 files usually have an ID3v2 tag at the start, so using the\n"
		"                         'id3v2' format is the better option anyway.\n"
		"\n"
		"                         The detection accuracy of MOD files is not much better and thus\n"
		"                         they are also per default disabled.\n"
		"\n"
		"                         NOTE: When using only the 'mpg123' format but not 'id3v2' any ID3v2\n"
		"                         tag will be stripped. ID3v1 tags will still be kept.\n"
		"\n"
		"                         If '-' is written before a format name the format will be\n"
		"                         removed from the set of formats to extract. E.g. extract\n"
		"                         everything except tracker files:\n"
		"\n"
		"                           %s --formats=all,-tracker data.bin\n"
		"\n"
		"  -o, --output=DIR       Directory where extracted files should be written. (default: \".\")\n"
		"  -m, --min-size=SIZE    Minumum size of extracted files (skip smaller). (default: 0)\n"
		"  -x, --max-size=SIZE    Maximum size of extracted files (skip larger).\n"
		"                         (default: max. possible size_t value)\n"
		"\n"
		"                         The last character of SIZE may be one of the following:\n"
		"                           B (or none)  for bytes\n"
		"                           k            for Kilobytes (units of 1024 bytes)\n"
		"                           M            for Megabytes (units of 1024 Kilobytes)\n"
		"                           G            for Gigabytes (units of 1024 Megabytes)\n"
		"                           T            for Terabytes (units of 1024 Gigabytes)\n"
		"\n",
		progname, progname);
	return 255;
}

int probalby_mod_text(const uint8_t *str, size_t length)
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
               const char *filename, size_t offset, const char *ext, char *pathbuf, size_t pathbuflen)
{
	snprintf(pathbuf, pathbuflen, EXTRACTED_FILE_FMT, options->outdir, PATH_SEP, filename, offset, ext);
	
	if (length < options->minsize)
	{
		if (!options->quiet)
			fprintf(stderr, "Skipped too small (" ZU_FMT ") %s\n", length, pathbuf);

		return 0;
	}
	else if (length > options->maxsize)
	{
		if (!options->quiet)
			fprintf(stderr, "Skipped too large (" ZU_FMT ") %s\n", length, pathbuf);

		return 0;
	}

	if (!options->quiet)
		printf("Writing %s\n", pathbuf);

	return write_data(pathbuf, data, length);
}

int do_extract(const uint8_t *filedata, size_t filesize, const struct extract_options *options, size_t *numfilesptr)
{
	const uint8_t *ptr = NULL, *end = NULL;
	enum fileformat format = NONE;

	size_t length = 0;
	int success = 1;
	int formats = options->formats;
	char *outfilename = NULL;

	size_t numfiles = 0;
	const char *filename = basename(options->filepath);
	// max. ext length is 4 characters
	size_t namelen = strlen(options->outdir) + strlen(filename) + 25;

	struct mpg123_info mpg123;
	struct file_info info = {0, 0};
	size_t count = 0; // e.g. for tracks count in midi
	const uint8_t *audio_start = NULL;
	size_t input_len = 0;

	outfilename = malloc(namelen);
	if (outfilename == NULL)
	{
		perror("malloc");
		goto error;
	}

#define WRITE_FILE(data, length, ext) \
	if (write_file((data), length, options, filename, (size_t)((data) - filedata), (ext), outfilename, namelen)) \
	{ \
		++ numfiles; \
	}
	
	ptr = filedata;
	end = filedata + filesize;
	for (input_len = filesize; input_len >= 4; input_len = (size_t)(end - ptr))
	{
		uint32_t magic = MAGIC(ptr);
		
		if (formats & OGG && magic == OGG_MAGIC && ogg_ispage(ptr, input_len, &length) && ogg_isinitial(ptr))
		{
			audio_start = ptr;

			do {
				ptr += length;
			} while (ogg_ispage(ptr, (size_t)(end - ptr), &length));
					
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
				fprintf(stderr, "warning: midi file misses " ZU_FMT " tracks\n", count);
			}

			WRITE_FILE(audio_start, ptr - audio_start, "mid");
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

		if (formats & BINK && IS_BINK_MAGIC(magic) && bink_isfile(ptr, input_len, &length))
		{
			WRITE_FILE(ptr, length, "bik");
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

	return success;
}

int parse_formats(const char *formats)
{
	unsigned int parsed = NONE;
	const char *start = formats;
	const char *end = strchr(start,',');

	while (*start)
	{
		if (!end)
			end = formats + strlen(formats);

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
		else if (strncasecmp("wave", start, len) == 0)
		{
			mask = RIFF | AIFF;
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
		else if (strncasecmp("tracker", start, len) == 0)
		{
			mask = TRACKER_FORMATS;
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
			fprintf(stderr, "\"\nSee --help for usage information.\n");
			return -1;
		}

		if (remove) parsed &= ~mask;
		else        parsed |= mask;

		if (!*end)
			break;

		start = end + 1;
		end = strchr(start, ',');
	}

	return parsed;
}

const struct option long_options[] = {
	{"formats",  required_argument, 0,  'f' },
	{"output",   required_argument, 0,  'o' },
	{"help",     no_argument,       0,  'h' },
	{"quiet",    no_argument,       0,  'q' },
	{"min-size", required_argument, 0,  'm' },
	{"max-size", no_argument,       0,  'x' },
	{0,         0,                  0,  0 }
};

int main(int argc, char **argv)
{
	struct extract_options options = { NULL, ".", 0, (size_t)-1, DEFAULT_FORMATS, 0 };
	int i = 0, opt = 0;
	size_t failures = 0;
	size_t sumnumfiles = 0;
	size_t numfiles = 0;
	long long tmp = 0;
	size_t size = 0;
	char sizeunit = 'B';
	char *endptr = NULL;

	while ((opt = getopt_long(argc, argv, "f:o:hqm:x:", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'f':
				options.formats = parse_formats(optarg);
				if (options.formats < 0)
					return 255;
				else if (options.formats == 0)
				{
					fprintf(stderr, "error: No formats specified.\n"
						"See --help for usage information.\n");
					return 255;
				}
				break;

			case 'o':
				options.outdir = optarg;
				break;

			case 'h':
				return usage(argc, argv);

			case 'q':
				options.quiet = 1;
				break;

			case 'x':
			case 'm':
				tmp = strtoull(optarg, &endptr, 10);
				sizeunit = *endptr;
				if (endptr == optarg)
				{
					fprintf(stderr, "error: Illegal size: \"%s\"\n"
						"See --help for usage information.\n", optarg);
					return 255;
				}
				/* tmp might be bigger than max. size_t on 32bit plattforms */
				else if ((sizeunit && endptr[1]) || tmp < 0L || (size_t)tmp > (size_t)(-1))
				{
					perror("strtoull");
					fprintf(stderr, "error: Illegal size: \"%s\"\n"
						"See --help for usage information.\n", optarg);
					return 255;
				}
				size = tmp;
				switch (sizeunit)
				{
					case '\0':
					case 'B':
						break;

					case 'k':
						if ((size_t)(-1) / 1024ll < size)
						{
							fprintf(stderr, "error: Illegal size (integer overflow): \"%s\"\n"
								"See --help for usage information.\n", optarg);
							return 255;
						}
						size *= 1024ll;
						break;

					case 'M':
						if ((size_t)(-1) / (1024ll * 1024ll) < size)
						{
							fprintf(stderr, "error: Illegal size (integer overflow): \"%s\"\n"
								"See --help for usage information.\n", optarg);
							return 255;
						}
						size *= 1024ll * 1024ll;
						break;

					case 'G':
						if ((size_t)(-1) / (1024ll * 1024ll * 1024ll) < size)
						{
							fprintf(stderr, "error: Illegal size (integer overflow): \"%s\"\n"
								"See --help for usage information.\n", optarg);
							return 255;
						}
						size *= 1024ll * 1024ll * 1024ll;
						break;

					case 'T':
						if ((size_t)(-1) / (1024ll * 1024ll * 1024ll * 1024ll) < size)
						{
							fprintf(stderr, "error: Illegal size (integer overflow): \"%s\"\n"
								"See --help for usage information.\n", optarg);
							return 255;
						}
						size *= 1024ll * 1024ll * 1024ll * 1024ll;
						break;

					default:
						fprintf(stderr, "error: Illegal size: \"%s\"\n"
							"See --help for usage information.\n", optarg);
						return 255;
				}
				if (opt == 'm')
					options.minsize = size;
				else
					options.maxsize = size;
				break;
		}
	}

	if (optind >= argc)
	{
		fprintf(stderr, "error: Not enough arguments.\nSee --help for usage information.\n");
		return 1;
	}

	for (i = optind; i < argc; ++ i)
	{
		options.filepath = argv[i];

		if (!options.quiet)
			printf("Extracting %s\n", options.filepath);

		if (extract(&options, &numfiles))
		{
			sumnumfiles += numfiles;
		}
		else {
			fprintf(stderr, "Error processing file: %s\n", options.filepath);
			failures += 1;
		}
	}

	if (!options.quiet)
		printf("Extracted " ZU_FMT " file(s).\n", sumnumfiles);

	if (failures > 0)
	{
		fprintf(stderr, ZU_FMT " error(s) during extraction.\n", failures);
		return 1;
	}
	return 0;
}

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
#include "wave.h"
#include "ogg.h"
#include "mpeg.h"
#include "id3.h"
#include "midi.h"
#include "mod.h"
#include "s3m.h"

enum fileformat {
	NONE  =   0,
	OGG   =   1,
	RIFF  =   2,
	AIFF  =   4,
	MPEG  =   8,
	ID3v2 =  16,
	MIDI  =  32,
	MOD   =  64,
	S3M   = 128,
// TODO:
//	IT    = 256,
//	XM    = 512,
};

#define ALL_FORMATS     (OGG | RIFF | AIFF | MPEG | ID3v2 | MIDI | MOD | S3M)
#define DEFAULT_FORMATS (OGG | RIFF | AIFF |        ID3v2 | MIDI | MOD | S3M)

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
		"                         Supported formats:\n"
		"                           all      all supported formats\n"
		"                           default  the default set of formats (AIFF, ID3v2, Ogg, RIFF, MIDI, MOD)\n"
		"                           aiff     big-endian (Apple) wave files\n"
		"                           id3v2    MPEG files with ID3v2 tags at the start\n"
		"                           midi     MIDI files\n"
		"                           mod      MOD files\n"
		"                           mpeg     any MPEG files (e.g. MP3)\n"
		"                           ogg      Ogg files (Vorbis, FLAC, Opus, Theora, etc.)\n"
		"                           riff     little-endian (Windows) wave files\n"
		"                           wave     both RIFF and AIFF wave files\n"
		"\n"
		"                         WARNING: Because MPEG files do not have a nice file magic, using\n"
		"                         the 'mpeg' format may cause *a lot* of false positives. Nowadays\n"
		"                         MP3 files usually have an ID3v2 tag at the start, so using the\n"
		"                         'id3v2' format is the better option anyway.\n"
		"\n"
		"                         NOTE: When using only the 'mpeg' format but not 'id3v2' any ID3v2\n"
		"                         tag will be stripped. ID3v1 tags will still be kept.\n"
		"\n"
		"                         If '-' is written before a format name the format will be\n"
		"                         removed from the set of formats to extract. E.g. extract\n"
		"                         everything except wave files:\n"
		"\n"
		"                           %s --formats=all,-wave data.bin\n"
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

const unsigned char *findmagic(const unsigned char *start, const unsigned char *end, int formats, enum fileformat *format)
{
	if ((intptr_t)end < 4)
		return NULL;
	end -= 4;

	for (; start < end; ++ start)
	{
		uint32_t magic = MAGIC(start);

		if (formats & OGG && magic == OGG_MAGIC)
		{
			*format = OGG;
			return start;
		}
		else if (formats & RIFF && magic == RIFF_MAGIC)
		{
			*format = RIFF;
			return start;
		}
		else if (formats & AIFF && magic == FORM_MAGIC)
		{
			*format = AIFF;
			return start;
		}
		else if (formats & MIDI && magic == MIDI_MAGIC)
		{
			*format = MIDI;
			return start;
		}
		else if (formats & ID3v2 && IS_ID3v2_MAGIC(start))
		{
			*format = ID3v2;
			return start;
		}
		else if (formats & MPEG && IS_MPEG_MAGIC(start))
		{
			*format = MPEG;
			return start;
		}
		else
		{
			size_t length = (size_t)(end - start);
			
			if (formats & S3M && length >= S3M_MAGIC_OFFSET && MAGIC(start + S3M_MAGIC_OFFSET) == S3M_MAGIC)
			{
				*format = S3M;
				return start;
			}
			else if (formats & MOD && length >= MOD_MAGIC_OFFSET)
			{
				uint32_t modmagic = MAGIC(start + MOD_MAGIC_OFFSET);
				if (IS_MOD_MAGIC(modmagic))
				{
					*format = MOD;
					return start;
				}
			}
		}
	}

	return NULL;
}

const char *basename(const char *path)
{
	const char *ptr = strrchr(path, '/');
#ifdef __WINDOWS__
	/* Windows supports both / and \ */
	const char *ptr2 = strrchr(path, '\\');
	if (ptr2 > ptr)
		ptr = ptr2;
#endif
	return ptr ? ptr + 1 : path;
}

int write_file(const char *outdir, const char *filename, size_t offset,
               const char *ext, char *pathbuf, size_t pathbuflen,
               const unsigned char *data, size_t length,
               size_t minsize, size_t maxsize, int quiet)
{
	snprintf(pathbuf, pathbuflen, "%s/%s_%08zx.%s", outdir, filename, offset, ext);
	
	if (length < minsize)
	{
		if (!quiet)
			fprintf(stderr, "Skipped too small (%zu) %s\n", length, pathbuf);

		return 0;
	}
	else if (length > maxsize)
	{
		if (!quiet)
			fprintf(stderr, "Skipped too large (%zu) %s\n", length, pathbuf);

		return 0;
	}

	int outfd = creat(pathbuf, 0644);
	if (outfd < 0)
	{
		perror("creat");
		return 0;
	}

	if (!quiet)
		printf("Writing %s\n", pathbuf);

	write(outfd, data, length);
	close(outfd);
	return 1;
}

int extract(const char *filepath, const char *outdir, size_t minsize, size_t maxsize, int formats, int quiet, size_t *numfilesptr)
{
	int fd = -1;
	struct stat statdata;
	size_t filesize = 0;
	unsigned char *filedata = NULL;
	const unsigned char *ptr = NULL, *end = NULL;
	enum fileformat format = NONE;

	size_t length = 0;
	int success = 1;
	char *outfilename = NULL;

	size_t numfiles = 0;
	const char *filename = basename(filepath);
	size_t namelen = strlen(outdir) + strlen(filename) + 24;

	struct mpeg_info mpeg;
	size_t count = 0; // e.g. for tracks count in midi
	const unsigned char *audio_start = NULL;

	if (!quiet)
		printf("Extracting %s\n", filepath);

	fd = open(filepath, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		goto error;
	}

	if (fstat(fd, &statdata) < 0)
	{
		perror("stat");
		goto error;
	}
	if (S_ISDIR(statdata.st_mode))
	{
		fprintf(stderr, "error: Is a directory: %s\n", filepath);
		goto error;
	}
	filesize = statdata.st_size;

	filedata = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (filedata == MAP_FAILED)
	{
		perror("mmap");
		goto error;
	}

	outfilename = malloc(namelen);
	if (outfilename == NULL)
	{
		perror("malloc");
		goto error;
	}

#define WRITE_FILE(data, length, ext) \
	if (write_file(outdir, filename, (size_t)((data) - filedata), (ext), outfilename, namelen, (data), (length), minsize, maxsize, quiet)) \
	{ \
		++ numfiles; \
	}
	
	ptr = filedata;
	for (end = filedata + filesize; (ptr = findmagic(ptr, end, formats, &format));)
	{
		switch (format)
		{
			case OGG:
				if (ogg_ispage(ptr, end, &length) && ogg_isinitial(ptr))
				{
					audio_start = ptr;

					do {
						ptr += length;
					} while (ogg_ispage(ptr, end, &length));
					
					WRITE_FILE(audio_start, ptr - audio_start, "ogg");
				}
				else ++ ptr;
				break;

			case RIFF:
				if (wave_ischunk(ptr, end, &length))
				{
					WRITE_FILE(ptr, length, "wav");
					ptr += length;
				}
				else ++ ptr;
				break;

			case AIFF:
				if (aiff_ischunk(ptr, end, &length))
				{
					WRITE_FILE(ptr, length, "aif");
					ptr += length;
				}
				else ++ ptr;
				break;

			case ID3v2:
			case MPEG:
				if (format == ID3v2)
				{
					if (!id3v2_istag(ptr, end, 0, &length))
					{
						++ ptr;
						break;
					}
				}
				else
					length = 0;

				if (mpeg_isframe(ptr + length, end, &mpeg))
				{
					uint8_t version = mpeg.version;
					uint8_t layer   = mpeg.layer;

					audio_start = ptr;
					ptr += length;

					do {
						ptr += mpeg.frame_size;
					} while (mpeg_isframe(ptr, end, &mpeg)
					      && mpeg.version == version
					      && mpeg.layer   == layer);
					
					if (id3v1_istag(ptr, end, &length))
					{
						ptr += length;
					}

					if (formats & ID3v2 && id3v2_istag(ptr, end, 1, &length))
					{
						ptr += length;
					}
					
					WRITE_FILE(audio_start, ptr - audio_start,
						layer == 1 ? "mp1" :
						layer == 2 ? "mp2" :
						layer == 3 ? "mp3" :
						             "mpeg");
				}
				else ++ ptr;
				break;

			case MIDI:
				if (midi_isheader(ptr, end, &length, &count))
				{
					audio_start = ptr;
					do {
						ptr += length;
					} while (count-- > 0 && midi_istrack(ptr, end, &length));

					if (count != 0 && !quiet)
					{
						fprintf(stderr, "warning: midi file misses %zu tracks\n", count);
					}

					WRITE_FILE(audio_start, ptr - audio_start, "mid");
				}
				else ++ ptr;
				break;

			case MOD:
				if (mod_isfile(ptr, end, &length))
				{
					WRITE_FILE(ptr, length, "mod");
					ptr += length;
				}
				else ++ ptr;
				break;

			case S3M:
				if (s3m_isfile(ptr, end, &length))
				{
					WRITE_FILE(ptr, length, "s3m");
					ptr += length;
				}
				else ++ ptr;
				break;

			case NONE:
				++ ptr;
				break;
		}
	}

	goto cleanup;

error:
	success = 0;

cleanup:
	if (outfilename)
		free(outfilename);

	if (filedata)
		munmap(filedata, filesize);

	if (fd >= 0)
		close(fd);

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
		else if (strncasecmp("mpeg", start, len) == 0)
		{
			mask = MPEG;
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
	int i = 0, opt = 0, quiet = 0;
	size_t failures = 0;
	size_t sumnumfiles = 0;
	size_t numfiles = 0;
	size_t minsize = 0;
	size_t maxsize = (size_t)-1;
	int formats = DEFAULT_FORMATS;
	const char *outdir = ".";
	long long tmp = 0;
	size_t size = 0;
	char sizeunit = 'B';
	char *endptr = NULL;

	while ((opt = getopt_long(argc, argv, "f:o:hqm:x:", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'f':
				formats = parse_formats(optarg);
				if (formats < 0)
					return 255;
				else if (formats == 0)
				{
					fprintf(stderr, "error: No formats specified.\n"
						"See --help for usage information.\n");
					return 255;
				}
				break;

			case 'o':
				outdir = optarg;
				break;

			case 'h':
				return usage(argc, argv);

			case 'q':
				quiet = 1;
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
				else if ((sizeunit && endptr[1]) || tmp < 0 || tmp > (size_t)(-1))
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
					minsize = size;
				else
					maxsize = size;
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
		if (extract(argv[i], outdir, minsize, maxsize, formats, quiet, &numfiles))
		{
			sumnumfiles += numfiles;
		}
		else {
			fprintf(stderr, "Error processing file: %s\n", argv[i]);
			failures += 1;
		}
	}

	if (!quiet)
		printf("Extracted %lu file(s).\n", numfiles);

	if (failures > 0)
	{
		fprintf(stderr, "%zu error(s) during extraction.\n", failures);
		return 1;
	}
	return 0;
}

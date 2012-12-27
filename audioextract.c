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

enum fileformat {
	NONE  =  0,
	OGG   =  1,
	RIFF  =  2,
	AIFF  =  4,
	MPEG  =  8,
	ID3v2 = 16

	/* TODO: AAC and MKV/WebM? */
};

int usage(int argc, char **argv)
{
	const char *progname = argc <= 0 ? "audioextract" : argv[0]; 
	fprintf(stderr,
		"Usage:\n"
		"  %s <filename> [<filename> ...]\n"
		"\n"
		"Options:\n"
		"  -h, --help             Print this help message.\n"
		"  -f, --formats=FORMATS  Comma separated list of formats (file magics) to extract.\n"
		"                         Supported formats:\n"
		"                           all       all supported formats\n"
		"                           default   the default set of formats (AIFF, ID3v2, Ogg, RIFF)\n"
		"                           aiff      big-endian (Apple) wave files\n"
		"                           id3v2     MPEG files with ID3V2 tags at the start\n"
		"                           mpeg      any MPEG files\n"
		"                           ogg       Ogg files (Vorbis, FLAC, Opus, Theora, etc.)\n"
		"                           riff      little-endian (Windows) wave files\n"
		"                           wav       alias for riff\n"
		"                           wave      both RIFF and AIFF wave files\n"
		"\n"
		"                         If '-' is written before a format name the format will be\n"
		"                         removed from the set of formats to extract. E.g. extract\n"
		"                         everything except wave files:\n"
		"\n"
		"                           %s --formats=all,-wave data.bin\n"
		"\n"
		"  -o, --output=DIR       Directory where extracted files should be written. (default: \".\")\n"
		"  -q, --quiet            Do not print status messages.\n"
		"\n",
		progname, progname);
	return 255;
}

const unsigned char *findmagic(const unsigned char *start, const unsigned char *end, int formats, enum fileformat *format)
{
	if (end < (unsigned char *)4)
		return NULL;
	end -= 4;

	for (; start < end; ++ start)
	{
		int32_t magic = *(const int32_t *)start;

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

int extract(const char *filepath, const char *outdir, int formats, int quiet, size_t *numfilesptr)
{
	int fd = -1;
	struct stat statdata;
	size_t filesize = 0;
	unsigned char *filedata = NULL;
	const unsigned char *ptr = NULL, *end = NULL;
	enum fileformat format = NONE;

	size_t length = 0;
	int outfd = -1;
	int success = 1;
	char *outfilename = NULL;

	size_t numfiles = 0;
	const char *filename = basename(filepath);
	size_t namelen = strlen(outdir) + strlen(filename) + 24;

	struct mpeg_info mpeg;

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

#define OPEN_OUTFD(ext) \
	snprintf(outfilename, namelen, "%s/%s_%08zx.%s", outdir, filename, (size_t)(ptr - filedata), (ext)); \
	outfd = creat(outfilename, -1); \
	if (outfd < 0) \
	{ \
		perror("creat"); \
		goto error; \
	} \
	++ numfiles; \
	if (!quiet) \
	{ \
		printf("Writing %s\n", outfilename); \
	}

	ptr = filedata;
	for (end = filedata + filesize; (ptr = findmagic(ptr, end, formats, &format));)
	{
		switch (format)
		{
			case OGG:
				if (ogg_ispage(ptr, end, &length) && ogg_isinitial(ptr))
				{
					OPEN_OUTFD("ogg");

					do {
						write(outfd, ptr, length);
						ptr += length;
					} while (ptr < end && ogg_ispage(ptr, end, &length));
					close(outfd);
					continue;
				}
				break;

			case RIFF:
				if (wave_ischunk(ptr, end, &length))
				{
					OPEN_OUTFD("wav");

					write(outfd, ptr, length);
					ptr += length;
					close(outfd);
					continue;
				}
				break;

			case AIFF:
				if (aiff_ischunk(ptr, end, &length))
				{
					OPEN_OUTFD("aif");

					write(outfd, ptr, length);
					ptr += length;
					close(outfd);
					continue;
				}
				break;

			case ID3v2:
			case MPEG:
				if (format == ID3v2)
				{
					if (!id3v2_istag(ptr, end, &length))
						break;
				}
				else
					length = 0;

				if (mpeg_isframe(ptr + length, end, &mpeg))
				{
					uint8_t version = mpeg.version;
					uint8_t layer   = mpeg.layer;

					OPEN_OUTFD(
						layer == 1 ? "mp1" :
						layer == 2 ? "mp2" :
						layer == 3 ? "mp3" :
						             "mpeg");

					write(outfd, ptr, length);
					ptr += length;

					do {
						write(outfd, ptr, mpeg.frame_size);
						ptr += mpeg.frame_size;
					} while (ptr < end
					      && mpeg_isframe(ptr, end, &mpeg)
					      && mpeg.version == version
					      && mpeg.layer == layer);
					
					if (id3v1_istag(ptr, end, &length))
					{
						write(outfd, ptr, length);
						ptr += length;
					}

					if (id3v2_istag(ptr, end, &length))
					{
						write(outfd, ptr, length);
						ptr += length;
					}
					
					close(outfd);
					continue;
				}
				break;

			case NONE:
				break;
		}

		++ ptr;
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
		if (strncasecmp("ogg", start, len) == 0)
		{
			parsed |= OGG;
		}
		else if (strncasecmp("riff", start, len) == 0 || strncasecmp("wav", start, len) == 0)
		{
			parsed |= RIFF;
		}
		else if (strncasecmp("aiff", start, len) == 0)
		{
			parsed |= AIFF;
		}
		else if (strncasecmp("wave", start, len) == 0)
		{
			parsed |= RIFF | AIFF;
		}
		else if (strncasecmp("mpeg", start, len) == 0)
		{
			parsed |= MPEG;
		}
		else if (strncasecmp("id3v2", start, len) == 0)
		{
			parsed |= ID3v2;
		}
		else if (strncasecmp("all", start, len) == 0)
		{
			parsed = OGG | RIFF | AIFF | MPEG | ID3v2;
		}
		else if (strncasecmp("default", start, len) == 0)
		{
			parsed |= OGG | RIFF | AIFF | ID3v2;
		}
		else if (strncasecmp("-ogg", start, len) == 0)
		{
			parsed &= ~OGG;
		}
		else if (strncasecmp("-riff", start, len) == 0 || strncasecmp("-wav", start, len) == 0)
		{
			parsed &= ~RIFF;
		}
		else if (strncasecmp("-aiff", start, len) == 0)
		{
			parsed &= ~AIFF;
		}
		else if (strncasecmp("-wave", start, len) == 0)
		{
			parsed &= ~(RIFF | AIFF);
		}
		else if (strncasecmp("-mpeg", start, len) == 0)
		{
			parsed &= ~MPEG;
		}
		else if (strncasecmp("-id3v2", start, len) == 0)
		{
			parsed &= ~ID3v2;
		}
		else if (strncasecmp("-all", start, len) == 0)
		{
			parsed &= ~(OGG | RIFF | AIFF | MPEG | ID3v2);
		}
		else if (strncasecmp("-default", start, len) == 0)
		{
			parsed &= ~(OGG | RIFF | AIFF | ID3v2);
		}
		else if (len != 0)
		{
			fprintf(stderr, "Unknown format: \"");
			fwrite(start, len, 1, stderr);
			fprintf(stderr, "\"\nSee --help for usage information.\n");
			return -1;
		}

		if (!*end)
			break;

		start = end + 1;
		end = strchr(start, ',');
	}

	return parsed;
}

const struct option long_options[] = {
	{"formats", required_argument, 0,  'f' },
	{"output",  required_argument, 0,  'o' },
	{"help",    no_argument,       0,  'h' },
	{"quiet",   no_argument,       0,  'q' },
	{0,         0,                 0,  0 }
};

int main(int argc, char **argv)
{
	int i = 0, opt = 0, quiet = 0;
	size_t failures = 0;
	size_t sumnumfiles = 0;
	size_t numfiles = 0;
	int formats = OGG | RIFF | AIFF | ID3v2;
	const char *outdir = ".";

	while ((opt = getopt_long(argc, argv, "f:o:hq", long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'f':
				formats = parse_formats(optarg);
				if (formats < 0)
					return 255;
				else if (formats == 0)
				{
					fprintf(stderr, "error: No formats specified.\nSee --help for usage information.\n");
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
		}
	}

	if (optind >= argc)
	{
		fprintf(stderr, "error: Not enough arguments.\nSee --help for usage information.\n");
		return 1;
	}

	for (i = optind; i < argc; ++ i)
	{
		if (extract(argv[i], outdir, formats, quiet, &numfiles))
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

/*
 * audioextract
 *
 * Author: Mathaias Panzenböck
 * This is derived from oggextract:
 * http://ner.mine.nu/oggextract/
 *
 * Original author of oggextract: Adrian Keet
 */

#include "audioextract.h"
#include "wave.h"
#include "ogg.h"
#include "mpeg.h"

enum fileformat {
	NONE,
	OGG,
	RIFF,
	AIFF,
	MPEG

	/* TODO: AAC and MKV/WebM? */
};

int usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <filename> [<filename> ...]\n",
		argc <= 0 ? "audioextract" : argv[0]);
	return 255;
}

const unsigned char *findmagic(const unsigned char *start, const unsigned char *end, enum fileformat *format)
{
	if (end < (unsigned char *)4)
		return NULL;
	end -= 4;

	for (; start < end; ++ start)
	{
		switch (*(const int32_t *)start)
		{
			case OGG_MAGIC:
				*format = OGG;
				return start;

			case RIFF_MAGIC:
				*format = RIFF;
				return start;

			case FORM_MAGIC:
				*format = AIFF;
				return start;

			default:
				if (IS_MPEG_MAGIC(start))
				{
					*format = MPEG;
					return start;
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

int extract(const char *filepath, size_t *numfilesptr)
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
	size_t namelen = strlen(filename) + 23;

	struct mpeg_info mpeg;

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
	snprintf(outfilename, namelen, "%s_%08zx.%s", filename, (size_t)(ptr - filedata), (ext)); \
	outfd = creat(outfilename, -1); \
	if (outfd < 0) \
	{ \
		perror("creat"); \
		goto error; \
	} \
	++ numfiles; \
	printf("Writing: %s\n", outfilename)

	ptr = filedata;
	for (end = filedata + filesize; (ptr = findmagic(ptr, end, &format));)
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

			case MPEG:
				if (mpeg_isframe(ptr, end, &mpeg))
				{
					uint8_t version = mpeg.version;
					uint8_t layer   = mpeg.layer;

					OPEN_OUTFD(
						layer == 1 ? "mp1" :
						layer == 2 ? "mp2" :
						layer == 3 ? "mp3" :
						             "mpeg");

					do {
						write(outfd, ptr, mpeg.frame_size);
						ptr += mpeg.frame_size;
					} while (ptr < end
					      && mpeg_isframe(ptr, end, &mpeg)
					      && mpeg.version == version
					      && mpeg.layer == layer);
					close(outfd);
					continue;
				}
				break;

			case NONE:
				break;
		}

		ptr += 4;
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

int main(int argc, char **argv)
{
	int i = 0;
	size_t failures = 0;
	size_t sumnumfiles = 0;
	size_t numfiles = 0;

	if (argc < 2)
		return usage(argc, argv);

	failures = 0;
	
	for (i = 1; i < argc; ++i)
	{
		if (extract(argv[i], &numfiles))
		{
			sumnumfiles += numfiles;
		}
		else {
			fprintf(stderr, "Error processing file: %s\n", argv[i]);
			failures += 1;
		}
	}

	printf("Extracted %lu file(s).\n", numfiles);
	if (failures > 0)
	{
		fprintf(stderr, "%zu error(s) during extraction.\n", failures);
		return 1;
	}
	return 0;
}

/*
 * audioextract
 *
 * Author: Mathaias Panzenböck
 * This is derived from oggextract:
 * http://ner.mine.nu/oggextract/
 *
 * Original author of oggextract: Adrian Keet
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)
#define __WINDOWS__
#endif

#ifndef __WINDOWS__
#include <endian.h>
#endif

#define OGG_HEADER_SIZE 27
#define ogg_isinitial(data) ((data)[5] & 2)

#define WAVE_HEADER_SIZE 8

#if defined(__WINDOWS__) || __BYTE_ORDER == __LITTLE_ENDIAN

#	define OGG_MAGIC  0x5367674f /* "OggS" (reversed) */

#	define RIFF_MAGIC 0x46464952 /* "RIFF" (reversed) */
#	define WAVE_MAGIC 0x45564157 /* "WAVE" (reversed) */

#	define FORM_MAGIC 0x4d524f46 /* "FORM" (reversed) */
#	define AIFF_MAGIC 0x46464941 /* "AIFF" (reversed) */
#	define AIFC_MAGIC 0x43464941 /* "AIFC" (reversed) */

#elif __BYTE_ORDER == __BIG_ENDIAN

#	define OGG_MAGIC  0x5367674f /* "OggS" */

#	define RIFF_MAGIC 0x46464952 /* "RIFF" */
#	define WAVE_MAGIC 0x57415645 /* "WAVE" */

#	define FORM_MAGIC 0x464f524d /* "FORM" */
#	define AIFF_MAGIC 0x41494646 /* "AIFF" */
#	define AIFC_MAGIC 0x41494643 /* "AIFC" */

#else

#error unsupported endian

#endif

enum fileformat {
	NONE = 0,
	OGG  = 1,
	RIFF = 2,
	AIFF = 3

	/* TODO: MP3, AAC and MKV? */
};

int usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <filename> [<filename> ...]\n", argc <= 0 ? "audioextract" : argv[0]);
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
		}
	}

	return NULL;
}

int ogg_ispage(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	unsigned char nsegs;
	size_t length, i;
	const unsigned char *segs = start + OGG_HEADER_SIZE;

	/* full header available? */
	if (end <= (unsigned char*)OGG_HEADER_SIZE || end - OGG_HEADER_SIZE < start)
		return 0;

	/* capture pattern */
	if (*(const int32_t *)start != OGG_MAGIC)
		return 0;

	/* stream structure version */
	if (start[4] != 0x00)
		return 0;

	/* header type flag */
	if ((start[5] & ~7) != 0x00)
		return 0;
	
	nsegs = start[26];
	length = OGG_HEADER_SIZE + nsegs;

	/* segment sizes fully available? */
	if (end <= (unsigned char*)length || end - length < start)
		return 0;

	for (i = 0; i < nsegs; ++ i)
	{
		length += segs[i];
	}

	/* segments fully available? */
	if (end <= (unsigned char*)length || end - length < start)
		return 0;
	
	if (lengthptr)
		*lengthptr = length;

	/* I think we can reasonably assume it is a real page now */
	return 1;
}

int wave_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t length;

	if (end <= (unsigned char *)WAVE_HEADER_SIZE || end - WAVE_HEADER_SIZE < start)
		return 0;
	
	if (*(const int32_t *)start != RIFF_MAGIC)
		return 0;
	
	length = *(const uint32_t *)(start + 4) + 8;

	if (end <= (unsigned char *)length || end - length < start)
		return 0;
	
	if (*(const uint32_t *)(start + 8) != WAVE_MAGIC)
		return 0;
	
	if (lengthptr)
		*lengthptr = length;

	return 1;
}

int aiff_ischunk(const unsigned char *start, const unsigned char *end, size_t *lengthptr)
{
	size_t length;
	int16_t format;

	if (end <= (unsigned char *)WAVE_HEADER_SIZE || end - WAVE_HEADER_SIZE < start)
		return 0;
	
	if (*(const int32_t *)start != FORM_MAGIC)
		return 0;
	
	length = ntohl(*(const uint32_t *)(start + 4)) + 8;

	if (end <= (unsigned char *)length || end - length < start)
		return 0;
	
	format = *(const uint32_t *)(start + 8);
	if (format != AIFF_MAGIC && format != AIFC_MAGIC)
		return 0;
	
	if (lengthptr)
		*lengthptr = length;

	return 1;
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
	size_t namelen = strlen(filename) + 22;

	printf("Extracting %s\n", filepath);

	fd = open(filepath, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		success = 0;
		goto exit_numfiles;
	}

	if (fstat(fd, &statdata) < 0)
	{
		perror("stat");
		success = 0;
		goto exit_fd;
	}
	if (S_ISDIR(statdata.st_mode))
	{
		fprintf(stderr, "error: Is a directory: %s\n", filepath);
		success = 0;
		goto exit_fd;
	}
	filesize = statdata.st_size;

	filedata = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (filedata == MAP_FAILED)
	{
		perror("mmap");
		success = 0;
		goto exit_fd;
	}

	outfilename = malloc(namelen);
	if (outfilename == NULL)
	{
		perror("malloc");
		success = 0;
		goto exit_munmap;
	}

#define OPEN_OUTFD(ext) \
	snprintf(outfilename, namelen, "%s_%08zx.%s", filename, (size_t)(ptr - filedata), ext); \
	outfd = creat(outfilename, -1); \
	if (outfd < 0) \
	{ \
		perror("creat"); \
		success = 0; \
		goto exit_free; \
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

			case NONE:
				break;
		}

		ptr += 4;
	}

exit_free:
	free(outfilename);

exit_munmap:
	munmap(filedata, filesize);

exit_fd:
	close(fd);

exit_numfiles:
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

/*
 * Author: Adrian Keet
 * Last modified: October 21, 2012
 *
 * This code is in the public domain. Go ahead and use it for whatever you
 * want. I don't care. :)
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define OGG_HEADER_SIZE 27
#define ogg_isinitial(data) ((data)[5] & 2)

int usage()
{
	fprintf(stderr, "Usage: oggextract <filename> [<filename> ...]\n");
	return 255;
}

const unsigned char *findpattern(const unsigned char *start, const unsigned char *end)
{
	end -= 4;
	for (; start < end; ++ start)
	{
		if (*(const int *)start == 0x5367674f) /* "OggS" */
			return start;
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
	if (*(const int *)start != 0x5367674f) /* "OggS" */
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

unsigned int ogg_getlength(const unsigned char *data)
{
	unsigned char nsegs = data[26];
	const unsigned char *segs = data + 27;
	int length = 27 + nsegs;
	int i;

	for (i = 0; i < nsegs; i++)
		length += segs[i];
	return length;
}

const char *basename(const char *path)
{
	const char *ptr = strrchr(path, '/');
#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
	/* Windows supports both / and \ */
	const char *ptr2 = strtchr(path, '\\');
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

	size_t pagelen = 0;
	int outfd = -1;
	int success = 1;
	char *outfilename = NULL;

	size_t numfiles = 0;
	const char *filename = basename(filepath);
	size_t namelen = strlen(filename) + 16;

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
	if (filedata == MAP_FAILED) {
		perror("mmap");
		success = 0;
		goto exit_fd;
	}

	outfilename = malloc(namelen);
	if (outfilename == NULL) {
		perror("malloc");
		success = 0;
		goto exit_munmap;
	}

	ptr = filedata;
	for (end = filedata + filesize; (ptr = findpattern(ptr, end));)
	{
		if (ogg_ispage(ptr, end, &pagelen) && (outfd >= 0 || ogg_isinitial(ptr)))
		{
			pagelen = ogg_getlength(ptr);

			if (outfd < 0)
			{
				snprintf(outfilename, namelen, "%s_%08x.ogg", filename, (unsigned int)(ptr - filedata));
				outfd = creat(outfilename, -1);
				if (outfd < 0)
				{
					perror("creat");
					success = 0;
					goto exit_free;
				}
				printf("writing file: %s\n", outfilename);
				++ numfiles;
			}

			if (write(outfd, ptr, pagelen) < 0)
			{
					perror("write");
					success = 0;
					goto exit_outfd;
			}
			ptr += pagelen;
		}
		else
		{
			if (outfd >= 0)
			{
				close(outfd);
				outfd = -1;
			}
			ptr += 4;
		}
	}

exit_outfd:
	if (outfd >= 0)
		close(outfd);

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
		return usage();

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
		fprintf(stderr, "%lu error(s) during extraction.\n", failures);
		return 1;
	}
	return 0;
}

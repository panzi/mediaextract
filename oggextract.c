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

int usage()
{
	fprintf(stderr, "Usage: oggextract <filename> [<filename> ...]");
	return 255;
}

int findpattern(unsigned char *data, int datalen, int start)
{
	int i;

	for (i = start; i < datalen - 4; i++)
		if (*(int *)(data + i) == 0x5367674f) /* "OggS" */
			return i;
	return -1;
}

int ogg_ispage(unsigned char *data)
{
	/* capture pattern */
	if (*(int *)data != 0x5367674f) /* "OggS" */
		return 0;

	/* stream structure version */
	if (data[4] != 0x00)
		return 0;

	/* header type flag */
	if ((data[5] & ~7) != 0x00)
		return 0;

	/* I think we can reasonably assume it is a real page now */
	return 1;
}

unsigned int ogg_getlength(unsigned char *data)
{
	unsigned char nsegs;
	unsigned char *segs;
	int i;
	int length;

	nsegs = data[26];
	segs = data + 27;
	length = 27 + nsegs;

	for (i = 0; i < nsegs; i++)
		length += segs[i];
	return length;
}

int ogg_isinitial(unsigned char *data)
{
	return data[5] & 2;
}

int ogg_isfinal(unsigned char *data)
{
	return data[5] & 4;
}

int extract(char *filename)
{
	int fd;
	struct stat statdata;
	int filesize;
	unsigned char *filedata, *oggdata;

	int pos = 0;

	int pagelen;
	int outfd = -1;
	char *outfilename;

	int numfiles = 0;

	printf("Extracting %s...\n", filename);
	if (stat(filename, &statdata) < 0)
	{
		perror("stat");
		return 1;
	}
	if (statdata.st_mode & S_IFDIR)
	{
		fprintf(stderr, "error: Is a directory\n", filename);
		return 1;
	}
	filesize = statdata.st_size;

	/* FIXME: prone to race conditions */

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}

	filedata = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);

	outfilename = malloc(strlen(filename) + 16);

	while (1)
	{
		pos = findpattern(filedata, filesize, pos);
		if (pos < 0)
			break;

		oggdata = filedata + pos;
		if (ogg_ispage(oggdata))
		{
			pagelen = ogg_getlength(oggdata);

			if (outfd < 0 && ogg_isinitial(oggdata))
			{
				sprintf(outfilename, "%s_%08x.ogg", filename, pos);
				outfd = creat(outfilename, -1);
				numfiles++;
			}
			if (outfd >= 0)
			{
				write(outfd, oggdata, pagelen);
				if (ogg_isfinal(oggdata))
				{
					close(outfd);
					outfd = -1;
				}
			}
		}

		pos += pagelen;
	}

	if (outfd != -1 && ogg_isfinal(oggdata))
		close(outfd);
	free(outfilename);
	munmap(filedata, filesize);
	close(fd);

	printf("%d file(s) written.\n", numfiles);

	return 0;
}

int main(int argc, char **argv)
{
	int i;
	int failures;

	if (argc < 2)
		return usage();

	failures = 0;
	for (i = 1; i < argc; i++)
		failures += extract(argv[i]);
	if (failures > 0)
		fprintf(stderr, "Errors processing %d file(s).\n", failures);
}

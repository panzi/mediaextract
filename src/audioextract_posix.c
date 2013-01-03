#include "audioextract.h"

int extract(const struct extract_options *options, size_t *numfilesptr)
{
	struct stat statdata;
	size_t filesize = 0;
	int fd = -1;
	int success = 1;
	uint8_t *filedata = NULL;

	fd = open(options->filepath, O_RDONLY, 0644);
	if (fd < 0)
	{
		perror(options->filepath);
		goto error;
	}

	if (fstat(fd, &statdata) < 0)
	{
		perror(options->filepath);
		goto error;
	}
	if (S_ISDIR(statdata.st_mode))
	{
		fprintf(stderr, "%s: Is a directory\n", options->filepath);
		goto error;
	}
	filesize = statdata.st_size;

	if (filesize == 0)
	{
		goto cleanup;
	}

	filedata = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (filedata == MAP_FAILED)
	{
		perror("mmap");
		goto error;
	}

	if (do_extract(filedata, filesize, options, numfilesptr))
		goto cleanup;

error:
	success = 0;

cleanup:
	if (filedata)
		munmap(filedata, filesize);

	if (fd >= 0)
		close(fd);

	return success;
}

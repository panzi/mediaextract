#include "audioextract.h"

#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

int write_data(const char *filename, const uint8_t *data, size_t length)
{
	int outfd = creat(filename, 0644);
	if (outfd < 0)
	{
		perror(filename);
		return 0;
	}

	write(outfd, data, length);
	close(outfd);
	return 1;
}

int extract(const struct extract_options *options, size_t *numfilesptr)
{
	struct stat statdata;
	size_t length = 0;
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

	if (statdata.st_size == 0 || options->length == 0)
	{
		goto cleanup;
	}
	else if (statdata.st_size < 0)
	{
		fprintf(stderr, "error: file has negative size (%"PRIi64")?\n", (int64_t)statdata.st_size);
		goto error;
	}

	length = (uint64_t)options->length > (uint64_t)statdata.st_size ? (size_t)statdata.st_size : options->length;
	filedata = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, (off_t)(options->offset));
	if (filedata == MAP_FAILED)
	{
		perror("mmap");
		goto error;
	}

	if (do_extract(filedata, length, options, numfilesptr))
		goto cleanup;

error:
	success = 0;

cleanup:
	if (filedata)
		munmap(filedata, length);

	if (fd >= 0)
		close(fd);

	return success;
}

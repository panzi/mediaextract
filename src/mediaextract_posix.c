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

#include "mediaextract.h"

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

	if (write(outfd, data, length) < 0) {
		perror(filename);
		return 0;
	}
	close(outfd);
	return 1;
}

int extract(const struct extract_options *options, size_t *numfilesptr, size_t *sumsizeptr)
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
		printf("%s: Skipping empty file.\n", options->filepath);
		goto cleanup;
	}
	else if (statdata.st_size < 0)
	{
		fprintf(stderr, "%s: File has negative size (%"PRIi64")?\n",
			options->filepath,
			(int64_t)statdata.st_size);
		goto error;
	}
	else if ((uint64_t)statdata.st_size <= options->offset)
	{
		printf("%s: Skipping file because offset is bigger than file.\n",
			options->filepath);
		goto cleanup;
	}

	uint64_t rest = (uint64_t)statdata.st_size - options->offset;
	length   = (uint64_t)options->length > rest ? (size_t)rest : options->length;
	filedata = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, (off_t)(options->offset));
	if (filedata == MAP_FAILED)
	{
		perror(options->filepath);
		goto error;
	}

	if (do_extract(filedata, length, options, numfilesptr, sumsizeptr))
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

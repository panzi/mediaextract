#include "audioextract.h"

#include <windows.h>
#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN64
#	define PRIuz PRIu64
#else
#	define PRIuz PRIu32
#endif

static void PrintError(const char *msg)
{
	char ErrStr[512];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		ErrStr, 512, NULL);
	fprintf(stderr, "%s: %s\n", msg, ErrStr);
}

int write_data(const char *filename, const uint8_t *data, size_t length)
{
	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 
		0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD written = 0;

	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintError(filename);
		return 0;
	}

	WriteFile(hFile, data, length, &written, NULL);
	CloseHandle(hFile);

	return 1;
}

int extract(const struct extract_options *options, size_t *numfilesptr)
{
	LARGE_INTEGER filesize = { .QuadPart = 0 };
	int success = 1;
	uint8_t *filedata = NULL;
	HANDLE hFile = CreateFile(options->filepath, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hMap = NULL;
	LARGE_INTEGER offset = { .QuadPart = options->offset };
	SIZE_T length = 0;

	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintError(options->filepath);
		goto error;
	}

	if (!GetFileSizeEx(hFile, &filesize))
	{
		PrintError(options->filepath);
		goto error;
	}

	if (filesize.QuadPart == 0 || options->length == 0)
	{
		printf("%s: Skipping empty file.\n", options->filepath);
		goto cleanup;
	}
	else if (filesize.QuadPart < 0)
	{
		fprintf(stderr, "%s: File has negative size (%"PRIi64")?\n",
			options->filepath,
			filesize.QuadPart);
		goto error;
	}
	else if ((uint64_t)filesize.QuadPart <= options->offset)
	{
		printf("%s: Skipping file because offset is bigger than file.\n",
			options->filepath);
		goto cleanup;
	}
	
	uint64_t rest = filesize.QuadPart - options->offset;
	length = (uint64_t)options->length > (uint64_t)rest ? rest : options->length;
	hMap   = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (hMap == NULL)
	{
		PrintError(options->filepath);
		goto error;
	}

	filedata = MapViewOfFile(hMap, FILE_MAP_READ, offset.HighPart, offset.LowPart, length);

	if (filedata == NULL)
	{
		PrintError(options->filepath);
		goto error;
	}

	if (do_extract(filedata, length, options, numfilesptr))
	{
		goto cleanup;
	}

error:
	success = 0;

cleanup:
	if (filedata != NULL)
		UnmapViewOfFile(filedata);
	
	if (hMap != NULL)
		CloseHandle(hMap);

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return success;
}

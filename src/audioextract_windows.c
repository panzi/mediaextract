#include "audioextract.h"

#include <windows.h>
#include <stdio.h>

static void PrintError(const char *msg)
{
	char ErrStr[512];
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
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

	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintError(options->filepath);
		goto error;
	}

	filesize.LowPart = GetFileSize(hFile, (LPDWORD)&(filesize.HighPart));
	if (filesize.LowPart == INVALID_FILE_SIZE)
	{
		PrintError(options->filepath);
		goto error;
	}

	if (filesize.QuadPart == 0)
		goto cleanup;
	else if ((ULONGLONG)filesize.QuadPart > (size_t)-1)
	{
		fprintf(stderr, "error: cannot map file of this size\n");
		goto error;
	}

	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (hMap == NULL)
	{
		PrintError("CreateFileMapping");
		goto error;
	}

	filedata = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, filesize.QuadPart);

	if (filedata == NULL)
	{
		PrintError("MapViewOfFile");
		goto error;
	}

	if (do_extract(filedata, filesize.QuadPart, options, numfilesptr))
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

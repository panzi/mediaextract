#ifndef MEDIAEXTRACT_FORMATSTRING_H__
#define MEDIAEXTRACT_FORMATSTRING_H__
#pragma once

#include <stddef.h>
#include <stdio.h>

// {filename}_{offset}.{ext}

ssize_t formatstring(char *buffer, size_t buffer_size, const char *format, const char *filename,
                     size_t index, size_t offset, size_t size, const char *ext);

#endif

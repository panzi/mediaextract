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

#ifndef MEDIAEXTRACT_MPEG2_H__
#define MEDIAEXTRACT_MPEG2_H__

#include "mediaextract.h"

#define MPEG_MAGIC           MAGIC("\0\0\x01\xBA")
#define MPEG_SYSHDR_MAGIC    MAGIC("\0\0\x01\xBB")
#define MPEG_PKG_MIN_MAGIC   MAGIC("\x00\x00\x01\xBB")
#define MPEG_PKG_MAX_MAGIC   MAGIC("\x00\x00\x01\xFF")
#define MPEG_END_MAGIC       MAGIC("\0\0\x01\xB9")

#define MPEG_VS_HDR_MAGIC    MAGIC("\0\0\x01\xB3")
#define MPEG_VS_EXT_MAGIC    MAGIC("\0\0\x01\xB5")
#define MPEG_VS_GRP_MAGIC    MAGIC("\0\0\x01\xB8")
#define MPEG_VS_PCT_MAGIC    MAGIC("\0\0\x01\x00")
#define MPEG_VS_END_MAGIC    MAGIC("\0\0\x01\xB7")

#define MPEG_VS_MAGIC        MPEG_VS_HDR_MAGIC
#define MPEG_TS_SYNC_BYTE    0x47

#define IS_MPEG_MAGIC(magic)  ((magic) == MPEG_MAGIC)
#define IS_MPEG_TS_MAGIC(ptr) ((ptr)[0] == MPEG_TS_SYNC_BYTE && (ptr)[3] & 0x30)

int mpeg_isfile(const uint8_t *data, size_t input_len, int formats, size_t *lengthptr);

#endif /* MEDIAEXTRACT_MPEG2_H__ */

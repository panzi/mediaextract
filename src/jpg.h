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

#ifndef MEDIAEXTRACT_JPG_H__
#define MEDIAEXTRACT_JPG_H__

#include "mediaextract.h"

#define JPG_2CC(PTR) (*(const uint16_t *)(PTR))

#define JPG_TEM    JPG_2CC("\xFF\x01")
#define JPG_SOF0   JPG_2CC("\xFF\xC0")
#define JPG_SOF15  JPG_2CC("\xFF\xCF")
#define JPG_DHT    JPG_2CC("\xFF\xC4")
#define JPG_DAC    JPG_2CC("\xFF\xCC")

#define JPG_RST0   JPG_2CC("\xFF\xD0")
#define JPG_RST7   JPG_2CC("\xFF\xD7")
#define JPG_SOI    JPG_2CC("\xFF\xD8")
#define JPG_EOI    JPG_2CC("\xFF\xD9")

#define JPG_SOS    JPG_2CC("\xFF\xDA")
#define JPG_DQT    JPG_2CC("\xFF\xDB")
#define JPG_DNL    JPG_2CC("\xFF\xDC")
#define JPG_DRI    JPG_2CC("\xFF\xDD")
#define JPG_DHP    JPG_2CC("\xFF\xDE")
#define JPG_EXP    JPG_2CC("\xFF\xDF")

#define JPG_APP0   JPG_2CC("\xFF\xE0")
#define JPG_APP15  JPG_2CC("\xFF\xEF")

#define JPG_COM    JPG_2CC("\xFF\xFE")

#define JPG_MAGIC MAGIC("\xFF\xD8\xFF\x00")
#define JPG_MASK  MAGIC("\xFF\xFF\xFF\x00")

#define JPG_SOF_MASK JPG_2CC("\xFF\xF0")
#define JPG_RST_MASK JPG_2CC("\xFF\xF8")
#define JPG_APP_MASK JPG_2CC("\xFF\xF0")

#define IS_JPG_MAGIC(M) (((M) & JPG_MASK) == JPG_MAGIC)

#define IS_JPG_MARKER(PTR) ((PTR)[0] == 0xFF && (PTR)[1] > 0 && (PTR)[1] < 0xFF)
#define IS_JPG_RSTN(PTR)   ((JPG_2CC(PTR) & JPG_RST_MASK) == JPG_RST0)

#define IS_JPG_STANDALONE(M) (((M) & JPG_RST_MASK) == JPG_RST0 || (M) == JPG_TEM)

#define JPG_HEADER_SIZE 4
#define JPG_MIN_SIZE    6

int jpg_isfile(const uint8_t *data, size_t input_len, size_t *lengthptr);

#endif /* MEDIAEXTRACT_JPG_H__ */

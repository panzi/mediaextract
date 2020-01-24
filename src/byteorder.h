/* Copyright (c) 2020 Mathias Panzenböck
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

#ifndef MEDIAEXTRACT_BYTEORDER_H__
#define MEDIAEXTRACT_BYTEORDER_H__
#pragma once

#include <inttypes.h>

#define GET_LE16(PTR) ((uint16_t)(PTR)[0]      | ((uint16_t)(PTR)[1] <<  8))
#define GET_LE32(PTR) ((uint32_t)(PTR)[0]      | ((uint32_t)(PTR)[1] <<  8) | ((uint32_t)(PTR)[2] << 16) | ((uint32_t)(PTR)[3] << 24))
#define GET_LE64(PTR) ((uint64_t)(PTR)[0]      | ((uint64_t)(PTR)[1] <<  8) | ((uint64_t)(PTR)[2] << 16) | ((uint64_t)(PTR)[3] << 24) | \
                       (uint64_t)(PTR)[4] << 32| ((uint64_t)(PTR)[5] << 40) | ((uint64_t)(PTR)[6] << 48) | ((uint64_t)(PTR)[7] << 56))

#define GET_BE16(PTR) ((uint16_t)(PTR)[1]      | ((uint16_t)(PTR)[0] <<  8))
#define GET_BE32(PTR) ((uint32_t)(PTR)[3]      | ((uint32_t)(PTR)[2] <<  8) | ((uint32_t)(PTR)[1] << 16) | ((uint32_t)(PTR)[0] << 24))
#define GET_BE64(PTR) ((uint64_t)(PTR)[7]      | ((uint64_t)(PTR)[6] <<  8) | ((uint64_t)(PTR)[5] << 16) | ((uint64_t)(PTR)[4] << 24) | \
                       (uint64_t)(PTR)[3] << 32| ((uint64_t)(PTR)[2] << 40) | ((uint64_t)(PTR)[1] << 48) | ((uint64_t)(PTR)[0] << 56))


#endif
/*
 *      Copyright (C) 2010-2015 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdafx.h"

#include <emmintrin.h>

#include "pixconv_internal.h"
#include "pixconv_sse2_templates.h"

// This function is only designed for NV12-like pixel formats, like NV12, P010, P016, ...
DECLARE_CONV_FUNC_IMPL(plane_copy_direct_sse4)
{
  const ptrdiff_t inStride     = srcStride[0];
  const ptrdiff_t outStride    = dstStride[0];
  const ptrdiff_t chromaHeight = (height >> 1);

  const ptrdiff_t byteWidth    = (outputFormat == LAVOutPixFmt_P010 || outputFormat == LAVOutPixFmt_P016) ? width << 1 : width;
  const ptrdiff_t stride       = min(FFALIGN(byteWidth, 64), min(inStride, outStride));

  __m128i xmm0,xmm1,xmm2,xmm3;

  _mm_sfence();

  ptrdiff_t line, i;

  for (line = 0; line < height; line++) {
    const uint8_t *y  = (src[0] + line * inStride);
          uint8_t *dy = (dst[0] + line * outStride);
    for (i = 0; i < (stride - 63); i += 64) {
      PIXCONV_STREAM_LOAD(xmm0, y + i +  0);
      PIXCONV_STREAM_LOAD(xmm1, y + i + 16);
      PIXCONV_STREAM_LOAD(xmm2, y + i + 32);
      PIXCONV_STREAM_LOAD(xmm3, y + i + 48);

      _ReadWriteBarrier();

      PIXCONV_PUT_STREAM(dy + i +  0, xmm0);
      PIXCONV_PUT_STREAM(dy + i + 16, xmm1);
      PIXCONV_PUT_STREAM(dy + i + 32, xmm2);
      PIXCONV_PUT_STREAM(dy + i + 48, xmm3);
    }

    for (; i < byteWidth; i += 16) {
      PIXCONV_LOAD_ALIGNED(xmm0, y + i);
      PIXCONV_PUT_STREAM(dy + i, xmm0);
    }
  }

  for (line = 0; line < chromaHeight; line++) {
    const uint8_t *uv  = (src[1] + line * inStride);
          uint8_t *duv = (dst[1] + line * outStride);
    for (i = 0; i < (stride - 63); i += 64) {
      PIXCONV_STREAM_LOAD(xmm0, uv + i +  0);
      PIXCONV_STREAM_LOAD(xmm1, uv + i + 16);
      PIXCONV_STREAM_LOAD(xmm2, uv + i + 32);
      PIXCONV_STREAM_LOAD(xmm3, uv + i + 48);

      _ReadWriteBarrier();

      PIXCONV_PUT_STREAM(duv + i +  0, xmm0);
      PIXCONV_PUT_STREAM(duv + i + 16, xmm1);
      PIXCONV_PUT_STREAM(duv + i + 32, xmm2);
      PIXCONV_PUT_STREAM(duv + i + 48, xmm3);
    }

    for (; i < byteWidth; i += 16) {
      PIXCONV_LOAD_ALIGNED(xmm0, uv + i);
      PIXCONV_PUT_STREAM(duv + i, xmm0);
    }
  }

  return S_OK;
}
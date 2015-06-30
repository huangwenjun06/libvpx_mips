/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "vpx/vpx_integer.h"

#if HAVE_SIMD

void vp8_copy_mem16x16_simd(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{

    int r;

    for (r = 0; r < 16; r++)
    {
#if !(CONFIG_FAST_UNALIGNED)
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
        dst[6] = src[6];
        dst[7] = src[7];
        dst[8] = src[8];
        dst[9] = src[9];
        dst[10] = src[10];
        dst[11] = src[11];
        dst[12] = src[12];
        dst[13] = src[13];
        dst[14] = src[14];
        dst[15] = src[15];

#else
        ((uint32_t *)dst)[0] = ((uint32_t *)src)[0] ;
        ((uint32_t *)dst)[1] = ((uint32_t *)src)[1] ;
        ((uint32_t *)dst)[2] = ((uint32_t *)src)[2] ;
        ((uint32_t *)dst)[3] = ((uint32_t *)src)[3] ;

#endif
        src += src_stride;
        dst += dst_stride;

    }
}

void vp8_copy_mem8x8_simd(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{
    int r;

    for (r = 0; r < 8; r++)
    {
#if !(CONFIG_FAST_UNALIGNED)
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
        dst[6] = src[6];
        dst[7] = src[7];
#else
        ((uint32_t *)dst)[0] = ((uint32_t *)src)[0] ;
        ((uint32_t *)dst)[1] = ((uint32_t *)src)[1] ;
#endif
        src += src_stride;
        dst += dst_stride;

    }

}

void vp8_copy_mem8x4_simd(
    unsigned char *src,
    int src_stride,
    unsigned char *dst,
    int dst_stride)
{
    int r;

    for (r = 0; r < 4; r++)
    {
#if !(CONFIG_FAST_UNALIGNED)
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
        dst[6] = src[6];
        dst[7] = src[7];
#else
        ((uint32_t *)dst)[0] = ((uint32_t *)src)[0] ;
        ((uint32_t *)dst)[1] = ((uint32_t *)src)[1] ;
#endif
        src += src_stride;
        dst += dst_stride;

    }

}

#endif

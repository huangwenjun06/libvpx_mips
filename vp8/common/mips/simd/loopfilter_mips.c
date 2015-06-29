/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdlib.h>
#include "loopfilter.h"
#include "onyxc_int.h"
#include "vp8_rtcd.h"

typedef unsigned char uc;

//common Directory: loopfilter_filters.c
//x86 Directory: loopfilter_x86.c
/*six function 
 **vp8_loop_filter_mbh_simd
 **vp8_loop_filter_mbv_simd
 **vp8_loop_filter_bh_simd
 **vp8_loop_filter_bhs_simd
 **vp8_loop_filter_bv_simd
 **vp8_loop_filter_bvs_simd
 * */

/* Horizontal MB filtering */
void vp8_loop_filter_mbh_simd(unsigned char *y_ptr, unsigned char *u_ptr,
                           unsigned char *v_ptr, int y_stride, int uv_stride,
                           loop_filter_info *lfi)
{
    vp8_mbloop_filter_horizontal_edge_simd(y_ptr, y_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_mbloop_filter_horizontal_edge_simd(u_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_mbloop_filter_horizontal_edge_simd(v_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
}

/* Vertical MB Filtering */
void vp8_loop_filter_mbv_simd(unsigned char *y_ptr, unsigned char *u_ptr,
                           unsigned char *v_ptr, int y_stride, int uv_stride,
                           loop_filter_info *lfi)
{
    vp8_mbloop_filter_vertical_edge_simd(y_ptr, y_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_mbloop_filter_vertical_edge_simd(u_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_mbloop_filter_vertical_edge_simd(v_ptr, uv_stride, lfi->mblim, lfi->lim, lfi->hev_thr, 1);
}

/* Horizontal B Filtering */
void vp8_loop_filter_bh_simd(unsigned char *y_ptr, unsigned char *u_ptr,
                          unsigned char *v_ptr, int y_stride, int uv_stride,
                          loop_filter_info *lfi)
{
    vp8_loop_filter_horizontal_edge_simd(y_ptr + 4 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_horizontal_edge_simd(y_ptr + 8 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_horizontal_edge_simd(y_ptr + 12 * y_stride, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_loop_filter_horizontal_edge_simd(u_ptr + 4 * uv_stride, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_loop_filter_horizontal_edge_simd(v_ptr + 4 * uv_stride, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);
}

void vp8_loop_filter_bhs_simd(unsigned char *y_ptr, int y_stride,
                           const unsigned char *blimit)
{
    vp8_loop_filter_simple_horizontal_edge_simd(y_ptr + 4 * y_stride, y_stride, blimit);
    vp8_loop_filter_simple_horizontal_edge_simd(y_ptr + 8 * y_stride, y_stride, blimit);
    vp8_loop_filter_simple_horizontal_edge_simd(y_ptr + 12 * y_stride, y_stride, blimit);
}

/* Vertical B Filtering */
void vp8_loop_filter_bv_simd(unsigned char *y_ptr, unsigned char *u_ptr,
                          unsigned char *v_ptr, int y_stride, int uv_stride,
                          loop_filter_info *lfi)
{
    vp8_loop_filter_vertical_edge_simd(y_ptr + 4, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_vertical_edge_simd(y_ptr + 8, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);
    vp8_loop_filter_vertical_edge_simd(y_ptr + 12, y_stride, lfi->blim, lfi->lim, lfi->hev_thr, 2);

    if (u_ptr)
        vp8_loop_filter_vertical_edge_simd(u_ptr + 4, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);

    if (v_ptr)
        vp8_loop_filter_vertical_edge_simd(v_ptr + 4, uv_stride, lfi->blim, lfi->lim, lfi->hev_thr, 1);
}

void vp8_loop_filter_bvs_simd(unsigned char *y_ptr, int y_stride,
                           const unsigned char *blimit)
{
    vp8_loop_filter_simple_vertical_edge_simdy_ptr + 4, y_stride, blimit);
    vp8_loop_filter_simple_vertical_edge_simd(y_ptr + 8, y_stride, blimit);
    vp8_loop_filter_simple_vertical_edge_simd(y_ptr + 12, y_stride, blimit);
}

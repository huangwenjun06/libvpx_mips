/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "vpx_ports/mem.h"

extern const short vp8_six_tap_simd[8][6*8];

extern void vp8_filter_block1d_h6_simd
(
    unsigned char   *src_ptr,
    unsigned short  *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    const short      *vp8_filter
);
extern void vp8_filter_block1dc_v6_simd
(
    unsigned short *src_ptr,
    unsigned char  *output_ptr,
    int             output_pitch,
    unsigned int    pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    const short    *vp8_filter
);
extern void vp8_filter_block1d8_h6_simd
(
    unsigned char  *src_ptr,
    unsigned short *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    const short    *vp8_filter
);
extern void vp8_filter_block1d16_h6_simd
(
    unsigned char  *src_ptr,
    unsigned short *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    const short    *vp8_filter
);
extern void vp8_filter_block1d8_v6_simd
(
    unsigned short *src_ptr,
    unsigned char *output_ptr,
    int dst_ptich,
    unsigned int pixels_per_line,
    unsigned int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short    *vp8_filter
);
extern void vp8_filter_block1d16_v6_simd
(
    unsigned short *src_ptr,
    unsigned char *output_ptr,
    int dst_ptich,
    unsigned int pixels_per_line,
    unsigned int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short    *vp8_filter
);
extern void vp8_unpack_block1d16_h6_simd
(
    unsigned char  *src_ptr,
    unsigned short *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    output_height,
    unsigned int    output_width
);
extern void vp8_filter_block1d8_h6_only_simd
(
    unsigned char  *src_ptr,
    unsigned int    src_pixels_per_line,
    unsigned char  *output_ptr,
    int dst_ptich,
    unsigned int    output_height,
    const short    *vp8_filter
);
extern void vp8_filter_block1d16_h6_only_simd
(
    unsigned char  *src_ptr,
    unsigned int    src_pixels_per_line,
    unsigned char  *output_ptr,
    int dst_ptich,
    unsigned int    output_height,
    const short    *vp8_filter
);
extern void vp8_filter_block1d8_v6_only_simd
(
    unsigned char *src_ptr,
    unsigned int   src_pixels_per_line,
    unsigned char *output_ptr,
    int dst_ptich,
    unsigned int   output_height,
    const short   *vp8_filter
);


#if HAVE_SIMD
void vp8_sixtap_predict4x4_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{
    DECLARE_ALIGNED_ARRAY(16, unsigned short, FData2, 16*16);  /* Temp data bufffer used in filtering */
    const short *HFilter, *VFilter;
    HFilter = vp8_six_tap_simd[xoffset];
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line), FData2, src_pixels_per_line, 1, 9, 8, HFilter);
    VFilter = vp8_six_tap_simd[yoffset];
    vp8_filter_block1dc_v6_simd(FData2 + 8, dst_ptr, dst_pitch, 8, 4 , 4, 4, VFilter);

}


void vp8_sixtap_predict16x16_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{

    DECLARE_ALIGNED_ARRAY(16, unsigned short, FData2, 24*24);  /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;


    HFilter = vp8_six_tap_simd[xoffset];

    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line),    FData2,   src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line) + 4,  FData2 + 4, src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line) + 8,  FData2 + 8, src_pixels_per_line, 1, 21, 32, HFilter);
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line) + 12, FData2 + 12, src_pixels_per_line, 1, 21, 32, HFilter);

    VFilter = vp8_six_tap_simd[yoffset];
    vp8_filter_block1dc_v6_simd(FData2 + 32, dst_ptr,   dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_simd(FData2 + 36, dst_ptr + 4, dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_simd(FData2 + 40, dst_ptr + 8, dst_pitch, 32, 16 , 16, 16, VFilter);
    vp8_filter_block1dc_v6_simd(FData2 + 44, dst_ptr + 12, dst_pitch, 32, 16 , 16, 16, VFilter);

}


void vp8_sixtap_predict8x8_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{

    DECLARE_ALIGNED_ARRAY(16, unsigned short, FData2, 256);    /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;

    HFilter = vp8_six_tap_simd[xoffset];
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line),    FData2,   src_pixels_per_line, 1, 13, 16, HFilter);
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line) + 4,  FData2 + 4, src_pixels_per_line, 1, 13, 16, HFilter);

    VFilter = vp8_six_tap_simd[yoffset];
    vp8_filter_block1dc_v6_simd(FData2 + 16, dst_ptr,   dst_pitch, 16, 8 , 8, 8, VFilter);
    vp8_filter_block1dc_v6_simd(FData2 + 20, dst_ptr + 4, dst_pitch, 16, 8 , 8, 8, VFilter);

}


void vp8_sixtap_predict8x4_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{

    DECLARE_ALIGNED_ARRAY(16, unsigned short, FData2, 256);    /* Temp data bufffer used in filtering */

    const short *HFilter, *VFilter;

    HFilter = vp8_six_tap_simd[xoffset];
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line),    FData2,   src_pixels_per_line, 1, 9, 16, HFilter);
    vp8_filter_block1d_h6_simd(src_ptr - (2 * src_pixels_per_line) + 4,  FData2 + 4, src_pixels_per_line, 1, 9, 16, HFilter);

    VFilter = vp8_six_tap_simd[yoffset];
    vp8_filter_block1dc_v6_simd(FData2 + 16, dst_ptr,   dst_pitch, 16, 8 , 4, 8, VFilter);
    vp8_filter_block1dc_v6_simd(FData2 + 20, dst_ptr + 4, dst_pitch, 16, 8 , 4, 8, VFilter);

}

static void filter_block2d_bil
(
    unsigned char *src_ptr,
    unsigned char *dst_ptr,
    unsigned int   src_pitch,
    unsigned int   dst_pitch,
    const short   *HFilter,
    const short   *VFilter,
    int            Width,
    int            Height
)
{

    unsigned short FData[17*16];    /* Temp data buffer used in filtering */

    /* First filter 1-D horizontally... */
    filter_block2d_bil_first_pass(src_ptr, FData, src_pitch, Height + 1, Width, HFilter);

    /* then 1-D vertically... */
    filter_block2d_bil_second_pass(FData, dst_ptr, dst_pitch, Height, Width, VFilter);
}


#endif



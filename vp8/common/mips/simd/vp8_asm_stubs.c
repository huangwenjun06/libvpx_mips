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

#if HAVE_SIMD

static void filter_block2d_first_pass
(
    unsigned char *src_ptr,
    int *output_ptr,
    unsigned int src_pixels_per_line,
    unsigned int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short *vp8_filter
)
{
    unsigned int i, j;
    int  Temp;

    for (i = 0; i < output_height; i++)
    {
        for (j = 0; j < output_width; j++)
        {
            Temp = ((int)src_ptr[-2 * (int)pixel_step] * vp8_filter[0]) +
                   ((int)src_ptr[-1 * (int)pixel_step] * vp8_filter[1]) +
                   ((int)src_ptr[0]                 * vp8_filter[2]) +
                   ((int)src_ptr[pixel_step]         * vp8_filter[3]) +
                   ((int)src_ptr[2*pixel_step]       * vp8_filter[4]) +
                   ((int)src_ptr[3*pixel_step]       * vp8_filter[5]) +
                   (VP8_FILTER_WEIGHT >> 1);      /* Rounding */

            /* Normalize back to 0-255 */
            Temp = Temp >> VP8_FILTER_SHIFT;

            if (Temp < 0)
                Temp = 0;
            else if (Temp > 255)
                Temp = 255;

            output_ptr[j] = Temp;
            src_ptr++;
        }

        /* Next row... */
        src_ptr    += src_pixels_per_line - output_width;
        output_ptr += output_width;
    }
}

static void filter_block2d_second_pass
(
    int *src_ptr,
    unsigned char *output_ptr,
    int output_pitch,
    unsigned int src_pixels_per_line,
    unsigned int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short *vp8_filter
)
{
    unsigned int i, j;
    int  Temp;

    for (i = 0; i < output_height; i++)
    {
        for (j = 0; j < output_width; j++)
        {
            /* Apply filter */
            Temp = ((int)src_ptr[-2 * (int)pixel_step] * vp8_filter[0]) +
                   ((int)src_ptr[-1 * (int)pixel_step] * vp8_filter[1]) +
                   ((int)src_ptr[0]                 * vp8_filter[2]) +
                   ((int)src_ptr[pixel_step]         * vp8_filter[3]) +
                   ((int)src_ptr[2*pixel_step]       * vp8_filter[4]) +
                   ((int)src_ptr[3*pixel_step]       * vp8_filter[5]) +
                   (VP8_FILTER_WEIGHT >> 1);   /* Rounding */

            /* Normalize back to 0-255 */
            Temp = Temp >> VP8_FILTER_SHIFT;

            if (Temp < 0)
                Temp = 0;
            else if (Temp > 255)
                Temp = 255;

            output_ptr[j] = (unsigned char)Temp;
            src_ptr++;

        }

        /* Start next row */
        src_ptr    += src_pixels_per_line - output_width;
        output_ptr += output_pitch;
    }
}


static void filter_block2d
(
    unsigned char  *src_ptr,
    unsigned char  *output_ptr,
    unsigned int src_pixels_per_line,
    int output_pitch,
    const short  *HFilter,
    const short  *VFilter
)
{
    int FData[9*4]; /* Temp data buffer used in filtering */

    /* First filter 1-D horizontally... */
    filter_block2d_first_pass(src_ptr - (2 * src_pixels_per_line), FData, src_pixels_per_line, 1, 9, 4, HFilter);

    /* then filter verticaly... */
    filter_block2d_second_pass(FData + 8, output_ptr, output_pitch, 4, 4, 4, 4, VFilter);
}

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
    const short  *HFilter;
    const short  *VFilter;

    HFilter = vp8_sub_pel_filters[xoffset];   /* 6 tap */
    VFilter = vp8_sub_pel_filters[yoffset];   /* 6 tap */

    filter_block2d(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter);
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
    const short  *HFilter;
    const short  *VFilter;
    int FData[21*24];   /* Temp data buffer used in filtering */


    HFilter = vp8_sub_pel_filters[xoffset];   /* 6 tap */
    VFilter = vp8_sub_pel_filters[yoffset];   /* 6 tap */

    /* First filter 1-D horizontally... */
    filter_block2d_first_pass(src_ptr - (2 * src_pixels_per_line), FData, src_pixels_per_line, 1, 21, 16, HFilter);

    /* then filter verticaly... */
    filter_block2d_second_pass(FData + 32, dst_ptr, dst_pitch, 16, 16, 16, 16, VFilter);

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
    const short  *HFilter;
    const short  *VFilter;
    int FData[13*16];   /* Temp data buffer used in filtering */

    HFilter = vp8_sub_pel_filters[xoffset];   /* 6 tap */
    VFilter = vp8_sub_pel_filters[yoffset];   /* 6 tap */

    /* First filter 1-D horizontally... */
    filter_block2d_first_pass(src_ptr - (2 * src_pixels_per_line), FData, src_pixels_per_line, 1, 13, 8, HFilter);


    /* then filter verticaly... */
    filter_block2d_second_pass(FData + 16, dst_ptr, dst_pitch, 8, 8, 8, 8, VFilter);

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
    const short  *HFilter;
    const short  *VFilter;
    int FData[13*16];   /* Temp data buffer used in filtering */

    HFilter = vp8_sub_pel_filters[xoffset];   /* 6 tap */
    VFilter = vp8_sub_pel_filters[yoffset];   /* 6 tap */

    /* First filter 1-D horizontally... */
    filter_block2d_first_pass(src_ptr - (2 * src_pixels_per_line), FData, src_pixels_per_line, 1, 9, 8, HFilter);


    /* then filter verticaly... */
    filter_block2d_second_pass(FData + 16, dst_ptr, dst_pitch, 8, 8, 4, 8, VFilter);

}

#endif



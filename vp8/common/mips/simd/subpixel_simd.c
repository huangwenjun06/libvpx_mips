#include <stdlib.h>
#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "filter_mips.h"
#include "../../filter.h"
/****************************************************************************
 *
 *  ROUTINE       : filter_block2d_bil
 *
 *  INPUTS        : UINT8  *src_ptr          : Pointer to source block.
 *                  UINT32  src_pitch        : Stride of source block.
 *                  UINT32  dst_pitch        : Stride of destination block.
 *                  INT32  *HFilter          : Array of 2 horizontal filter taps.
 *                  INT32  *VFilter          : Array of 2 vertical filter taps.
 *                  INT32  Width             : Block width
 *                  INT32  Height            : Block height
 *
 *  OUTPUTS       : UINT16 *dst_ptr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 2-D filters an input block by applying a 2-tap
 *                  bi-linear filter horizontally followed by a 2-tap
 *                  bi-linear filter vertically on the result.
 *
 *  SPECIAL NOTES : The largest block size can be handled here is 16x16
 *
 ****************************************************************************/
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

void vp8_bilinear_predict4x4_c
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];
#if 0
    {
        int i;
        unsigned char temp1[16];
        unsigned char temp2[16];

        bilinear_predict4x4_mmx(src_ptr, src_pixels_per_line, xoffset, yoffset, temp1, 4);
        filter_block2d_bil(src_ptr, temp2, src_pixels_per_line, 4, HFilter, VFilter, 4, 4);

        for (i = 0; i < 16; i++)
        {
            if (temp1[i] != temp2[i])
            {
                bilinear_predict4x4_mmx(src_ptr, src_pixels_per_line, xoffset, yoffset, temp1, 4);
                filter_block2d_bil(src_ptr, temp2, src_pixels_per_line, 4, HFilter, VFilter, 4, 4);
            }
        }
    }
#endif
    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 4, 4);

}

void vp8_bilinear_predict8x8_c
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 8);

}

void vp8_bilinear_predict8x4_c
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 4);

}

void vp8_bilinear_predict16x16_c
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 16, 16);
}
/*
 *x86 Directory: x86/subpixel_mmx.asm
 *common Directory: filter.c
 *function:
 *	vp8_bilinear_predict8x8_simd
 *	vp8_bilinear_predict8x4_simd
 * 	vp8_bilinear_predict4x4_simd
 *
 */
void vp8_bilinear_predict8x8_simd
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 8);

}

void vp8_bilinear_predict8x4_simd
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 4);

}

void vp8_bilinear_predict4x4_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];
    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 4, 4);

}

/*
 *x86 Directory: x86/vp8_asm_stubs.c
 *common Directory: filter.c
 *function:
 *      vp8_bilinear_predict16x16_simd
 *
 */
void vp8_bilinear_predict16x16_simd
(
    unsigned char  *src_ptr,
    int   src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int dst_pitch
)
{
    vp8_bilinear_predict8x8_simd(src_ptr,   src_pixels_per_line, xoffset, yoffset, dst_ptr,   dst_pitch);
    vp8_bilinear_predict8x8_simd(src_ptr + 8, src_pixels_per_line, xoffset, yoffset, dst_ptr + 8, dst_pitch);
    vp8_bilinear_predict8x8_simd(src_ptr + 8 * src_pixels_per_line,   src_pixels_per_line, xoffset, yoffset, dst_ptr + dst_pitch * 8,   dst_pitch);
    vp8_bilinear_predict8x8_simd(src_ptr + 8 * src_pixels_per_line + 8, src_pixels_per_line, xoffset, yoffset, dst_ptr + dst_pitch * 8 + 8, dst_pitch);
}


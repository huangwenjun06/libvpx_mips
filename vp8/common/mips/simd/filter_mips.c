#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "../../filter.h"
#include "filter_mips.h"


const short vp8_bilinear_filters_mips_4[8][8] =
{
    { 128, 128, 128, 128,   0,   0,   0,   0 },
    { 112, 112, 112, 112,  16,  16,  16,  16 },
    {  96,  96,  96,  96,  32,  32,  32,  32 },
    {  80,  80,  80,  80,  48,  48,  48,  48 },
    {  64,  64,  64,  64,  64,  64,  64,  64 },
    {  48,  48,  48,  48,  80,  80,  80,  80 },
    {  32,  32,  32,  32,  96,  96,  96,  96 },
    {  16,  16,  16,  16, 112, 112, 112, 112 }
};

const short vp8_bilinear_filters_mips_8[8][16] =
{
    { 128, 128, 128, 128, 128, 128, 128, 128,   0,   0,   0,   0,   0,   0,   0,   0 },
    { 112, 112, 112, 112, 112, 112, 112, 112,  16,  16,  16,  16,  16,  16,  16,  16 },
    {  96,  96,  96,  96,  96,  96,  96,  96,  32,  32,  32,  32,  32,  32,  32,  32 },
    {  80,  80,  80,  80,  80,  80,  80,  80,  48,  48,  48,  48,  48,  48,  48,  48 },
    {  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64 },
    {  48,  48,  48,  48,  48,  48,  48,  48,  80,  80,  80,  80,  80,  80,  80,  80 },
    {  32,  32,  32,  32,  32,  32,  32,  32,  96,  96,  96,  96,  96,  96,  96,  96 },
    {  16,  16,  16,  16,  16,  16,  16,  16, 112, 112, 112, 112, 112, 112, 112, 112 }
};

/* x86 Directory: x86/variance_mmx.c
 * common Directiry: filter.c
 *
 * function:
 * 	filter_block2d_bil_first_pass
 *	filter_block2d_bil_second_pass
 *	filter_block2d_bil
 *	var_filter_block2d_bil_first_pass
 * */

/* x86 Directory: x86/variance_mmx.c
 * common Directiry: variance_c.c
 *
 * function:
 *	var_filter_block2d_bil_first_pass
 *	var_filter_block2d_bil_second_pass
 */
/****************************************************************************
 *
 *  ROUTINE       : filter_block2d_bil_first_pass
 *
 *  INPUTS        : UINT8  *src_ptr    : Pointer to source block.
 *                  UINT32  src_stride : Stride of source block.
 *                  UINT32  height     : Block height.
 *                  UINT32  width      : Block width.
 *                  INT32  *vp8_filter : Array of 2 bi-linear filter taps.
 *
 *  OUTPUTS       : INT32  *dst_ptr    : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block
 *                  in the horizontal direction to produce the filtered output
 *                  block. Used to implement first-pass of 2-D separable filter.
 *
 *  SPECIAL NOTES : Produces INT32 output to retain precision for next pass.
 *                  Two filter taps should sum to VP8_FILTER_WEIGHT.
 *
 ****************************************************************************/
static void filter_block2d_bil_first_pass
(
    unsigned char  *src_ptr,
    unsigned short *dst_ptr,
    unsigned int    src_stride,
    unsigned int    height,
    unsigned int    width,
    const short    *vp8_filter
)
{
    unsigned int i, j;
    short vp8_filter_head=vp8_filter[0];
    short vp8_filter_second=vp8_filter[1];
    short filter_weight=VP8_FILTER_WEIGHT/2;//	64=128/2=VP8_FILTER_WEIGHT/2
    int width_src= src_stride-width;  // width_src=src_stride - width;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            /* Apply bilinear filter */
            dst_ptr[j] = (((int)src_ptr[0] * vp8_filter_head) +
                          ((int)src_ptr[1] * vp8_filter_second) +
                          filter_weight) >> VP8_FILTER_SHIFT;	//>>7
//            dst_ptr[j] = (((int)src_ptr[0] * vp8_filter[0]) +
//                          ((int)src_ptr[1] * vp8_filter[1]) +
//                          (VP8_FILTER_WEIGHT / 2)) >> VP8_FILTER_SHIFT;	//>>7
            src_ptr++;
        }

        /* Next row... */
//        src_ptr += src_stride - width;
        src_ptr += width_src;
        dst_ptr += width;
    }
}

/****************************************************************************
 *
 *  ROUTINE       : filter_block2d_bil_second_pass
 *
 *  INPUTS        : INT32  *src_ptr    : Pointer to source block.
 *                  UINT32  dst_pitch  : Destination block pitch.
 *                  UINT32  height     : Block height.
 *                  UINT32  width      : Block width.
 *                  INT32  *vp8_filter : Array of 2 bi-linear filter taps.
 *
 *  OUTPUTS       : UINT16 *dst_ptr    : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block
 *                  in the vertical direction to produce the filtered output
 *                  block. Used to implement second-pass of 2-D separable filter.
 *
 *  SPECIAL NOTES : Requires 32-bit input as produced by filter_block2d_bil_first_pass.
 *                  Two filter taps should sum to VP8_FILTER_WEIGHT.
 *
 ****************************************************************************/
static void filter_block2d_bil_second_pass
(
    unsigned short *src_ptr,
    unsigned char  *dst_ptr,
    int             dst_pitch,
    unsigned int    height,
    unsigned int    width,
    const short    *vp8_filter
)
{
    unsigned int  i, j;
    int  Temp;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            /* Apply filter */
            Temp = ((int)src_ptr[0]     * vp8_filter[0]) +
                   ((int)src_ptr[width] * vp8_filter[1]) +
                   (VP8_FILTER_WEIGHT / 2);
            dst_ptr[j] = (unsigned int)(Temp >> VP8_FILTER_SHIFT);
            src_ptr++;
        }

        /* Next row... */
        dst_ptr += dst_pitch;
    }
}


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
//static void filter_block2d_bil
void filter_block2d_bil
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
    filter_block2d_bil_first_pass(src_ptr, FData, src_pitch, Height + 1, Width, HFilter);//(8+1,8),(8+1,4),(4+1,4)

    /* then 1-D vertically... */
    filter_block2d_bil_second_pass(FData, dst_ptr, dst_pitch, Height, Width, VFilter);
}

/****************************************************************************
 *
 *  ROUTINE       : var_filter_block2d_bil_first_pass
 *
 *  INPUTS        : UINT8  *src_ptr          : Pointer to source block.
 *                  UINT32 src_pixels_per_line : Stride of input block.
 *                  UINT32 pixel_step        : Offset between filter input samples (see notes).
 *                  UINT32 output_height     : Input block height.
 *                  UINT32 output_width      : Input block width.
 *                  INT32  *vp8_filter          : Array of 2 bi-linear filter taps.
 *
 *  OUTPUTS       : INT32 *output_ptr        : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block in
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement first-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Produces INT32 output to retain precision for next pass.
 *                  Two filter taps should sum to VP8_FILTER_WEIGHT.
 *                  pixel_step defines whether the filter is applied
 *                  horizontally (pixel_step=1) or vertically (pixel_step=stride).
 *                  It defines the offset required to move from one input
 *                  to the next.
 *
 ****************************************************************************/
//static void var_filter_block2d_bil_first_pass
void var_filter_block2d_bil_first_pass
(
    const unsigned char *src_ptr,
    unsigned short *output_ptr,
    unsigned int src_pixels_per_line,
    int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short *vp8_filter
)
{
    unsigned int i, j;

    short vp8_filter_head = vp8_filter[0];
    short vp8_filter_second = vp8_filter[1];
	
    for (i = 0; i < output_height; i++)
    {
        for (j = 0; j < output_width; j++)
        {
            /* Apply bilinear filter */
            output_ptr[j] = (((int)src_ptr[0]          * vp8_filter[0]) +
                             ((int)src_ptr[pixel_step] * vp8_filter[1]) +
                             (VP8_FILTER_WEIGHT / 2)) >> VP8_FILTER_SHIFT;
            src_ptr++;
        }

        /* Next row... */
        src_ptr    += src_pixels_per_line - output_width;
        output_ptr += output_width;
    }
}

/****************************************************************************
 *
 *  ROUTINE       : var_filter_block2d_bil_second_pass
 *
 *  INPUTS        : INT32  *src_ptr          : Pointer to source block.
 *                  UINT32 src_pixels_per_line : Stride of input block.
 *                  UINT32 pixel_step        : Offset between filter input samples (see notes).
 *                  UINT32 output_height     : Input block height.
 *                  UINT32 output_width      : Input block width.
 *                  INT32  *vp8_filter          : Array of 2 bi-linear filter taps.
 *
 *  OUTPUTS       : UINT16 *output_ptr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block in
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement second-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Requires 32-bit input as produced by filter_block2d_bil_first_pass.
 *                  Two filter taps should sum to VP8_FILTER_WEIGHT.
 *                  pixel_step defines whether the filter is applied
 *                  horizontally (pixel_step=1) or vertically (pixel_step=stride).
 *                  It defines the offset required to move from one input
 *                  to the next.
 *
 ****************************************************************************/
//static void var_filter_block2d_bil_second_pass
void var_filter_block2d_bil_second_pass
(
    const unsigned short *src_ptr,
    unsigned char  *output_ptr,
    unsigned int  src_pixels_per_line,
    unsigned int  pixel_step,
    unsigned int  output_height,
    unsigned int  output_width,
    const short *vp8_filter
)
{
    unsigned int  i, j;
    int  Temp;

    for (i = 0; i < output_height; i++)
    {
        for (j = 0; j < output_width; j++)
        {
            /* Apply filter */
            Temp = ((int)src_ptr[0]          * vp8_filter[0]) +
                   ((int)src_ptr[pixel_step] * vp8_filter[1]) +
                   (VP8_FILTER_WEIGHT / 2);
            output_ptr[j] = (unsigned int)(Temp >> VP8_FILTER_SHIFT);
            src_ptr++;
        }

        /* Next row... */
        src_ptr    += src_pixels_per_line - output_width;
        output_ptr += output_width;
    }
}


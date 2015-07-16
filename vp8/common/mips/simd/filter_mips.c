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

short mmx_bi_rd[4] = { 64, 64, 64, 64};
//#define mmx_filter_shift 7

//x86:vp8/common/x86/variance_impl_mmx.asm
void vp8_filter_block2d_bil_var_simd
(
    const unsigned char *ref_ptr,
    int ref_pixels_per_line,
    const unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    const short *HFilter,
    const short *VFilter,
    //unsigned short *HFilter,
    //unsigned short *VFilter,
    int *sum,
    unsigned int *sumsquared
)
{
    __asm__ volatile(
        ".set push \n\t"
        ".set noreorder \n\t"
        ".set arch=loongson3a    \n\t"
	"psubh $f12, $f12, $f12	\n\t"	//pxor mm6, mm6
	"psubh $f14, $f14, $f14	\n\t"	//pxor mm7, mm7
					//rax : HFilter

					//rdx : VFilter
					//rsi : ref_ptr

					//rdi : src_ptr
					//rcx : Height

	"psubh $f0, $f0, $f0	\n\t"	//pxor mm0, mm0
	"ldc1 $f2, 0(%[rsi])	\n\t"	//movq mm1, [rsi]
	"nop		\n\t"
	
	"gsldlc1 $f6, 0x1+0x7(%[rsi]) \n\t"	//movq mm3, [rsi+1]
	"gsldlc1 $f6, 0x1(%[rsi])	\n\t"
	"mov.d $f4, $f2		\n\t"	//movq mm3, mm1
	
	"mov.d $f8, $f6		\n\t"	//movq mm4, mm3
	"punpcklbh $f2, $f2, $f0	\n\t"	//punpcklbw mm1, mm0
	
	"punpckhbh $f4, $f4, $f0	\n\t"	//punpckhbw mm2, mm0
	"ldc1 $f16, 0(%[rax])	\n\t"	//pmullw mm1, [rax]
	"nop	\n\t"
	"pmullh $f2, $f2, $f16	\n\t"

	"pmullh $f4, $f4, $f16	\n\t"	//pmullw mm2, [rax]
	"punpcklbh $f6, $f6, $f0 \n\t"	//punpcklbw mm3, mm0

	"punpckhbh $f4, $f4, $f0	\n\t"	//punpckhbw mm4, mm0
	"ldc1 $f16, 0x8(%[rax])	\n\t"	//pmullw mm3, [rax+8]
	"nop	\n\t"
	"pmullh $f6, $f6, $f16	\n\t"	

	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, [rax+8]
	"paddh $f2, $f2, $f6	\n\t"	//paddw mm1, mm3

	"paddh $f4, $f4, $f8	\n\t"	//paddw mm2, mm4
	"ldc1 $f16, 0(%[bi_rd])	\n\t"	//GLOBAL(mmx_bi_rd)
	"nop	\n\t"
	"paddh $f2, $f2, $f16	\n\t"	//paddw mm1, [GLOBAL(mmx_bi_rd)]
	
	"li $t2, 0x7	\n\t"
	"dmtc1 $t2, $f18	\n\t"
	"psrah $f2, $f2, $f18	\n\t"	//psraw mm1, 7
	"paddh $f4, $f4, $f16	\n\t"	//paddw mm2, [GLOBAL(mmx_bi_rd)]

	"psrah $f4, $f4, $f18	\n\t"	//psraw mm2, 7
	"mov.d $f10, $f2	\n\t"	//movq mm5, mm1

	"packushb $f10, $f10, $f4 \n\t"	//packuswb mm5,mm2
	
	//ABI_IS_32BIT
	"add %[rsi], %[rsi], %[rsi_pixels] \n\t" //add rsi, ref_pixels_per_line

".filter_block2d_bil_var_mmx_loop: \n\t"
	"ldc1 $f2, 0(%[rsi])	\n\t"	//movq mm1, [rsi]
	"nop		\n\t"
	"gsldlc1 $f6, 1+0x7(%[rsi]) \n\t" //movq mm3, [rsi+1]
	"gsldlc1 $f6, 1(%[rsi])	\n\t"
	
	"mov.d $f4, $f2	\n\t"	//movq mm2, mm1
	"mov.d $f8, $f6	\n\t"	//movq mm4, mm3
	
	"punpcklbh $f2, $f2, $f0 \n\t"	//punpcklbw mm1, mm0
	"punpckhbh $f4, $f4, $f0 \n\t"	//punpckhbw mm2, mm0
	
	"ldc1 $f16, 0(%[rax]) \n\t"
	"nop	\n\t"
	"pmullh $f2, $f2, $f16	\n\t"	//pmullw mm1, [rax]
	"pmullh $f4, $f4, $f16	\n\t"	//pmullw mm2, [rax]

	"punpcklbh $f6, $f6, $f0 \n\t"	//punpcklbw mm3, mm0
	"punpckhbh $f8, $f8, $f0 \n\t"	//punpckhbw mm4, mm0

	"ldc1 $f16, 0x8(%[rax]) \n\t"
	"nop	\n\t"
	"pmullh $f6, $f6, $f16	\n\t"	//pmullw mm3, [rax+8]
	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, [rax+8]

	"paddh $f2, $f2, $f6	\n\t"	//paddw mm1, mm3
	"paddh $f4, $f4, $f8	\n\t"	//paddw mm2, mm4

	"ldc1 $f18, 0(%[bi_rd])	\n\t"	//GLOBAL(mmx_bi_rd)
	"nop	\n\t"
	"paddh $f2, $f2, $f18	\n\t"	//paddw mm1, [GLOBAL(mmx_bi_rd)]
	"li $t2, 0x7	\n\t"
	"dmtc1 $t2, $f16	\n\t"
	"psrah $f2, $f2, $f16	\n\t"	//psraw mm1, 7
	
	"paddh $f4, $f4, $f18	\n\t"	//paddw mm2, [GLOBAL(mmx_bi_rd)]
	"psrah $f4, $f4, $f16	\n\t"	//psraw mm2, 7
	
	"mov.d $f6, $f10	\n\t"	//movq mm3, mm5
	"mov.d $f8, $f10	\n\t"	//movq mm4, mm5
	
	"punpcklbh $f6, $f6, $f0 \n\t"	//punpcklbw mm3, mm0
	"punpckhbh $f8, $f8, $f0 \n\t"	//punpckhbw mm4, mm0

	"mov.d $f10, $f2	\n\t"	//movq mm5, mm1
	"packushb $f10, $f10, $f4 \n\t"	//packuswb mm5, mm2
	
	"ldc1 $f16, 0(%[rdx]) \n\t"
	"nop	\n\t"
	"pmullh $f6, $f6, $f16	\n\t"	//pmullw mm3, [rdx]
	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, [rdx]

	"ldc1 $f16, 8(%[rdx]) \n\t"
	"nop	\n\t"
	"pmullh $f2, $f2, $f16	\n\t"	//pmullw mm1, [rdx]
	"pmullh $f4, $f4, $f16	\n\t"	//pmullw mm2, [rdx]
	
	"paddh $f2, $f2, $f6	\n\t"	//paddw mm1, mm3
	"paddh $f4, $f4, $f8	\n\t"	//paddw mm2, mm4
		
	"paddh $f2, $f2, $f18	\n\t"	//paddw mm1, [GLOBAL(mmx_bi_rd)]
	"paddh $f4, $f4, $f18	\n\t"	//paddw mm2, [GLOBAL(mmx_bi_rd)]
	
	"li $t2, 0x7	\n\t"
	"dmtc1 $t2, $f16	\n\t"
	"psrah $f2, $f2, $f16	\n\t"	//psraw mm1, 7
	"psrah $f4, $f4, $f16	\n\t"	//psraw mm2, 7

	"ldc1 $f6, 0(%[rdi]) \n\t"	//movq mm3, [rdi]
	"nop	\n\t"
	"mov.d $f8, $f6	\n\t"		//movq mm4, mm3
	
	"punpcklbh $f6, $f6, $f0 \n\t"	//punpcklbw mm3, mm0
	"punpckhbh $f8, $f8, $f0 \n\t"	//punpckhbw mm4, mm0
	
	"psubh $f2, $f2, $f6 \n\t"	//psubw mm1, mm3
	"psubh $f4, $f4, $f8 \n\t"	//psubw mm2, mm4
	
	"paddh $f12, $f12, $f2	\n\t"	//paddw mm6, mm1
	"pmaddhw $f2, $f2, $f2	\n\t"	//pmaddwd mm1, mm1 //check
	
	"paddh $f12, $f12, $f4	\n\t"	//paddw mm6, mm2
	"pmaddhw $f4, $f4, $f4	\n\t"	//pmaddwd mm2, mm2

	"paddw $f14, $f14, $f2	\n\t"	//paddd mm7, mm1
	"paddw $f14, $f14, $f4	\n\t"	//paddd mm7, mm2

	//ABI_IS_32BIT
	"add %[rsi], %[rsi], %[rsi_pixels] \n\t" //add rsi, ref_pixels_per_line
	"add %[rdi], %[rdi], %[rdi_pixels] \n\t" //add rdi, src_pixels_per_line
	
	"addiu %[rcx], %[rcx], -1	\n\t"	//sub rcx, 1
	"bnez %[rcx], .filter_block2d_bil_var_mmx_loop \n\t"	//jnz

	"psubh $f6, $f6, $f6	\n\t"	//pxor mm3, mm3
	"psubh $f4, $f4, $f4	\n\t"	//pxor mm2, mm2
	
	//check
	"punpcklhw $f4, $f4, $f12	\n\t"	//punpcklwd mm2, mm6
	"punpckhhw $f6, $f6, $f12	\n\t"	//punpckhwd mm3, mm6
	
	"paddw $f4, $f4, $f6	\n\t"	//paddd mm2, mm3
	"mov.d $f12, $f4	\n\t"	//movq mm6, mm2

	"dli $t1,0x20	\n\t"	//32
	"dmtc1 $t1, $f18	\n\t"
	"dsrl $f12, $f12, $f18	\n\t"	//psrlq mm6,32
	"paddw $f4, $f4, $f12	\n\t"	//paddd mm2, mm6

	"dli $t1,0x10	\n\t"	//16
	"dmtc1 $t1, $f16	\n\t"
	"psraw $f12, $f12, $f16	\n\t"	//psrad mm6,16
	"mov.d $f8, $f14	\n\t"	//movq mm4, mm7

	"dsrl $f8, $f8, $f18	\n\t"	//psrlq mm4,32
	"paddw $f8, $f8, $f14	\n\t"	//paddd mm4, mm7

	"swc1 $f4, 0(%[rdi_sum])	\n\t"	//mov rdi, arg(7), movd [rdi], mm2
	"swc1 $f8, 0(%[rsi_sumsquared])	\n\t"	//mov rsi arg(8), movd [rsi], mm4
	//"sdc1 $f4, 0(%[rdi_sum])	\n\t"	//mov rdi, arg(7), movd [rdi], mm2
	//"sdc1 $f8, 0(%[rsi_sumsquared])	\n\t"	//mov rsi arg(8), movd [rsi], mm4
        :
        :[rax]"r"(HFilter),[rdx]"r"(VFilter),[rsi]"r"(ref_ptr),[rdi]"r"(src_ptr),[rcx]"r"(Height),[rsi_pixels]"r"(ref_pixels_per_line),[rdi_pixels]"r"(src_pixels_per_line),[rdi_sum]"r"(sum),[rsi_sumsquared]"r"(sumsquared),[bi_rd]"r"(mmx_bi_rd)
        : "$f0", "$f2","$f4", "$f6", "$f8","$f10", "$f12", "$f14", "$f16", "$f18","$9", "$10","memory"	//t1, t2
	);

}



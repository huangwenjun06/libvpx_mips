#include <stdlib.h>
#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "filter_mips.h"
#include "../../filter.h"

unsigned short rd[] = {
//unsigned char rd[] = {
	0x0040,0x0040,0x0040,0x0040
};

//begin
void vp8_filter_block1d_h6_simd
(
    unsigned char   *src_ptr,
    unsigned short  *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    short           * vp8_filter
)
{
    __asm__ volatile(
        ".set push \n\t"
        ".set noreorder \n\t"
        ".set arch=loongson3a    \n\t"
					//vp8_filter rdx
	//"break 0x05	\n\t"
	"ldc1 $f2, 0x10(%[rdx])	 \n\t"
	"ldc1 $f4, 0x20(%[rdx])	 \n\t"
	"ldc1 $f12, 0x30(%[rdx]) \n\t"
	"ldc1 $f14, 0x40(%[rdx]) \n\t"

					//output_ptr rdi
					//src_ptr rsi
					//output_height rcx
					//output_width rax
	"psubh $f0, $f0, $f0	\n\t"	//pxor mm0,mm0
	
     ".nextrow: \n\t"
	//"ldc1 $f6, -2(%[rsi])	\n\t"	//movq mm3, [rsi-2]
	"gsldlc1 $f6,-2+0x7(%[rsi]) \n\t"
	"gsldrc1 $f6,-2(%[rsi]) \n\t"

	"dli $t1, 0x08	\n\t"
	"mov.d $f8, $f6	\n\t"		//movq mm4, mm3
	"dmtc1 $t1, $f18 \n\t"
	"dsrl $f6, $f6, $f18 \n\t"	//psrlq mm3, 8
	"punpcklbh $f6, $f6, $f0 \n\t"	//punpcklbw mm3, mm0
	"pmullh $f6, $f6, $f2 \n\t"	//pmullw mm3,mm1

	"mov.d $f10, $f8 \n\t"		//movq mm5, mm4
	"punpckhbh $f8, $f8, $f0 \n\t"	//punpckhbw mm4, mm0
	"pmullh $f8, $f8, $f14 \n\t"	//pmullw mm4, mm7
	"paddsh $f6, $f6, $f8 \n\t"	//paddsw mm3, mm4

	"mov.d $f8, $f10 \n\t"		//movq mm4, mm5
	"dli $t1, 0x10	\n\t"
	"dmtc1 $t1, $f18 \n\t"
	"dsrl $f10, $f10, $f18 \n\t"	//psrlq mm5, 16
	"punpcklbh $f10, $f10, $f0 \n\t"//punpcklbw mm5, mm0
	"pmullh $f10, $f10, $f4 \n\t"	//pmullw mm5,mm2
	"paddsh $f6, $f6, $f10 \n\t"	//paddsw mm3, mm5

	
	"mov.d $f10, $f8 \n\t"		//movq mm5, mm4
	"dli $t1, 0x18	\n\t"
	"dmtc1 $t1, $f18 \n\t"
	"dsrl $f8, $f8, $f18 \n\t"	//psrlq mm4, 24
	"punpcklbh $f8, $f8, $f0 \n\t"  //punpcklbw mm8, mm0
	"pmullh $f8, $f8, $f12 \n\t"	//pmullw mm4,mm6
	"paddsh $f6, $f6, $f8 \n\t"	//paddsw mm3, mm4
	
        // do outer positive taps
        "gslwlc1 $f8 ,6(%[rsi])	\n\t"	//movd mm4,[rsi+3]
        "gslwrc1 $f8 ,3(%[rsi])	\n\t"	//movd mm4,[rsi+3]
        //"lwc1 $f8 ,3(%[rsi])	\n\t"	//movd mm4,[rsi+3]	//aligned	//need debug
        "ldc1 $f16 ,0x50(%[rdx]) \n\t"	//$f16 temp
	"punpcklbh $f8, $f8, $f0 \n\t"  //punpcklbw mm8, mm0
	"pmullh $f8, $f8, $f16 \n\t"	//pmullw mm4, [rdx+80]
	"paddsh $f6, $f6, $f8 \n\t"	//paddsw mm3, mm4
	
        "ldc1 $f16 ,0(%[rdx]) \n\t"	//$f16 temp
	"punpcklbh $f10, $f10, $f0 \n\t"//punpcklbw mm5, mm0
	"pmullh $f10, $f10, $f16 \n\t"	//pmullw mm5, [rdx]
	"paddsh $f6, $f6, $f10	\n\t"	//paddsw mm3, mm5

	"ldc1 $f16, 0(%[rd])	\n\t"
	"nop	\n\t"
	"paddsh $f6, $f6, $f16	\n\t"	//paddsw mm3, [GLOBAL(rd)]

	"li $t2, 0x07		\n\t"
	"dmtc1 $t2, $f16	\n\t"
	"psrah $f6, $f6, $f16	\n\t"	//psraw mm3, VP8_FILTER_SHIFT
	"packushb $f6, $f6, $f0	\n\t"	//packuswb mm3, mm0
	"punpcklbh $f6, $f6, $f0\n\t"	//punpcklbw mm3, mm0
	
	"sdc1 $f6, 0(%[rdi]) \n\t"	//movq [rdi], mm3

	//ABI_IS_32BIT
	"add %[rsi], %[rsi], %[rdx_line] \n\t"	//add rsi, dword ptr arg[2]
	"add %[rdi], %[rdi], %[rax]	\n\t"	//add rdi, rax

	"addiu %[rcx],%[rcx], -1	\n\t"	//dec rcx
	"bnez %[rcx], .nextrow	\n\t"		//jnz .nextrow
	"nop	\n\t"
        ".set pop       \n\t"
        ".set reorder \n\t"
        :
        :[rdi]"r"(output_ptr),[rsi]"r"(src_ptr),[rdx_line]"r"(src_pixels_per_line),[rcx]"r"(output_height),[rax]"r"(output_width),[rdx]"r"(vp8_filter),[rd]"r"(rd)
        : "$f0", "$f2","$f4", "$f6", "$f8","$f10", "$f12", "$f14", "$f16", "$f18","$9", "$10","memory"	//t1, t2
	);
}

void vp8_filter_block1dc_v6_simd
(
   short *src_ptr,
   unsigned char *output_ptr,
   int output_pitch,
   unsigned int src_pixels_per_line,
   unsigned int pixel_step,
   unsigned int output_height,
   unsigned int output_width,
   short * vp8_filter
)
{
    __asm__ volatile(
        ".set push \n\t"
        ".set noreorder \n\t"
        ".set arch=loongson3a   \n\t"
	//"break 0x02	\n\t"
	"ldc1 $f10, 0(%[rd])	\n\t"	//movq mm5, [GLOBAL(rd)]

					//rbx vp8_filter rbx
	"ldc1 $f2, 0x10(%[rbx])	\n\t"	//movq mm1, [rbx + 16]
	"ldc1 $f4, 0x20(%[rbx])	\n\t"	//movq mm2, [rbx + 32]
	"ldc1 $f12, 0x30(%[rbx])	\n\t"	//movq mm6, [rbx + 48]
	"ldc1 $f14, 0x40(%[rbx])	\n\t"	//movq mm7, [rbx + 64]
	"nop	\n\t"
	
					//rdx src_pixels_per_line
					//rdi output_ptr
					//rsi src_ptr
	//"break 0x01	\n\t"
	"subu %[rsi], %[rsi], %[rdx]	\n\t"	//sub rsi,rdx
	"subu %[rsi], %[rsi], %[rdx]	\n\t"	//sub rsi,rdx
					//rcx output_height
					//rax output_pitch
	
	"psubh $f0, $f0, $f0	\n\t"	//pxor mm0 mm0

   ".nextrow_cv: \n\t"
	"add $t2, %[rsi], %[rdx]	\n\t"	//rsi+rdx
	"ldc1 $f6, 0($t2)	\n\t"	//movq mm3, [rsi+rdx]
	"nop	\n\t"
	"pmullh $f6, $f6, $f2	\n\t"	//pmullw mm3, mm1
	
	//"dli $t2, 4 \n\t"	
	"li $t2, 4 \n\t"	
	"mul $t2, %[rdx], $t2	\n\t"	//4*rdx
	"add $t2, %[rsi], $t2	\n\t"	//rsi + 4*rdx
	"ldc1 $f8, 0($t2)	\n\t"	//movq mm4, [rsi + 4*rdx]
	"nop	\n\t"
	"pmullh $f8, $f8, $f14	\n\t"	//pmullw mm4, mm7
	"paddsh $f6, $f6, $f8	\n\t"	//paddsw mm3, mm4

	"li $t2, 2 \n\t"	
	//"dli $t2, 2 \n\t"	
	"mul $t2, %[rdx], $t2	\n\t"	//2*rdx
	"add $t2, %[rsi], $t2	\n\t"	//rsi + 4*rdx
	"ldc1 $f8, 0($t2)	\n\t"	//movq mm4, [rsi + 4*rdx]
	"nop	\n\t"
	"pmullh $f8, $f8, $f4	\n\t"	//pmullw mm4, mm2
	"paddsh $f6, $f6, $f8	\n\t"	//paddsw mm3, mm4
	
	"ldc1 $f8, 0(%[rsi])	\n\t"	//movq mm4, [rsi]
	"ldc1 $f16, 0(%[rbx])	\n\t"	//[rbx]
	"nop	\n\t"
	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, [rbx]
	"paddsh $f6, $f6, $f8	\n\t"	//paddsw mm3, mm4
	
	"add %[rsi], %[rsi], %[rdx]	\n\t"	//add rsi, rdx
	//"dli $t2, 2 \n\t"	
	"li $t2, 2 \n\t"	
	"mul $t2, %[rdx], $t2	\n\t"	//2*rdx
	"add $t2, %[rsi], $t2	\n\t"	//rsi + 4*rdx
	"ldc1 $f8, 0($t2)	\n\t"	//movq mm4, [rsi + 4*rdx]
	"nop	\n\t"
	"pmullh $f8, $f8, $f12	\n\t"	//pmullw mm4, mm6
	"paddsh $f6, $f6, $f8	\n\t"	//paddsw mm3, mm4
	
	//"dli $t2, 4 \n\t"	
	"li $t2, 4 \n\t"	
	"mul $t2, %[rdx], $t2	\n\t"	//4*rdx
	"add $t2, %[rsi], $t2	\n\t"	//rsi + 4*rdx
	"ldc1 $f8, 0($t2)	\n\t"	//movq mm4, [rsi + 4*rdx]
	"ldc1 $16, 80(%[rbx])	\n\t"	//[rbx+80]
	"nop	\n\t"
	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, mm7
	"paddsh $f6, $f6, $f8	\n\t"	//paddsw mm3, mm4

	"paddsh $f6, $f6, $f10	\n\t"	//paddsw mm3, mm5
	"li $t2, 0x07	\n\t"
	"dmtc1 $t2,$f16	\n\t"
	"psrah $f6, $f6, $f16	\n\t"	//psraw mm3, VP8_FILTER_SHIFT
	"packushb $f6, $f6, $f0	\n\t"	//packuswb mm3, mm0
	
	"sdc1 $f6, 0(%[rdi])	\n\t"	//movd [rdi], mm3

	"add %[rdi], %[rdi], %[rax]	\n\t"	//lea rdi, [rdi+rax]
	"addiu %[rcx],%[rcx], -1	\n\t"	//dec rcx
	"bnez %[rcx], .nextrow_cv	\n\t"		//jnz .nextrow_cv
	"nop	\n\t"
        ".set pop       \n\t"
        ".set reorder \n\t"
        :
        :[rdi]"r"(output_ptr),[rsi]"r"(src_ptr),[rax]"r"(output_pitch),[rdx]"r"(src_pixels_per_line),[rcx]"r"(output_height),[rbx]"r"(vp8_filter),[rd]"r"(rd)
        : "$f0", "$f2","$f4", "$f6", "$f8","$f10", "$f12", "$f14", "$f16", "$9", "$10","memory"	//t1, t2
	);
}

//end

/*
 *x86 Directory: x86/subpixel_mmx.asm
 *common Directory: filter.c
 *function:
 *	vp8_bilinear_predict8x8_simd
 *	vp8_bilinear_predict8x4_simd
 * 	vp8_bilinear_predict4x4_simd
 *
 */
//unsigned short rd[] = {
////unsigned char rd[] = {
//	0x0040,0x0040,0x0040,0x0040
//};

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
//	for(int i=0
//    HFilter = vp8_bilinear_filters[xoffset];
//    VFilter = vp8_bilinear_filters[yoffset];

    __asm__ volatile(
        ".set push \n\t"
        ".set noreorder \n\t"
        ".set arch=loongson3a   \n\t"
	//"break 0x01	\n\t"
			     //%3 xoffset
			     //%0 dst_ptr	//rdi

			     //%6 address(vp8_bilinear_filters array);	//rcx=%6
	"sll %[x_rax], %[x_rax], 5 \n\t"	     //xoffset * 32	//%3=rax
	
			     //%1 src_ptr
	"add %[x_rax], %[x_rax], %[rcx] \n\t"     //add rax, rcx  //%3 = %6+%3

			     //%5 = dst_pitch	//rdx %5
	"ldc1 $f2,0(%[x_rax]) \n\t"	//load data to f0,f1 from address %3

	"ldc1 $f4,0+0x10(%[x_rax]) \n\t"	//load data to f2,f3 from address %3+8*8	//not 16*4 as x86 //disasembler code
				//%4 yoffset	%4= rax
	
	"psubh $f0,$f0,$f0 \n\t"	//pxor mm0, mm0
	//"fxor $f0,$f0,$f0 \n\t"	//pxor mm0, mm0
	//"xor $f0,$f0,$f0 \n\t"	//pxor mm0, mm0
	"sll %[y_rax], %[y_rax], 5 \n\t"	//yoffset *32

	//"add %[rcx], %[rcx], %[y_rax] \n\t"	//%4=%6+%4	//add rax, rcx
	"add %[y_rax], %[rcx], %[y_rax] \n\t"	//%4=%6+%4	//add rax, rcx
	//"mul $t0, %5, 4	\n\t"	//$t0=%5*4	//rdx * 4	//dst_pitch*4 //%5=rdx
	"sll $t0, %[rdx_pitch], 2	\n\t"
	"add %[rcx], %[rdi], $t0 \n\t"	//%6=%0+$t0	//lea rcx, [rdi+rdx*4]	//%0=rdi
	
				//%2 src_pixels_per_line //rdx %2 //override
	
	//get the first horizontal line done
	"ldc1 $f6,0(%[rsi])	\n\t"	//xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
	"nop	\n\t"
	"mov.d $f8, $f6	\n\t"	//make a copy of current line

	"punpcklbh $f6, $f6 ,$f0 \n\t"	//xx 00 01 02 03 04 05 06punpcklbh $mm3 ,$mm3,$mm0	==> mm0 mm3 mm0 mm3
	"punpckhbh $f8, $f8 ,$f0 \n\t"	//punpckhbh $mm4 ,$mm4,$mm0	==> mm0 mm4 mm0 mm4
	
	"pmullh $f6, $f6 ,$f2	\n\t"	//pmullw mm3, mm1
	"pmullh $f8, $f8 ,$f2	\n\t"	//pmullw mm4, mm1

	"gsldlc1 $f10,0x1+0x7(%[rsi])	\n\t"	//movq mm5, [rsi+1]
	"gsldrc1 $f10,0x1(%[rsi])	\n\t"	//movq mm5, [rsi+1]
	//"ldc1 $f10,0x1(%1)	\n\t"	//movq mm5, [rsi+1]
	//"nop	\n\t"
	"mov.d $f12, $f10	\n\t"	//movq mm6, mm5	

	"punpcklbh $f10, $f10 ,$f0 \n\t"	//punpcklbh $mm5 ,$mm5,$mm0	==> mm0 mm5 mm0 mm5
	"punpckhbh $f12, $f12 ,$f0 \n\t"	//punpckhbh $mm6 ,$mm6,$mm0	==> mm0 mm6 mm0 mm6

	"pmullh $f10, $f10 ,$f4	\n\t"	//pmullw mm5, mm2
	"pmullh $f12, $f12 ,$f4	\n\t"	//pmullw mm6, mm2	//hwj
	
	"paddh $f6, $f6, $f10	\n\t"	//paddw mm3, mm5
	"paddh $f8, $f8, $f12	\n\t"	//paddw mm4, mm6
	
	"ldc1 $f14, 0(%[rd])	\n\t"			//access GLOBLA(rd)
	"nop	\n\t"
	"paddh $f6, $f6, $f14	\n\t"	//paddw mm3, [GLOBAL(rd)]
	//"break 0x09	\n\t"
	"li $t2,0x07		\n\t"
	"dmtc1 $t2,$f16		\n\t"	//not mtc1 $t2,$f16
	"psrah	$f6, $f6, $f16	\n\t"	//xmm3 /= 128

	"paddh $f8, $f8, $f14	\n\t"	//paddw mm3, [GLOBAL(rd)]
	"psrah	$f8, $f8, $f16	\n\t"	//xmm4 /= 128

	"mov.d $f14, $f6	\n\t"	//movq mm7, mm3
	"packushb $f14, $f14, $f8	\n\t"	//packuswb mm7, mm4 ; result=mm4,mm4,mm4,mm4,mm7,mm7,mm7,mm7	f14 f8 f14 f8

	"add %[rsi], %[rdx_line], %[rsi]	\n\t"	//add rsi, rdx; next line
".next_row_84: \n\t"		//label next_row_8*4 start	//same as above code
	"ldc1 $f6, 0(%[rsi])	\n\t"	//xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
	"nop	\n\t"
	//"break 0x02	\n\t"
	"mov.d $f8, $f6	\n\t"	//make a copy of current line
	
	"punpcklbh $f6, $f6 ,$f0 \n\t"	//xx 00 01 02 03 04 05 06punpcklbh $mm3 ,$mm3,$mm0	==> mm0 mm3 mm0 mm3
	"punpckhbh $f8, $f8 ,$f0 \n\t"	//punpckhbh $mm4 ,$mm4,$mm0	==> mm0 mm3 mm0 mm3
	
	"pmullh $f6, $f6 ,$f2	\n\t"	//pmullw mm3, mm1
	"pmullh $f8, $f8 ,$f2	\n\t"	//pmullw mm4, mm1

	"gsldlc1 $f10,0x1+0x7(%[rsi])	\n\t"	//movq mm5, [rsi+1]
	"gsldrc1 $f10,0x1(%[rsi])	\n\t"	//movq mm5, [rsi+1]
	//"ldc1 $f10,0x1(%1)	\n\t"	//movq mm5, [rsi+1]
	"mov.d $f12, $f10	\n\t"	//movq mm6, mm5	

	"punpcklbh $f10, $f10 ,$f0 \n\t"	//punpcklbh $mm5 ,$mm5,$mm0	==> mm0 mm3 mm0 mm3	//punpcklbw mm5, mm0
	"punpckhbh $f12, $f12 ,$f0 \n\t"	//punpckhbh $mm6 ,$mm6,$mm0	==> mm0 mm3 mm0 mm3	//punpckhbw mm6,mm0

	"pmullh $f10, $f10 ,$f4	\n\t"	//pmullw mm5, mm2
	"pmullh $f12, $f12 ,$f4	\n\t"	//pmullw mm6, mm2	//here
	
	"paddh $f6, $f6, $f10	\n\t"	//paddw mm3, mm5
	"paddh $f8, $f8, $f12	\n\t"	//paddw mm4, mm6
	
	"mov.d $f10, $f14	\n\t"	//movq mm5, mm7		//start different from above code
	"mov.d $f12, $f14	\n\t"	//movq mm6, mm7
	
	"punpcklbh $f10, $f10, $f0 \n\t"	//f0,f10,f0,f10	//punpcklbw mm5, mm0	//mm0,mm5,mm0,mm5
	"punpckhbh $f12, $f12, $f0 \n\t"	//f0,f12,f0,f12	//punpckhbw mm6, mm0	//mm0,mm6,mm0,mm6
	
	"ldc1 $f14, 0(%[y_rax])	\n\t"
	"nop	\n\t"
	"pmullh $f10, $f10 , $f14	\n\t"	//pmullw mm5, [rax]
	"pmullh $f12, $f12, $f14	\n\t"	//pmullw mm6, [rax]	//hwj2

	"ldc1 $f14, 0(%[rd])	\n\t"			//access GLOBLA(rd)
	"nop	\n\t"
	"paddh $f6, $f6, $f14	\n\t"	//paddw mm3, [GLOBAL(rd)]	//xmm3 += round value
	"psrah	$f6, $f6, $f16	\n\t"	//xmm3 /= 128

	"paddh $f8, $f8, $f14	\n\t"	//paddw mm4, [GLOBAL(rd)]
	"psrah	$f8, $f8, $f16	\n\t"	//xmm4 /= 128
	//"psrah	$f8, $f8, 7	\n\t"	//xmm4 /= 128
	
	"mov.d $f14, $f6 \n\t"	//movq mm7, mm3
	"packushb $f14, $f14, $f8	\n\t"	//packuswb mm7, mm4
	
	//"ldc1 $f16, 256(%4)	\n\t"	//f16 temp use	//0x10(%4)
	"ldc1 $f16, 16(%[y_rax])	\n\t"	//f16 temp use	//0x10(%4)
	"nop	\n\t"
	"pmullh $f6, $f6 , $f16	\n\t"	//pmullw mm3, [rax]
	"pmullh $f8, $f8, $f16	\n\t"	//pmullw mm4, [rax]
	
	"paddh $f6, $f10, $f6	\n\t"	//paddw mm3, mm5
	"paddh $f8, $f12, $f8	\n\t"	//paddw mm4, mm6

	"ldc1 $f16, 0(%[rd])	\n\t"	//access GLOBLA(rd)
	"nop	\n\t"
	"paddh $f6, $f6, $f16	\n\t"	//paddw mm3, [GLOBAL(rd)]	//xmm3 += round value
	//"mfc1 $t2,$f16"
	//"psrah	$f6, $f6, $f16	\n\t"	//postpone two line //xmm3 /= 128	

	"paddh $f8, $f8, $f16	\n\t"	//paddw mm4, [GLOBAL(rd)]
	//"psrah	$f8, $f8, 7 \n\t"	//xmm4 /= 128
	
	"dmtc1 $t2,$f16	\n\t"		//again set f16=7 VP8_FILTER_SHIFT
	"psrah	$f6, $f6, $f16	\n\t"	//xmm3 /= 128
	"psrah	$f8, $f8, $f16 \n\t"	//xmm4 /= 128

	"packushb $f6, $f6, $f8	\n\t"	//packuswb mm3, mm4

	"sdc1 $f6, 0(%[rdi])	\n\t"	//mov1[rdi], mm3	; store the results in the destination //rdi-->dst_ptr
	
	//if ABI_IS_32BIT
	//abi 32 bit
	"add %[rsi], %[rsi], %[rdx_line]	\n\t"	//add rsi, rdx; //next line	src_ptr + src_pixels_per_line
	"add %[rdi], %[rdi], %[rdx_pitch]	\n\t"	//add rdi, dst_pitch
	//abi 64 bit
	//empty	ToDo
	"bne %[rdi], %[rcx], .next_row_84	\n\t"	//cmp rdi, rcx; jne .next_row_8*4
	"nop	\n\t"
        ".set pop       \n\t"
        ".set reorder \n\t"
	//: "=&r"(dst_ptr)
        //: "=r"(dst_ptr)
        :
        :[rdi]"r"(dst_ptr),[rsi]"r"(src_ptr),[rdx_line]"r"(src_pixels_per_line),[x_rax]"r"(xoffset),[y_rax]"r"(yoffset),[rdx_pitch]"r"(dst_pitch),[rcx]"r"(vp8_bilinear_filters_mips_8),[rd]"r"(rd)
        : "$f0", "$f2","$f4", "$f6", "$f8","$f10", "$f12", "$f14", "$f16", "$8", "$10","memory"	//t0, t2
    );
    /*
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 4);
    */
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


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
//#include "loopfilter.h"
//#include "onyxc_int.h"

typedef unsigned char uc;

//Common Directory: loopfilter_filters.c
//X86 Directory: loopfilter_mmx.asm 
/*six function 
 **vp8_loop_filter_horizontal_edge_simd
 **vp8_loop_filter_vertical_edge_simd
 **vp8_mbloop_filter_horizontal_edge_simd
 **vp8_mbloop_filter_vertical_edge_simd
 **vp8_loop_filter_simple_horizontal_edge_simd
 **vp8_loop_filter_simple_vertical_edge_simd
 * */

unsigned char tfe[] = {
  0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe
};

unsigned char t80[] = {
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

unsigned char t3[] = {
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};

unsigned char t4[] = {
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,0x04
};

unsigned short s27[] = {
  0x1b00, 0x1b00, 0x1b00, 0x1b00
};

unsigned short s63[] = {
  0x003f, 0x003f, 0x003f, 0x003f
};

unsigned short s18[] = {
  0x1200, 0x1200, 0x1200, 0x1200
};

unsigned short s9[] = {
  0x0900, 0x0900, 0x0900, 0x0900
};
static signed char vp8_signed_char_clamp(int t)
{
    t = (t < -128 ? -128 : t);
    t = (t > 127 ? 127 : t);
    return (signed char) t;
}


/* should we apply any filter at all ( 11111111 yes, 00000000 no) */
static signed char vp8_filter_mask(uc limit, uc blimit,
                            uc p3, uc p2, uc p1, uc p0,
                            uc q0, uc q1, uc q2, uc q3)
{
    signed char mask = 0;
    mask |= (abs(p3 - p2) > limit);
    mask |= (abs(p2 - p1) > limit);
    mask |= (abs(p1 - p0) > limit);
    mask |= (abs(q1 - q0) > limit);
    mask |= (abs(q2 - q1) > limit);
    mask |= (abs(q3 - q2) > limit);
    mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > blimit);
    return mask - 1;
}

/* is there high variance internal edge ( 11111111 yes, 00000000 no) */
static signed char vp8_hevmask(uc thresh, uc p1, uc p0, uc q0, uc q1)
{
    signed char hev = 0;
    hev  |= (abs(p1 - p0) > thresh) * -1;
    hev  |= (abs(q1 - q0) > thresh) * -1;
    return hev;
}

static void vp8_filter(signed char mask, uc hev, uc *op1,
        uc *op0, uc *oq0, uc *oq1)

{
    signed char ps0, qs0;
    signed char ps1, qs1;
    signed char filter_value, Filter1, Filter2;
    signed char u;

    ps1 = (signed char) * op1 ^ 0x80;
    ps0 = (signed char) * op0 ^ 0x80;
    qs0 = (signed char) * oq0 ^ 0x80;
    qs1 = (signed char) * oq1 ^ 0x80;

    /* add outer taps if we have high edge variance */
    filter_value = vp8_signed_char_clamp(ps1 - qs1);
    filter_value &= hev;

    /* inner taps */
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (qs0 - ps0));
    filter_value &= mask;

    /* save bottom 3 bits so that we round one side +4 and the other +3
     * if it equals 4 we'll set to adjust by -1 to account for the fact
     * we'd round 3 the other way
     */
    Filter1 = vp8_signed_char_clamp(filter_value + 4);
    Filter2 = vp8_signed_char_clamp(filter_value + 3);
    Filter1 >>= 3;
    Filter2 >>= 3;
    u = vp8_signed_char_clamp(qs0 - Filter1);
    *oq0 = u ^ 0x80;
    u = vp8_signed_char_clamp(ps0 + Filter2);
    *op0 = u ^ 0x80;
    filter_value = Filter1;

    /* outer tap adjustments */
    filter_value += 1;
    filter_value >>= 1;
    filter_value &= ~hev;

    u = vp8_signed_char_clamp(qs1 - filter_value);
    *oq1 = u ^ 0x80;
    u = vp8_signed_char_clamp(ps1 + filter_value);
    *op1 = u ^ 0x80;

}

//loop_filter start
void vp8_loop_filter_horizontal_edge_simd
(
    unsigned char *s,
    int p, /* pitch */
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    int  hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    do
    {
        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4*p], s[-3*p], s[-2*p], s[-1*p],
                               s[0*p], s[1*p], s[2*p], s[3*p]);

        hev = vp8_hevmask(thresh[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);

        vp8_filter(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);

        ++s;
    }
    while (++i < count * 8);
}

void vp8_loop_filter_vertical_edge_simd
(
    unsigned char *s,
    int p,
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    int  hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    do
    {
        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4], s[-3], s[-2], s[-1], s[0], s[1], s[2], s[3]);

        hev = vp8_hevmask(thresh[0], s[-2], s[-1], s[0], s[1]);

        vp8_filter(mask, hev, s - 2, s - 1, s, s + 1);

        s += p;
    }
    while (++i < count * 8);
}

static void vp8_mbfilter(signed char mask, uc hev,
                           uc *op2, uc *op1, uc *op0, uc *oq0, uc *oq1, uc *oq2)
{
    signed char s, u;
    signed char filter_value, Filter1, Filter2;
    signed char ps2 = (signed char) * op2 ^ 0x80;
    signed char ps1 = (signed char) * op1 ^ 0x80;
    signed char ps0 = (signed char) * op0 ^ 0x80;
    signed char qs0 = (signed char) * oq0 ^ 0x80;
    signed char qs1 = (signed char) * oq1 ^ 0x80;
    signed char qs2 = (signed char) * oq2 ^ 0x80;

    /* add outer taps if we have high edge variance */
    filter_value = vp8_signed_char_clamp(ps1 - qs1);
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (qs0 - ps0));
    filter_value &= mask;

    Filter2 = filter_value;
    Filter2 &= hev;

    /* save bottom 3 bits so that we round one side +4 and the other +3 */
    Filter1 = vp8_signed_char_clamp(Filter2 + 4);
    Filter2 = vp8_signed_char_clamp(Filter2 + 3);
    Filter1 >>= 3;
    Filter2 >>= 3;
    qs0 = vp8_signed_char_clamp(qs0 - Filter1);
    ps0 = vp8_signed_char_clamp(ps0 + Filter2);


    /* only apply wider filter if not high edge variance */
    filter_value &= ~hev;
    Filter2 = filter_value;

    /* roughly 3/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 27) >> 7);
    s = vp8_signed_char_clamp(qs0 - u);
    *oq0 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps0 + u);
    *op0 = s ^ 0x80;

    /* roughly 2/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 18) >> 7);
    s = vp8_signed_char_clamp(qs1 - u);
    *oq1 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps1 + u);
    *op1 = s ^ 0x80;

    /* roughly 1/7th difference across boundary */
    u = vp8_signed_char_clamp((63 + Filter2 * 9) >> 7);
    s = vp8_signed_char_clamp(qs2 - u);
    *oq2 = s ^ 0x80;
    s = vp8_signed_char_clamp(ps2 + u);
    *op2 = s ^ 0x80;
}


//mbloop_filter start
void vp8_mbloop_filter_horizontal_edge_simd
(
    unsigned char *s,
    int p,
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    //signed char hev = 0; /* high edge variance */
    //signed char mask = 0;
    //int i = 0;

    /* loop filter designed to work using chars so that we can make maximum use
     * of 8 bit simd instructions.
     */
    /*do
    {

        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4*p], s[-3*p], s[-2*p], s[-1*p],
                               s[0*p], s[1*p], s[2*p], s[3*p]);

        hev = vp8_hevmask(thresh[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);

        vp8_mbfilter(mask, hev, s - 3 * p, s - 2 * p, s - 1 * p, s, s + 1 * p, s + 2 * p);

        ++s;
    }
    while (++i < count * 8);*/
	//__declspec(align(16)) 
	unsigned char t00[8] = {0x0};
	//__declspec(align(16)) 
	unsigned char t01[8] = {0x0};
    __asm__ volatile(
    ".set push \n\t"
    ".set noreorder \n\t"
    ".set arch=loongson3a   \n\t"
    //%0 [rsi_s]: s                           //mov         rsi, arg(0) ;src_ptr       
    //%1 [rax_p]: p                           //movsxd      rax, dword ptr arg(1) ;src_pixel_step     ; destination pitch?
    //%5 [rcx_count]: count                   //movsxd      rcx, dword ptr arg(5) ;count       
".next8_mbh: \n\t"            //label next8_mbh start      //same as above code
    //%3 [rli_limit]: limit                   //mov         rdx, arg(3) ;limit
    "ldc1 $f14, 0(%[rli_limit]) \n\t"         //movq        mm7, [rdx]
    "move $t3, %[rsi_s] \n\t"                 //mov         rdi, rsi              ; rdi points to row +1 for indirect addressing
    "add $t3, $t3, %[rax_p] \n\t"       	  //add         rdi, rax
	
	"sll $t0, %[rax_p], 1 \n\t"              
	"add $t0, $t3, $t0 \n\t"               
	"ldc1 $f4, 0($t0) \n\t"                   //movq        mm2, [rdi+2*rax]      
	"nop \n\t"
	"sll $t0, %[rax_p], 1 \n\t"             
    "add $t0, %[rsi_s], $t0 \n\t"            
    "ldc1 $f2, 0($t0) \n\t"                   //movq        mm1, [rsi+2*rax]      
	"mov.d $f12, $f2 \n\t"                    //movq        mm6, mm1              
	"psubusb $f2, $f2, $f4 \n\t"              //psubusb     mm1, mm2             
	"psubusb $f4, $f4, $f12 \n\t"             //psubusb     mm2, mm6             
	"or $f2, $f2, $f4 \n\t"                   //por         mm1, mm2
	"psubusb $f2, $f2, $f14 \n\t"             //psubusb     mm1, mm7
        
	"add $t0, %[rsi_s], %[rax_p] \n\t"        
	"ldc1 $f8, 0($t0) \n\t"                   //movq        mm4, [rsi+rax]
	"mov.d $f6, $f8 \n\t"                     //movq        mm3, mm4 //make a copy of current line
	"psubusb $f8, $f8, $f12 \n\t"             //psubusb     mm4, mm6
	"psubusb $f12, $f12, $f6 \n\t"            //psubusb     mm6, mm3
	"or $f8, $f8, $f12 \n\t"                  //por         mm4, mm6
	"psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
	"or $f2, $f2, $f8 \n\t"                   //por         mm1, mm4
	
	"ldc1 $f8, 0(%[rsi_s]) \n\t"              //movq        mm4, [rsi] //load data to f4,f5 from address %[rsi_s]
	"mov.d $f0, $f8 \n\t"                     //movq        mm0, mm4 //make a copy of current line
	"psubusb $f8, $f8, $f6 \n\t"              //psubusb     mm4, mm3
	"psubusb $f6, $f6, $f0 \n\t"              //psubusb     mm3, mm0
	"or $f8, $f8, $f6 \n\t"                   //por         mm4, mm3
	"sdc1 $f8, 0(%[t00]) \n\t"                //movq        t0, mm4  //save to t00
	"psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
	"or $f2, $f2, $f8 \n\t"                   //por         mm1, mm4

	"sub %[rax_p], $zero, %[rax_p] \n\t"      //neg         rax                   ; negate pitch to deal with above border
	
	"sll $t1, %[rax_p], 2 \n\t"
    "add $t1, %[rsi_s], $t1 \n\t"
    "ldc1 $f4, 0($t1) \n\t"                   //movq        mm2, [rsi+4*rax]	
	"sll $t1, %[rax_p], 2 \n\t"             
    "add $t1, $t3, $t1 \n\t"            
	"ldc1 $f8, 0($t1) \n\t"                   //movq        mm4, [rdi+4*rax]
	"mov.d $f10, $f8 \n\t"                    //movq        mm5, mm4
	"psubusb $f8, $f8, $f4 \n\t"              //psubusb     mm4, mm2
	"psubusb $f4, $f4, $f10 \n\t"             //psubusb     mm2, mm5
	"or $f8, $f8, $f4 \n\t"                   //por         mm4, mm2
	"psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
	"or $f2, $f2, $f8 \n\t"                   //por         mm1, mm4

	"sll $t1, %[rax_p], 1 \n\t"
    "add $t1, %[rsi_s], $t1 \n\t"
    "ldc1 $f8, 0($t1) \n\t"                   //movq        mm4, [rsi+2*rax]
	"mov.d $f6, $f8 \n\t"                     //movq        mm3, mm4
	"psubusb $f8, $f8, $f10 \n\t"             //psubusb     mm4, mm5
	"psubusb $f10, $f10, $f6 \n\t"            //psubusb     mm5, mm3
	"or $f8, $f8, $f10 \n\t"                  //por         mm4, mm5
	"psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
	"or $f2, $f2, $f8 \n\t"                   //por         mm1, mm4
	
	"mov.d $f4, $f6 \n\t"                     //movq        mm2, mm3

	"add $t1, %[rsi_s], %[rax_p] \n\t"        
	"ldc1 $f8, 0($t1) \n\t"                   //movq        mm4, [rsi+rax]
	"mov.d $f10, $f8 \n\t"                    //movq        mm5, mm4
	"psubusb $f8, $f8, $f6 \n\t"              //psubusb     mm4, mm3
	"psubusb $f6, $f6, $f10 \n\t"             //psubusb     mm3, mm5              
    "or $f8, $f8, $f6 \n\t"                   //por         mm4, mm3              
    "sdc1 $f8, 0(%[t01]) \n\t"                //movq        t1, mm4    //save to t01
    "psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
    "or $f2, $f2, $f8 \n\t"                   //por         mm1, mm4

	"ldc1 $f6, 0($t3) \n\t"                   //movq        mm3, [rdi]
	"mov.d $f8, $f6 \n\t"	                  //movq        mm4, mm3
	"psubusb $f6, $f6, $f4 \n\t"              //psubusb     mm3, mm2
	"psubusb $f4, $f4, $f8 \n\t"              //psubusb     mm2, mm4
	"or $f4, $f4, $f6 \n\t"                   //por         mm2, mm3
	"ldc1 $f22, 0(%[tfe]) \n\t"               //access GLOBAL(tfe)
	"nop    \n\t"
	"and $f4, $f4, $f22 \n\t"	              //pand        mm2, [GLOBAL(tfe)]    ; set lsb of each byte to zero	
	"li $t2, 0x01 \n\t"                       
	"dmtc1 $t2, $f20 \n\t" 
	"psrlh $f4, $f4, $f20 \n\t"               //psrlw       mm2, 1                ; abs(p1-q1)/2
	
    "mov.d $f12, $f10 \n\t"	                  //movq        mm6, mm5
	"mov.d $f6, $f0 \n\t"	                  //movq        mm3, mm0
	"psubusb $f10, $f10, $f6 \n\t"            //psubusb     mm5, mm3
	"psubusb $f6, $f6, $f12 \n\t"             //psubusb     mm3, mm6
	"or $f10, $f10, $f6 \n\t"	              //por         mm5, mm3
	"paddusb $f10, $f10, $f10 \n\t"           //paddusb     mm5, mm5
	"paddusb $f10, $f10, $f4 \n\t"            //paddusb     mm5, mm2
	
	//%2 [rbi_blimit]: blimit	              //mov         rdx, arg(2) ;blimit           ; get blimit
	"ldc1 $f14, 0(%[rbi_blimit]) \n\t"        //movq        mm7, [rdx]
	
	"psubusb $f10, $f10, $f14 \n\t"           //psubusb     mm5,    mm7
	"or $f2, $f2, $f10 \n\t"                  //por         mm1,    mm5
	"psubh $f10, $f10, $f10 \n\t"             //pxor        mm5,    mm5
	"pcmpeqb $f2, $f2, $f10\n\t"              //pcmpeqb     mm1,    mm5 


	//%4 [rti_thresh]:thresh                  //mov         rdx, arg(4) ;thresh
	"ldc1 $f14, 0(%[rti_thresh]) \n\t"        //movq        mm7, [rdx]
	"ldc1 $f8, 0(%[t00]) \n\t"                //movq        mm4, t0
	"psubusb $f8, $f8, $f14 \n\t"             //psubusb     mm4, mm7
	"ldc1 $f6, 0(%[t01]) \n\t"                //movq        mm3, t1
	"psubusb $f6, $f6, $f14 \n\t"             //psubusb     mm3, mm7
	"paddb $f8, $f8, $f6 \n\t"	              //paddb       mm4, mm3
	
	"pcmpeqb $f8, $f8, $f10 \n\t"             //pcmpeqb     mm4,        mm5
	
	"pcmpeqb $f10, $f10, $f10 \n\t"           //pcmpeqb     mm5,        mm5
	"xor $f8, $f8, $f10 \n\t"                 //pxor        mm4,        mm5
	
	"sll $t1, %[rax_p], 1 \n\t"
    "add $t1, %[rsi_s], $t1 \n\t"
    "ldc1 $f4, 0($t1) \n\t"                   //movq        mm2, [rsi+2*rax]
	"ldc1 $f14, 0($t3) \n\t"                  //movq        mm7, [rdi]
	"ldc1 $f22, 0(%[t80]) \n\t"               //access GLOBAL(t80)
    "nop    \n\t"
	"xor $f4, $f4, $f22 \n\t"                 //pxor        mm2, [GLOBAL(t80)]    ; p1 offset to convert to signed values	
	"xor $f14, $f14, $f22 \n\t"               //pxor        mm7, [GLOBAL(t80)]    ; q1 offset to convert to signed values
	"psubsb $f4, $f4, $f14 \n\t"              //psubsb      mm2, mm7
	
	"xor $f12, $f12, $f22 \n\t"	              //pxor        mm6, [GLOBAL(t80)]    ; offset to convert to signed values
	"xor $f0, $f0, $f22 \n\t"                 //pxor        mm0, [GLOBAL(t80)]    ; offset to convert to signed values
	"mov.d $f6, $f0 \n\t"                     //movq        mm3, mm0
	"psubsb $f0, $f0, $f12 \n\t"              //psubsb      mm0, mm6
	"paddsb $f4, $f4, $f0 \n\t"	              //paddsb      mm2, mm0
	"paddsb $f4, $f4, $f0 \n\t"               //paddsb      mm2, mm0
	"paddsb $f4, $f4, $f0 \n\t"               //paddsb      mm2, mm0
	"and $f2, $f2, $f4 \n\t"                  //pand        mm1, mm2
	
	"mov.d $f4, $f2 \n\t"                     //movq        mm2, mm1 
	"and $f4, $f4, $f8 \n\t"                  //pand        mm2, mm4;

	"mov.d $f10, $f4 \n\t"                    //movq        mm5,        mm2
	"ldc1 $f22, 0(%[t3]) \n\t"                //access GLOBAL(t3)	
	"nop    \n\t"
	"paddsb $f10, $f10, $f22 \n\t"	          //paddsb      mm5,        [GLOBAL(t3)]

	"psubh $f0, $f0, $f0 \n\t"	              //pxor        mm0, mm0
	"psubh $f14, $f14, $f14 \n\t"          	  //pxor        mm7, mm7

	"punpcklbh $f0, $f0 ,$f10 \n\t"           //punpcklbw   mm0, mm5
	"li $t2, 0x0B \n\t"
	"dmtc1 $t2, $f20 \n\t"
	"psrah  $f0, $f0, $f20 \n\t"              //psraw       mm0, 11
	"punpckhbh $f14, $f14, $f10 \n\t"         //punpckhbw   mm7, mm5
	"psrah  $f14, $f14, $f20 \n\t"            //psraw       mm7, 11	
	"packsshb $f0, $f0, $f14 \n\t"	          //packsswb    mm0, mm7

	"mov.d $f10, $f0 \n\t"                    //movq        mm5, mm0
	
	"ldc1 $f22, 0(%[t4]) \n\t"                //access GLOBAL(t4)
	"paddsb $f4, $f4, $f22 \n\t"           	  //paddsb      mm2, [GLOBAL(t4)]
	"psubh $f0, $f0, $f0 \n\t"                //pxor        mm0, mm0
	"psubh $f14, $f14, $f14 \n\t"             //pxor        mm7, mm7

	"punpcklbh $f0, $f0 ,$f4 \n\t"            //punpcklbw   mm0, mm2
	"psrah $f0, $f0, $f20 \n\t"               //psraw       mm0, 11
	"punpckhbh $f14, $f14, $f4 \n\t"	      //punpckhbw   mm7, mm2
	"psrah $f14, $f14, $f20 \n\t"             //psraw       mm7, 11
	"packsshb $f0, $f0, $f14 \n\t"            //packsswb    mm0, mm7
	
	"psubsb $f6, $f6, $f0 \n\t"               //psubsb      mm3, mm0
	"paddsb $f12, $f12, $f10 \n\t"            //paddsb      mm6, mm5

	"pandn $f8, $f8, $f2 \n\t"	              //pandn       mm4, mm1
	
	"psubh $f0, $f0, $f0 \n\t"                //pxor        mm0, mm0	
	
	"psubh $f2, $f2, $f2 \n\t"                //pxor        mm1, mm1
	"psubh $f4, $f4, $f4 \n\t"                //pxor        mm2, mm2
	"punpcklbh $f2, $f2 ,$f8 \n\t"            //punpcklbw   mm1, mm4
	"punpckhbh $f4, $f4 ,$f8 \n\t"            //punpckhbw   mm2, mm4
	"ldc1 $f24, 0(%[s27]) \n\t"               //access GLOBAL(s27)
	"pmulhh $f2, $f2, $f24 \n\t"              //pmulhw      mm1, [GLOBAL(s27)]
	"pmulhh $f4, $f4, $f24 \n\t"              //pmulhw      mm2, [GLOBAL(s27)]
	"ldc1 $f26, 0(%[s63]) \n\t"               //access GLOBAL(s63)
	"paddh $f2, $f2, $f26 \n\t"               //paddw       mm1, [GLOBAL(s63)]	
	"paddh $f4, $f4, $f26 \n\t"               //paddw       mm2, [GLOBAL(s63)]
	"li $t2, 0x07 \n\t"
	"dmtc1 $t2, $f20 \n\t"
	"psrah $f2, $f2, $f20 \n\t"               //psraw       mm1, 7
	"psrah $f4, $f4, $f20 \n\t"               //psraw       mm2, 7
	"packsshb $f2, $f2, $f4 \n\t"             //packsswb    mm1, mm2
	
	"psubsb $f6, $f6, $f2 \n\t"               //psubsb      mm3, mm1
	"paddsb $f12, $f12, $f2 \n\t"             //paddsb      mm6, mm1
	
	"ldc1 $f22, 0(%[t80]) \n\t"               //access GLOBAL(t80)	
	"xor $f6, $f6, $f22 \n\t"                 //pxor        mm3, [GLOBAL(t80)]
	"xor $f12, $f12, $f22 \n\t"               //pxor        mm6, [GLOBAL(t80)]
	"add $t1, %[rsi_s], %[rax_p] \n\t"
	"sdc1 $f12, 0($t1) \n\t"                  //movq        [rsi+rax], mm6
	"sdc1 $f6, 0(%[rsi_s]) \n\t"              //movq        [rsi],     mm3

	"psubh $f2, $f2, $f2 \n\t"                //pxor        mm1, mm1
	"psubh $f4, $f4, $f4 \n\t"                //pxor        mm2, mm2
	"punpcklbh $f2, $f2 ,$f8 \n\t"            //punpcklbw   mm1, mm4
	"punpckhbh $f4, $f4 ,$f8 \n\t"            //punpckhbw   mm2, mm4
	"ldc1 $f24, 0(%[s18]) \n\t"               //access GLOBAL(s18)	
	"pmulhh $f2, $f2, $f24 \n\t"              //pmulhw      mm1, [GLOBAL(s18)]
	"pmulhh $f4, $f4, $f24 \n\t"              //pmulhw      mm2, [GLOBAL(s18)]
	"paddh $f2, $f2, $f26 \n\t"               //paddw       mm1, [GLOBAL(s63)]
    "paddh $f4, $f4, $f26 \n\t"               //paddw       mm2, [GLOBAL(s63)]
	"psrah $f2, $f2, $f20 \n\t"	              //psraw       mm1, 7
	"psrah $f4, $f4, $f20 \n\t"               //psraw       mm2, 7
	"packsshb $f2, $f2, $f4 \n\t"             //packsswb    mm1, mm2
	
	"ldc1 $f6, 0($t3) \n\t"                   //movq        mm3, [rdi]
	"sll $t1, %[rax_p], 1 \n\t"
    "add $t1, %[rsi_s], $t1 \n\t"
    "ldc1 $f12, 0($t1) \n\t"                  //movq        mm6, [rsi+2*rax]
	
    "xor $f6, $f6, $f22 \n\t"                 //pxor        mm3, [GLOBAL(t80)]
    "xor $f12, $f12, $f22 \n\t"               //pxor        mm6, [GLOBAL(t80)]
	
	"paddsb $f12, $f12, $f2 \n\t"             //paddsb      mm6, mm1
	"psubsb $f6, $f6, $f2 \n\t"               //psubsb      mm3, mm1
	
	"xor $f12, $f12, $f22 \n\t"               //pxor        mm6, [GLOBAL(t80)]
	"xor $f6, $f6, $f22 \n\t"                 //pxor        mm3, [GLOBAL(t80)]
	"sdc1 $f6, 0($t3) \n\t"                   //movq        [rdi], mm3
	"sll $t1, %[rax_p], 1 \n\t"
	"add $t1, %[rsi_s], $t1 \n\t"
	"sdc1 $f12, 0($t1) \n\t"                  //movq        [rsi+rax*2], mm6
	
	"psubh $f2, $f2, $f2 \n\t"                //pxor        mm1, mm1
    "psubh $f4, $f4, $f4 \n\t"                //pxor        mm2, mm2
    "punpcklbh $f2, $f2 ,$f8 \n\t"            //punpcklbw   mm1, mm4
    "punpckhbh $f4, $f4 ,$f8 \n\t"            //punpckhbw   mm2, mm4
	"ldc1 $f24, 0(%[s9]) \n\t"                //access GLOBAL(s9)   	
    "pmulhh $f2, $f2, $f24 \n\t"              //pmulhw      mm1, [GLOBAL(s9)]
    "pmulhh $f4, $f4, $f24 \n\t"              //pmulhw      mm2, [GLOBAL(s9)]
    "paddh $f2, $f2, $f26 \n\t"               //paddw       mm1, [GLOBAL(s63)]
    "paddh $f4, $f4, $f26 \n\t"               //paddw       mm2, [GLOBAL(s63)]
    "psrah $f2, $f2, $f20 \n\t"               //psraw       mm1, 7
	"psrah $f4, $f4, $f20 \n\t"               //psraw       mm2, 7
	"packsshb $f2, $f2, $f4 \n\t"             //packsswb    mm1, mm2
	
	"sll $t1, %[rax_p], 2 \n\t"
	"add $t1, $t3, $t1 \n\t"
	"ldc1 $f12, 0($t1) \n\t"                  //movq        mm6, [rdi+rax*4]
    "sub %[rax_p], $zero, %[rax_p] \n\t"      //neg         rax
	"add $t1, $t3, %[rax_p] \n\t"
	"ldc1 $f6, 0($t1) \n\t"                   //movq        mm3, [rdi+rax  ]

	"xor $f12, $f12, $f22 \n\t"               //pxor        mm6, [GLOBAL(t80)]
	"xor $f6, $f6, $f22 \n\t"                 //pxor        mm3, [GLOBAL(t80)]
	
	"paddsb $f12, $f12, $f2 \n\t"             //paddsb      mm6, mm1
	"psubsb $f6, $f6, $f2 \n\t"               //psubsb      mm3, mm1
	
	"xor $f12, $f12, $f22 \n\t"     	      //pxor        mm6, [GLOBAL(t80)]
	"xor $f6, $f6, $f22 \n\t"                 //pxor        mm3, [GLOBAL(t80)]
	"add $t1, $t3, %[rax_p] \n\t"
	"sdc1 $f6, 0($t1) \n\t"	                  //movq        [rdi+rax  ], mm3
	"sub %[rax_p], $zero, %[rax_p] \n\t"      //neg         rax
	"sll $t1, %[rax_p], 2 \n\t"
	"add $t1, $t3, $t1 \n\t"
	"sdc1 $f12, 0($t1) \n\t"                  //movq        [rdi+rax*4], mm6
	
	"sub %[rax_p], $zero, %[rax_p] \n\t"      //neg         rax
	"addi %[rsi_s], %[rsi_s], 0x08 \n\t"      //add         rsi,8
	"addiu %[rcx_count], %[rcx_count], -1 \n\t"    //dec         rcx
	"bnez %[rcx_count], .next8_mbh \n\t"	  //jnz         .next8_mbh
	"nop    \n\t"
	".set pop \n\t"
	".set reorder \n\t"
	
    :
    :[rsi_s]"r"(s),[rax_p]"r"(p),[rbi_blimit]"r"(blimit),[rli_limit]"r"(limit),[rti_thresh]"r"(thresh),[rcx_count]"r"(count),[tfe]"r"(tfe),[t80]"r"(t80),[t3]"r"(t3),[t4]"r"(t4),[s27]"r"(s27),[s63]"r"(s63),[s18]"r"(s18),[s9]"r"(s9),[t00]"r"(t00),[t01]"r"(t01)
    : "$f0", "$f2","$f4", "$f6", "$f8","$f10", "$f12", "$f14", "$f16", "$f18", "$f20", "$f22", "$f24", "$f26", "$8", "$9", "$10", "$11",  "memory" //t0, t1, t2, t3
    );

}


void vp8_mbloop_filter_vertical_edge_simd
(
    unsigned char *s,
    int p,
    const unsigned char *blimit,
    const unsigned char *limit,
    const unsigned char *thresh,
    int count
)
{
    signed char hev = 0; /* high edge variance */
    signed char mask = 0;
    int i = 0;

    do
    {

        mask = vp8_filter_mask(limit[0], blimit[0],
                               s[-4], s[-3], s[-2], s[-1], s[0], s[1], s[2], s[3]);

        hev = vp8_hevmask(thresh[0], s[-2], s[-1], s[0], s[1]);

        vp8_mbfilter(mask, hev, s - 3, s - 2, s - 1, s, s + 1, s + 2);

        s += p;
    }
    while (++i < count * 8);

}

/* should we apply any filter at all ( 11111111 yes, 00000000 no) */
static signed char vp8_simple_filter_mask(uc blimit, uc p1, uc p0, uc q0, uc q1)
{
/* Why does this cause problems for win32?
 * error C2143: syntax error : missing ';' before 'type'
 *  (void) limit;
 */
    signed char mask = (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  <= blimit) * -1;
    return mask;
}

static void vp8_simple_filter(signed char mask, uc *op1, uc *op0, uc *oq0, uc *oq1)
{
    signed char filter_value, Filter1, Filter2;
    signed char p1 = (signed char) * op1 ^ 0x80;
    signed char p0 = (signed char) * op0 ^ 0x80;
    signed char q0 = (signed char) * oq0 ^ 0x80;
    signed char q1 = (signed char) * oq1 ^ 0x80;
    signed char u;

    filter_value = vp8_signed_char_clamp(p1 - q1);
    filter_value = vp8_signed_char_clamp(filter_value + 3 * (q0 - p0));
    filter_value &= mask;

    /* save bottom 3 bits so that we round one side +4 and the other +3 */
    Filter1 = vp8_signed_char_clamp(filter_value + 4);
    Filter1 >>= 3;
    u = vp8_signed_char_clamp(q0 - Filter1);
    *oq0  = u ^ 0x80;

    Filter2 = vp8_signed_char_clamp(filter_value + 3);
    Filter2 >>= 3;
    u = vp8_signed_char_clamp(p0 + Filter2);
    *op0 = u ^ 0x80;
}

//loop_filter_simple start
void vp8_loop_filter_simple_horizontal_edge_simd
(
    unsigned char *s,
    int p,
    const unsigned char *blimit
)
{
    signed char mask = 0;
    int i = 0;

    do
    {
        mask = vp8_simple_filter_mask(blimit[0], s[-2*p], s[-1*p], s[0*p], s[1*p]);
        vp8_simple_filter(mask, s - 2 * p, s - 1 * p, s, s + 1 * p);
        ++s;
    }
    while (++i < 16);
}

void vp8_loop_filter_simple_vertical_edge_simd
(
    unsigned char *s,
    int p,
    const unsigned char *blimit
)
{
    signed char mask = 0;
    int i = 0;

    do
    {
        mask = vp8_simple_filter_mask(blimit[0], s[-2], s[-1], s[0], s[1]);
        vp8_simple_filter(mask, s - 2, s - 1, s, s + 1);
        s += p;
    }
    while (++i < 16);

}

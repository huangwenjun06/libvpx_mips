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
#include "vpx_mem/vpx_mem.h"
#include <stdio.h>
//x86 Directory:	x86/dequantize_mmx.asm
//common Directory: 	dequantize.c
/*
 ** vp8_dequant_idct_add_simd
 ** vp8_dequantize_b_impl_mmx : function not implement ; it Doesn't matter; it just called by vp8_dequantize_b_mmx in idct_blk_mmx.c
 * */
#if HAVE_SIMD
void vp8_dequant_idct_add_simd(short *input, short *dq,
                                unsigned char *dest, int stride)
{
    __asm__ volatile( 
	".set push \n\t"
	".set noreorder	\n\t"
	".set arch=loongson2f	\n\t"
	//"break 01	\n\t"
	"ldc1 $f0,0(%1) \n\t"   //load 64 bit data to f0
        "ldc1 $f2,0(%2) \n\t"   //load 64 bit data to f2
        "nop            \n\t"
        "pmullh $f0, $f0, $f2   \n\t"   //four 16bit data signed mul, get low
        "sdc1 $f0,0(%0) \n\t"   //store 64 bit data from f0 to memory
        "ldc1 $f0,8(%1) \n\t"   //second:load 64 bit data to f0
        "ldc1 $f2,8(%2) \n\t"   //load 64 bit data to f2
        "nop            \n\t"
        "pmullh $f0, $f0, $f2   \n\t"   //four 16bit data signed mul, get low
        "sdc1 $f0,8(%0) \n\t"   //store 64 bit data from f0 to memory
        "ldc1 $f0,16(%1) \n\t"   //third:load 64 bit data to f0
        "ldc1 $f2,16(%2) \n\t"   //load 64 bit data to f2
        "nop            \n\t"
        "pmullh $f0, $f0, $f2   \n\t"   //four 16bit data signed mul, get low
        "sdc1 $f0,16(%0) \n\t"   //store 64 bit data from f0 to memory
        "ldc1 $f0,24(%1)        \n\t"   //forth:load 64 bit data to f0
        "ldc1 $f2,24(%2)        \n\t"   //load 64 bit data to f2
        "nop            \n\t"
        "pmullh $f0, $f0, $f2   \n\t"   //four 16bit data signed mul, get low
        "sdc1 $f0,24(%0)        \n\t"   //store 64 bit data from f0 to memory
        : "=r"(input)
        : "0"(input),"r"(dq)
        : "$f0", "$f2","memory"
    );
    //printf("message from mips simd\n");
    
//    int i;
//    for (i = 0; i < 16; i++)
//    {
//        input[i] = dq[i] * input[i];
//    }
    vp8_short_idct4x4llm_simd(input, dest, stride, dest, stride);

    vpx_memset(input, 0, 32);

}

#endif

/* x86 Directory: x86/sad_mmx.asm
 * common Directory:sad_c.c
 *
 * function:
 * 	sad_mx_n_simd
 *	vp8_sad16x16_simd
 *	vp8_sad8x8_simd
 *	vp8_sad16x8_simd
 *	vp8_sad8x16_simd
 *	vp8_sad4x4_simd
 *
 * */
static unsigned int sad_mx_n_simd(const unsigned char *src_ptr, int src_stride,
                               const unsigned char *ref_ptr, int ref_stride,
                               unsigned int max_sad, int m, int n)
{
    int r, c;
    unsigned int sad = 0;

    for (r = 0; r < n; r++)
    {
        for (c = 0; c < m; c++)
        {
            sad += abs(src_ptr[c] - ref_ptr[c]);
        }

        if (sad > max_sad)
          break;

        src_ptr += src_stride;
        ref_ptr += ref_stride;
    }

    return sad;
}

/* max_sad is provided as an optional optimization point. Alternative
 * implementations of these functions are not required to check it.
 */

unsigned int vp8_sad16x16_simd(const unsigned char *src_ptr, int src_stride,
                            const unsigned char *ref_ptr, int ref_stride,
                            unsigned int max_sad)
{
    unsigned int ret;
    asm volatile(
".set arch=loongson2f\n\t"
".set noreorder\n\t"
"   move   $t0,%2\n\t"//move args to register because a0-a4 maybe changed
"   move   $t1,%3\n\t"
"   move   $t2,%4\n\t"
"   move   $t3,%5\n\t"
"   move $t4,$zero\n\t"//loop count
"   pandn $f0,$f0,$f0\n\t"//result
"   b sad16x16loopend\n\t"//loop
"   nop\n\t"
"   sad16x16loopbegin:\n\t"
"   ldc1 $f4,0($t0)\n\t"//load eight numbers to f4 from src_ptr
"   ldc1 $f6,8($t0)\n\t"//load eight numbers to f6 from src_ptr+8
"   ldc1 $f8,0($t2)\n\t"//load eight numbers to f8 from ref_ptr
"   ldc1 $f10,8($t2)\n\t"//load eight numbers to f10 from ref_ptr+8
"   pasubub $f4,$f4,$f8\n\t"//abs (f4-f8) by byte,reference to godson2e.mmi.pdf
"   pasubub $f6,$f6,$f10\n\t"//abs (f6-f10) by byte
"   biadd   $f4,$f4\n\t" //add f4 by byte
"   biadd   $f6,$f6\n\t" //add f6 by byte
"   paddw   $f4,$f4,$f6\n\t"//add f4,f6
"   paddw   $f0,$f0,$f4\n\t"//add f4,f0
"   mfc1    $t6,$f0\n\t"
"   sltu    $t5,$t6,%1\n\t"//f0<max_sad?1:0
"   beqz    $t5,loopend16x16\n\t"// f0 > max_sad ,break loop
"   nop\n\t"
"   addiu   $t4,$t4,1\n\t"//loop count ++
"   addu    $t0,$t0,$t1\n\t"//modify src_ptr
"   addu    $t2,$t2,$t3\n\t"//modify ref_ptr

"   sad16x16loopend:\n\t"
"   slti    $t5,$t4,16\n\t"
"   bnez    $t5,sad16x16loopbegin\n\t"
"   nop\n\t"
"   loopend16x16:\n\t"
"   sdc1    $f0,%0\n\t"
".set reorder\n\t"
".set mips0\n\t"
        :"=m"(ret)
        :"r"(max_sad),"r"(src_ptr),"r"(src_stride),"r"(ref_ptr),"r"(ref_stride)
        :"t0","t1","t2","t3","t4","t5","t6","$f0","$f4","$f6","$f8","$f10","memory"
   );
return ret;
   //return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 16, 16);
}

unsigned int vp8_sad8x8_simd(const unsigned char *src_ptr, int src_stride,
                          const unsigned char *ref_ptr, int ref_stride,
                          unsigned int max_sad)
{
    unsigned int ret;
    asm volatile(
".set arch=loongson2f\n\t"
".set noreorder\n\t"

"   move   $t0,%2\n\t"
"   move   $t1,%3\n\t"
"   move   $t2,%4\n\t"
"   move   $t3,%5\n\t"
"   move $t4,$zero\n\t"
"   pandn $f0,$f0,$f0\n\t"
"   b sad8x8loopend\n\t"
"   nop\n\t"
"   sad8x8loopbegin:\n\t"
"   ldc1 $f4,0($t0)\n\t"
"   ldc1 $f8,0($t2)\n\t"
                           
"   pasubub $f4,$f4,$f8\n\t"
"   biadd   $f4,$f4\n\t"
"   paddw   $f0,$f0,$f4\n\t"
"   mfc1    $t6,$f0\n\t"   

"   sltu    $t5,$t6,%1\n\t"
"   beqz    $t5,loopend8x8\n\t"

"   nop\n\t"
"   addiu   $t4,$t4,1\n\t"
"   addu    $t0,$t0,$t1\n\t"
"   addu    $t2,$t2,$t3\n\t"

"   sad8x8loopend:\n\t"
"   slti    $t5,$t4,8\n\t"
"   bnez    $t5,sad8x8loopbegin\n\t"
"   nop\n\t"
"   loopend8x8:\n\t"
"   sdc1    $f0,%0\n\t"
".set reorder\n\t"
".set mips0\n\t"
        :"=m"(ret)
        :"r"(max_sad),"r"(src_ptr),"r"(src_stride),"r"(ref_ptr),"r"(ref_stride)
        :"t0","t1","t2","t3","t4","t5","t6","$f0","$f4","$f8","memory"
   );
return ret;

//   return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 8, 8);
}

unsigned int vp8_sad16x8_simd(const unsigned char *src_ptr, int src_stride,
                           const unsigned char *ref_ptr, int ref_stride,
                           unsigned int max_sad)
{
    unsigned int ret;
    asm volatile(
".set arch=loongson2f\n\t"
".set noreorder\n\t"
"   move   $t0,%2\n\t"
"   move   $t1,%3\n\t"
"   move   $t2,%4\n\t"
"   move   $t3,%5\n\t"
"   move $t4,$zero\n\t"
"   pandn $f0,$f0,$f0\n\t"
"   b sad16x8loopend\n\t"
"   nop\n\t"
"   sad16x8loopbegin:\n\t"
"   ldc1 $f4,0($t0)\n\t"
"   ldc1 $f6,8($t0)\n\t"
"   ldc1 $f8,0($t2)\n\t"
"   ldc1 $f10,8($t2)\n\t"  
                           
"   pasubub $f4,$f4,$f8\n\t"
"   pasubub $f6,$f6,$f10\n\t"
"   biadd   $f4,$f4\n\t"
"   biadd   $f6,$f6\n\t"
"   paddw   $f4,$f4,$f6\n\t"
"   paddw   $f0,$f0,$f4\n\t"
"   mfc1    $t6,$f0\n\t"   
"   sltu    $t5,$t6,%1\n\t"
"   beqz    $t5,loopend16x8\n\t"

"   nop\n\t"
"   addiu   $t4,$t4,1\n\t"
"   addu    $t0,$t0,$t1\n\t"
"   addu    $t2,$t2,$t3\n\t"

"   sad16x8loopend:\n\t"
"   slti    $t5,$t4,8\n\t"
"   bnez    $t5,sad16x8loopbegin\n\t"
"   nop\n\t"
"   loopend16x8:\n\t"
"   sdc1    $f0,%0\n\t"
".set reorder\n\t"
".set mips0\n\t"
        :"=m"(ret)
        :"r"(max_sad),"r"(src_ptr),"r"(src_stride),"r"(ref_ptr),"r"(ref_stride)
        :"t0","t1","t2","t3","t4","t5","t6","$f0","$f4","$f6","$f8","$f10","memory"
   );
return ret;

//   return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 16, 8);

}

unsigned int vp8_sad8x16_simd(const unsigned char *src_ptr, int src_stride,
                           const unsigned char *ref_ptr, int ref_stride,
                           unsigned int max_sad)
{
    unsigned int ret;
    asm volatile(
".set arch=loongson2f\n\t"
".set noreorder\n\t"
"   move   $t0,%2\n\t"
"   move   $t1,%3\n\t"
"   move   $t2,%4\n\t"
"   move   $t3,%5\n\t"
"   move $t4,$zero\n\t"
"   pandn $f0,$f0,$f0\n\t"
"   b sad8x16loopend\n\t"
"   nop\n\t"
"   sad8x16loopbegin:\n\t"
"   ldc1 $f4,0($t0)\n\t"
"   ldc1 $f8,0($t2)\n\t"  
                           
"   pasubub $f4,$f4,$f8\n\t"
"   biadd   $f4,$f4\n\t"
"   paddw   $f0,$f0,$f4\n\t"
"   mfc1    $t6,$f0\n\t"   
"   sltu    $t5,$t6,%1\n\t"
"   beqz    $t5,loopend8x16\n\t"

"   nop\n\t"
"   addiu   $t4,$t4,1\n\t"
"   addu    $t0,$t0,$t1\n\t"
"   addu    $t2,$t2,$t3\n\t"

"   sad8x16loopend:\n\t"
"   slti    $t5,$t4,16\n\t"
"   bnez    $t5,sad8x16loopbegin\n\t"
"   nop\n\t"
"   loopend8x16:\n\t"
"   sdc1    $f0,%0\n\t"
".set reorder\n\t"
".set mips0\n\t"
        :"=m"(ret)
        :"r"(max_sad),"r"(src_ptr),"r"(src_stride),"r"(ref_ptr),"r"(ref_stride)
        :"t0","t1","t2","t3","t4","t5","t6","$f0","$f4","$f8","memory"
   );
return ret;

//   return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 8, 16);
}

unsigned int vp8_sad4x4_simd(const unsigned char *src_ptr, int src_stride,
                          const unsigned char *ref_ptr, int ref_stride,
                          unsigned int max_sad)
{
    unsigned int ret;
    asm volatile(
".set arch=loongson2f\n\t"
".set noreorder\n\t"
"   move   $t0,%2\n\t"
"   move   $t1,%3\n\t"
"   move   $t2,%4\n\t"
"   move   $t3,%5\n\t"
"   move $t4,$zero\n\t"
"   pandn $f0,$f0,$f0\n\t"
"   b sad4x4loopend\n\t"
"   nop\n\t"
"   sad4x4loopbegin:\n\t"
"   ldc1 $f4,0($t0)\n\t"
"   ldc1 $f8,0($t2)\n\t"  
"   pandn   $f6,$f6,$f6\n\t"
"   punpcklwd $f4,$f4,$f6\n\t"                         
"   pandn   $f10,$f10,$f10\n\t"                       
"   punpcklwd $f8,$f8,$f10\n\t"  
"   pasubub $f4,$f4,$f8\n\t"
"   biadd   $f4,$f4\n\t"
"   paddw   $f0,$f0,$f4\n\t"
"   mfc1    $t6,$f0\n\t"   
"   sltu    $t5,$t6,%1\n\t"
"   beqz    $t5,loopend4x4\n\t"

"   nop\n\t"
"   addiu   $t4,$t4,1\n\t"
"   addu    $t0,$t0,$t1\n\t"
"   addu    $t2,$t2,$t3\n\t"

"   sad4x4loopend:\n\t"
"   slti    $t5,$t4,4\n\t"
"   bnez    $t5,sad4x4loopbegin\n\t"
"   nop\n\t"
"   loopend4x4:\n\t"
"   sdc1    $f0,%0\n\t"
".set reorder\n\t"
".set mips0\n\t"
        :"=m"(ret)
        :"r"(max_sad),"r"(src_ptr),"r"(src_stride),"r"(ref_ptr),"r"(ref_stride)
        :"t0","t1","t2","t3","t4","t5","t6","$f0","$f4","$f6","$f8","$f10","memory"
   );
return ret;

//    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 4, 4);
}



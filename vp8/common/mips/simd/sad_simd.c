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
    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 16, 16);
}

unsigned int vp8_sad8x8_simd(const unsigned char *src_ptr, int src_stride,
                          const unsigned char *ref_ptr, int ref_stride,
                          unsigned int max_sad)
{
    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 8, 8);
}

unsigned int vp8_sad16x8_simd(const unsigned char *src_ptr, int src_stride,
                           const unsigned char *ref_ptr, int ref_stride,
                           unsigned int max_sad)
{
    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 16, 8);

}

unsigned int vp8_sad8x16_simd(const unsigned char *src_ptr, int src_stride,
                           const unsigned char *ref_ptr, int ref_stride,
                           unsigned int max_sad)
{
    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 8, 16);
}

unsigned int vp8_sad4x4_simd(const unsigned char *src_ptr, int src_stride,
                          const unsigned char *ref_ptr, int ref_stride,
                          unsigned int max_sad)
{
    return sad_mx_n_simd(src_ptr, src_stride, ref_ptr, ref_stride, max_sad, 4, 4);
}



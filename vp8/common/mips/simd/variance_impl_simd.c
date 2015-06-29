/* x86 Directory :x86/variance_impl_mmx.asm
 * common Directory: variance_c.c
 *
 * function :
 * 	vp8_get_mb_ss_simd
 *
 * */
unsigned int vp8_get_mb_ss_simd
(
    const short *src_ptr
)
{
    unsigned int i = 0, sum = 0;

    do
    {
        sum += (src_ptr[i] * src_ptr[i]);
        i++;
    }
    while (i < 256);

    return sum;
}


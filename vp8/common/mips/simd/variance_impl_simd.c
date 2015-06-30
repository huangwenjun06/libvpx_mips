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

/* x86 Directory :x86/variance_impl_mmx.asm
 * common Directory: encoder/pickinter.c
 *
 * function :
 *      vp8_get4x4sse_cs_simd
 *
 */

unsigned int vp8_get4x4sse_cs_simd
(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride
)
{
    int distortion = 0;
    int r, c;

    for (r = 0; r < 4; r++)
    {
        for (c = 0; c < 4; c++)
        {
            int diff = src_ptr[c] - ref_ptr[c];
            distortion += diff * diff;
        }

        src_ptr += source_stride;
        ref_ptr += recon_stride;
    }

    return distortion;
}

/*
 * x86 Directory :x86/variance_impl_mmx.asm
 * common Directory: encoder/dct.c
 *
 *  function :
 *  	vp8_short_fdct4x4_simd
 *  	vp8_short_fdct8x4_simd
 *
 * */
void vp8_short_fdct4x4_simd(short *input, short *output, int pitch)
{
    int i;
    int a1, b1, c1, d1;
    short *ip = input;
    short *op = output;

    for (i = 0; i < 4; i++)
    {
        a1 = ((ip[0] + ip[3])<<3);
        b1 = ((ip[1] + ip[2])<<3);
        c1 = ((ip[1] - ip[2])<<3);
        d1 = ((ip[0] - ip[3])<<3);

        op[0] = a1 + b1;
        op[2] = a1 - b1;

        op[1] = (c1 * 2217 + d1 * 5352 +  14500)>>12;
        op[3] = (d1 * 2217 - c1 * 5352 +   7500)>>12;

        ip += pitch / 2;
        op += 4;

    }
    ip = output;
    op = output;
    for (i = 0; i < 4; i++)
    {
        a1 = ip[0] + ip[12];
        b1 = ip[4] + ip[8];
        c1 = ip[4] - ip[8];
        d1 = ip[0] - ip[12];

        op[0]  = ( a1 + b1 + 7)>>4;
        op[8]  = ( a1 - b1 + 7)>>4;

        op[4]  =((c1 * 2217 + d1 * 5352 +  12000)>>16) + (d1!=0);
        op[12] = (d1 * 2217 - c1 * 5352 +  51000)>>16;

        ip++;
        op++;
    }
}

void vp8_short_fdct8x4_simd(short *input, short *output, int pitch)
{
    vp8_short_fdct4x4_simd(input,   output,    pitch);
    vp8_short_fdct4x4_simd(input + 4, output + 16, pitch);
}


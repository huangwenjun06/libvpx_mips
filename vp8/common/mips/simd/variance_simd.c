#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "../../filter.h"
#include "filter_mips.h"

/* x86 Directory: x86/variance_mmx.c
 * common Directiry: variance_c.c
 *
 * function:
 * 	all
 *
 * */
static void variance(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    int  w,
    int  h,
    unsigned int *sse,
    int *sum)
{
    int i, j;
    int diff;

    *sum = 0;
    *sse = 0;

    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w; j++)
        {
            diff = src_ptr[j] - ref_ptr[j];
            *sum += diff;
            *sse += diff * diff;
        }

        src_ptr += source_stride;
        ref_ptr += recon_stride;
    }
}

unsigned int vp8_mse16x16_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;

    variance(src_ptr, source_stride, ref_ptr, recon_stride, 16, 16, &var, &avg);
    *sse = var;
    return var;
}

unsigned int vp8_variance16x16_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;


    variance(src_ptr, source_stride, ref_ptr, recon_stride, 16, 16, &var, &avg);
    *sse = var;
    return (var - (((unsigned int)avg * avg) >> 8));
}

unsigned int vp8_variance8x16_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;


    variance(src_ptr, source_stride, ref_ptr, recon_stride, 8, 16, &var, &avg);
    *sse = var;
    return (var - (((unsigned int)avg * avg) >> 7));
}

unsigned int vp8_variance16x8_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;


    variance(src_ptr, source_stride, ref_ptr, recon_stride, 16, 8, &var, &avg);
    *sse = var;
    return (var - (((unsigned int)avg * avg) >> 7));
}


unsigned int vp8_variance8x8_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;


    variance(src_ptr, source_stride, ref_ptr, recon_stride, 8, 8, &var, &avg);
    *sse = var;
    return (var - (((unsigned int)avg * avg) >> 6));
}

unsigned int vp8_variance4x4_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    unsigned int var;
    int avg;


    variance(src_ptr, source_stride, ref_ptr, recon_stride, 4, 4, &var, &avg);
    *sse = var;
    return (var - (((unsigned int)avg * avg) >> 4));
}

//need todo
unsigned int vp8_sub_pixel_variance4x4_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{

    unsigned char  temp2[20*16];
    const short *HFilter, *VFilter;
    unsigned short FData3[5*4]; /* Temp data bufffer used in filtering */

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    /* First filter 1d Horizontal */
    var_filter_block2d_bil_first_pass(src_ptr, FData3, src_pixels_per_line, 1, 5, 4, HFilter);

    /* Now filter Verticaly */
    var_filter_block2d_bil_second_pass(FData3, temp2, 4,  4,  4,  4, VFilter);

    return vp8_variance4x4_simd(temp2, 4, dst_ptr, dst_pixels_per_line, sse);
//    int xsum;
//    unsigned int xxsum;
//    vp8_filter_block2d_bil4x4_var_mmx(
//        src_ptr, src_pixels_per_line,
//        dst_ptr, dst_pixels_per_line,
//        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
//        &xsum, &xxsum
//    );
//    *sse = xxsum;
//    return (xxsum - (((unsigned int)xsum * xsum) >> 4));
}


unsigned int vp8_sub_pixel_variance8x8_mmx
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
/*
    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil_var_mmx(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_x86_4[xoffset], vp8_bilinear_filters_x86_4[yoffset],
        &xsum, &xxsum
    );
    *sse = xxsum;
    return (xxsum - (((unsigned int)xsum * xsum) >> 6));
*/
    unsigned short FData3[9*8]; /* Temp data bufffer used in filtering */
    unsigned char  temp2[20*16];
    const short *HFilter, *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    var_filter_block2d_bil_first_pass(src_ptr, FData3, src_pixels_per_line, 1, 9, 8, HFilter);
    var_filter_block2d_bil_second_pass(FData3, temp2, 8, 8, 8, 8, VFilter);

    return vp8_variance8x8_simd(temp2, 8, dst_ptr, dst_pixels_per_line, sse);

}

unsigned int vp8_sub_pixel_variance16x16_mmx
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 16,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum0, &xxsum0
    );

    vp8_filter_block2d_bil_var_simd(
        src_ptr + 8, src_pixels_per_line,
        dst_ptr + 8, dst_pixels_per_line, 16,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum1, &xxsum1
    );

    xsum0 += xsum1;
    xxsum0 += xxsum1;

    *sse = xxsum0;
    return (xxsum0 - (((unsigned int)xsum0 * xsum0) >> 8));


}

unsigned int vp8_sub_pixel_mse16x16_mmx(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    vp8_sub_pixel_variance16x16_mmx(src_ptr, src_pixels_per_line, xoffset, yoffset, dst_ptr, dst_pixels_per_line, sse);
    return *sse;
}

unsigned int vp8_sub_pixel_variance16x8_mmx
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum0, &xxsum0
   );


    vp8_filter_block2d_bil_var_simd(
        src_ptr + 8, src_pixels_per_line,
        dst_ptr + 8, dst_pixels_per_line, 8,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum1, &xxsum1
    );

    xsum0 += xsum1;
    xxsum0 += xxsum1;

    *sse = xxsum0;
    return (xxsum0 - (((unsigned int)xsum0 * xsum0) >> 7));
}


//--gtest_filter=VP8/CQTest.LinearPSNRIsHigherForCQLevel/3
unsigned int vp8_sub_pixel_variance8x8_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{

    unsigned short FData3[9*8];

 /* Temp data bufffer used in filtering */

    unsigned char  temp2[20*16];
    const short *HFilter, *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    var_filter_block2d_bil_first_pass(src_ptr, FData3, src_pixels_per_line, 1, 9, 8, HFilter);
    var_filter_block2d_bil_second_pass(FData3, temp2, 8, 8, 8, 8, VFilter);

    return vp8_variance8x8_simd(temp2, 8, dst_ptr, dst_pixels_per_line, sse);
/*
    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],	//8*8
        &xsum, &xxsum
    );
    *sse = xxsum;
    return (xxsum - (((unsigned int)xsum * xsum) >> 6));
*/
}

unsigned int vp8_sub_pixel_variance16x16_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 16,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],	//16*16
        &xsum0, &xxsum0
    );


    vp8_filter_block2d_bil_var_simd(
        src_ptr + 8, src_pixels_per_line,
        dst_ptr + 8, dst_pixels_per_line, 16,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum1, &xxsum1
    );

    xsum0 += xsum1;
    xxsum0 += xxsum1;

    *sse = xxsum0;
    return (xxsum0 - (((unsigned int)xsum0 * xsum0) >> 8));

}

unsigned int vp8_sub_pixel_mse16x16_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    vp8_sub_pixel_variance16x16_simd(src_ptr, src_pixels_per_line, xoffset, yoffset, dst_ptr, dst_pixels_per_line, sse);
    return *sse;
}

unsigned int vp8_sub_pixel_variance16x8_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{
    int xsum0, xsum1;
    unsigned int xxsum0, xxsum1;


    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 8,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],	//16*8
        &xsum0, &xxsum0
    );


    vp8_filter_block2d_bil_var_simd(
        src_ptr + 8, src_pixels_per_line,
        dst_ptr + 8, dst_pixels_per_line, 8,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],
        &xsum1, &xxsum1
    );

    xsum0 += xsum1;
    xxsum0 += xxsum1;

    *sse = xxsum0;
    return (xxsum0 - (((unsigned int)xsum0 * xsum0) >> 7));
}

//--gtest_filter=VP8/CQTest.LinearPSNRIsHigherForCQLevel/3
unsigned int vp8_sub_pixel_variance8x16_simd
(
    const unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    const unsigned char *dst_ptr,
    int dst_pixels_per_line,
    unsigned int *sse
)
{

    unsigned short FData3[9*16];    /* Temp data bufffer used in filtering */

    unsigned char  temp2[20*16];
    const short *HFilter, *VFilter;


    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];


    var_filter_block2d_bil_first_pass(src_ptr, FData3, src_pixels_per_line, 1, 17, 8, HFilter);
    var_filter_block2d_bil_second_pass(FData3, temp2, 8, 8, 16, 8, VFilter);

    return vp8_variance8x16_simd(temp2, 8, dst_ptr, dst_pixels_per_line, sse);

/*
    int xsum;
    unsigned int xxsum;
    vp8_filter_block2d_bil_var_simd(
        src_ptr, src_pixels_per_line,
        dst_ptr, dst_pixels_per_line, 16,
        vp8_bilinear_filters_mips_4[xoffset], vp8_bilinear_filters_mips_4[yoffset],	//8*16
        &xsum, &xxsum
    );
    *sse = xxsum;
    return (xxsum - (((unsigned int)xsum * xsum) >> 7));
*/
}

unsigned int vp8_variance_halfpixvar16x16_h_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    return vp8_sub_pixel_variance16x16_simd(src_ptr, source_stride, 4, 0,
                                         ref_ptr, recon_stride, sse);
}


unsigned int vp8_variance_halfpixvar16x16_v_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    return vp8_sub_pixel_variance16x16_simd(src_ptr, source_stride, 0, 4,
                                         ref_ptr, recon_stride, sse);
}


unsigned int vp8_variance_halfpixvar16x16_hv_simd(
    const unsigned char *src_ptr,
    int  source_stride,
    const unsigned char *ref_ptr,
    int  recon_stride,
    unsigned int *sse)
{
    return vp8_sub_pixel_variance16x16_simd(src_ptr, source_stride, 4, 4,
                                         ref_ptr, recon_stride, sse);
}



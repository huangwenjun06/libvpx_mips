/*
 *x86 Directory: x86/subpixel_mmx.asm
 *common Directory: filter.c
 *function:
 *	vp8_bilinear_predict8x8_simd
 *	vp8_bilinear_predict8x4_simd
 * 	vp8_bilinear_predict4x4_simd
 *
 */
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
    const short *HFilter;
    const short *VFilter;

    HFilter = vp8_bilinear_filters[xoffset];
    VFilter = vp8_bilinear_filters[yoffset];

    filter_block2d_bil(src_ptr, dst_ptr, src_pixels_per_line, dst_pitch, HFilter, VFilter, 8, 4);

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
vp8_bilinear_predict16x16_simd
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


static void filter_block2d_bil
(
    unsigned char *src_ptr,
    unsigned char *dst_ptr,
    unsigned int   src_pitch,
    unsigned int   dst_pitch,
    const short   *HFilter,
    const short   *VFilter,
    int            Width,
    int            Height
);
//declare var_filter_block2d_bil_first_pass and var_filter_block2d_bil_second_pass 
static void var_filter_block2d_bil_first_pass
(
    const unsigned char *src_ptr,
    unsigned short *output_ptr,
    unsigned int src_pixels_per_line,
    int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short *vp8_filter
);
static void var_filter_block2d_bil_second_pass
(
    const unsigned short *src_ptr,
    unsigned char  *output_ptr,
    unsigned int  src_pixels_per_line,
    unsigned int  pixel_step,
    unsigned int  output_height,
    unsigned int  output_width,
    const short *vp8_filter
);


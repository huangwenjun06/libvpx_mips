const short vp8_bilinear_filters_mips_4[8][8];

const short vp8_bilinear_filters_mips_8[8][16];


#define VP8_FILTER_SHIFT  7
#define VP8_FILTER_WEIGHT 128


void vp8_filter_block1d_h6_simd
(
    unsigned char   *src_ptr,
    unsigned short  *output_ptr,
    unsigned int    src_pixels_per_line,
    unsigned int    pixel_step,
    unsigned int    output_height,
    unsigned int    output_width,
    short           * vp8_filter
);

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
);

//static void filter_block2d_bil
void filter_block2d_bil
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
//static void var_filter_block2d_bil_first_pass
void var_filter_block2d_bil_first_pass
(
    const unsigned char *src_ptr,
    unsigned short *output_ptr,
    unsigned int src_pixels_per_line,
    int pixel_step,
    unsigned int output_height,
    unsigned int output_width,
    const short *vp8_filter
);
//static void var_filter_block2d_bil_second_pass
void var_filter_block2d_bil_second_pass
(
    const unsigned short *src_ptr,
    unsigned char  *output_ptr,
    unsigned int  src_pixels_per_line,
    unsigned int  pixel_step,
    unsigned int  output_height,
    unsigned int  output_width,
    const short *vp8_filter
);

void vp8_filter_block2d_bil_var_simd
(
    const unsigned char *ref_ptr,
    int ref_pixels_per_line,
    const unsigned char *src_ptr,
    int src_pixels_per_line,
    unsigned int Height,
    const short *HFilter,	//const short
    const short *VFilter,
    //const unsigned short *HFilter,	//const short
    //const unsigned short *VFilter,
    int *sum,
    unsigned int *sumsquared
);


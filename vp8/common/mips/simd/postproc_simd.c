/*x86 Directory: x86/postproc_mmx.asm
 *common Directory: postproc.c
 *
 *function:
 *	vp8_post_proc_down_and_across_mb_row_simd
 *	vp8_mbpost_proc_down_simd
 *	vp8_plane_add_noise_simd
 *
 * */
void vp8_post_proc_down_and_across_mb_row_simd
(
    unsigned char *src_ptr,
    unsigned char *dst_ptr,
    int src_pixels_per_line,
    int dst_pixels_per_line,
    int cols,
    unsigned char *f,
    int size
)
{
    unsigned char *p_src, *p_dst;
    int row;
    int col;
    unsigned char v;
    unsigned char d[4];

    for (row = 0; row < size; row++)
    {
        /* post_proc_down for one row */
        p_src = src_ptr;
        p_dst = dst_ptr;

        for (col = 0; col < cols; col++)
        {
            unsigned char p_above2 = p_src[col - 2 * src_pixels_per_line];
            unsigned char p_above1 = p_src[col - src_pixels_per_line];
            unsigned char p_below1 = p_src[col + src_pixels_per_line];
            unsigned char p_below2 = p_src[col + 2 * src_pixels_per_line];

            v = p_src[col];

            if ((abs(v - p_above2) < f[col]) && (abs(v - p_above1) < f[col])
                && (abs(v - p_below1) < f[col]) && (abs(v - p_below2) < f[col]))
            {
                v = (p_above2 + p_above1 + v << 2 + p_below1 + p_below2) >> 3;
            }
            p_dst[col] = v;
        }
        
        /* now post_proc_across */
        p_src = dst_ptr;
        p_dst = dst_ptr;

        p_src[-2] = p_src[-1] = p_src[0];
        p_src[cols] = p_src[cols + 1] = p_src[cols - 1];

        for (col = 0; col < cols; col++)
        {
            v = p_src[col];

            if ((abs(v - p_src[col - 2]) < f[col])
                && (abs(v - p_src[col - 1]) < f[col])
                && (abs(v - p_src[col + 1]) < f[col])
                && (abs(v - p_src[col + 2]) < f[col]))
            {
                v = (p_src[col - 2] + p_src[col - 1] + v << 2 + p_src[col + 1] + p_src[col + 2]) >> 3;
            }

            d[col & 3] = v;

            if (col >= 2)
                p_dst[col - 2] = d[(col - 2) & 3];
        }

        /* handle the last two pixels */
        p_dst[col - 2] = d[(col - 2) & 3];
        p_dst[col - 1] = d[(col - 1) & 3];

        /* next row */
        src_ptr += src_pixels_per_line;
        dst_ptr += dst_pixels_per_line;
    }
}

void vp8_mbpost_proc_down_simd(unsigned char *dst, int pitch, int rows, int cols, int flimit)
{
    int r, c, i;
    const short *rv3 = &vp8_rv[63&rand()];

    for (c = 0; c < cols; c++ )
    {
        unsigned char *s = &dst[c];
        int sumsq = 0;
        int sum   = 0;
        unsigned char d[16];
        const short *rv2 = rv3 + ((c * 17) & 127);

        for (i = -8; i < 0; i++)
          s[i*pitch]=s[0];

        /* 17 avoids valgrind warning - we buffer values in c in d
 *          * and only write them when we've read 8 ahead...
 *                   */
        for (i = rows; i < rows+17; i++)
          s[i*pitch]=s[(rows-1)*pitch];

        for (i = -8; i <= 6; i++)
        {
            sumsq += s[i*pitch] * s[i*pitch];
            sum   += s[i*pitch];
        }

        for (r = 0; r < rows + 8; r++)
        {
            sumsq += s[7*pitch] * s[ 7*pitch] - s[-8*pitch] * s[-8*pitch];
            sum  += s[7*pitch] - s[-8*pitch];
            d[r&15] = s[0];

            if (sumsq * 15 - sum * sum < flimit)
            {
                d[r&15] = (rv2[r&127] + sum + s[0]) >> 4;
            }
            s[-8*pitch] = d[(r-8)&15];
            s += pitch;
        }
    }
}

/****************************************************************************
 *
 *  ROUTINE       : plane_add_noise_simd
 *
 *  INPUTS        : unsigned char *Start    starting address of buffer to add gaussian
 *                                  noise to
 *                  unsigned int Width    width of plane
 *                  unsigned int Height   height of plane
 *                  int  Pitch    distance between subsequent lines of frame
 *                  int  q        quantizer used to determine amount of noise
 *                                  to add
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void.
 *
 *  FUNCTION      : adds gaussian noise to a plane of pixels
 *
 *  SPECIAL NOTES : None.

 ****************************************************************************/
void vp8_plane_add_noise_simd(unsigned char *Start, char *noise,
                           char blackclamp[16],
                           char whiteclamp[16],
                           char bothclamp[16],
                           unsigned int Width, unsigned int Height, int Pitch)
{
    unsigned int i, j;

    for (i = 0; i < Height; i++)
    {
        unsigned char *Pos = Start + i * Pitch;
        char  *Ref = (char *)(noise + (rand() & 0xff));

        for (j = 0; j < Width; j++)
        {
            if (Pos[j] < blackclamp[0])
                Pos[j] = blackclamp[0];

            if (Pos[j] > 255 + whiteclamp[0])
                Pos[j] = 255 + whiteclamp[0];

            Pos[j] += Ref[j];
        }
    }
}


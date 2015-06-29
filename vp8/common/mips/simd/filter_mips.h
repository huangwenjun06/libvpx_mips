/*
 *  Copyright (c) 2011 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef FILTER_MIPS_H
#define FILTER_MIPS_H

//X86 Directory: filter_x86.c
//Common Directory: NULL ; because never user vp8_bilinear_fiter_mips_* Array
/* MIPS assembly specific copy of vp8/common/filter.c:vp8_bilinear_filters with
 * duplicated values */
extern const short vp8_bilinear_filters_MIPS_4[8][8];  /* duplicated 4x */
extern const short vp8_bilinear_filters_MIPS_8[8][16]; /* duplicated 8x */

#endif /* FILTER_MIPS_H */

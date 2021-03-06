/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include <math.h>
#include <string.h>

#include "./aom_dsp_rtcd.h"
#include "aom_dsp/inv_txfm.h"
#if CONFIG_DAALA_DCT4 || CONFIG_DAALA_DCT8 || CONFIG_DAALA_DCT16 || \
    CONFIG_DAALA_DCT32 || CONFIG_DAALA_DCT64
#include "av1/common/daala_tx.h"
#endif

void aom_iwht4x4_16_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  /* 4-point reversible, orthonormal inverse Walsh-Hadamard in 3.5 adds,
     0.5 shifts per pixel. */
  int i;
  tran_low_t output[16];
  tran_high_t a1, b1, c1, d1, e1;
  const tran_low_t *ip = input;
  tran_low_t *op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] >> UNIT_QUANT_SHIFT;
    c1 = ip[1] >> UNIT_QUANT_SHIFT;
    d1 = ip[2] >> UNIT_QUANT_SHIFT;
    b1 = ip[3] >> UNIT_QUANT_SHIFT;
    a1 += c1;
    d1 -= b1;
    e1 = (a1 - d1) >> 1;
    b1 = e1 - b1;
    c1 = e1 - c1;
    a1 -= b1;
    d1 += c1;
    op[0] = WRAPLOW(a1);
    op[1] = WRAPLOW(b1);
    op[2] = WRAPLOW(c1);
    op[3] = WRAPLOW(d1);
    ip += 4;
    op += 4;
  }

  ip = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[4 * 0];
    c1 = ip[4 * 1];
    d1 = ip[4 * 2];
    b1 = ip[4 * 3];
    a1 += c1;
    d1 -= b1;
    e1 = (a1 - d1) >> 1;
    b1 = e1 - b1;
    c1 = e1 - c1;
    a1 -= b1;
    d1 += c1;
    dest[stride * 0] = clip_pixel_add(dest[stride * 0], WRAPLOW(a1));
    dest[stride * 1] = clip_pixel_add(dest[stride * 1], WRAPLOW(b1));
    dest[stride * 2] = clip_pixel_add(dest[stride * 2], WRAPLOW(c1));
    dest[stride * 3] = clip_pixel_add(dest[stride * 3], WRAPLOW(d1));

    ip++;
    dest++;
  }
}

void aom_iwht4x4_1_add_c(const tran_low_t *in, uint8_t *dest, int dest_stride) {
  int i;
  tran_high_t a1, e1;
  tran_low_t tmp[4];
  const tran_low_t *ip = in;
  tran_low_t *op = tmp;

  a1 = ip[0] >> UNIT_QUANT_SHIFT;
  e1 = a1 >> 1;
  a1 -= e1;
  op[0] = WRAPLOW(a1);
  op[1] = op[2] = op[3] = WRAPLOW(e1);

  ip = tmp;
  for (i = 0; i < 4; i++) {
    e1 = ip[0] >> 1;
    a1 = ip[0] - e1;
    dest[dest_stride * 0] = clip_pixel_add(dest[dest_stride * 0], a1);
    dest[dest_stride * 1] = clip_pixel_add(dest[dest_stride * 1], e1);
    dest[dest_stride * 2] = clip_pixel_add(dest[dest_stride * 2], e1);
    dest[dest_stride * 3] = clip_pixel_add(dest[dest_stride * 3], e1);
    ip++;
    dest++;
  }
}

#if CONFIG_DAALA_DCT4
void aom_idct4_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[4];
  od_coeff y[4];
  for (i = 0; i < 4; i++) y[i] = input[i];
  od_bin_idct4(x, 1, y);
  for (i = 0; i < 4; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_idct4_c(const tran_low_t *input, tran_low_t *output) {
  tran_low_t step[4];
  tran_high_t temp1, temp2;
  // stage 1
  temp1 = (input[0] + input[2]) * cospi_16_64;
  temp2 = (input[0] - input[2]) * cospi_16_64;
  step[0] = WRAPLOW(dct_const_round_shift(temp1));
  step[1] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = input[1] * cospi_24_64 - input[3] * cospi_8_64;
  temp2 = input[1] * cospi_8_64 + input[3] * cospi_24_64;
  step[2] = WRAPLOW(dct_const_round_shift(temp1));
  step[3] = WRAPLOW(dct_const_round_shift(temp2));

  // stage 2
  output[0] = WRAPLOW(step[0] + step[3]);
  output[1] = WRAPLOW(step[1] + step[2]);
  output[2] = WRAPLOW(step[1] - step[2]);
  output[3] = WRAPLOW(step[0] - step[3]);
}
#endif

void aom_idct4x4_16_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  tran_low_t out[4 * 4];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[4], temp_out[4];

  // Rows
  for (i = 0; i < 4; ++i) {
    aom_idct4_c(input, outptr);
    input += 4;
    outptr += 4;
  }

  // Columns
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) temp_in[j] = out[j * 4 + i];
    aom_idct4_c(temp_in, temp_out);
    for (j = 0; j < 4; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 4));
    }
  }
}

void aom_idct4x4_1_add_c(const tran_low_t *input, uint8_t *dest,
                         int dest_stride) {
  int i;
  tran_high_t a1;
  tran_low_t out = WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64));
  out = WRAPLOW(dct_const_round_shift(out * cospi_16_64));
  a1 = ROUND_POWER_OF_TWO(out, 4);

  if (a1 == 0) return;

  for (i = 0; i < 4; i++) {
    dest[0] = clip_pixel_add(dest[0], a1);
    dest[1] = clip_pixel_add(dest[1], a1);
    dest[2] = clip_pixel_add(dest[2], a1);
    dest[3] = clip_pixel_add(dest[3], a1);
    dest += dest_stride;
  }
}

#if CONFIG_DAALA_DCT8
void aom_idct8_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[8];
  od_coeff y[8];
  for (i = 0; i < 8; i++) y[i] = (od_coeff)input[i];
  od_bin_idct8(x, 1, y);
  for (i = 0; i < 8; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_idct8_c(const tran_low_t *input, tran_low_t *output) {
  tran_low_t step1[8], step2[8];
  tran_high_t temp1, temp2;
  // stage 1
  step1[0] = input[0];
  step1[2] = input[4];
  step1[1] = input[2];
  step1[3] = input[6];
  temp1 = input[1] * cospi_28_64 - input[7] * cospi_4_64;
  temp2 = input[1] * cospi_4_64 + input[7] * cospi_28_64;
  step1[4] = WRAPLOW(dct_const_round_shift(temp1));
  step1[7] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = input[5] * cospi_12_64 - input[3] * cospi_20_64;
  temp2 = input[5] * cospi_20_64 + input[3] * cospi_12_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));

  // stage 2
  temp1 = (step1[0] + step1[2]) * cospi_16_64;
  temp2 = (step1[0] - step1[2]) * cospi_16_64;
  step2[0] = WRAPLOW(dct_const_round_shift(temp1));
  step2[1] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = step1[1] * cospi_24_64 - step1[3] * cospi_8_64;
  temp2 = step1[1] * cospi_8_64 + step1[3] * cospi_24_64;
  step2[2] = WRAPLOW(dct_const_round_shift(temp1));
  step2[3] = WRAPLOW(dct_const_round_shift(temp2));
  step2[4] = WRAPLOW(step1[4] + step1[5]);
  step2[5] = WRAPLOW(step1[4] - step1[5]);
  step2[6] = WRAPLOW(-step1[6] + step1[7]);
  step2[7] = WRAPLOW(step1[6] + step1[7]);

  // stage 3
  step1[0] = WRAPLOW(step2[0] + step2[3]);
  step1[1] = WRAPLOW(step2[1] + step2[2]);
  step1[2] = WRAPLOW(step2[1] - step2[2]);
  step1[3] = WRAPLOW(step2[0] - step2[3]);
  step1[4] = step2[4];
  temp1 = (step2[6] - step2[5]) * cospi_16_64;
  temp2 = (step2[5] + step2[6]) * cospi_16_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));
  step1[7] = step2[7];

  // stage 4
  output[0] = WRAPLOW(step1[0] + step1[7]);
  output[1] = WRAPLOW(step1[1] + step1[6]);
  output[2] = WRAPLOW(step1[2] + step1[5]);
  output[3] = WRAPLOW(step1[3] + step1[4]);
  output[4] = WRAPLOW(step1[3] - step1[4]);
  output[5] = WRAPLOW(step1[2] - step1[5]);
  output[6] = WRAPLOW(step1[1] - step1[6]);
  output[7] = WRAPLOW(step1[0] - step1[7]);
}
#endif

void aom_idct8x8_64_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  tran_low_t out[8 * 8];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[8], temp_out[8];

  // First transform rows
  for (i = 0; i < 8; ++i) {
    aom_idct8_c(input, outptr);
    input += 8;
    outptr += 8;
  }

  // Then transform columns
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j) temp_in[j] = out[j * 8 + i];
    aom_idct8_c(temp_in, temp_out);
    for (j = 0; j < 8; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 5));
    }
  }
}

void aom_idct8x8_1_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  int i, j;
  tran_high_t a1;
  tran_low_t out = WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64));
  out = WRAPLOW(dct_const_round_shift(out * cospi_16_64));
  a1 = ROUND_POWER_OF_TWO(out, 5);
  if (a1 == 0) return;
  for (j = 0; j < 8; ++j) {
    for (i = 0; i < 8; ++i) dest[i] = clip_pixel_add(dest[i], a1);
    dest += stride;
  }
}

#if CONFIG_DAALA_DCT4
void aom_iadst4_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[4];
  od_coeff y[4];
  for (i = 0; i < 4; i++) y[i] = input[i];
  od_bin_idst4(x, 1, y);
  for (i = 0; i < 4; i++) output[i] = (tran_low_t)x[i];
}
#else
void aom_iadst4_c(const tran_low_t *input, tran_low_t *output) {
  tran_high_t s0, s1, s2, s3, s4, s5, s6, s7;

  tran_low_t x0 = input[0];
  tran_low_t x1 = input[1];
  tran_low_t x2 = input[2];
  tran_low_t x3 = input[3];

  if (!(x0 | x1 | x2 | x3)) {
    output[0] = output[1] = output[2] = output[3] = 0;
    return;
  }

  s0 = sinpi_1_9 * x0;
  s1 = sinpi_2_9 * x0;
  s2 = sinpi_3_9 * x1;
  s3 = sinpi_4_9 * x2;
  s4 = sinpi_1_9 * x2;
  s5 = sinpi_2_9 * x3;
  s6 = sinpi_4_9 * x3;
  s7 = WRAPLOW(x0 - x2 + x3);

  s0 = s0 + s3 + s5;
  s1 = s1 - s4 - s6;
  s3 = s2;
  s2 = sinpi_3_9 * s7;

  // 1-D transform scaling factor is sqrt(2).
  // The overall dynamic range is 14b (input) + 14b (multiplication scaling)
  // + 1b (addition) = 29b.
  // Hence the output bit depth is 15b.
  output[0] = WRAPLOW(dct_const_round_shift(s0 + s3));
  output[1] = WRAPLOW(dct_const_round_shift(s1 + s3));
  output[2] = WRAPLOW(dct_const_round_shift(s2));
  output[3] = WRAPLOW(dct_const_round_shift(s0 + s1 - s3));
}
#endif

#if CONFIG_DAALA_DCT8
void aom_iadst8_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[8];
  od_coeff y[8];
  for (i = 0; i < 8; i++) y[i] = (od_coeff)input[i];
  od_bin_idst8(x, 1, y);
  for (i = 0; i < 8; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_iadst8_c(const tran_low_t *input, tran_low_t *output) {
  int s0, s1, s2, s3, s4, s5, s6, s7;

  tran_high_t x0 = input[7];
  tran_high_t x1 = input[0];
  tran_high_t x2 = input[5];
  tran_high_t x3 = input[2];
  tran_high_t x4 = input[3];
  tran_high_t x5 = input[4];
  tran_high_t x6 = input[1];
  tran_high_t x7 = input[6];

  if (!(x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7)) {
    output[0] = output[1] = output[2] = output[3] = output[4] = output[5] =
        output[6] = output[7] = 0;
    return;
  }

  // stage 1
  s0 = (int)(cospi_2_64 * x0 + cospi_30_64 * x1);
  s1 = (int)(cospi_30_64 * x0 - cospi_2_64 * x1);
  s2 = (int)(cospi_10_64 * x2 + cospi_22_64 * x3);
  s3 = (int)(cospi_22_64 * x2 - cospi_10_64 * x3);
  s4 = (int)(cospi_18_64 * x4 + cospi_14_64 * x5);
  s5 = (int)(cospi_14_64 * x4 - cospi_18_64 * x5);
  s6 = (int)(cospi_26_64 * x6 + cospi_6_64 * x7);
  s7 = (int)(cospi_6_64 * x6 - cospi_26_64 * x7);

  x0 = WRAPLOW(dct_const_round_shift(s0 + s4));
  x1 = WRAPLOW(dct_const_round_shift(s1 + s5));
  x2 = WRAPLOW(dct_const_round_shift(s2 + s6));
  x3 = WRAPLOW(dct_const_round_shift(s3 + s7));
  x4 = WRAPLOW(dct_const_round_shift(s0 - s4));
  x5 = WRAPLOW(dct_const_round_shift(s1 - s5));
  x6 = WRAPLOW(dct_const_round_shift(s2 - s6));
  x7 = WRAPLOW(dct_const_round_shift(s3 - s7));

  // stage 2
  s0 = (int)x0;
  s1 = (int)x1;
  s2 = (int)x2;
  s3 = (int)x3;
  s4 = (int)(cospi_8_64 * x4 + cospi_24_64 * x5);
  s5 = (int)(cospi_24_64 * x4 - cospi_8_64 * x5);
  s6 = (int)(-cospi_24_64 * x6 + cospi_8_64 * x7);
  s7 = (int)(cospi_8_64 * x6 + cospi_24_64 * x7);

  x0 = WRAPLOW(s0 + s2);
  x1 = WRAPLOW(s1 + s3);
  x2 = WRAPLOW(s0 - s2);
  x3 = WRAPLOW(s1 - s3);
  x4 = WRAPLOW(dct_const_round_shift(s4 + s6));
  x5 = WRAPLOW(dct_const_round_shift(s5 + s7));
  x6 = WRAPLOW(dct_const_round_shift(s4 - s6));
  x7 = WRAPLOW(dct_const_round_shift(s5 - s7));

  // stage 3
  s2 = (int)(cospi_16_64 * (x2 + x3));
  s3 = (int)(cospi_16_64 * (x2 - x3));
  s6 = (int)(cospi_16_64 * (x6 + x7));
  s7 = (int)(cospi_16_64 * (x6 - x7));

  x2 = WRAPLOW(dct_const_round_shift(s2));
  x3 = WRAPLOW(dct_const_round_shift(s3));
  x6 = WRAPLOW(dct_const_round_shift(s6));
  x7 = WRAPLOW(dct_const_round_shift(s7));

  output[0] = WRAPLOW(x0);
  output[1] = WRAPLOW(-x4);
  output[2] = WRAPLOW(x6);
  output[3] = WRAPLOW(-x2);
  output[4] = WRAPLOW(x3);
  output[5] = WRAPLOW(-x7);
  output[6] = WRAPLOW(x5);
  output[7] = WRAPLOW(-x1);
}

#endif

void aom_idct8x8_12_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  tran_low_t out[8 * 8] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[8], temp_out[8];

  // First transform rows
  // only first 4 row has non-zero coefs
  for (i = 0; i < 4; ++i) {
    aom_idct8_c(input, outptr);
    input += 8;
    outptr += 8;
  }

  // Then transform columns
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j) temp_in[j] = out[j * 8 + i];
    aom_idct8_c(temp_in, temp_out);
    for (j = 0; j < 8; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 5));
    }
  }
}

#if CONFIG_DAALA_DCT16
void aom_idct16_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[16];
  od_coeff y[16];
  for (i = 0; i < 16; i++) y[i] = (od_coeff)input[i];
  od_bin_idct16(x, 1, y);
  for (i = 0; i < 16; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_idct16_c(const tran_low_t *input, tran_low_t *output) {
  tran_low_t step1[16], step2[16];
  tran_high_t temp1, temp2;

  // stage 1
  step1[0] = input[0 / 2];
  step1[1] = input[16 / 2];
  step1[2] = input[8 / 2];
  step1[3] = input[24 / 2];
  step1[4] = input[4 / 2];
  step1[5] = input[20 / 2];
  step1[6] = input[12 / 2];
  step1[7] = input[28 / 2];
  step1[8] = input[2 / 2];
  step1[9] = input[18 / 2];
  step1[10] = input[10 / 2];
  step1[11] = input[26 / 2];
  step1[12] = input[6 / 2];
  step1[13] = input[22 / 2];
  step1[14] = input[14 / 2];
  step1[15] = input[30 / 2];

  // stage 2
  step2[0] = step1[0];
  step2[1] = step1[1];
  step2[2] = step1[2];
  step2[3] = step1[3];
  step2[4] = step1[4];
  step2[5] = step1[5];
  step2[6] = step1[6];
  step2[7] = step1[7];

  temp1 = step1[8] * cospi_30_64 - step1[15] * cospi_2_64;
  temp2 = step1[8] * cospi_2_64 + step1[15] * cospi_30_64;
  step2[8] = WRAPLOW(dct_const_round_shift(temp1));
  step2[15] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[9] * cospi_14_64 - step1[14] * cospi_18_64;
  temp2 = step1[9] * cospi_18_64 + step1[14] * cospi_14_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1));
  step2[14] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[10] * cospi_22_64 - step1[13] * cospi_10_64;
  temp2 = step1[10] * cospi_10_64 + step1[13] * cospi_22_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[11] * cospi_6_64 - step1[12] * cospi_26_64;
  temp2 = step1[11] * cospi_26_64 + step1[12] * cospi_6_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1));
  step2[12] = WRAPLOW(dct_const_round_shift(temp2));

  // stage 3
  step1[0] = step2[0];
  step1[1] = step2[1];
  step1[2] = step2[2];
  step1[3] = step2[3];

  temp1 = step2[4] * cospi_28_64 - step2[7] * cospi_4_64;
  temp2 = step2[4] * cospi_4_64 + step2[7] * cospi_28_64;
  step1[4] = WRAPLOW(dct_const_round_shift(temp1));
  step1[7] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = step2[5] * cospi_12_64 - step2[6] * cospi_20_64;
  temp2 = step2[5] * cospi_20_64 + step2[6] * cospi_12_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));

  step1[8] = WRAPLOW(step2[8] + step2[9]);
  step1[9] = WRAPLOW(step2[8] - step2[9]);
  step1[10] = WRAPLOW(-step2[10] + step2[11]);
  step1[11] = WRAPLOW(step2[10] + step2[11]);
  step1[12] = WRAPLOW(step2[12] + step2[13]);
  step1[13] = WRAPLOW(step2[12] - step2[13]);
  step1[14] = WRAPLOW(-step2[14] + step2[15]);
  step1[15] = WRAPLOW(step2[14] + step2[15]);

  // stage 4
  temp1 = (step1[0] + step1[1]) * cospi_16_64;
  temp2 = (step1[0] - step1[1]) * cospi_16_64;
  step2[0] = WRAPLOW(dct_const_round_shift(temp1));
  step2[1] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = step1[2] * cospi_24_64 - step1[3] * cospi_8_64;
  temp2 = step1[2] * cospi_8_64 + step1[3] * cospi_24_64;
  step2[2] = WRAPLOW(dct_const_round_shift(temp1));
  step2[3] = WRAPLOW(dct_const_round_shift(temp2));
  step2[4] = WRAPLOW(step1[4] + step1[5]);
  step2[5] = WRAPLOW(step1[4] - step1[5]);
  step2[6] = WRAPLOW(-step1[6] + step1[7]);
  step2[7] = WRAPLOW(step1[6] + step1[7]);

  step2[8] = step1[8];
  step2[15] = step1[15];
  temp1 = -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
  temp2 = step1[9] * cospi_24_64 + step1[14] * cospi_8_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1));
  step2[14] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step1[10] * cospi_24_64 - step1[13] * cospi_8_64;
  temp2 = -step1[10] * cospi_8_64 + step1[13] * cospi_24_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));
  step2[11] = step1[11];
  step2[12] = step1[12];

  // stage 5
  step1[0] = WRAPLOW(step2[0] + step2[3]);
  step1[1] = WRAPLOW(step2[1] + step2[2]);
  step1[2] = WRAPLOW(step2[1] - step2[2]);
  step1[3] = WRAPLOW(step2[0] - step2[3]);
  step1[4] = step2[4];
  temp1 = (step2[6] - step2[5]) * cospi_16_64;
  temp2 = (step2[5] + step2[6]) * cospi_16_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));
  step1[7] = step2[7];

  step1[8] = WRAPLOW(step2[8] + step2[11]);
  step1[9] = WRAPLOW(step2[9] + step2[10]);
  step1[10] = WRAPLOW(step2[9] - step2[10]);
  step1[11] = WRAPLOW(step2[8] - step2[11]);
  step1[12] = WRAPLOW(-step2[12] + step2[15]);
  step1[13] = WRAPLOW(-step2[13] + step2[14]);
  step1[14] = WRAPLOW(step2[13] + step2[14]);
  step1[15] = WRAPLOW(step2[12] + step2[15]);

  // stage 6
  step2[0] = WRAPLOW(step1[0] + step1[7]);
  step2[1] = WRAPLOW(step1[1] + step1[6]);
  step2[2] = WRAPLOW(step1[2] + step1[5]);
  step2[3] = WRAPLOW(step1[3] + step1[4]);
  step2[4] = WRAPLOW(step1[3] - step1[4]);
  step2[5] = WRAPLOW(step1[2] - step1[5]);
  step2[6] = WRAPLOW(step1[1] - step1[6]);
  step2[7] = WRAPLOW(step1[0] - step1[7]);
  step2[8] = step1[8];
  step2[9] = step1[9];
  temp1 = (-step1[10] + step1[13]) * cospi_16_64;
  temp2 = (step1[10] + step1[13]) * cospi_16_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = (-step1[11] + step1[12]) * cospi_16_64;
  temp2 = (step1[11] + step1[12]) * cospi_16_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1));
  step2[12] = WRAPLOW(dct_const_round_shift(temp2));
  step2[14] = step1[14];
  step2[15] = step1[15];

  // stage 7
  output[0] = WRAPLOW(step2[0] + step2[15]);
  output[1] = WRAPLOW(step2[1] + step2[14]);
  output[2] = WRAPLOW(step2[2] + step2[13]);
  output[3] = WRAPLOW(step2[3] + step2[12]);
  output[4] = WRAPLOW(step2[4] + step2[11]);
  output[5] = WRAPLOW(step2[5] + step2[10]);
  output[6] = WRAPLOW(step2[6] + step2[9]);
  output[7] = WRAPLOW(step2[7] + step2[8]);
  output[8] = WRAPLOW(step2[7] - step2[8]);
  output[9] = WRAPLOW(step2[6] - step2[9]);
  output[10] = WRAPLOW(step2[5] - step2[10]);
  output[11] = WRAPLOW(step2[4] - step2[11]);
  output[12] = WRAPLOW(step2[3] - step2[12]);
  output[13] = WRAPLOW(step2[2] - step2[13]);
  output[14] = WRAPLOW(step2[1] - step2[14]);
  output[15] = WRAPLOW(step2[0] - step2[15]);
}
#endif  // CONFIG_DAALA_DCT16

void aom_idct16x16_256_add_c(const tran_low_t *input, uint8_t *dest,
                             int stride) {
  tran_low_t out[16 * 16];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[16], temp_out[16];

  // First transform rows
  for (i = 0; i < 16; ++i) {
    aom_idct16_c(input, outptr);
    input += 16;
    outptr += 16;
  }

  // Then transform columns
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j) temp_in[j] = out[j * 16 + i];
    aom_idct16_c(temp_in, temp_out);
    for (j = 0; j < 16; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

#if CONFIG_DAALA_DCT16
void aom_iadst16_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[16];
  od_coeff y[16];
  for (i = 0; i < 16; i++) y[i] = (od_coeff)input[i];
  od_bin_idst16(x, 1, y);
  for (i = 0; i < 16; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_iadst16_c(const tran_low_t *input, tran_low_t *output) {
  tran_high_t s0, s1, s2, s3, s4, s5, s6, s7, s8;
  tran_high_t s9, s10, s11, s12, s13, s14, s15;

  tran_high_t x0 = input[15];
  tran_high_t x1 = input[0];
  tran_high_t x2 = input[13];
  tran_high_t x3 = input[2];
  tran_high_t x4 = input[11];
  tran_high_t x5 = input[4];
  tran_high_t x6 = input[9];
  tran_high_t x7 = input[6];
  tran_high_t x8 = input[7];
  tran_high_t x9 = input[8];
  tran_high_t x10 = input[5];
  tran_high_t x11 = input[10];
  tran_high_t x12 = input[3];
  tran_high_t x13 = input[12];
  tran_high_t x14 = input[1];
  tran_high_t x15 = input[14];

  if (!(x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8 | x9 | x10 | x11 | x12 |
        x13 | x14 | x15)) {
    output[0] = output[1] = output[2] = output[3] = output[4] = output[5] =
        output[6] = output[7] = output[8] = output[9] = output[10] =
            output[11] = output[12] = output[13] = output[14] = output[15] = 0;
    return;
  }

  // stage 1
  s0 = x0 * cospi_1_64 + x1 * cospi_31_64;
  s1 = x0 * cospi_31_64 - x1 * cospi_1_64;
  s2 = x2 * cospi_5_64 + x3 * cospi_27_64;
  s3 = x2 * cospi_27_64 - x3 * cospi_5_64;
  s4 = x4 * cospi_9_64 + x5 * cospi_23_64;
  s5 = x4 * cospi_23_64 - x5 * cospi_9_64;
  s6 = x6 * cospi_13_64 + x7 * cospi_19_64;
  s7 = x6 * cospi_19_64 - x7 * cospi_13_64;
  s8 = x8 * cospi_17_64 + x9 * cospi_15_64;
  s9 = x8 * cospi_15_64 - x9 * cospi_17_64;
  s10 = x10 * cospi_21_64 + x11 * cospi_11_64;
  s11 = x10 * cospi_11_64 - x11 * cospi_21_64;
  s12 = x12 * cospi_25_64 + x13 * cospi_7_64;
  s13 = x12 * cospi_7_64 - x13 * cospi_25_64;
  s14 = x14 * cospi_29_64 + x15 * cospi_3_64;
  s15 = x14 * cospi_3_64 - x15 * cospi_29_64;

  x0 = WRAPLOW(dct_const_round_shift(s0 + s8));
  x1 = WRAPLOW(dct_const_round_shift(s1 + s9));
  x2 = WRAPLOW(dct_const_round_shift(s2 + s10));
  x3 = WRAPLOW(dct_const_round_shift(s3 + s11));
  x4 = WRAPLOW(dct_const_round_shift(s4 + s12));
  x5 = WRAPLOW(dct_const_round_shift(s5 + s13));
  x6 = WRAPLOW(dct_const_round_shift(s6 + s14));
  x7 = WRAPLOW(dct_const_round_shift(s7 + s15));
  x8 = WRAPLOW(dct_const_round_shift(s0 - s8));
  x9 = WRAPLOW(dct_const_round_shift(s1 - s9));
  x10 = WRAPLOW(dct_const_round_shift(s2 - s10));
  x11 = WRAPLOW(dct_const_round_shift(s3 - s11));
  x12 = WRAPLOW(dct_const_round_shift(s4 - s12));
  x13 = WRAPLOW(dct_const_round_shift(s5 - s13));
  x14 = WRAPLOW(dct_const_round_shift(s6 - s14));
  x15 = WRAPLOW(dct_const_round_shift(s7 - s15));

  // stage 2
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 = x4;
  s5 = x5;
  s6 = x6;
  s7 = x7;
  s8 = x8 * cospi_4_64 + x9 * cospi_28_64;
  s9 = x8 * cospi_28_64 - x9 * cospi_4_64;
  s10 = x10 * cospi_20_64 + x11 * cospi_12_64;
  s11 = x10 * cospi_12_64 - x11 * cospi_20_64;
  s12 = -x12 * cospi_28_64 + x13 * cospi_4_64;
  s13 = x12 * cospi_4_64 + x13 * cospi_28_64;
  s14 = -x14 * cospi_12_64 + x15 * cospi_20_64;
  s15 = x14 * cospi_20_64 + x15 * cospi_12_64;

  x0 = WRAPLOW(s0 + s4);
  x1 = WRAPLOW(s1 + s5);
  x2 = WRAPLOW(s2 + s6);
  x3 = WRAPLOW(s3 + s7);
  x4 = WRAPLOW(s0 - s4);
  x5 = WRAPLOW(s1 - s5);
  x6 = WRAPLOW(s2 - s6);
  x7 = WRAPLOW(s3 - s7);
  x8 = WRAPLOW(dct_const_round_shift(s8 + s12));
  x9 = WRAPLOW(dct_const_round_shift(s9 + s13));
  x10 = WRAPLOW(dct_const_round_shift(s10 + s14));
  x11 = WRAPLOW(dct_const_round_shift(s11 + s15));
  x12 = WRAPLOW(dct_const_round_shift(s8 - s12));
  x13 = WRAPLOW(dct_const_round_shift(s9 - s13));
  x14 = WRAPLOW(dct_const_round_shift(s10 - s14));
  x15 = WRAPLOW(dct_const_round_shift(s11 - s15));

  // stage 3
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 = x4 * cospi_8_64 + x5 * cospi_24_64;
  s5 = x4 * cospi_24_64 - x5 * cospi_8_64;
  s6 = -x6 * cospi_24_64 + x7 * cospi_8_64;
  s7 = x6 * cospi_8_64 + x7 * cospi_24_64;
  s8 = x8;
  s9 = x9;
  s10 = x10;
  s11 = x11;
  s12 = x12 * cospi_8_64 + x13 * cospi_24_64;
  s13 = x12 * cospi_24_64 - x13 * cospi_8_64;
  s14 = -x14 * cospi_24_64 + x15 * cospi_8_64;
  s15 = x14 * cospi_8_64 + x15 * cospi_24_64;

  x0 = WRAPLOW(s0 + s2);
  x1 = WRAPLOW(s1 + s3);
  x2 = WRAPLOW(s0 - s2);
  x3 = WRAPLOW(s1 - s3);
  x4 = WRAPLOW(dct_const_round_shift(s4 + s6));
  x5 = WRAPLOW(dct_const_round_shift(s5 + s7));
  x6 = WRAPLOW(dct_const_round_shift(s4 - s6));
  x7 = WRAPLOW(dct_const_round_shift(s5 - s7));
  x8 = WRAPLOW(s8 + s10);
  x9 = WRAPLOW(s9 + s11);
  x10 = WRAPLOW(s8 - s10);
  x11 = WRAPLOW(s9 - s11);
  x12 = WRAPLOW(dct_const_round_shift(s12 + s14));
  x13 = WRAPLOW(dct_const_round_shift(s13 + s15));
  x14 = WRAPLOW(dct_const_round_shift(s12 - s14));
  x15 = WRAPLOW(dct_const_round_shift(s13 - s15));

  // stage 4
  s2 = (-cospi_16_64) * (x2 + x3);
  s3 = cospi_16_64 * (x2 - x3);
  s6 = cospi_16_64 * (x6 + x7);
  s7 = cospi_16_64 * (-x6 + x7);
  s10 = cospi_16_64 * (x10 + x11);
  s11 = cospi_16_64 * (-x10 + x11);
  s14 = (-cospi_16_64) * (x14 + x15);
  s15 = cospi_16_64 * (x14 - x15);

  x2 = WRAPLOW(dct_const_round_shift(s2));
  x3 = WRAPLOW(dct_const_round_shift(s3));
  x6 = WRAPLOW(dct_const_round_shift(s6));
  x7 = WRAPLOW(dct_const_round_shift(s7));
  x10 = WRAPLOW(dct_const_round_shift(s10));
  x11 = WRAPLOW(dct_const_round_shift(s11));
  x14 = WRAPLOW(dct_const_round_shift(s14));
  x15 = WRAPLOW(dct_const_round_shift(s15));

  output[0] = WRAPLOW(x0);
  output[1] = WRAPLOW(-x8);
  output[2] = WRAPLOW(x12);
  output[3] = WRAPLOW(-x4);
  output[4] = WRAPLOW(x6);
  output[5] = WRAPLOW(x14);
  output[6] = WRAPLOW(x10);
  output[7] = WRAPLOW(x2);
  output[8] = WRAPLOW(x3);
  output[9] = WRAPLOW(x11);
  output[10] = WRAPLOW(x15);
  output[11] = WRAPLOW(x7);
  output[12] = WRAPLOW(x5);
  output[13] = WRAPLOW(-x13);
  output[14] = WRAPLOW(x9);
  output[15] = WRAPLOW(-x1);
}
#endif

void aom_idct16x16_38_add_c(const tran_low_t *input, uint8_t *dest,
                            int stride) {
  int i, j;
  tran_low_t out[16 * 16] = { 0 };
  tran_low_t *outptr = out;
  tran_low_t temp_in[16], temp_out[16];

  // First transform rows. Since all non-zero dct coefficients are in
  // upper-left 8x8 area, we only need to calculate first 8 rows here.
  for (i = 0; i < 8; ++i) {
    aom_idct16_c(input, outptr);
    input += 16;
    outptr += 16;
  }

  // Then transform columns
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j) temp_in[j] = out[j * 16 + i];
    aom_idct16_c(temp_in, temp_out);
    for (j = 0; j < 16; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

void aom_idct16x16_10_add_c(const tran_low_t *input, uint8_t *dest,
                            int stride) {
  tran_low_t out[16 * 16] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[16], temp_out[16];

  // First transform rows. Since all non-zero dct coefficients are in
  // upper-left 4x4 area, we only need to calculate first 4 rows here.
  for (i = 0; i < 4; ++i) {
    aom_idct16_c(input, outptr);
    input += 16;
    outptr += 16;
  }

  // Then transform columns
  for (i = 0; i < 16; ++i) {
    for (j = 0; j < 16; ++j) temp_in[j] = out[j * 16 + i];
    aom_idct16_c(temp_in, temp_out);
    for (j = 0; j < 16; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

void aom_idct16x16_1_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  int i, j;
  tran_high_t a1;
  tran_low_t out = WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64));
  out = WRAPLOW(dct_const_round_shift(out * cospi_16_64));
  a1 = ROUND_POWER_OF_TWO(out, 6);
  if (a1 == 0) return;
  for (j = 0; j < 16; ++j) {
    for (i = 0; i < 16; ++i) dest[i] = clip_pixel_add(dest[i], a1);
    dest += stride;
  }
}

#if CONFIG_DAALA_DCT32
void aom_idct32_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[32];
  od_coeff y[32];
  for (i = 0; i < 32; i++) y[i] = (od_coeff)input[i];
  od_bin_idct32(x, 1, y);
  for (i = 0; i < 32; i++) output[i] = (tran_low_t)x[i];
}

#else

void aom_idct32_c(const tran_low_t *input, tran_low_t *output) {
  tran_low_t step1[32], step2[32];
  tran_high_t temp1, temp2;

  // stage 1
  step1[0] = input[0];
  step1[1] = input[16];
  step1[2] = input[8];
  step1[3] = input[24];
  step1[4] = input[4];
  step1[5] = input[20];
  step1[6] = input[12];
  step1[7] = input[28];
  step1[8] = input[2];
  step1[9] = input[18];
  step1[10] = input[10];
  step1[11] = input[26];
  step1[12] = input[6];
  step1[13] = input[22];
  step1[14] = input[14];
  step1[15] = input[30];

  temp1 = input[1] * cospi_31_64 - input[31] * cospi_1_64;
  temp2 = input[1] * cospi_1_64 + input[31] * cospi_31_64;
  step1[16] = WRAPLOW(dct_const_round_shift(temp1));
  step1[31] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[17] * cospi_15_64 - input[15] * cospi_17_64;
  temp2 = input[17] * cospi_17_64 + input[15] * cospi_15_64;
  step1[17] = WRAPLOW(dct_const_round_shift(temp1));
  step1[30] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[9] * cospi_23_64 - input[23] * cospi_9_64;
  temp2 = input[9] * cospi_9_64 + input[23] * cospi_23_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1));
  step1[29] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[25] * cospi_7_64 - input[7] * cospi_25_64;
  temp2 = input[25] * cospi_25_64 + input[7] * cospi_7_64;
  step1[19] = WRAPLOW(dct_const_round_shift(temp1));
  step1[28] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[5] * cospi_27_64 - input[27] * cospi_5_64;
  temp2 = input[5] * cospi_5_64 + input[27] * cospi_27_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1));
  step1[27] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[21] * cospi_11_64 - input[11] * cospi_21_64;
  temp2 = input[21] * cospi_21_64 + input[11] * cospi_11_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1));
  step1[26] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[13] * cospi_19_64 - input[19] * cospi_13_64;
  temp2 = input[13] * cospi_13_64 + input[19] * cospi_19_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1));
  step1[25] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = input[29] * cospi_3_64 - input[3] * cospi_29_64;
  temp2 = input[29] * cospi_29_64 + input[3] * cospi_3_64;
  step1[23] = WRAPLOW(dct_const_round_shift(temp1));
  step1[24] = WRAPLOW(dct_const_round_shift(temp2));

  // stage 2
  step2[0] = step1[0];
  step2[1] = step1[1];
  step2[2] = step1[2];
  step2[3] = step1[3];
  step2[4] = step1[4];
  step2[5] = step1[5];
  step2[6] = step1[6];
  step2[7] = step1[7];

  temp1 = step1[8] * cospi_30_64 - step1[15] * cospi_2_64;
  temp2 = step1[8] * cospi_2_64 + step1[15] * cospi_30_64;
  step2[8] = WRAPLOW(dct_const_round_shift(temp1));
  step2[15] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[9] * cospi_14_64 - step1[14] * cospi_18_64;
  temp2 = step1[9] * cospi_18_64 + step1[14] * cospi_14_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1));
  step2[14] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[10] * cospi_22_64 - step1[13] * cospi_10_64;
  temp2 = step1[10] * cospi_10_64 + step1[13] * cospi_22_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));

  temp1 = step1[11] * cospi_6_64 - step1[12] * cospi_26_64;
  temp2 = step1[11] * cospi_26_64 + step1[12] * cospi_6_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1));
  step2[12] = WRAPLOW(dct_const_round_shift(temp2));

  step2[16] = WRAPLOW(step1[16] + step1[17]);
  step2[17] = WRAPLOW(step1[16] - step1[17]);
  step2[18] = WRAPLOW(-step1[18] + step1[19]);
  step2[19] = WRAPLOW(step1[18] + step1[19]);
  step2[20] = WRAPLOW(step1[20] + step1[21]);
  step2[21] = WRAPLOW(step1[20] - step1[21]);
  step2[22] = WRAPLOW(-step1[22] + step1[23]);
  step2[23] = WRAPLOW(step1[22] + step1[23]);
  step2[24] = WRAPLOW(step1[24] + step1[25]);
  step2[25] = WRAPLOW(step1[24] - step1[25]);
  step2[26] = WRAPLOW(-step1[26] + step1[27]);
  step2[27] = WRAPLOW(step1[26] + step1[27]);
  step2[28] = WRAPLOW(step1[28] + step1[29]);
  step2[29] = WRAPLOW(step1[28] - step1[29]);
  step2[30] = WRAPLOW(-step1[30] + step1[31]);
  step2[31] = WRAPLOW(step1[30] + step1[31]);

  // stage 3
  step1[0] = step2[0];
  step1[1] = step2[1];
  step1[2] = step2[2];
  step1[3] = step2[3];

  temp1 = step2[4] * cospi_28_64 - step2[7] * cospi_4_64;
  temp2 = step2[4] * cospi_4_64 + step2[7] * cospi_28_64;
  step1[4] = WRAPLOW(dct_const_round_shift(temp1));
  step1[7] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = step2[5] * cospi_12_64 - step2[6] * cospi_20_64;
  temp2 = step2[5] * cospi_20_64 + step2[6] * cospi_12_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));

  step1[8] = WRAPLOW(step2[8] + step2[9]);
  step1[9] = WRAPLOW(step2[8] - step2[9]);
  step1[10] = WRAPLOW(-step2[10] + step2[11]);
  step1[11] = WRAPLOW(step2[10] + step2[11]);
  step1[12] = WRAPLOW(step2[12] + step2[13]);
  step1[13] = WRAPLOW(step2[12] - step2[13]);
  step1[14] = WRAPLOW(-step2[14] + step2[15]);
  step1[15] = WRAPLOW(step2[14] + step2[15]);

  step1[16] = step2[16];
  step1[31] = step2[31];
  temp1 = -step2[17] * cospi_4_64 + step2[30] * cospi_28_64;
  temp2 = step2[17] * cospi_28_64 + step2[30] * cospi_4_64;
  step1[17] = WRAPLOW(dct_const_round_shift(temp1));
  step1[30] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step2[18] * cospi_28_64 - step2[29] * cospi_4_64;
  temp2 = -step2[18] * cospi_4_64 + step2[29] * cospi_28_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1));
  step1[29] = WRAPLOW(dct_const_round_shift(temp2));
  step1[19] = step2[19];
  step1[20] = step2[20];
  temp1 = -step2[21] * cospi_20_64 + step2[26] * cospi_12_64;
  temp2 = step2[21] * cospi_12_64 + step2[26] * cospi_20_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1));
  step1[26] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step2[22] * cospi_12_64 - step2[25] * cospi_20_64;
  temp2 = -step2[22] * cospi_20_64 + step2[25] * cospi_12_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1));
  step1[25] = WRAPLOW(dct_const_round_shift(temp2));
  step1[23] = step2[23];
  step1[24] = step2[24];
  step1[27] = step2[27];
  step1[28] = step2[28];

  // stage 4
  temp1 = (step1[0] + step1[1]) * cospi_16_64;
  temp2 = (step1[0] - step1[1]) * cospi_16_64;
  step2[0] = WRAPLOW(dct_const_round_shift(temp1));
  step2[1] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = step1[2] * cospi_24_64 - step1[3] * cospi_8_64;
  temp2 = step1[2] * cospi_8_64 + step1[3] * cospi_24_64;
  step2[2] = WRAPLOW(dct_const_round_shift(temp1));
  step2[3] = WRAPLOW(dct_const_round_shift(temp2));
  step2[4] = WRAPLOW(step1[4] + step1[5]);
  step2[5] = WRAPLOW(step1[4] - step1[5]);
  step2[6] = WRAPLOW(-step1[6] + step1[7]);
  step2[7] = WRAPLOW(step1[6] + step1[7]);

  step2[8] = step1[8];
  step2[15] = step1[15];
  temp1 = -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
  temp2 = step1[9] * cospi_24_64 + step1[14] * cospi_8_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1));
  step2[14] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step1[10] * cospi_24_64 - step1[13] * cospi_8_64;
  temp2 = -step1[10] * cospi_8_64 + step1[13] * cospi_24_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));
  step2[11] = step1[11];
  step2[12] = step1[12];

  step2[16] = WRAPLOW(step1[16] + step1[19]);
  step2[17] = WRAPLOW(step1[17] + step1[18]);
  step2[18] = WRAPLOW(step1[17] - step1[18]);
  step2[19] = WRAPLOW(step1[16] - step1[19]);
  step2[20] = WRAPLOW(-step1[20] + step1[23]);
  step2[21] = WRAPLOW(-step1[21] + step1[22]);
  step2[22] = WRAPLOW(step1[21] + step1[22]);
  step2[23] = WRAPLOW(step1[20] + step1[23]);

  step2[24] = WRAPLOW(step1[24] + step1[27]);
  step2[25] = WRAPLOW(step1[25] + step1[26]);
  step2[26] = WRAPLOW(step1[25] - step1[26]);
  step2[27] = WRAPLOW(step1[24] - step1[27]);
  step2[28] = WRAPLOW(-step1[28] + step1[31]);
  step2[29] = WRAPLOW(-step1[29] + step1[30]);
  step2[30] = WRAPLOW(step1[29] + step1[30]);
  step2[31] = WRAPLOW(step1[28] + step1[31]);

  // stage 5
  step1[0] = WRAPLOW(step2[0] + step2[3]);
  step1[1] = WRAPLOW(step2[1] + step2[2]);
  step1[2] = WRAPLOW(step2[1] - step2[2]);
  step1[3] = WRAPLOW(step2[0] - step2[3]);
  step1[4] = step2[4];
  temp1 = (step2[6] - step2[5]) * cospi_16_64;
  temp2 = (step2[5] + step2[6]) * cospi_16_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1));
  step1[6] = WRAPLOW(dct_const_round_shift(temp2));
  step1[7] = step2[7];

  step1[8] = WRAPLOW(step2[8] + step2[11]);
  step1[9] = WRAPLOW(step2[9] + step2[10]);
  step1[10] = WRAPLOW(step2[9] - step2[10]);
  step1[11] = WRAPLOW(step2[8] - step2[11]);
  step1[12] = WRAPLOW(-step2[12] + step2[15]);
  step1[13] = WRAPLOW(-step2[13] + step2[14]);
  step1[14] = WRAPLOW(step2[13] + step2[14]);
  step1[15] = WRAPLOW(step2[12] + step2[15]);

  step1[16] = step2[16];
  step1[17] = step2[17];
  temp1 = -step2[18] * cospi_8_64 + step2[29] * cospi_24_64;
  temp2 = step2[18] * cospi_24_64 + step2[29] * cospi_8_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1));
  step1[29] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step2[19] * cospi_8_64 + step2[28] * cospi_24_64;
  temp2 = step2[19] * cospi_24_64 + step2[28] * cospi_8_64;
  step1[19] = WRAPLOW(dct_const_round_shift(temp1));
  step1[28] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step2[20] * cospi_24_64 - step2[27] * cospi_8_64;
  temp2 = -step2[20] * cospi_8_64 + step2[27] * cospi_24_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1));
  step1[27] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = -step2[21] * cospi_24_64 - step2[26] * cospi_8_64;
  temp2 = -step2[21] * cospi_8_64 + step2[26] * cospi_24_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1));
  step1[26] = WRAPLOW(dct_const_round_shift(temp2));
  step1[22] = step2[22];
  step1[23] = step2[23];
  step1[24] = step2[24];
  step1[25] = step2[25];
  step1[30] = step2[30];
  step1[31] = step2[31];

  // stage 6
  step2[0] = WRAPLOW(step1[0] + step1[7]);
  step2[1] = WRAPLOW(step1[1] + step1[6]);
  step2[2] = WRAPLOW(step1[2] + step1[5]);
  step2[3] = WRAPLOW(step1[3] + step1[4]);
  step2[4] = WRAPLOW(step1[3] - step1[4]);
  step2[5] = WRAPLOW(step1[2] - step1[5]);
  step2[6] = WRAPLOW(step1[1] - step1[6]);
  step2[7] = WRAPLOW(step1[0] - step1[7]);
  step2[8] = step1[8];
  step2[9] = step1[9];
  temp1 = (-step1[10] + step1[13]) * cospi_16_64;
  temp2 = (step1[10] + step1[13]) * cospi_16_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1));
  step2[13] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = (-step1[11] + step1[12]) * cospi_16_64;
  temp2 = (step1[11] + step1[12]) * cospi_16_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1));
  step2[12] = WRAPLOW(dct_const_round_shift(temp2));
  step2[14] = step1[14];
  step2[15] = step1[15];

  step2[16] = WRAPLOW(step1[16] + step1[23]);
  step2[17] = WRAPLOW(step1[17] + step1[22]);
  step2[18] = WRAPLOW(step1[18] + step1[21]);
  step2[19] = WRAPLOW(step1[19] + step1[20]);
  step2[20] = WRAPLOW(step1[19] - step1[20]);
  step2[21] = WRAPLOW(step1[18] - step1[21]);
  step2[22] = WRAPLOW(step1[17] - step1[22]);
  step2[23] = WRAPLOW(step1[16] - step1[23]);

  step2[24] = WRAPLOW(-step1[24] + step1[31]);
  step2[25] = WRAPLOW(-step1[25] + step1[30]);
  step2[26] = WRAPLOW(-step1[26] + step1[29]);
  step2[27] = WRAPLOW(-step1[27] + step1[28]);
  step2[28] = WRAPLOW(step1[27] + step1[28]);
  step2[29] = WRAPLOW(step1[26] + step1[29]);
  step2[30] = WRAPLOW(step1[25] + step1[30]);
  step2[31] = WRAPLOW(step1[24] + step1[31]);

  // stage 7
  step1[0] = WRAPLOW(step2[0] + step2[15]);
  step1[1] = WRAPLOW(step2[1] + step2[14]);
  step1[2] = WRAPLOW(step2[2] + step2[13]);
  step1[3] = WRAPLOW(step2[3] + step2[12]);
  step1[4] = WRAPLOW(step2[4] + step2[11]);
  step1[5] = WRAPLOW(step2[5] + step2[10]);
  step1[6] = WRAPLOW(step2[6] + step2[9]);
  step1[7] = WRAPLOW(step2[7] + step2[8]);
  step1[8] = WRAPLOW(step2[7] - step2[8]);
  step1[9] = WRAPLOW(step2[6] - step2[9]);
  step1[10] = WRAPLOW(step2[5] - step2[10]);
  step1[11] = WRAPLOW(step2[4] - step2[11]);
  step1[12] = WRAPLOW(step2[3] - step2[12]);
  step1[13] = WRAPLOW(step2[2] - step2[13]);
  step1[14] = WRAPLOW(step2[1] - step2[14]);
  step1[15] = WRAPLOW(step2[0] - step2[15]);

  step1[16] = step2[16];
  step1[17] = step2[17];
  step1[18] = step2[18];
  step1[19] = step2[19];
  temp1 = (-step2[20] + step2[27]) * cospi_16_64;
  temp2 = (step2[20] + step2[27]) * cospi_16_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1));
  step1[27] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = (-step2[21] + step2[26]) * cospi_16_64;
  temp2 = (step2[21] + step2[26]) * cospi_16_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1));
  step1[26] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = (-step2[22] + step2[25]) * cospi_16_64;
  temp2 = (step2[22] + step2[25]) * cospi_16_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1));
  step1[25] = WRAPLOW(dct_const_round_shift(temp2));
  temp1 = (-step2[23] + step2[24]) * cospi_16_64;
  temp2 = (step2[23] + step2[24]) * cospi_16_64;
  step1[23] = WRAPLOW(dct_const_round_shift(temp1));
  step1[24] = WRAPLOW(dct_const_round_shift(temp2));
  step1[28] = step2[28];
  step1[29] = step2[29];
  step1[30] = step2[30];
  step1[31] = step2[31];

  // final stage
  output[0] = WRAPLOW(step1[0] + step1[31]);
  output[1] = WRAPLOW(step1[1] + step1[30]);
  output[2] = WRAPLOW(step1[2] + step1[29]);
  output[3] = WRAPLOW(step1[3] + step1[28]);
  output[4] = WRAPLOW(step1[4] + step1[27]);
  output[5] = WRAPLOW(step1[5] + step1[26]);
  output[6] = WRAPLOW(step1[6] + step1[25]);
  output[7] = WRAPLOW(step1[7] + step1[24]);
  output[8] = WRAPLOW(step1[8] + step1[23]);
  output[9] = WRAPLOW(step1[9] + step1[22]);
  output[10] = WRAPLOW(step1[10] + step1[21]);
  output[11] = WRAPLOW(step1[11] + step1[20]);
  output[12] = WRAPLOW(step1[12] + step1[19]);
  output[13] = WRAPLOW(step1[13] + step1[18]);
  output[14] = WRAPLOW(step1[14] + step1[17]);
  output[15] = WRAPLOW(step1[15] + step1[16]);
  output[16] = WRAPLOW(step1[15] - step1[16]);
  output[17] = WRAPLOW(step1[14] - step1[17]);
  output[18] = WRAPLOW(step1[13] - step1[18]);
  output[19] = WRAPLOW(step1[12] - step1[19]);
  output[20] = WRAPLOW(step1[11] - step1[20]);
  output[21] = WRAPLOW(step1[10] - step1[21]);
  output[22] = WRAPLOW(step1[9] - step1[22]);
  output[23] = WRAPLOW(step1[8] - step1[23]);
  output[24] = WRAPLOW(step1[7] - step1[24]);
  output[25] = WRAPLOW(step1[6] - step1[25]);
  output[26] = WRAPLOW(step1[5] - step1[26]);
  output[27] = WRAPLOW(step1[4] - step1[27]);
  output[28] = WRAPLOW(step1[3] - step1[28]);
  output[29] = WRAPLOW(step1[2] - step1[29]);
  output[30] = WRAPLOW(step1[1] - step1[30]);
  output[31] = WRAPLOW(step1[0] - step1[31]);
}
#endif

#if CONFIG_MRC_TX
void aom_imrc32x32_1024_add_c(const tran_low_t *input, uint8_t *dest,
                              int stride, uint8_t *mask) {
  tran_low_t out[32 * 32];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  for (i = 0; i < 32; ++i) {
    int16_t zero_coeff[16];
    for (j = 0; j < 16; ++j) zero_coeff[j] = input[2 * j] | input[2 * j + 1];
    for (j = 0; j < 8; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
    for (j = 0; j < 4; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
    for (j = 0; j < 2; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];

    if (zero_coeff[0] | zero_coeff[1])
      aom_idct32_c(input, outptr);
    else
      memset(outptr, 0, sizeof(tran_low_t) * 32);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      // Only add the coefficient if the mask value is 1
      int mask_val = mask[j * 32 + i];
      dest[j * stride + i] =
          mask_val ? clip_pixel_add(dest[j * stride + i],
                                    ROUND_POWER_OF_TWO(temp_out[j], 6))
                   : dest[j * stride + i];
    }
  }
}

void aom_imrc32x32_135_add_c(const tran_low_t *input, uint8_t *dest, int stride,
                             uint8_t *mask) {
  tran_low_t out[32 * 32] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  // only upper-left 16x16 has non-zero coeff
  for (i = 0; i < 16; ++i) {
    aom_idct32_c(input, outptr);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      // Only add the coefficient if the mask value is 1
      int mask_val = mask[j * 32 + i];
      dest[j * stride + i] =
          mask_val ? clip_pixel_add(dest[j * stride + i],
                                    ROUND_POWER_OF_TWO(temp_out[j], 6))
                   : dest[j * stride + i];
    }
  }
}

void aom_imrc32x32_34_add_c(const tran_low_t *input, uint8_t *dest, int stride,
                            uint8_t *mask) {
  tran_low_t out[32 * 32] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  // only upper-left 8x8 has non-zero coeff
  for (i = 0; i < 8; ++i) {
    aom_idct32_c(input, outptr);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      // Only add the coefficient if the mask value is 1
      int mask_val = mask[j * 32 + i];
      dest[j * stride + i] =
          mask_val ? clip_pixel_add(dest[j * stride + i],
                                    ROUND_POWER_OF_TWO(temp_out[j], 6))
                   : dest[j * stride + i];
    }
  }
}
#endif  // CONFIG_MRC_TX

void aom_idct32x32_1024_add_c(const tran_low_t *input, uint8_t *dest,
                              int stride) {
  tran_low_t out[32 * 32];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  for (i = 0; i < 32; ++i) {
    int16_t zero_coeff[16];
    for (j = 0; j < 16; ++j) zero_coeff[j] = input[2 * j] | input[2 * j + 1];
    for (j = 0; j < 8; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
    for (j = 0; j < 4; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];
    for (j = 0; j < 2; ++j)
      zero_coeff[j] = zero_coeff[2 * j] | zero_coeff[2 * j + 1];

    if (zero_coeff[0] | zero_coeff[1])
      aom_idct32_c(input, outptr);
    else
      memset(outptr, 0, sizeof(tran_low_t) * 32);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

void aom_idct32x32_135_add_c(const tran_low_t *input, uint8_t *dest,
                             int stride) {
  tran_low_t out[32 * 32] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  // only upper-left 16x16 has non-zero coeff
  for (i = 0; i < 16; ++i) {
    aom_idct32_c(input, outptr);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

void aom_idct32x32_34_add_c(const tran_low_t *input, uint8_t *dest,
                            int stride) {
  tran_low_t out[32 * 32] = { 0 };
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  // only upper-left 8x8 has non-zero coeff
  for (i = 0; i < 8; ++i) {
    aom_idct32_c(input, outptr);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j) temp_in[j] = out[j * 32 + i];
    aom_idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

void aom_idct32x32_1_add_c(const tran_low_t *input, uint8_t *dest, int stride) {
  int i, j;
  tran_high_t a1;

  tran_low_t out = WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64));
  out = WRAPLOW(dct_const_round_shift(out * cospi_16_64));
  a1 = ROUND_POWER_OF_TWO(out, 6);
  if (a1 == 0) return;

  for (j = 0; j < 32; ++j) {
    for (i = 0; i < 32; ++i) dest[i] = clip_pixel_add(dest[i], a1);
    dest += stride;
  }
}

#if CONFIG_TX64X64 && CONFIG_DAALA_DCT64
void aom_idct64_c(const tran_low_t *input, tran_low_t *output) {
  int i;
  od_coeff x[64];
  od_coeff y[64];
  for (i = 0; i < 64; i++) y[i] = (od_coeff)input[i];
  od_bin_idct64(x, 1, y);
  for (i = 0; i < 64; i++) output[i] = (tran_low_t)x[i];
}
#endif

void aom_highbd_iwht4x4_16_add_c(const tran_low_t *input, uint8_t *dest8,
                                 int stride, int bd) {
  /* 4-point reversible, orthonormal inverse Walsh-Hadamard in 3.5 adds,
     0.5 shifts per pixel. */
  int i;
  tran_low_t output[16];
  tran_high_t a1, b1, c1, d1, e1;
  const tran_low_t *ip = input;
  tran_low_t *op = output;
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);

  for (i = 0; i < 4; i++) {
    a1 = ip[0] >> UNIT_QUANT_SHIFT;
    c1 = ip[1] >> UNIT_QUANT_SHIFT;
    d1 = ip[2] >> UNIT_QUANT_SHIFT;
    b1 = ip[3] >> UNIT_QUANT_SHIFT;
    a1 += c1;
    d1 -= b1;
    e1 = (a1 - d1) >> 1;
    b1 = e1 - b1;
    c1 = e1 - c1;
    a1 -= b1;
    d1 += c1;
    op[0] = HIGHBD_WRAPLOW(a1, bd);
    op[1] = HIGHBD_WRAPLOW(b1, bd);
    op[2] = HIGHBD_WRAPLOW(c1, bd);
    op[3] = HIGHBD_WRAPLOW(d1, bd);
    ip += 4;
    op += 4;
  }

  ip = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[4 * 0];
    c1 = ip[4 * 1];
    d1 = ip[4 * 2];
    b1 = ip[4 * 3];
    a1 += c1;
    d1 -= b1;
    e1 = (a1 - d1) >> 1;
    b1 = e1 - b1;
    c1 = e1 - c1;
    a1 -= b1;
    d1 += c1;
    dest[stride * 0] =
        highbd_clip_pixel_add(dest[stride * 0], HIGHBD_WRAPLOW(a1, bd), bd);
    dest[stride * 1] =
        highbd_clip_pixel_add(dest[stride * 1], HIGHBD_WRAPLOW(b1, bd), bd);
    dest[stride * 2] =
        highbd_clip_pixel_add(dest[stride * 2], HIGHBD_WRAPLOW(c1, bd), bd);
    dest[stride * 3] =
        highbd_clip_pixel_add(dest[stride * 3], HIGHBD_WRAPLOW(d1, bd), bd);

    ip++;
    dest++;
  }
}

void aom_highbd_iwht4x4_1_add_c(const tran_low_t *in, uint8_t *dest8,
                                int dest_stride, int bd) {
  int i;
  tran_high_t a1, e1;
  tran_low_t tmp[4];
  const tran_low_t *ip = in;
  tran_low_t *op = tmp;
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  (void)bd;

  a1 = ip[0] >> UNIT_QUANT_SHIFT;
  e1 = a1 >> 1;
  a1 -= e1;
  op[0] = HIGHBD_WRAPLOW(a1, bd);
  op[1] = op[2] = op[3] = HIGHBD_WRAPLOW(e1, bd);

  ip = tmp;
  for (i = 0; i < 4; i++) {
    e1 = ip[0] >> 1;
    a1 = ip[0] - e1;
    dest[dest_stride * 0] =
        highbd_clip_pixel_add(dest[dest_stride * 0], a1, bd);
    dest[dest_stride * 1] =
        highbd_clip_pixel_add(dest[dest_stride * 1], e1, bd);
    dest[dest_stride * 2] =
        highbd_clip_pixel_add(dest[dest_stride * 2], e1, bd);
    dest[dest_stride * 3] =
        highbd_clip_pixel_add(dest[dest_stride * 3], e1, bd);
    ip++;
    dest++;
  }
}

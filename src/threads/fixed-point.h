#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

/* Fixed-point arithmetic using 17.14 format (17 integer bits, 14 fractional bits).
   Value = n * 2^14, where n is the actual number.
   Range: ~ +/- 131,071.9999, precision: 1/16384 ≈ 0.000061. */

#define FP_SHIFT 14
#define FP_SCALE (1 << FP_SHIFT)  // 16384

/* Convert int to fixed-point */
#define int_to_fp(n) ((int32_t)(n) * FP_SCALE)

/* Convert fixed-point to int (truncate toward zero) */
#define fp_to_int_floor(x) ((int32_t)(x) / FP_SCALE)

/* Convert fixed-point to int (round to nearest) */
#define fp_to_int_round(x) (((int32_t)(x) + (FP_SCALE / 2)) / FP_SCALE)

/* Fixed-point arithmetic operations */
#define fp_add(x, y) ((int32_t)(x) + (int32_t)(y))
#define fp_sub(x, y) ((int32_t)(x) - (int32_t)(y))
#define fp_neg(x) (-(int32_t)(x))

/* Multiplication and division require 64-bit intermediate values */
#define fp_mul(x, y) ((int32_t)(((int64_t)(x) * (int64_t)(y)) / FP_SCALE))
#define fp_div(x, y) ((int32_t)(((int64_t)(x) * FP_SCALE) / (int64_t)(y)))

/* Add int to fixed-point */
#define fp_add_int(x, n) ((int32_t)(x) + (int32_t)(n) * FP_SCALE)
#define fp_sub_int(x, n) ((int32_t)(x) - (int32_t)(n) * FP_SCALE)

/* Multiply/divide fixed-point by int */
#define fp_mul_int(x, n) ((int32_t)((int64_t)(x) * (int64_t)(n)))
#define fp_div_int(x, n) ((int32_t)((int64_t)(x) / (int64_t)(n)))

#endif /* threads/fixed_point_h */
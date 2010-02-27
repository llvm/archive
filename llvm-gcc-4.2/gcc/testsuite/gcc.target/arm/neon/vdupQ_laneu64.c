/* APPLE LOCAL file v7 merge */
/* Test the `vdupQ_laneu64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vdupQ_laneu64 (void)
{
  uint64x2_t out_uint64x2_t;
  uint64x1_t arg0_uint64x1_t;

  out_uint64x2_t = vdupq_lane_u64 (arg0_uint64x1_t, 0);
}

/* { dg-final { cleanup-saved-temps } } */

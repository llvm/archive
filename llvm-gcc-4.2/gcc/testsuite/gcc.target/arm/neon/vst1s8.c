/* APPLE LOCAL file v7 merge */
/* Test the `vst1s8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vst1s8 (void)
{
  int8_t *arg0_int8_t;
  int8x8_t arg1_int8x8_t;

  vst1_s8 (arg0_int8_t, arg1_int8x8_t);
}

/* { dg-final { scan-assembler "vst1\.8\[ 	\]+((\\\{\[dD\]\[0-9\]+\\\})|(\[dD\]\[0-9\]+)), \\\[\[rR\]\[0-9\]+\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */

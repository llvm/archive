/* APPLE LOCAL file CW asm blocks */
/* { dg-do assemble { target i?86*-*-darwin* } } */
/* { dg-options { -fasm-blocks -msse3 } } */
/* Radar 4371551 */

unsigned long long __attribute__ ((aligned(8))) MaskMoveTable[8];
unsigned short gGaussianWeights[ 31 ];

void
foo() {
  asm {
    movq mm4, MaskMoveTable[ebx*8-8]
    movzx ebx, word ptr gGaussianWeights[edx+edx]
  }
}

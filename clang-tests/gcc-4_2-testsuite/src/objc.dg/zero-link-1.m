/* Check if the '-fzero-link' flag correctly emits an objc_getClass() call. */
/* Contributed by Ziemowit Laski <zlaski@apple.com>.  */
/* { dg-options "-fnext-runtime -fzero-link" } */
/* { dg-do compile } */

#include <objc/objc.h>
#include <objc/Object.h>

extern void abort(void);
#define CHECK_IF(expr) if(!(expr)) abort();

@interface Base: Object
+ (int) getValue;
@end

@implementation Base
+ (int) getValue { return 1593; }
@end

int main(void) {
  int val = [Base getValue];
  CHECK_IF(val == 1593);
  return 0;
}

/* { dg-final { scan-assembler-not "_OBJC_CLASS_REFERENCES_0" } } */
/* { dg-final { scan-assembler "objc_getClass" } } */


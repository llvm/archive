/* APPLE LOCAL file mainline */
/* Test for spurious "may or may not return a value" warnings.  */

/* { dg-do compile } */
/* { dg-options "-Wextra" } */

/* APPLE LOCAL radar 4894756 */
#include "../objc/execute/Object2.h"

@interface Foo: Object
- (id) meth1;
- (void) meth2;
@end

extern int bar;

@implementation Foo
- (id) meth1 {
  if (bar)
    return [Object new];
  return;
} /* { dg-warning "this function may return with or without a value" } */
- (void) meth2 {
  if (!bar)
    return;
  bar = 0;
} /* { dg-bogus "this function may return with or without a value" } */
@end

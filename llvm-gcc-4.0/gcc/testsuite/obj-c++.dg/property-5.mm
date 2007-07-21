/* APPLE LOCAL file radar 4505126 */
/* Test lookup of properties in super classes. */
/* Program should compile with no error or warning. */
/* { dg-do compile { target *-*-darwin* } } */
/* { dg-options "-fobjc-abi-version=2" } */

#import <Foundation/Foundation.h>

@interface test : NSObject
{
}
@property NSString *foo;
@end

@interface test_sub : test
@end

int main (int argc, char **argv)
{
    test_sub *outer = nil;
    test *inner = outer;
    NSString *x;

    x = outer.foo;
    outer.foo = @"two";

    x = inner.foo;
    inner.foo = @"one";

    return 0;
}

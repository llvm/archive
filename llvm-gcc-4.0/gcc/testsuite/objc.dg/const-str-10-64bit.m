/* APPLE LOCAL file 4492976 */
/* Test if ObjC constant string layout is checked properly, regardless of how
   constant string classes get derived.  */
/* { dg-options "-fnext-runtime -fno-constant-cfstrings -m64" } */
/* { dg-do compile { target *-*-darwin* } } */

#include <objc/Object.h>

@interface NSString: Object
@end

@interface NSSimpleCString : NSString {
@protected
    char *bytes;
    unsigned int numBytes;
}
@end
    
@interface NSConstantString : NSSimpleCString
@end

extern struct objc_class _NSConstantStringClassReference;

const NSConstantString *appKey = @"MyApp";

/* { dg-final { scan-assembler ".section __OBJC, __cstring_object" } } */
/* { dg-final { scan-assembler ".quad\t__NSConstantStringClassReference\n\t.quad\t.*\n\t.long\t5\n\t.space 4\n\t.data" } } */

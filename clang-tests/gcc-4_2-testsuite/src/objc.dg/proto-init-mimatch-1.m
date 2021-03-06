/* APPLE LOCAL file radar 4293709 */
/* Test to warn on protocol mismatch in a variety of initializations. */

/* { dg-do compile { target *-*-darwin* } } */

typedef struct objc_class *Class;

typedef struct objc_object {
        Class isa;
} *id;

@protocol NSObject
@end

__attribute__((objc_root_class)) @interface NSObject <NSObject> 
@end

@protocol NSCopying
- (void)copyWithZone;
@end

@interface Foo:NSObject <NSCopying> 
@end


extern id <NSObject> NSCopyObject();

@implementation Foo
- (void)copyWithZone {
    Foo *copy = NSCopyObject(); /* { dg-warning "type \\'id <NSObject>\\' does not conform to the \\'NSCopying\\' protocol" } */

    Foo<NSObject,NSCopying> *g = NSCopyObject(); /* { dg-warning "type \\'id <NSObject>\\' does not conform to the \\'NSCopying\\' protocol" } */

    id<NSObject,NSCopying> h = NSCopyObject(); /* { dg-warning "type \\'id <NSObject>\\' does not conform to the \\'NSCopying\\' protocol" } */
}
@end	

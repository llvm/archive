/* APPLE LOCAL file radar 3803157, 5005756, 5556192 */
/* Test implementaion of attributes on methods. */
/* { dg-do compile } */

#include <objc/objc.h>

#define EMPTY

#define ATTR __attribute__ ((deprecated))
__attribute__((objc_root_class)) @interface INTF
- (void) foo __attribute__ ((deprecated)) __attribute__ ((unavailable)); /* { dg-error "" } */

- (int) variarg : (int)anchor, ... ATTR;

- (void) fun1 : (int)arg1 : (int)arg2 __attribute__ ((deprecated));

- bar ATTR __attribute__ ((unavailable));	/* { dg-error "" } */

- (int) final EMPTY ATTR;

- ok EMPTY;

- ok1:(int)anchor, ... EMPTY;
@end

int main()
{
	INTF *p;
	id pid;

	[p foo];	/* { dg-error "\\'foo\\' is unavailable" } */
	[p variarg:1,2]; /* { dg-warning "\\'variarg:\\' is deprecated" } */
	[p fun1:1:2]; /* { dg-warning "\\'fun1::\\' is deprecated" } */
	[p bar];	/* { dg-error "\\'bar\\' is unavailable" } */

	[pid foo];	// OK
	[pid variarg:1,2]; // OK	
	[pid fun1:1:2];		// OK
	[pid bar];	// OK

	[p ok];
	[p ok1:1,2];
	[pid ok];
	[pid ok1:1,2];

	return [p final];	/* { dg-warning "\\'final\\' is deprecated" } */
}

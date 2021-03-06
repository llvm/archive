/* Test diagnostics for missing and extra semicolons in structures.
   Test with no special options.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */
/* { dg-do compile } */
/* { dg-options "" } */

struct s0 { ; };
struct s1 {
  int a;
  ;
  int b;
};
struct s2 {
  ;
  int c /* { dg-warning "expected" } */
}; 
struct s3 {
  int d /* { dg-warning "expected" } */
}; 

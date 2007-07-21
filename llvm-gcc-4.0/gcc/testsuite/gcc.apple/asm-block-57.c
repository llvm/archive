/* APPLE LOCAL file CW asm blocks */
/* { dg-do run { target i?86*-*-darwin* } } */
/* { dg-options { -fasm-blocks -msse3 } } */
/* Radar 4682427 */

void bar() __attribute ((weak));
void bar() { }
void bardef() { }

int main() {
  asm {
    call  bar
    call  bardef
    jmp   foo
    call  eax
    call  foo
    jmp   bar
    jmp   bardef
    jmp   eax
    nop
   foo: nop
  }
  return 0;
}

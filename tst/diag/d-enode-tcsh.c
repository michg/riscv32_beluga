void f1(void) { char c; int i; c = c << i; }
void f2(void) { struct t x; int i; i = x << i; }
void f3(void) { struct t x; int i; i = i << x; }
void f4(void) { int *p; int i; i = p << i; }

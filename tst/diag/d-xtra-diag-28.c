int *p;

void f(void)
{
    int x;

    ((1)? p: p)++;
    ((p)? p: p)++;

    ((1)? p: p) = p;
    ((p)? p: p) = p;

    ((1)? p: p) += 0;
    ((p)? p: p) += 0;

    &((1)? p: p);
    &((p)? p: p);

    ~~x = 0;
    - -x = 0;

    (&*p) = 0;

    +x = 0;
    + +x = 0;
}

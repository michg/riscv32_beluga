
/* n_27.c:  Rescanning of a macro replace any macro call in the replacement
        text after substitution of parameters by pre-expanded-arguments.  This
        re-examination may involve the succeding sequences from the source
        file (what a queer thing!). */

main( void)
{
    int     a = 1, b = 2, c, m = 1, n = 2;

    fputs( "started\n", stderr);

/* 27.1:    Cascaded use of object-like macros. */
/*  1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;  */
#define NEST8   NEST7 + 8
#define NEST7   NEST6 + 7
#define NEST6   NEST5 + 6
#define NEST5   NEST4 + 5
#define NEST4   NEST3 + 4
#define NEST3   NEST2 + 3
#define NEST2   NEST1 + 2
#define NEST1   1
    assert( NEST8 == 36);

/* 27.2:    Cascaded use of function-like macros.   */
/*  (1) + (1 + 2) + 1 + 2 + 1 + 2 + 3 + 1 + 2 + 3 + 4;  */
#define FUNC4( a, b)    FUNC3( a, b) + NEST4
#define FUNC3( a, b)    FUNC2( a, b) + NEST3
#define FUNC2( a, b)    FUNC1( a, b) + NEST2
#define FUNC1( a, b)    (a) + (b)
    assert( FUNC4( NEST1, NEST2) == 23);

/* 27.3:    An identifier generated by ## operator is subject to expansion. */
#define glue( a, b)     a ## b
#define MACRO_1         1
    assert( glue( MACRO_, 1) == 1);

#define sub( x, y)      (x - y)
#define head            sub(
#define math( op, a, b) op( (a), (b))

/* 27.4:    'sub' as an argument of math() is not pre-expanded, since '(' is
        missing.    */
    assert( math( sub, a, b) == -1);

/* 27.5:    Queer thing.    */
    c = head a,b );
    assert( c == -1);

/* 27.6:    Recursive macro (the 2nd 'm' is expanded to 'n' since it is in
        source file).   */
#define m       n
#define n( a)   a
    c = m( m);
    assert( c == 2);

    fputs( "success\n", stderr);
    return  0;
}


void f()
{
    int x1[2][268435455] = {
        0, 0, 0,
    };

    int x2[2][268435455] = {
        { 0, }, { 0 }, 0, 0
    };

    int x3[2][268435455] = {
        { 0, }, { 0 }, { 0, }, { 0 }
    };

    int x4[2][2][134217727] = {
        { { 0, }, { 0 }, { 0 }, },
        { { 0, }, { 0 }, 0, 0 },
        0,
        { 0 },
    };

    int x5[][268435455] = {
        { 0, }, { 0 }, { 0, }, { 0 }
    };

    int x6[2] = { 0, 1, if, while };
    int x7[]  = { 0, 1, if, while };

    f(x1, x2, x3, x4, x5, x6, x7);
}

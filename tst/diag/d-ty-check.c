/* --std=c11 -Wv */

struct t {
    const struct tag {
        int a;    /* ambiguous */
    };
    int a;    /* ambiguous */
    struct {
        int a;    /* ambiguous */
    };
    struct {
        struct {
           int a;    /* ambiguous */
        };
        struct {
           int a;    /* ambiguous */
        };
    } a;
} a;

struct u {
    union uag {
        int a;    /* ambiguous */
    };
    struct {
        int a;    /* ambiguous */
    };
} b;

union {
    union {
        int a;
    };
    struct {
        union {
            int b;
        };
    };
} c;

void f(void) {
    a.a;
    b.a;
    c.a;
    (&c)->a;
    c.b;
    (&c)->b;
}

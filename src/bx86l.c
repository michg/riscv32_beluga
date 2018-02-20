/*
 *  x86-linux binding
 */

#include <stddef.h>        /* NULL */
#include <stdio.h>         /* FILE, fprintf, fputs */
#include <string.h>        /* strlen */
#include <cbl/assert.h>    /* assert */
#include <cdsl/hash.h>     /* hash_int */
#include <cel/opt.h>       /* opt_t, opt_reinit, opt_parse, opt_errmsg */
#ifndef NDEBUG
#include <stdio.h>         /* stderr */
#endif    /* !NDEBUG */

#include "bnull.h"    /* common.h, dag.h, gen.h, ir.h, lex.h, op.h, sym.h */
#include "clx.h"
#include "cgr.h"
#include "err.h"
#include "init.h"
#include "lmap.h"
#include "reg.h"
#include "strg.h"
#include "ty.h"

#define P(x) bx86l_##x         /* prefix to avoid name conflict */
#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */

/* token pasting */
#define xpaste(p, n) paste(p, n)
#define paste(p, n)  p ## n


/* non-terminals;
   ASSUMPTION: int is at least 32-bit wide on the host (see op.h) */
enum {
    P(_) = OP_2+1,    /* +1 as non-terminals start at 1 */
#define tt(t) P(t),
#define rr(n, ops, c, cf, t)
#include "bx86l.r"
    P(max)
};

/* registers */
enum {
    EAX,
    ECX,
    EDX,
    EBX,
    ESI = 6,
    EDI,
    MAX
};


static int fprog;         /* true when progbeg() invoked */
static int finit;         /* true when init() invoked */
static FILE *out;         /* output file */
static int endian = 1;    /* for LITTLE from common.h */

static cgr_t rule[
#define tt(t)
#define rr(n, ops, c, cf, t) +1
#include "bx86l.r"
+1];    /* for end marker */

static sym_t *intreg[MAX],      /* register set for 4-byte integers */
             *shortreg[MAX],    /* register set for 2-byte integers */
             *charreg[MAX],     /* register set for 1-byte integers */
             *fltreg[MAX];      /* register set for floating-points */
static sym_t *intregw,          /* wildcard for intreg */
             *shortregw,        /* wildcard for shortreg */
             *charregw,         /* wildcard for charreg */
             *fltregw;          /* wildcard for fltreg */
static sym_t *quo, *rem;        /* registers for div/rem */


/*
 *  cost function: con1
 */
static int con1(dag_node_t *p)
{
    assert(p);
    return (xe(p->sym[0]->u.c.v.u, xI))? 0: CGR_CSTMAX;
}


/*
 *  cost function: con2
 */
static int con2(dag_node_t *p)
{
    assert(p);
    return (xe(p->sym[0]->u.c.v.u, xiu(2)))? 0: CGR_CSTMAX;
}


/*
 *  cost function: con3
 */
static int con3(dag_node_t *p)
{
    assert(p);
    return (xe(p->sym[0]->u.c.v.u, xiu(3)))? 0: CGR_CSTMAX;
}


/*
 *  cost function: range31
 */
static int range31(dag_node_t *p)
{
    assert(p);
    return (xls(p->sym[0]->u.c.v.u, xiu(32)))? 0: CGR_CSTMAX;
}


/*
 *  checks if two dags are identical for memop()
 */
static int same(const dag_node_t *p, const dag_node_t *q) {
    return (!p && !q) ||
           (p && q && p->op == q->op && p->sym[0] == q->sym[0] &&
            same(p->kid[0], q->kid[0]) && same(p->kid[1], q->kid[1]));
}


/*
 *  cost function: memop
 */
static int memop(dag_node_t *p)
{
    assert(p);
    assert(op_generic(p->op) == OP_ASGN);
    assert(p->kid[0]);
    assert(p->kid[1] && p->kid[1]->kid[0]);

    return (op_generic(p->kid[1]->kid[0]->op) == OP_INDIR &&
            same(p->kid[0], p->kid[1]->kid[0]->kid[0]))? 3: CGR_CSTMAX;
}


/*
 *  cost function: arg
 */
static int arg(dag_node_t *p)
{
    assert(p);
    assert(p->sym[0]);

    return (xgs(p->sym[0]->u.c.v.s, xO))? 1: CGR_CSTMAX;
}


/*
 *  cost function: args
 */
static int args(dag_node_t *p)
{
    ty_t *rty;

    assert(p);
    assert(p->sym[0]);

    return (xe(p->sym[0]->u.c.v.s, xO))? CGR_CSTMAX:
           (assert(p->sym[1]), rty=ty_freturn(p->sym[1]->type), TY_ISSTRUNI(rty))? 0: 1;
}


/*
 *  initializes the back-end
 */
static void init(void)
{
    int i = 0;

#define tt(t)
#define rr(n, ops, c, cf, t) static const int xpaste(rule, __LINE__)[] = { ops, -1 };
#include "bx86l.r"

    assert(!finit);
    finit = 1;

#define tt(t)
#define rr(n, ops, c, cf, t)                                         \
    rule[i].rn = i,                                                  \
    rule[i].nt = n,                                                  \
    rule[i].ot = xpaste(rule, __LINE__),                             \
    rule[i].cost = c,                                                \
    rule[i].costf = cf,                                              \
    rule[i].tmpl = t,                                                \
    rule[i++].isinst = (sizeof(t) > 1 && t[sizeof(t)-2] == '\n');
#include "bx86l.r"
    rule[i].nt = -1;

    for (i = 0; rule[i].nt > 0; i++) {
        rule[i].tree = cgr_tree(rule[i].ot, &rule[i].nnt);
        cgr_add(&rule[i]);
    }
    assert(i == NELEM(rule)-1);
}


/*
 *  sets x of an address symbol
 */
static void symaddr(sym_t *p, sym_t *q, ssz_t n)
{
    assert(p);
    assert(q);

    assert(!p->x.name);
    assert(q->x.name);

    if (q->scope == SYM_SGLOBAL || q->sclass == LEX_STATIC || q->sclass == LEX_EXTERN)
        p->x.name = gen_sfmt(strlen(q->x.name) + 1 + STRG_BUFN, "%s%+ld" , q->x.name, n);
    else {
        p->x.offset = q->x.offset + n;
        p->x.name = hash_int(p->x.offset);
    }
}


/*
 *  sets x of a global/static/constant symbol
 */
static void symgsc(sym_t *p)
{
    assert(p);

    if (p->x.name)
        return;

    if (p->scope == SYM_SCONST)    /* must precede check for GENSYM() */
        p->x.name = p->name;
    else if (GENSYM(p))
        p->x.name = gen_sfmt(3 + STRG_BUFN, ".LC%s", p->name);
    else if (p->scope >= SYM_SLOCAL && p->sclass == LEX_STATIC)    /* static local */
        p->x.name = gen_sfmt(strlen(p->name) + 1 + STRG_BUFN, "%s.%d", p->name, sym_genlab(1));
    else {
        assert(p->sclass == LEX_EXTERN || (p->scope == SYM_SGLOBAL &&
                                           (p->sclass == LEX_AUTO || p->sclass == LEX_STATIC)));
        p->x.name = p->name;
    }
}


/*
 *  returns a wildcard for an operation
 */
static sym_t *rmapw(int op)
{
    sym_t **ira[] = { NULL, &charregw, &shortregw, NULL, &intregw };

    switch(op_type(op)) {
        case OP_F:
            return fltregw;
        case OP_I:
        case OP_U:
            assert(op_size(op) < NELEM(ira) && ira[op_size(op)]);
            return *ira[op_size(op)];
        case OP_P:
        case OP_B:
            return intregw;
    }

    return NULL;
}


/*
 *  returns the register set for an operation
 */
static int rmaps(int op)
{
    switch(op_type(op)) {
        case OP_F:
            return REG_SFP;
        default:
            return REG_SINT;
    }
}


/*
 *  sets x of a local
 */
static void symlocal(sym_t *p)
{
    assert(p);

    if (TY_ISFP(p->type))
        p->sclass = LEX_AUTO;
    if (!reg_askvar(p, rmapw(op_sfx(p->type))))
        gen_auto(p, 4);
}


/*
 *  recognizes target-specific options
 */
static void option(int *pc, char **pv[], void (*oerr)(const char *, ...))
{
    static opt_t tab[] = {
        NULL,
    };

    int c;
    const void *argptr;

    assert(oerr);

    if (!opt_reinit(tab, pc, pv, &argptr))
        oerr("failed to parse options\n");

    while ((c = opt_parse()) != -1) {
        switch(c) {
            /* common case labels follow */
            case 0:    /* flag variable set; do nothing else now */
                break;
            case '?':    /* unrecognized option */
            case '-':    /* no or invalid argument given for option */
            case '+':    /* argument given to option that takes none */
            case '*':    /* ambiguous option */
                oerr(opt_errmsg(c), (const char *)argptr);
                break;
            default:
                assert(!"not all options covered -- should never reach here");
                break;
        }
    }
}


/*
 *  initializes a program
 */
static void progbeg(FILE *outfp)
{
    int i;

    assert(outfp);

    fprog = 1;

    /* initializes register sets */
    intreg[EAX] = reg_new("%%eax", EAX, REG_SINT, EAX, -1);
    intreg[ECX] = reg_new("%%ecx", ECX, REG_SINT, ECX, -1);
    intreg[EDX] = reg_new("%%edx", EDX, REG_SINT, EDX, -1);
    intreg[EBX] = reg_new("%%ebx", EBX, REG_SINT, EBX, -1);
    intreg[ESI] = reg_new("%%esi", ESI, REG_SINT, ESI, -1);
    intreg[EDI] = reg_new("%%edi", EDI, REG_SINT, EDI, -1);
    shortreg[EAX] = reg_new("%%ax", EAX, REG_SINT, EAX, -1);
    shortreg[ECX] = reg_new("%%cx", ECX, REG_SINT, ECX, -1);
    shortreg[EDX] = reg_new("%%dx", EDX, REG_SINT, EDX, -1);
    shortreg[EBX] = reg_new("%%bx", EBX, REG_SINT, EBX, -1);
    shortreg[ESI] = reg_new("%%si", ESI, REG_SINT, ESI, -1);
    shortreg[EDI] = reg_new("%%di", EDI, REG_SINT, EDI, -1);
    charreg[EAX] = reg_new("%%al", EAX, REG_SINT, EAX, -1);
    charreg[ECX] = reg_new("%%cl", ECX, REG_SINT, ECX, -1);
    charreg[EDX] = reg_new("%%dl", EDX, REG_SINT, EDX, -1);
    charreg[EBX] = reg_new("%%bl", EBX, REG_SINT, EBX, -1);
    for (i = 0; i < NELEM(fltreg); i++)
        fltreg[i] = reg_new("%u", i, REG_SFP, -1);

    /* initializes wildcards */
    intregw = reg_wildcard(intreg);
    shortregw = reg_wildcard(shortreg);
    charregw = reg_wildcard(charreg);
    fltregw = reg_wildcard(fltreg);

    /* initializes masks */
    for (i = 0; i < REG_SMAX; i++) {
        reg_fmask[i] = reg_mask(strg_perm, -1);
        reg_umask[i] = reg_mask(strg_perm, -1);
    }
    reg_tmask[REG_SINT] = reg_mask(strg_perm, EAX, ECX, EDX, EBX, ESI, EDI, -1);
    reg_vmask[REG_SINT] = reg_mask(strg_perm, -1);
    reg_tmask[REG_SFP] = reg_mask(strg_perm, 0, 1, 2, 3, 4, 5, 6, 7, -1);
    reg_vmask[REG_SFP] = reg_mask(strg_perm, -1);

    /* initializes special registers */
    quo = reg_new("%%eax", EAX, REG_SINT, EAX, EDX, -1);
    rem = reg_new("%%edx", EDX, REG_SINT, EDX, EAX, -1);

    ir_cur->out = out = outfp;
}


/*
 *  finalizes a program
 */
static void progend(void)
{
    assert(fprog);

    init_swtoseg(INIT_SEGCODE);
    fputs(".ident \"beluga: 0.0.1\"\n", out);
}


/*
 *  defines a global
 */
static void defglobal(sym_t *p)
{
    assert(p);
    assert(p->x.name);
    assert(p->u.seg);
    assert(!p->x.f.imported);
    assert(p->sclass == LEX_STATIC || p->x.f.exported);

    fprintf(out, ".align %d\n", (p->type->align > 4)? 4: p->type->align);
    if (!GENSYM(p)) {
	#ifndef __CYGWIN__
        fprintf(out, ".type %s,@%s\n", p->x.name, (TY_ISFUNC(p->type))? "function": "object");
        if (p->type->size > 0)
            fprintf(out, ".size %s,%"FMTSZ"d\n", p->x.name, p->type->size);
	#endif
    }
    if (p->u.seg == INIT_SEGBSS)
        fprintf(out, ".%scomm %s,%"FMTSZ"d\n", (p->sclass == LEX_STATIC)? "l": "", p->x.name,
                p->type->size);
    else
        fprintf(out, "%s:\n", p->x.name);

}


/*
 *  completes a global array
 */
static void cmpglobal(sym_t *p)
{
    assert(p);
    assert(p->u.seg);
    assert(!p->x.f.imported);
    assert(p->sclass == LEX_STATIC || p->x.f.exported);
    assert(TY_ISARRAY(p->type));
    assert(p->type->size > 0);

    fprintf(out, ".size %s,%"FMTSZ"d\n", p->x.name, p->type->size);
}


/*
 *  provides an initializer for an address symbol
 */
static void initaddr(sym_t *p)
{
    assert(p);
    assert(p->x.name);

    fprintf(out, ".long %s\n", p->x.name);
}


/*
 *  provides a constant initializer;
 *  ASSUMPTION: ux_t can represent void * on the host;
 *  ASSUMPTION: fp types of the host are same as those of the target
 */
static void initconst(int op, sym_val_t v)
{
    switch(op_type(op)) {
        case OP_I:
            switch(op_size(op)) {
                case 1:
                    fprintf(out, ".byte %s\n", xtsd(v.s));
                    break;
                case 2:
                    fprintf(out, ".word %s\n", xtsd(v.s));
                    break;
                case 4:
                    fprintf(out, ".long %s\n", xtsd(v.s));
                    break;
                default:
                    assert(!"invalid scode -- should never reach here");
                    break;
            }
            break;
        case OP_U:
            assert(op_size(op) == 4);
            fprintf(out, ".long %s\n", xtud(v.u));
            break;
        case OP_P:
            assert(op_size(op) == 4);
            fprintf(out, ".long %s\n", xtud(v.p));
            break;
        case OP_F:
            {
                int i;
                unsigned char *p;
                int size = op_size(op);
                int swap = (LITTLE != ir_cur->f.little_endian);

                switch(size) {
                    case 4:
                        p = (unsigned char *)&v.f;
                        assert(sizeof(v.f) == size);
                        break;
                    case 8:
                        p = (unsigned char *)&v.d;
                        assert(sizeof(v.d) == size);
                        break;
                    case 12:
                        p = (unsigned char *)&v.ld;
                        assert(sizeof(v.ld) == size);
#ifndef NDEBUG
                        size = 10;
#endif    /* NDEBUG */
                        break;
                    default:
                        assert(!"invalid scode -- should never reach here");
                        break;
                }
                for (i = 0; i < size; i++)
                    fprintf(out, ".byte %d\n", (unsigned)p[(swap)? size-i: i]);
#ifndef NDEBUG
                if (i == 10)
                    fputs(".byte 0\n.byte 0\n", out);
#endif    /* NDEBUG */
            }
            break;
        default:
            assert(!"invalid type suffix -- should never reach here");
            break;
    }
}


/*
 *  provides a string initializer
 */
static void initstr(ssz_t n, const char *s)
{
    assert(n > 0);
    assert(s);

    while (n-- > 0)
        fprintf(out, ".byte %d\n", (unsigned)*(unsigned char *)s++);
}


/*
 *  provides a zero-padded initializer
 */
static void initspace(ssz_t n)
{
    assert(n > 0);

    if (init_curseg() != INIT_SEGBSS)
        fprintf(out, ".space %"FMTSZ"d\n", n);
}


/*
 *  exports a symbol
 */
static void export(sym_t *p)
{
    assert(p);
    assert(p->x.name);
    assert(!p->x.f.exported);

    fprintf(out, ".globl %s\n", p->x.name);
    p->x.f.exported = 1;
}


/*
 *  imports a symbol
 */
static void import(sym_t *p)
{
    assert(p);
#ifdef NDEBUG
    UNUSED(p);
#endif    /* NDEBUG */

    assert(p->x.name);
    assert(!p->x.f.imported);
    assert(!p->x.f.exported);
}


/*
 *  processes a function
 */
static void function(sym_t *f, void *caller[], void *callee[], int n)    /* sym_t */
{
    int i;
    ty_t *rty;

    assert(f);
    assert(caller);
    assert(callee);
    UNUSED(n);

    assert(!f->x.f.imported);
    assert(f->sclass == LEX_STATIC || f->x.f.exported);

    if (!finit)
        init();
    for (i = 0; i < REG_SMAX; i++) {
        reg_mclear(reg_umask[i]);
        reg_mfill(reg_fmask[i]);
    }

    assert(f->x.name);
    fputs(".align 16\n", out);
	#ifndef __CYGWIN__
		fprintf(out, ".type %s,@function\n", f->x.name);
	#endif
    fprintf(out, "%s:\n", f->x.name);
    fputs("pushl %ebp\n", out);
    fputs("pushl %ebx\n", out);
    fputs("pushl %esi\n", out);
    fputs("pushl %edi\n", out);
    fputs("movl %esp,%ebp\n", out);

    gen_off = 16 + 4;
    for (i = 0; callee[i]; i++) {
        sym_t *p = callee[i],
              *q = caller[i];
        assert(p && q);
        gen_off = ROUNDUP(gen_off, q->type->align);
        p->x.offset = q->x.offset = gen_off;
        p->x.name = q->x.name = hash_int(p->x.offset);
        p->sclass = q->sclass = LEX_AUTO;
        gen_off += ROUNDUP(q->type->size, 4);
    }
    assert(!caller[i]);

    gen_off = gen_maxoff = 0;
    dag_gencode(caller, callee);
    gen_frame = ROUNDUP(gen_maxoff, 4);
    if (gen_frame > 0)
        fprintf(out, "subl $%"FMTSZ"d,%%esp\n", gen_frame);
    dag_emitcode();

    fputs("movl %ebp,%esp\n", out);
    fputs("popl %edi\n", out);
    fputs("popl %esi\n", out);
    fputs("popl %ebx\n", out);
    fputs("popl %ebp\n", out);
    rty = ty_freturn(f->type);
    fprintf(out, "ret%s\n", (TY_ISSTRUNI(rty))? " $4": "");

    i = sym_genlab(1);
    fprintf(out, ".Lf%d:\n", i);
	#ifndef __CYGWIN__
		fprintf(out, ".size %s,.Lf%d-%s\n", f->x.name, i, f->x.name);
	#endif
}


/*
 *  changes the segment;
 *  should be invoked via init_swtoseg()
 */
static void segment(int seg)
{
    switch(seg) {
        case INIT_SEGCODE:
            fputs(".text\n", out);
            break;
        case INIT_SEGBSS:
            fputs(".bss\n", out);
            break;
        case INIT_SEGDATA:
        case INIT_SEGLIT:
            fputs(".data\n", out);
            break;
        default:
            assert(!"invalid segment -- should never reach here");
            break;
    }
}


/*
 *  returns the name of a non-terminal
 */
static const char *ntname(int nt)
{
    static const char *n[] = {
        NULL,
#define tt(t) #t,
#define rr(n, ops, c, cf, t)
#include "bx86l.r"
    };

    nt = cgr_ntidx(nt);
    assert(nt > 0 && nt < NELEM(n));

    return n[nt];
}


/*
 *  prepares rewriting;
 *  - calculates the argument offset for ARG
 */
static void prerewrite(dag_node_t *p)
{
    assert(p);

    switch(op_generic(p->op)) {
        case OP_ARG:
            gen_arg(xns(p->sym[0]->u.c.v.s), 4);
            break;
    }
}


/*
 *  targets a register;
 *  has to use reg_setnt() instead of reg_set()
 */
static void target(dag_node_t *p)
{
    assert(p);

    switch(op_optype(p->op)) {
        case OP_LSHI:
        case OP_LSHU:
        case OP_RSHI:
        case OP_RSHU:
            if (!((op_generic(p->kid[1]->op) == OP_CNST && range31(p->kid[1]) != CGR_CSTMAX) ||
                  (GEN_READCSE(p->kid[1]) && op_generic(GEN_CSE(p->kid[1])->op) == OP_CNST &&
                   range31(GEN_CSE(p->kid[1])) != CGR_CSTMAX)))
                reg_target(p, 1, intreg[ECX]);
            break;
        case OP_DIVI:
        case OP_DIVU:
            reg_setnt(p, quo);
            reg_target(p, 0, intreg[EAX]);
            reg_ptarget(p, 1, intreg[EDX]);
            break;
        case OP_MODI:
        case OP_MODU:
            reg_setnt(p, rem);
            reg_target(p, 0, intreg[EAX]);
            reg_ptarget(p, 1, intreg[EDX]);
            break;
        case OP_ASGNB:
            reg_target(p, 0, intreg[EDI]);
            reg_target(p->kid[1], 0, intreg[ESI]);
            break;
        case OP_ARGB:
            reg_target(p->kid[0], 0, intreg[ESI]);
            break;
        case OP_CVFI:
            reg_pmset(p, intreg[EDX]);
            break;
        case OP_CALLI:
        case OP_CALLV:
            reg_setnt(p, intreg[EAX]);
            break;
        case OP_RETI:
            reg_target(p, 0, intreg[EAX]);
            break;
    }
}


/*
 *  checks if floating-point registers spill
 */
static int chkstck(const dag_node_t *p, int n)
{
    int i;
#ifndef NDEBUG
    int f = 0;
#endif    /* !NDEBUG */

    assert(p);

    FORXKIDS(p, 0) {
        if (op_type(p->x.kid[i]->op) == OP_F)
#ifndef NDEBUG
            f = 1,
#endif    /* !NDEBUG */
            n--;
    }

    if (op_type(p->op) == OP_F && !p->x.f.listed &&
#ifndef NDEBUG
        (f=1) &&
#endif    /* !NDEBUG */
        ++n > 8)
        err_dpos(lmap_pin(clx_cpos), ERR_X86_FPREGSPILL);
    DEBUG((void)(f && fprintf(stderr, " - chkstck: %d\n", n)));

    assert(n >= 0);
    return n;
}


/*
 *  clobbers registers
 */
static void clobber(dag_node_t *p)
{
    static int nstck;

    assert(p);

    nstck = chkstck(p, nstck);
    switch(op_optype(p->op)) {
        case OP_ASGNB:
        case OP_ARGB:
            reg_spill(reg_mask(strg_func, ECX, ESI, EDI, -1), REG_SINT, p);
            break;
        case OP_EQF:
        case OP_LEF:
        case OP_GEF:
        case OP_LTF:
        case OP_GTF:
        case OP_NEF:
            reg_spill(intreg[EAX]->x.regnode->bv, REG_SINT, p);
            break;
        case OP_CVFI:
            reg_spill(intreg[EDX]->x.regnode->bv, REG_SINT, p);
            break;
        case OP_CALLF:
            reg_spill(reg_mask(strg_func, EDX, EAX, ECX, -1), REG_SINT, p);
            break;
        case OP_CALLI:
        case OP_CALLV:
            reg_spill(reg_mask(strg_func, EDX, ECX, -1), REG_SINT, p);
            break;
    }
}


/*
 *  returns the register number allocated to a dag node
 */
static int getnum(const dag_node_t *p)
{
    assert(p);
    assert(p->sym[REG_RX] && p->sym[REG_RX]->x.regnode);

    return p->sym[REG_RX]->x.regnode->num;
}


#define preg(f) ((f)[getnum(p->x.kid[0])]->x.name)

/*
 *  target-specific emitter
 */
static void emit(dag_node_t *p)
{
    assert(p);

    switch(op_optype(p->op)) {
        case OP_CVII:
        case OP_CVUI:
        case OP_CVIU:
            {
                static sym_t *(*r[])[MAX] = { NULL, &charreg, &shortreg, NULL, NULL };
                static const char sfx[] =   { '?',  'b',      'w',       '?',  '?' };

                int c = op_cvsize(p->op);

                if (c < op_size(p->op)) {    /* extend */
                    assert(c > 0 && c < NELEM(r) && c < NELEM(sfx) && r[c] && sfx[c] != '?');
                    fprintf(out, "mov%c%cl %s,%s\n", (OP_ISSINT(p->op))? 's': 'z', sfx[c],
                            preg(*r[c]), p->sym[REG_RX]->x.name);
                    break;
                } else {    /* truncate */
                    const char *src, *dst;
                    assert(c > op_size(p->op));
                    src = preg(intreg);
                    dst = intreg[getnum(p)]->x.name;
                    if (src != dst)
                        fprintf(out, "movl %s,%s\n", src, dst);
                }
            }
            break;
        case OP_ARGB:
            fprintf(out, "subl $%s,%%esp\n"
                         "movl %%esp,%%edi\n"
                         "movl $%s,%%ecx\n"
                         "rep\n"
                         "movsb\n",
                     xtsd(XROUNDUP(p->sym[0]->u.c.v.s, 4)), xtsd(p->sym[0]->u.c.v.s));
            break;
    }
}


/* IR interface for null binding */
ir_t ir_bx86l = {
     1, 1, 0,     /* charmetric */
     2, 2, 0,     /* shortmetric */
     4, 4, 0,     /* intmetric */
     4, 4, 0,     /* longmetric */
#ifdef SUPPORT_LL
     8, 4, 1,     /* llongmetric */
#endif    /* SUPPORT_LL */
     4, 4, 1,     /* floatmetric */
     8, 4, 1,     /* doublemetric */
    12, 4, 1,     /* ldoublemetric */
     4, 4, 0,     /* ptrmetric */
     0, 1, 0,     /* structmetric */
    {
        1,    /* little_endian */
        1,    /* little_bit */
        0,    /* want_callb */
        1,    /* want_argb */
        0,    /* left_to_right */
        0,    /* want_dag */
    },
    NULL,          /* out; set by progbeg() */
    symaddr,
    symgsc,
    symlocal,
    option,
    progbeg,
    progend,
    gen_blkbeg,
    gen_blkend,
    defglobal,
    cmpglobal,
    initaddr,
    initconst,
    initstr,
    initspace,
    export,
    import,
    function,
    gen_emit,
    gen_code,
    segment,
    {
        '%',                  /* fmt */
        MAX,                  /* nreg */
        cgr_ntidx(P(max)),    /* nnt */
        rule,
        ntname,
        rmapw,
        rmaps,
        prerewrite,
        target,
        clobber,
        emit
    }
};

/* end of bx86l.c */

/*
 *  riscv32-target
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

#define P(x) briscv32_##x         /* prefix to avoid name conflict */
#define S(p) ((sym_t *)(p))    /* shorthand for cast to sym_t * */

/* token pasting */
#define xpaste(p, n) paste(p, n)
#define paste(p, n)  p ## n

#define MAX 32
#define	N_LSYM	0x80		/* local sym: name,,0,type,offset */ 
#define	N_RSYM	0x40		/* register sym: name,,0,type,register */ 
#define	N_FUN	0x24		/* procedure: name,,0,linenumber,address */ 
#define	N_GSYM	0x20		/* global symbol: name,,0,type,0 */ 
#define	N_PSYM	0xa0		/* parameter: name,,0,type,offset */ 
#define	N_LCSYM	0x28		/* .lcomm symbol: name,,0,type,address */ 
#define	N_STSYM	0x26		/* static symbol: name,,0,type,address */ 

/* non-terminals;
   ASSUMPTION: int is at least 32-bit wide on the host (see op.h) */
enum {
    P(_) = OP_2+1,    /* +1 as non-terminals start at 1 */
#define tt(t) P(t),
#define rr(n, ops, c, cf, t)
#include "briscv32.r"
    P(max)
};



static int fprog;         /* true when progbeg() invoked */
static int finit;         /* true when init() invoked */
static FILE *out;         /* output file */
static FILE *curfile;
static int endian = 1;    /* for LITTLE from common.h */
static int ntypes; 
static ty_t *missing[16];
static int indmissing; 

static cgr_t rule[
#define tt(t)
#define rr(n, ops, c, cf, t) +1
#include "briscv32.r"
+1];    /* for end marker */

static sym_t *intreg[32],      /* register set for 4-byte integers */             
             *fltreg[32];      /* register set for floating-points */
static sym_t *intregw,          /* wildcard for intreg */
             *fltregw;          /* wildcard for fltreg */
static sym_t *quo, *rem;        /* registers for div/rem */



/*
 *  cost function: rangeu5
 */
static int rangeu5(dag_node_t *p)
{
    assert(p);
    return (xlu(p->sym[0]->u.c.v.u, xiu(32)))? 0: CGR_CSTMAX;
}

/*
 *  cost function: ranges12
 */
static int ranges12(dag_node_t *p)
{
    assert(p);
    return (xges(p->sym[0]->u.c.v.s, xis(-2048)) && xles(p->sym[0]->u.c.v.s, xis(2047)))? 0: CGR_CSTMAX;
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
 *  initializes the back-end
 */
static void init(void)
{
    int i = 0;

#define tt(t)
#define rr(n, ops, c, cf, t) static const int xpaste(rule, __LINE__)[] = { ops, -1 };
#include "briscv32.r"

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
#include "briscv32.r"
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
 *  sets x of a global/static/constant symbol;
 *  ASSUMPTION: ux_t can represent void * on the host
 */
static void symgsc(sym_t *p)
{
    assert(p);
    assert(ty_inttype);

    if (p->x.name)
        return;

    if (p->scope == SYM_SCONST) {    /* must precede check for GENSYM() */
        if (TY_ISUNSIGN(p->type) && xgs(p->u.c.v.u, TG_INT_MAX))
            p->x.name = gen_sfmt(1 + STRG_BUFN + 1, "0x%s", xtux(p->u.c.v.u));
        else if (TY_ISPTR(p->type))
            p->x.name = gen_sfmt(1 + STRG_BUFN + 1, "0x%s", xtux(p->u.c.v.p));
        else
            p->x.name = p->name;
    } else if (GENSYM(p))
        p->x.name = gen_sfmt(1 + STRG_BUFN, "L%s", p->name);
    else if (p->scope >= SYM_SLOCAL && p->sclass == LEX_STATIC)    /* static local */
        p->x.name = gen_sfmt(1 + STRG_BUFN, "L%d", sym_genlab(1));
    else if (p->sclass == LEX_EXTERN || (p->scope == SYM_SGLOBAL && p->sclass == LEX_AUTO))
        p->x.name = gen_sfmt(1 + strlen(p->name), "%s", p->name);
    else {
        assert(p->scope == SYM_SGLOBAL && p->sclass == LEX_STATIC);
        p->x.name = p->name;
    }
}


/*
 *  returns a wildcard for an operation
 */
static sym_t *rmapw(int op)
{
    switch(op_type(op)) {
        case OP_F:     
        case OP_I:
        case OP_U:
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
        gen_auto(p, 0);
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
	for (i = 0; i < 32; i++)
        intreg[i] = reg_new("x%u", i, REG_SINT, i, -1);
    
    for (i = 0; i < 32; i++)
        fltreg[i] = reg_new("x%u", i, REG_SFP, -1);

    /* initializes wildcards */
    intregw = reg_wildcard(intreg);    
    fltregw = reg_wildcard(fltreg);

    /* initializes masks */
    for (i = 0; i < REG_SMAX; i++) {
        reg_fmask[i] = reg_mask(strg_perm, -1);
        reg_umask[i] = reg_mask(strg_perm, -1);
    }
    reg_tmask[REG_SINT] = reg_mask(strg_perm, 5, 6, 7, 28, 29, 30, -1);
    reg_vmask[REG_SINT] = reg_mask(strg_perm, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1);
    reg_tmask[REG_SFP] = reg_mask(strg_perm, 4, 5, 6, 7, 8, 9 ,10, 11, 16, 17, 18, 19, -1);
    reg_vmask[REG_SFP] = reg_mask(strg_perm, 24, 25, 26, 27, 28, 29, 30, 31, -1);
    
    ir_cur->out = out = outfp;    
}


/*
 *  finalizes a program
 */
static void progend(void)
{
    assert(fprog);

    init_swtoseg(0);    
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
    fprintf(out, "%s:\n", p->x.name);    
}


/*
 *  completes a global array
 */
static void cmpglobal(sym_t *p)
{
    assert(p);
#ifdef NDEBUG
    UNUSED(p);
#endif    /* NDEBUG */

    assert(p->u.seg);
    assert(!p->x.f.imported);
    assert(p->sclass == LEX_STATIC || p->x.f.exported);
    assert(TY_ISARRAY(p->type));
    assert(p->type->size > 0);
}


/*
 *  provides an initializer for an address symbol
 */
static void initaddr(sym_t *p)
{
    assert(p);
    assert(p->x.name);

    fprintf(out, "dd %s\n", p->x.name);
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
                    fprintf(out, ".byte 0x%s\n", xtsd(v.s));
                    break;
                case 2:
                    fprintf(out, ".half 0x%s\n", xtsd(v.s));
                    break;
                case 4:
                    fprintf(out, ".word 0x%s\n", xtsd(v.s));
                    break;
                default:
                    assert(!"invalid scode -- should never reach here");
                    break;
            }
            break;
        case OP_U:
            assert(op_size(op) == 4);
            fprintf(out, ".word 0x%s\n", xtux(v.u));
            break;
        case OP_P:
            assert(op_size(op) == 4);
            fprintf(out, ".word 0x%s\n", xtux(v.p));
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
    int seg;

    assert(p);
    assert(p->x.name);
    assert(!p->x.f.imported);
    assert(!p->x.f.exported);

    if (p->ref > 0) {
        seg = init_swtoseg(0);        
        init_swtoseg(seg);
        p->x.f.imported = 1;
    }
}




static int bitcount(reg_mask_t *mask) {
  unsigned i, n;

  n = 0;
  for (i = 0; i < 32; i++) {
    if (*reg_umask[REG_SINT] & 1<<i) {
      n++;
    }
  }
  return n;
} 

#define maxregargnr 6

static sym_t *argreg(int argno) {
  if (argno < maxregargnr) {
	return intreg[12 + argno];    
  }
  return NULL;
}
 

/*
 *  processes a function
 */
static void function(sym_t *f, void *caller[], void *callee[], int n)    /* sym_t */
{
    int i, j, saved, sizeisave, ofs; 
	sym_t *r;
	
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
	fputs(".align 4\n", out);
    fprintf(out, "%s:\n", f->x.name);
    
    gen_off = 0;
    for (i = 0; callee[i]; i++) {
        sym_t *p = callee[i],
              *q = caller[i];
        assert(p && q);	
        p->x.offset = q->x.offset = gen_off;
        p->x.name = q->x.name = hash_int(p->x.offset);		
        gen_off += ROUNDUP(q->type->size, 4);
		r = argreg(i);      
		if(r!=NULL && !p->f.structarg && !p->f.addressed && !ty_variadic(f->type) && (f->u.f.ncall==0) ) { 
			p->sclass = q->sclass = LEX_REGISTER;			
			reg_askvar(p, r);
			assert(p->x.regnode && p->x.regnode->vbl == p);	
			q->type = p->type;
			q->x = p->x;
			
		} else {
			if(reg_askvar(p, intregw) && r!=NULL) {
				p->sclass = q->sclass = LEX_REGISTER;
				q->type = p->type;					
			}
		}
		
    }
    assert(!caller[i]);
     
	fputs("sw x1, -4(x2)\n", out);
	fputs("sw x8, -8(x2)\n", out);
	fputs("mv x8, x2\n", out);
    
	gen_off = gen_maxoff = 8;
	gen_aoff = gen_maxaoff = 0;	
    dag_gencode(caller, callee);
	
	if (f->u.f.ncall!=0 && gen_maxaoff < 24) gen_maxaoff = 24;
    gen_maxaoff = ROUNDUP(gen_maxaoff, 4);
	sizeisave = 4 * bitcount(reg_umask[REG_SINT]); 	
    gen_frame = ROUNDUP(gen_maxoff + gen_maxaoff + sizeisave + 8, 16);
    	
	if (gen_frame > 0)
        fprintf(out, "addi x2, x2, -%"FMTSZ"d\n", gen_frame);
	saved =  gen_maxaoff;
	for (i = 0; i < 32; i++) {
    if (*reg_umask[REG_SINT] & *reg_vmask[REG_SINT] & 1<<i) {
		fprintf(out, "sw x%d, %d(x2)\n", i, saved);
		saved += 4;
		}  
    }
	for (i = 0; callee[i] && i < maxregargnr; i++) {
		sym_t * p = callee[i];
		r = argreg(i);
		if(r!=NULL && r->x.regnode != p->x.regnode) {			
			if(p->sclass == LEX_REGISTER) {
				fprintf(out, "mv x%d, x%d \n", p->x.regnode->num, r->x.regnode->num);
			} else {			
				fprintf(out, "sw x%d, %d(x8) \n", i + 12, p->x.offset);
			}
		}
	}
	if(ty_variadic(f->type) && callee[i-1]!=NULL) {
		for(j = i; j < maxregargnr; j++)
			fprintf(out, "sw x%d, %d(x8) \n", j + 12, j*4);
	}
		
	dag_emitcode();
	
	saved =  gen_maxaoff;
	for (i = 0; i < 32; i++) {
		if (*reg_umask[REG_SINT] & *reg_vmask[REG_SINT] & 1<<i) {
			fprintf(out, "lw x%d, %d(x2)\n", i, saved);
			saved += 4;
		} 
	}    
	fputs("mv x2, x8\n", out);
	fputs("lw x8, -8(x2)\n", out);
	fputs("lw x1, -4(x2)\n", out);
    fputs("jalr x0, x1, 0\n", out);
    for(i=0;r=argreg(i);i++) {
		if(r->x.regnode) r->x.regnode->vbl = NULL;		
	}	
}


/*
 *  changes the segment;
 *  should be invoked via init_swtoseg()
 */
static void segment(int seg)
{
    int c = init_curseg();
    
    if(seg==c) return;
    /* open */
	fputs(".align 4\n", out);
    switch(seg) {
        case 0:
            break;
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
#include "briscv32.r"
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
	static int argno; 
    assert(p);

    switch(op_generic(p->op)) {		
        case OP_ARG:
			if(gen_aoff == 0) argno = 0;
			p->x.argno = argno++;
		    p->sym[1] = sym_findint(gen_arg(xns(p->sym[0]->u.c.v.s), 4));            
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
    sym_t *q;
	
    switch(op_optype(p->op)) {
        case OP_LSHI:
        case OP_LSHU:
        case OP_RSHI:
        case OP_RSHU:
            //if (!((op_generic(p->kid[1]->op) == OP_CNST && ranges12(p->kid[1]) != CGR_CSTMAX) ||
             //     (GEN_READCSE(p->kid[1]) && op_generic(GEN_CSE(p->kid[1])->op) == OP_CNST &&
             //      ranges12(GEN_CSE(p->kid[1])) != CGR_CSTMAX)))
                //reg_target(p, 1, intreg[ECX]);
            break;
        case OP_DIVI:
        case OP_DIVU:
            //reg_setnt(p, quo);
            //reg_target(p, 0, intreg[EAX]);
            //reg_ptarget(p, 1, intreg[EDX]);
            break;
        case OP_MODI:
        case OP_MODU:
            //reg_setnt(p, rem);
            //reg_target(p, 0, intreg[EAX]);
            //reg_ptarget(p, 1, intreg[EDX]);
            break;
        case OP_ASGNB:
            reg_target(p, 0, intreg[6]);
            reg_target(p->kid[1], 0, intreg[5]);
            break;
		case OP_ARGI:
		case OP_ARGP:
			q = argreg(p->x.argno);
			if(q) reg_target(p, 0, q);
            break;
        case OP_CALLI:
        case OP_CALLV:
            reg_setnt(p, intreg[10]);
            break;
        case OP_RETI:
            reg_target(p, 0, intreg[10]);
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
            reg_spill(reg_mask(strg_perm, 5, 6, 7, 11, -1), REG_SINT, p);
            break; 
		case OP_CALLI:
		case OP_CALLV:        
            reg_spill(reg_tmask[REG_SINT], REG_SINT, p);
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
    int src = 0, l;
    sym_t *s;	
	switch(op_optype(p->op)) {
		case OP_ARGI:
		    s = argreg(p->x.argno);
		    if(s == NULL) {				
				src = getnum(p->x.kid[0]);		
				fprintf(out, "sw x%d, %d(x2)\n", src, p->sym[1]->u.c.v.s);
			}
			break;
		case OP_ASGNB:		
				fprintf(out, "li x7, %d\n", p->sym[0]->u.c.v.s);				
				fprintf(out, "L%d:\n", l=sym_genlab(1));
				fputs("lb x11, 0(x5)\n", out);
				fputs("sb x11, 0(x6)\n", out);
				fputs("addi x5, x5, 1\n", out);
				fputs("addi x6, x6, 1\n", out);
				fputs("addi x7, x7, -1\n", out);	
				fprintf(out, "bgtu x7, x0, L%d\n", l);
		    break;
    }
}

static void stabline(const lmap_t *cp) {            
	static int curfileno = 1;
	static char *curfile = 0;
	const lmap_t *f; 
	cp = lmap_mstrip(cp); 
	f = lmap_nfrom(cp);
	if(curfile!=f->u.i.f) {
		fprintf(out, ".file %d \"%s\"\n",curfileno, f->u.i.f);	
		curfileno++;
		curfile = f->u.i.f;
	}
	fprintf(out, ".loc 1 %d\n", cp->u.n.py + f->u.i.yoff);	
}

/* emittype - emit ty's type number, emitting its definition if necessary. */
static void emittype(ty_t *ty) {
	int tc = ty->x.typeno;
    unsigned tmp;
	if (TY_ISCONST(ty) || TY_ISVOLATILE(ty)) {
		emittype(ty->type);
		ty->x.typeno = ty->type->x.typeno;
		ty->x.printed = 1;
		return;
	}
	if (tc == 0) {
		ty->x.typeno = tc = ++ntypes;
	}
	fprintf(out, "%d", tc);
	if (ty->x.printed)
		return;
	ty->x.printed = 1;
	switch (ty->op) {
	case TY_VOID:	/* void is defined as itself */
		fprintf(out, "=r1");
		break;
	case TY_INT:
		fprintf(out, "=r1;%D;%D;", ty->u.sym->u.lim.min, ty->u.sym->u.lim.max);		
		break;
	case TY_UNSIGNED:		
		fprintf(out, "=r1;0;%U;", ty->u.sym->u.lim.max);		
		break;
	case TY_DOUBLE:
	case TY_LDOUBLE:
	case TY_FLOAT:	/* float, double, long double get sizes, not ranges */
		fprintf(out, "=r1;%d;0;", ty->size);
		break;
	case TY_POINTER:    
		fputs("=*", out);
        tmp = ty->type->x.printed;
        if(!tmp) {                    
            ty->type->x.printed = 1;
            missing[++indmissing] = ty->type;
        }
		emittype(ty->type);
        ty->type->x.printed = tmp; 
		break;
	case TY_FUNCTION:
		fputs("=f", out);
		emittype(ty->type);
		break;
	case TY_ARRAY:	/* array includes subscript as an int range */
		fputs("=a", out);
        tmp = ty->type->x.printed;
        if(!tmp) {                    
            ty->type->x.printed = 1;
            missing[++indmissing] = ty->type;
        }		
        emittype(ty->type);
        ty->type->x.printed = tmp; 
		fprintf(out,";0;%d;", ty->size/ty->type->size - 1);        
		break;
	case TY_STRUCT: case TY_UNION: {
        unsigned tmp;
        sym_field_t *p, *q = ty->u.sym->u.s.flist;
		fprintf(out, "=%c%d", ty->op == TY_STRUCT ? 's' : 'u', ty->size);
        fputs(";", out);
        for (p=q; p; p = p->link) {
			if (p->name)
				fprintf(out, "%s,", p->name);
			else
				fputs(",", out);
            tmp = p->type->x.printed;
            if(!tmp) {                    
                p->type->x.printed = 1;
                missing[++indmissing] = p->type;
            }             
            emittype(p->type);
            p->type->x.printed = tmp;                        
            if (p->lsb)
				fprintf(out, ",%d;", 8*p->offset + SYM_FLDRIGHT(p));				
			else fprintf(out, ",%d;", 8*p->offset);
		}
        break;
		}		
	case TY_ENUM: {
		alist_t *p = (alist_t *)ty->u.sym->u.idlist, *r;		
		size_t n;
		fputs("=e", out);
		ALIST_FOREACH(n, r, p) {
			fprintf(out,"%s:%d,", ((sym_t *)r->data)->name, ((sym_t *)r->data)->u.value);			
		}
		fputs(";", out);
		break;
		}
	default:
		fprintf(out, "=r1;0;0;");
	}
	return;
} 



/* dbxout - output .stabs entry for type ty */
static void dbxout(ty_t* ty) {
	if (!ty->x.printed) {		
		fputs(".stabs \"", out);
		if (ty->u.sym && !(TY_ISFUNC(ty) || TY_ISARRAY(ty) || TY_ISPTR(ty)))
			fprintf(out, "%s", ty->u.sym->name);
		fprintf(out, ":%c", TY_ISSTRUCT(ty) || TY_ISENUM(ty) ? 'T' : 't');
		emittype(ty);
		fprintf(out, "\",%d,0,0,0\n", N_LSYM);
	}
} 

/* dbxtype - emit a stabs entry for type ty, return type code */  
static int dbxtype(ty_t* ty) {
	indmissing = 0; 
    dbxout(ty);
    while(indmissing) dbxout(missing[indmissing--]); 
	return ty->x.typeno;
}


/* stabsym - output a stab entry for symbol p */
void stabsym(sym_t* p) {
	int code, tc, sz = p->type->size;

	if (p->f.computed)
		return;
	if (TY_ISFUNC(p->type)) {
		fprintf(out, ".stabs \"%s:%c%d\",%d,0,0,%s\n", p->name,
			p->sclass == LEX_STATIC ? 'f' : 'F', dbxtype(ty_freturn(p->type)),
			N_FUN, p->x.name);
		return;
	}
	if (p->scope == SYM_SPARAM && p->f.structarg) {
		assert(TY_ISPTR(p->type) && TY_ISSTRUCT(p->type->type));
		tc = dbxtype(p->type->type);
		sz = p->type->type->size;
	} else
		tc = dbxtype(p->type); 
	if (p->sclass == LEX_AUTO && p->scope == SYM_SGLOBAL || p->sclass == LEX_EXTERN) {
		fprintf(out, ".stabs \"%s:G", p->name);
		code = N_GSYM;
	} else if (p->sclass == LEX_STATIC) {
		fprintf(out, ".stabs \"%s:%c%d\",%d,0,0,%s\n", p->name, p->scope == SYM_SGLOBAL ? 'S' : 'V',
			tc, p->u.seg == INIT_SEGBSS ? N_LCSYM : N_STSYM, p->x.name);
		return;
	} else if (p->sclass == LEX_REGISTER) {
		if (p->x.regnode) {
			int r = p->x.regnode->num;
			if (p->x.regnode->set == REG_SFP)
				r += 32;	/* floating point */
				fprintf(out, ".stabs \"%s:%c%d\",%d,0,", p->name,
					p->scope == SYM_SPARAM ? 'P' : 'r', tc, N_RSYM);
			fprintf(out, "%d,%d\n", sz, r);
		}
		return;
	} else if (p->scope == SYM_SPARAM) {
		fprintf(out, ".stabs \"%s:p", p->name);
		code = N_PSYM;
	} else if (p->scope >= SYM_SLOCAL) {
		fprintf(out, ".stabs \"%s:", p->name);
		code = N_LSYM;
	} else
		assert(0);
	fprintf(out, "%d\",%d,0,0,%s\n", tc, code,
		p->scope >= SYM_SPARAM && p->sclass != LEX_EXTERN ? p->x.name : "0");
}

static void stabtype(sym_t *s, void *cl) {
	 //if(!TY_ISFUNC(s->type)) fprintf(out, "; function type:%s,sclass:%d\n", s->name,s->sclass);
	 // 	else fprintf(out, "; type:%s,sclass:%d\n", s->name,s->sclass);
	 if(s->sclass == LEX_TYPEDEF) {
		fprintf(out, ".stabs \"%s:=d%d\",%d,0,0,0\n", s->name, dbxtype(s->type), N_LSYM);
		return;
	 }
	 if(s->sclass==0) dbxtype(s->type);
 }

/* IR interface for null binding */
ir_t ir_riscv32 = {
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
     0, 4, 0,     /* structmetric */
    {
        1,    /* little_endian */
        1,    /* little_bit */
        0,    /* want_callb */
        0,    /* want_argb */
        1,    /* left_to_right */
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
	stabline,
	stabtype,
	stabsym,
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

/* end of riscv32.c */

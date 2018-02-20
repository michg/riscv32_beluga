/*
 *  common definitions
 */

#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>     /* isdigit */
#include <limits.h>    /* CHAR_BIT */
#ifdef SUPPORT_LL
#ifdef LLONG_MAX
#include <limits.h>    /* ULLONG_MAX, LLONG_MAX, LLONG_MIN */
#include <stdlib.h>    /* lldiv */
#else    /* !LLONG_MAX */
#include <cdsl/dwa.h>
#endif    /* LLONG_MAX */
#else    /* !SUPPORT_LL */
#include <limits.h>    /* ULONG_MAX, LONG_MAX, LONG_MIN */
#include <stdlib.h>    /* ldiv */
#endif    /* SUPPORT_LL */
#ifdef HAVE_ICONV
#include <errno.h>         /* errno, E2BIG */
#include <stddef.h>        /* size_t, NULL */
#include <cbl/memory.h>    /* MEM_RESIZE */
#include <iconv.h>         /* iconv */
#endif    /* HAVE_ICONV */


/*
 *  type definitions and wrappers;
 *  avoids size_t to confirm to C90 and above;
 *  ASSUMPTION: the host has types to contain pp arithmetic types for the target;
 *  ASSUMPTION: ux_t can represent pointers both on the host and the target (see sym_val_t);
 *  ASSUMPTION: 2sC for signed integers assumed
 */

#ifdef SUPPORT_LL
#ifdef LLONG_MAX
    typedef long long sx_t;             /* largest signed integer on the host */
    typedef unsigned long long ux_t;    /* largest unsigned integer on the host */

    #define xinit() ((void)0)

    #define xiu(v) ((ux_t)(v))
    #define xis(v) ((sx_t)(v))
    #define xnu(x) ((ux_t)(x))
    #define xns(x) ((sx_t)(x))

    #define xw   (sizeof(ux_t)*CHAR_BIT)
    #define xmxu ULLONG_MAX
    #define xmxs LLONG_MAX
    #define xmns LLONG_MIN
    #define xO   0LL
    #define xI   1LL
    #define x_I  (-1LL)

    #define xtud(x) (sprintf(strg_nbuf, "%llu", (x)), strg_nbuf)
    #define xtsd(x) (sprintf(strg_nbuf, "%lld", (x)), strg_nbuf)
    #define xtux(x) (sprintf(strg_nbuf, "%llx", (x)), strg_nbuf)

    #define xn(x)     (-(x))
    #define xinc(x)   ((x)++)
    #define xdec(x)   ((x)--)
    #define xau(x, y) ((x) + (y))
    #define xas(x, y) ((x) + (y))
    #define xsu(x, y) ((x) - (y))
    #define xss(x, y) ((x) - (y))
    #define xmu(x, y) ((x) * (y))
    #define xms(x, y) ((x) * (y))
    #define xdu(x, y) ((x) / (y))
    #define xds(x, y) ((x) / (y))
    #define xvs(x, y) (lldiv((x), (y)).quot)
    #define xru(x, y) ((x) % (y))
    #define xrs(x, y) ((x) % (y))

    #define xsl(x, n)  ((x) << (n))
    #define xsrl(x, n) ((ux_t)(x) >> (n))
    #define xsra(x, n) ((sx_t)(((x) < 0 && (n) > 0)?    \
                                   (xsrl((x), (n)) | ((ux_t)-1 << (xw-(n)))): xsrl((x), (n))))
    #define xbc(x)     (~(x))
    #define xba(x, y)  ((x) & (y))
    #define xbx(x, y)  ((x) ^ (y))
    #define xbo(x, y)  ((x) | (y))

    #define xe(x, y)   ((x) == (y))
    #define xne(x, y)  ((x) != (y))
    #define xf(x)      ((x) == 0)
    #define xt(x)      ((x) != 0)
    #define xlu(x, y)  ((x) < (y))
    #define xleu(x, y) ((x) <= (y))
    #define xgeu(x, y) ((x) >= (y))
    #define xgu(x, y)  ((x) > (y))
    #define xls(x, y)  ((x) < (y))
    #define xles(x, y) ((x) <= (y))
    #define xges(x, y) ((x) >= (y))
    #define xgs(x, y)  ((x) > (y))

    #define xctu(x) ((ux_t)(x))
    #define xcts(x) ((sx_t)(x))
    #define xcfp(p) ((ux_t)(p))
    #define xctp(x) ((void *)(x))
    #define xcfu(v) ((ux_t)(v))
    #define xcfs(v) ((sx_t)(v))
    #define xcuf(x) ((long double)(ux_t)(x))
    #define xcsf(x) ((long double)(sx_t)(x))
#else    /* !LLONG_MAX */
    typedef dwa_t sx_t;            /* largest signed integer on the host */
    typedef dwa_t ux_t;            /* largest unsigned integer on the host */

    #define xinit() (dwa_prep())

    #define xis(v) (dwa_fromint(v))
    #define xiu(v) (dwa_fromuint(v))
    #define xns(x) (dwa_toint(x))
    #define xnu(x) (dwa_touint(x))

    #define xw   DWA_WIDTH
    #define xmxu dwa_umax
    #define xmxs dwa_max
    #define xmns dwa_min
    #define xO   dwa_0
    #define xI   dwa_1
    #define x_I  dwa_neg1

    #define xtud(x) (dwa_tostru(strg_nbuf, (x), 10))
    #define xtsd(x) (dwa_tostr(strg_nbuf, (x), 10))
    #define xtux(x) (dwa_tostru(strg_nbuf, (x), 16))

    #define xn(x)     (dwa_neg(x))
    #define xinc(x)   ((x) = dwa_addu((x), xI))
    #define xdec(x)   ((x) = dwa_subu((x), xI))
    #define xau(x, y) (dwa_addu((x), (y)))
    #define xas(x, y) (dwa_add((x), (y)))
    #define xsu(x, y) (dwa_subu((x), (y)))
    #define xss(x, y) (dwa_sub((x), (y)))
    #define xmu(x, y) (dwa_mulu((x), (y)))
    #define xms(x, y) (dwa_mul((x), (y)))
    #define xdu(x, y) (dwa_divu((x), (y), 0))
    #define xds(x, y) (dwa_div((x), (y), 0))
    #define xvs(x, y) (dwa_div((x), (y), 0))
    #define xru(x, y) (dwa_divu((x), (y), 1))
    #define xrs(x, y) (dwa_div((x), (y), 1))

    #define xsl(x, n)  (dwa_lsh((x), (n)))
    #define xsrl(x, n) (dwa_rshl((x), (n)))
    #define xsra(x, n) (dwa_rsha((x), (n)))
    #define xbc(x)     (dwa_bcom(x))
    #define xba(x, y)  (dwa_bit((x), (y), DWA_AND))
    #define xbx(x, y)  (dwa_bit((x), (y), DWA_XOR))
    #define xbo(x, y)  (dwa_bit((x), (y), DWA_OR))

    #define xe(x, y)   (dwa_cmpu((x), (y)) == 0)
    #define xne(x, y)  (dwa_cmpu((x), (y)) != 0)
    #define xf(x)      (dwa_cmpu((x), xO) == 0)
    #define xt(x)      (dwa_cmpu((x), xO) != 0)
    #define xlu(x, y)  (dwa_cmpu((x), (y)) < 0)
    #define xleu(x, y) (dwa_cmpu((x), (y)) <= 0)
    #define xgeu(x, y) (dwa_cmpu((x), (y)) >= 0)
    #define xgu(x, y)  (dwa_cmpu((x), (y)) > 0)
    #define xls(x, y)  (dwa_cmp((x), (y)) < 0)
    #define xles(x, y) (dwa_cmp((x), (y)) <= 0)
    #define xges(x, y) (dwa_cmp((x), (y)) >= 0)
    #define xgs(x, y)  (dwa_cmp((x), (y)) > 0)

    #define xctu(x) x
    #define xcts(x) x
    #define xcfp(p) (dwa_fromuint((dwa_ubase_t)p))
    #define xctp(x) ((void *)dwa_touint(x))
    #define xcfu(v) (dwa_fromfp(v))
    #define xcfs(v) (dwa_fromfp(v))
    #define xcuf(x) (dwa_tofpu(x))
    #define xcsf(x) (dwa_tofp(x))
#endif    /* LLONG_MAX */
#else    /* !SUPPORT_LL */
    typedef long sx_t;             /* largest signed integer on the host */
    typedef unsigned long ux_t;    /* largest unsigned integer on the host */

    #define xinit() ((void)0)

    #define xiu(v) ((ux_t)(v))
    #define xis(v) ((sx_t)(v))
    #define xnu(x) ((ux_t)(x))
    #define xns(x) ((sx_t)(x))

    #define xw   (sizeof(ux_t)*CHAR_BIT)
    #define xmxu ULONG_MAX
    #define xmxs LONG_MAX
    #define xmns LONG_MIN
    #define xO   0L
    #define xI   1L
    #define x_I  (-1L)

    #define xtud(x) (sprintf(strg_nbuf, "%lu", (x)), strg_nbuf)
    #define xtsd(x) (sprintf(strg_nbuf, "%ld", (x)), strg_nbuf)
    #define xtux(x) (sprintf(strg_nbuf, "%lx", (x)), strg_nbuf)

    #define xn(x)     (-(x))
    #define xinc(x)   ((x)++)
    #define xdec(x)   ((x)--)
    #define xau(x, y) ((x) + (y))
    #define xas(x, y) ((x) + (y))
    #define xsu(x, y) ((x) - (y))
    #define xss(x, y) ((x) - (y))
    #define xmu(x, y) ((x) * (y))
    #define xms(x, y) ((x) * (y))
    #define xdu(x, y) ((x) / (y))
    #define xds(x, y) ((x) / (y))
    #define xvs(x, y) (ldiv((x), (y)).quot)
    #define xru(x, y) ((x) % (y))
    #define xrs(x, y) ((x) % (y))

    #define xsl(x, n)  ((x) << (n))
    #define xsrl(x, n) ((ux_t)(x) >> (n))
    #define xsra(x, n) ((sx_t)(((x) < 0 && (n) > 0)?    \
                                   (xsrl((x), (n)) | ((ux_t)-1 << (xw-(n)))): xsrl((x), (n))))
    #define xbc(x)     (~(x))
    #define xba(x, y)  ((x) & (y))
    #define xbx(x, y)  ((x) ^ (y))
    #define xbo(x, y)  ((x) | (y))

    #define xe(x, y)   ((x) == (y))
    #define xne(x, y)  ((x) != (y))
    #define xf(x)      ((x) == 0)
    #define xt(x)      ((x) != 0)
    #define xlu(x, y)  ((x) < (y))
    #define xleu(x, y) ((x) <= (y))
    #define xgeu(x, y) ((x) >= (y))
    #define xgu(x, y)  ((x) > (y))
    #define xls(x, y)  ((x) < (y))
    #define xles(x, y) ((x) <= (y))
    #define xges(x, y) ((x) >= (y))
    #define xgs(x, y)  ((x) > (y))

    #define xctu(x) ((ux_t)(x))
    #define xcts(x) ((sx_t)(x))
    #define xcfp(p) ((ux_t)(p))
    #define xctp(x) ((void *)(x))
    #define xcfu(v) ((ux_t)(v))
    #define xcfs(v) ((sx_t)(v))
    #define xcuf(x) ((long double)(ux_t)(x))
    #define xcsf(x) ((long double)(sx_t)(x))
#endif    /* SUPPORT_LL */

typedef unsigned long sz_t;    /* represents sizes; unsigned */
typedef long ssz_t;            /* signed counterpart of sz_t */

#define FMTSZ "l"    /* size modifier for sz_t */

#define SZ_MAX  ((sz_t)-1)                /* largest value of sz_t */
#define SSZ_MAX ((ssz_t)(SZ_MAX >> 1))    /* largest value of ssz_t */


#include "main.h"    /* ssz_t is used in main.h */
#include "strg.h"    /* sz_t and ux_t are used in strg.h */


/*
 *  common macros
 */

/* generates hash key from pointer */
#define hashkey(p, n) (((unsigned)(p) >> 3) & ((n)-1))

/* suppresses warnings for unused identifiers */
#define UNUSED(id) ((void)(id))

/* # of elements in array */
#define NELEM(a) ((sz_t)(sizeof(a)/sizeof(*(a))))

/* rounds up x to closest power of 2 */
#define ROUNDUP(x, n)  (((x)+((n)-1)) & (~((n)-1)))
#define XROUNDUP(x, n) (xba(xas((x), xis((n)-1)), xbc(xis((n)-1))))

#define BUFUNIT 128    /* buffer resize unit */

/* constructs n 1s;
   ASSUMPTION: sx_t on the host can represent all signed integers on the target;
   ASSUMPTION: ux_t on the host can represent all unsigned integers on the target */
#define ONES(n) (((n) == 0)? xO: (xsrl(xctu(x_I), xw - (n))))

/* checks if name/symbol is generated */
#define GENNAME(name) (isdigit(*(unsigned char *)(name)))
#define GENSYM(sym)   GENNAME((sym)->name)

/* checks if c is first byte of UTF-8;
   ASSUMPTION: UTF-8 used as default and (HAVE_ICONV) internal pivot encoding */
#define FIRSTUTF8(c) (*(unsigned char *)&(c) >> 6 != 0x02)

#ifdef HAVE_ICONV
/* declares variables for iconv */
#define ICONV_DECL(ibufv_i, ilenv_i)    \
    char *ibufv = ibufv_i;              \
    size_t ilenv = ilenv_i;             \
    char *obufv, *obuf;                 \
    size_t olenv, olen

/* performs iconv conversions */
#define ICONV_DO(cd, init, handle)                                          \
    if (init)                                                               \
        iconv(*(cd), NULL, NULL, NULL, NULL);                               \
    errno = 0;                                                              \
    while (iconv(*(cd), &ibufv, &ilenv, &obufv, &olenv) == (size_t)-1) {    \
        if (errno == E2BIG) {                                               \
            MEM_RESIZE(obuf, olen+BUFUNIT);                                 \
            obufv = obuf + (olen - olenv);                                  \
            olen += BUFUNIT;                                                \
            olenv += BUFUNIT;                                               \
        } else {                                                            \
            handle                                                          \
            break;                                                          \
        }                                                                   \
    }

/* inserts initial sequence */
#define ICONV_INIT(cd)                                                  \
    errno = 0;                                                          \
    while (iconv(*(cd), NULL, NULL, &obufv, &olenv) == (size_t)-1) {    \
        if (errno == E2BIG) {                                           \
            MEM_RESIZE(obuf, olen+BUFUNIT);                             \
            obufv = obuf + (olen - olenv);                              \
            olen += BUFUNIT;                                            \
            olenv += BUFUNIT;                                           \
        } else                                                          \
            break;                                                      \
    }
#endif    /* HAVE_ICONV */

/* character categories */
#define ISCH_I(c)  (main_ch[(unsigned char)(c)] & 0x01)    /* isalnum  || _ */
#define ISCH_IP(c) (main_ch[(unsigned char)(c)] & 0x02)    /* isalnum  || _ || . */
#define ISCH_SP(c) (main_ch[(unsigned char)(c)] & 0x04)    /* isspace but \n */

/* skip spaces;
   need to #include lex.h */
#define SKIPSP(t) while ((t)->id == LEX_SPACE) (t) = lst_nexti()
#define NEXTSP(t) do { (t) = lst_nexti(); } while((t)->id == LEX_SPACE)

/* skips to newline;
   need to #include lex.h */
#define SKIPNL(t) while ((t)->id != LEX_NEWLINE && (t)->id != LEX_EOI) (t) = lst_nexti()

/* true if the host uses little endian;
   need to declare endian as static int and initialize to 1;
   ASSUMPTION: the host uses either little or big endian */
#define LITTLE (((unsigned char *)(&endian))[0] == 1)

/* changes endianness */
#define CHGENDIAN(x, s)                                                       \
    do {                                                                      \
        int i;                                                                \
        unsigned char t;                                                      \
        for (i = 0; i < (s) / 2; i++)                                         \
            t = ((unsigned char *)&(x))[i],                                   \
            ((unsigned char *)&(x))[i] = ((unsigned char *)&(x))[(s)-1-i],    \
            ((unsigned char *)&(x))[(s)-1-i] = t;                             \
    } while(0)

/* iterates on x.kid's of dag node */
#define FORXKIDS(p, s) for (i = (s); i < NELEM((p)->x.kid) && (p)->x.kid[i]; i++)

/* debugging wrapper for back-end */
#ifdef NDEBUG
#define DEBUG(x) ((void)0)
#else    /* !DEBUG */
#define DEBUG(x) ((void)(main_opt()->_debug && ((x), 0)))
#endif    /* DEBUG */


/*
 *  translation limits
 */

/* nesting levels of blocks */
#define TL_BLOCK_STD (main_tl()->block)
#define TL_BLOCK_C90 15                    /* C90 5.2.4.1 */
#define TL_BLOCK_C99 127                   /* C99 5.2.4.1 */

/* pointer, array and function modifications in declarations */
#define TL_DECL_STD (main_tl()->decl)
#define TL_DECL_C90 12                   /* C90 5.2.4.1 */
#define TL_DECL_C99 12                   /* C99 5.2.4.1 */

/* nesting levels of parenthesized declarators */
#define TL_PAREND_STD (main_tl()->parend)
#define TL_PAREND_C90 31                     /* C90 5.2.4.1 */
#define TL_PAREND_C99 63                     /* C99 5.2.4.1 */

/* significant initial characters in external names */
#define TL_ENAME_STD (main_tl()->ename)
#define TL_ENAME_C90 6     /* C90 5.2.4.1 */
#define TL_ENAME_C99 63    /* C99 5.2.4.1 */

/* external identifiers in translation unit */
#define TL_NAME_STD (main_tl()->name)
#define TL_NAME_C90 511                  /* C90 5.2.4.1 */
#define TL_NAME_C99 4095                 /* C99 5.2.4.1 */

/* identifiers with block scope in block */
#define TL_NAMEB_STD (main_tl()->nameb)
#define TL_NAMEB_C90 127                   /* C90 5.2.4.1 */
#define TL_NAMEB_C99 511                   /* C99 5.2.4.1 */

/* parameters in function definition */
#define TL_PARAM_STD (main_tl()->param)
#define TL_PARAM_C90 31                    /* C90 5.2.4.1 */
#define TL_PARAM_C99 127                   /* C99 5.2.4.1 */

/* arguments in function call */
#define TL_ARG_STD (main_tl()->arg)
#define TL_ARG_C90 31                  /* C90 5.2.4.1 */
#define TL_ARG_C99 127                 /* C99 5.2.4.1 */

/* length of string literal */
#define TL_STR     4095
#define TL_STR_STD (main_tl()->str)
#define TL_STR_C90 509                 /* C90 5.2.4.1 */
#define TL_STR_C99 4095                /* C99 5.2.4.1 */

/* bytes in object (hosted only) */
#define TL_OBJ_STD (main_tl()->obj)
#define TL_OBJ_C90 32767UL             /* C90 5.2.4.1 */
#define TL_OBJ_C99 65535UL             /* C99 5.2.4.1 */

/* case labels in switch (excluding nested ones) */
#define TL_NCASE_STD (main_tl()->ncase)
#define TL_NCASE_C90 257                   /* C90 5.2.4.1 */
#define TL_NCASE_C99 1023                  /* C99 5.2.4.1 */

/* members in single structure or union */
#define TL_MBR_STD (main_tl()->mbr)
#define TL_MBR_C90 127                 /* C90 5.2.4.1 */
#define TL_MBR_C99 1023                /* C99 5.2.4.1 */

/* enumeration constants in enumeration */
#define TL_ENUMC_STD (main_tl()->enumc)
#define TL_ENUMC_C90 127                   /* C90 5.2.4.1 */
#define TL_ENUMC_C99 1023                  /* C99 5.2.4.1 */

/* nesting levels of structure or union definitions */
#define TL_STRCT_STD (main_tl()->strct)
#define TL_STRCT_C90 15                    /* C90 5.2.4.1 */
#define TL_STRCT_C99 63                    /* C99 5.2.4.1 */

/* significant initial characters in internal or (pp) macro names */
#define TL_INAME_STD (main_tl()->iname)
#define TL_INAME_C90 31                    /* C90 5.2.4.1 */
#define TL_INAME_C99 63                    /* C99 5.2.4.1 */

/* nesting levels of parenthesized expressions */
#define TL_PARENE_STD (main_tl()->parene)
#define TL_PARENE_C90 32                     /* C90 5.2.4.1 */
#define TL_PARENE_C99 63                     /* C99 5.2.4.1 */

/* length of logical source line */
#define TL_LINE     4095
#define TL_LINE_STD (main_tl()->line)
#define TL_LINE_C90 509                  /* C90 5.2.4.1 */
#define TL_LINE_C99 4095                 /* C99 5.2.4.1 */

/* line number */
#define TL_LINENO_STD (main_tl()->lineno)
#define TL_LINENO_C90 ((sz_t)-1)             /* unspecified */
#define TL_LINENO_C99 2147483647UL           /* C99 6.10.4 */

/* (pp) nesting levels for #included files */
#define TL_INC     256
#define TL_INC_STD (main_tl()->inc)
#define TL_INC_C90 8                   /* C90 5.2.4.1 */
#define TL_INC_C99 15                  /* C99 5.2.4.1 */

/* (pp) nesting levels of conditional inclusions */
#define TL_COND_STD (main_tl()->cond)
#define TL_COND_C90 8                    /* C90 5.2.4.1 */
#define TL_COND_C99 63                   /* C99 5.2.4.1 */

/* (pp) macros simultaneously defined in pp-translation unit */
#define TL_PPNAME_STD (main_tl()->ppname)
#define TL_PPNAME_C90 1024                   /* C90 5.2.4.1 */
#define TL_PPNAME_C99 4095                   /* C99 5.2.4.1 */

/* (pp) parameters in macro definition */
#define TL_PARAMP_STD (main_tl()->paramp)
#define TL_PARAMP_C90 31                     /* C90 5.2.4.1 */
#define TL_PARAMP_C99 127                    /* C99 5.2.4.1 */

/* (pp) arguments in macro invocation */
#define TL_ARGP_STD (main_tl()->argp)
#define TL_ARGP_C90 31                   /* C90 5.2.4.1 */
#define TL_ARGP_C99 127                  /* C99 5.2.4.1 */

/* (pp) TODO: tbd */
#define TL_VER_STD (main_tl()->ver)
#define TL_VER_C90 "199409L"
#define TL_VER_C99 "199901L"


/*
 *  target parameters;
 *  need to #include ty.h
 */

/* bits in a byte;
   best effort to distinguish the host's byte from the target's
   but would not work when TG_CHAR_BIT != CHAR_BIT */
#ifndef TG_CHAR_BIT
#define TG_CHAR_BIT CHAR_BIT
#endif    /* TG_CHAR_BIT */

/* smallest/largest values of signed char */
#define TG_SCHAR_MIN (ty_schartype->u.sym->u.lim.min.s)
#define TG_SCHAR_MAX (ty_schartype->u.sym->u.lim.max.s)

/* largest value of unsigned char */
#define TG_UCHAR_MAX (ty_uchartype->u.sym->u.lim.max.u)

/* smallest/largest values of short */
#define TG_SHRT_MIN (ty_shorttype->u.sym->u.lim.min.s)
#define TG_SHRT_MAX (ty_shorttype->u.sym->u.lim.max.s)

/* largest value of unsigned short */
#define TG_USHRT_MAX (ty_ushorttype->u.sym->u.lim.max.u)

/* smallest/largest values of int */
#define TG_INT_MIN (ty_inttype->u.sym->u.lim.min.s)
#define TG_INT_MAX (ty_inttype->u.sym->u.lim.max.s)

/* largest value of unsigned int */
#define TG_UINT_MAX (ty_unsignedtype->u.sym->u.lim.max.u)

/* smallest/largest values of long */
#define TG_LONG_MIN (ty_longtype->u.sym->u.lim.min.s)
#define TG_LONG_MAX (ty_longtype->u.sym->u.lim.max.s)

/* largest value of unsigned long */
#define TG_ULONG_MAX (ty_ulongtype->u.sym->u.lim.max.u)

#ifdef SUPPORT_LL
/* smallest/largest values of long long */
#define TG_LLONG_MIN (ty_llongtype->u.sym->u.lim.min.s)
#define TG_LLONG_MAX (ty_llongtype->u.sym->u.lim.max.s)

/* largest value of unsigned long long */
#define TG_ULLONG_MAX (ty_ullongtype->u.sym->u.lim.max.u)
#endif    /* SUPPORT_LL */

/* largest values of wchar_t, unsigned wchar_t and wint_t */
#define TG_WUCHAR_MAX (ty_wuchartype->u.sym->u.lim.max.u)
#define TG_WCHAR_MIN  (ty_wchartype->u.sym->u.lim.min.s)
#define TG_WCHAR_MAX  (ty_wchartype->u.sym->u.lim.max.u)
#define TG_WINT_MIN   (ty_winttype->u.sym->u.lim.min.s)
#define TG_WINT_MAX   (ty_winttype->u.sym->u.lim.max.s)

/* smallest/largest values of float */
#define TG_FLT_MIN (ty_floattype->u.sym->u.lim.min.f)
#define TG_FLT_MAX (ty_floattype->u.sym->u.lim.max.f)

/* smallest/largest values of double */
#define TG_DBL_MIN (ty_doubletype->u.sym->u.lim.min.d)
#define TG_DBL_MAX (ty_doubletype->u.sym->u.lim.max.d)

/* smallest/largest values of long double */
#define TG_LDBL_MIN (ty_ldoubletype->u.sym->u.lim.min.ld)
#define TG_LDBL_MAX (ty_ldoubletype->u.sym->u.lim.max.ld)

/* NaNs */
#define TG_FLT_NAN  (ty_floattype->u.sym->u.lim.max.f)
#define TG_DBL_NAN  (ty_doubletype->u.sym->u.lim.max.d)
#define TG_LDBL_NAN (ty_ldoubletype->u.sym->u.lim.max.ld)


#endif    /* COMMON_H */

/* end of common.h */

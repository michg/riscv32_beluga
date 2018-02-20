/*
 *  operation codes
 */

#include <limits.h>        /* CHAR_BIT */
#include <cbl/assert.h>    /* assert */
#ifndef NDEBUG
#include <stdio.h>         /* sprintf */
#include <cdsl/hash.h>     /* hash_int */
#endif    /* !NDEBUG */

#include "ty.h"
#include "op.h"
#ifndef NDEBUG
#include "common.h"
#endif    /* !NDEBUG */


/*
 *  converts a type to an op suffix;
 *  ASSUMPTION: function pointers are compatible with object pointers
 */
int (op_sfx)(const ty_t *ty)
{
    int op;

    assert(ty);

    switch(ty->op) {
        case TY_UNKNOWN:
            return 0;
        case TY_CONST:
        case TY_VOLATILE:
        case TY_CONSVOL:
        case TY_ENUM:
            return op_sfx(ty->type);
        case TY_VOID:
            return OP_V;
        case TY_CHAR:
        case TY_SHORT:
        case TY_INT:
        case TY_LONG:
#ifdef SUPPORT_LL
        case TY_LLONG:
#endif    /* SUPPORT_LL */
            op = OP_I;
            break;
        case TY_UNSIGNED:
        case TY_ULONG:
#ifdef SUPPORT_LL
        case TY_ULLONG:
#endif    /* SUPPORT_LL */
            op = OP_U;
            break;
        case TY_FLOAT:
        case TY_DOUBLE:
        case TY_LDOUBLE:
            op = OP_F;
            break;
        case TY_POINTER:
        case TY_FUNCTION:
            op = OP_P;
            break;
        case TY_ARRAY:
        case TY_STRUCT:
        case TY_UNION:
            return OP_B;
        default:
            assert(!"invalid type operator -- should never reach here");
            break;
    }

    return op + ty->size;
}


/*
 *  op_sfxs() with converting unsigned types to their signed conterparts
 */
int (op_sfxs)(const ty_t *ty)
{
    assert(ty);

    switch(ty->op) {
        case TY_CONST:
        case TY_VOLATILE:
        case TY_CONSVOL:
        case TY_ENUM:
            return op_sfxs(ty->type);
        case TY_UNSIGNED:
        case TY_ULONG:
#ifdef SUPPORT__LL
        case TY_ULLONG:
#endif    /* SUPPORT_LL */
            return OP_I + ty->size;
        default:
            return op_sfx(ty);
    }

    /* assert(!"impossible control flow -- should never reach here");
       return 0; */
}


/*
 *  converts an op code with a suffix to a type;
 *  op_tyscode(op) == op_sfx(op_stot(op)) guaranteed;
 *  ASSUMPTION: function pointers are compatible with object pointers
 */
ty_t *(op_stot)(int op)
{
    int size;

    assert(ty_doubletype);    /* ensures types initialized */

    size = op_size(op);
    switch(op_type(op)) {
        case OP_F:
            if (size == ty_doubletype->size)
                return ty_doubletype;
            else if (size == ty_floattype->size)
                return ty_floattype;
            assert(size == ty_ldoubletype->size);
            return ty_ldoubletype;
        case OP_I:
            if (size == ty_inttype->size)
                return ty_inttype;
            else if (size == ty_longtype->size)
                return ty_longtype;
#ifdef SUPPORT_LL
            else if (size == ty_ullongtype->size)
                return ty_ullongtype;
#endif    /* SUPPORT_LL */
            else if (size == ty_chartype->size)
                return ty_chartype;
            assert(size == ty_shorttype->size);
            return ty_shorttype;
        case OP_U:
            if (size == ty_unsignedtype->size)
                return ty_unsignedtype;
#ifdef SUPPORT_LL
            else if (size == ty_ulongtype->size)
                return ty_ulongtype;
            assert(size == ty_ullongtype->size);
            return ty_ullongtype;
#else    /* !SUPPORT_LL */
            assert(size == ty_ulongtype->size);
            return ty_ulongtype;
#endif    /* SUPPORT_LL */
        case OP_P:
            return ty_voidptype;
#if 0    /* op_stot() never called with V and B */
        case OP_V:
        case OP_B:
            assert(!"unsupported type suffix -- should never reach here");
            break;
#endif    /* disabled */
        default:
            assert(!"invalid type suffix -- should never reach here");
            break;
    }

    return ty_inttype;
}


#ifndef NDEBUG
/*
 *  returns the name of an operation
 */
const char *(op_name)(int op)
{
    static const char *name[] = {    /* see op.h for the order of initializers */
        "",
        "CNST",
        "ARG",
        "ASGN",
        "INDIR",
        "CVF",
        "CVI",
        "CVU",
        "CVP",
        "NEG",
        "CALL",
        "LOAD",     /* not appeared in tree */
        "RET",      /* statement-like, but appeared in tree */
        "ADDRG",
        "ADDRF",
        "ADDRL",
        "ADD",
        "SUB",
        "LSH",
        "MOD",
        "RSH",
        "BAND",
        "BCOM",
        "BOR",
        "BXOR",
        "DIV",
        "MUL",
        "EQ",
        "GE",
        "GT",
        "LE",
        "LT",
        "NE",
        /* statement-like, but appeared in tree */
        "JMP",
        "LABEL",
        /* with no type suffix */
        "AND",
        "NOT",
        "OR",
        "COND",
        "RIGHT",
        "FIELD",
        /* only for diagnostics */
        "POS",
        "INCR",
        "DECR",
        "SUBS",
        "CADD",
        "CSUB",
        "CLSH",
        "CMOD",
        "CRSH",
        "CBAND",
        "CBOR",
        "CBXOR",
        "CDIV",
        "CMUL",
        "VREG"      /* not appeared in tree */
    }, suffix[] = " FdxcsIeUlm  PVB",
       code[] = "0123456789abcdefg";

    static char buf[sizeof(op)*CHAR_BIT+2 + 1];

    const char *s;

    if (op_index(op) > 0 && op_index(op) < NELEM(name))
        s = name[op_index(op)];
    else
        s = hash_int(op_index(op));
    if (op >= OP_AND && op <= OP_FIELD)    /* with no type suffix */
        return s;
    else {
        assert(op_size(op) < NELEM(code));
        if (OP_ISCV(op)) {
            assert(op_cvsize(op) < NELEM(code));
            sprintf(buf, "%s%c%c%c", s, suffix[op_type(op) >> OP_STY], code[op_cvsize(op)],
                    code[op_size(op)]);
        } else if (op_type(op) > 0 && (op_type(op) >> OP_STY) < sizeof(suffix)-1)
            sprintf(buf, ((op_size(op) == 0)? "%s%c": "%s%c%c"),
                    s, suffix[op_type(op) >> OP_STY], code[op_size(op)]);
        else
            sprintf(buf, "%s+%d+%d", s, op_type(op), op_scode(op));
    }

    return buf;
}
#endif    /* !NDEBUG */

/* end of op.c */

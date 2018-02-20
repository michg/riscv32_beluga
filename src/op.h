/*
 *  operation codes
 */

#ifndef OP_H
#define OP_H

#include "ty.h"


/* helper shift offset */
#define OP_SOP 20
#define OP_STY 16
#define OP_SCV 8
#define OP_SSZ 0

/* ASSUMPTION: long double is 8, 10, 12 or 16-byte wide on the target */
#define a 10
#define c 12
#define g 16

#define xx(o, t, s)    OP_##o##t##s    = OP_##o + OP_##t                 + (s << OP_SSZ)
#define yy(o, t, s, d) OP_##o##t##s##d = OP_##o + OP_##t + (s << OP_SCV) + (d << OP_SSZ)
#define zz(o, t)       OP_##o##t       = OP_##o + OP_##t


/* type suffixes;
   ASSUMPTION: values of type operators are below 16 (see ty.h and xtoken.h) */
enum {
    OP_F = TY_FLOAT    << OP_STY,    /* floating-point types */
    OP_I = TY_INT      << OP_STY,    /* signed integer types */
    OP_U = TY_UNSIGNED << OP_STY,    /* unsigned integer types */
    OP_P = TY_POINTER  << OP_STY,
    OP_V = TY_VOID     << OP_STY,
    OP_B = TY_STRUCT   << OP_STY
};

/* op codes;
   ASSUMPTION: int is at least 32-bit wide on the host */
enum {
    /* constant; F48acg I124(8) U4(8) P4 */
    OP_CNST = 1 << OP_SOP,
    zz(CNST, F),
    xx(CNST, F, 4),
    xx(CNST, F, 8),
    xx(CNST, F, a),
    xx(CNST, F, c),
    xx(CNST, F, g),
    zz(CNST, I),
    xx(CNST, I, 1),
    xx(CNST, I, 2),
    xx(CNST, I, 4),
#ifdef SUPPORT_LL
    xx(CNST, I, 8),
#endif    /* SUPPORT_LL */
    zz(CNST, U),
    xx(CNST, U, 4),
#ifdef SUPPORT_LL
    xx(CNST, U, 8),
#endif    /* SUPPORT_LL */
    xx(CNST, P, 4),
    /* V: no constant of void type exists */
    /* B: no constant of array/struct types exists */

    /* argument; F48acg I4(8) P4 B
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_ARG = OP_CNST + (1 << OP_SOP),
    zz(ARG, F),
    xx(ARG, F, 4),
    xx(ARG, F, 8),
    xx(ARG, F, a),
    xx(ARG, F, c),
    xx(ARG, F, g),
    zz(ARG, I),
    xx(ARG, I, 4),
#ifdef SUPPORT_LL
    xx(ARG, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    zz(ARG, P),
    xx(ARG, P, 4),
    /* V: no argument of void type exists */
    zz(ARG, B),

    /* assignment; F48acg I124(8) P4 B
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_ASGN = OP_ARG + (1 << OP_SOP),
    zz(ASGN, F),
    xx(ASGN, F, 4),
    xx(ASGN, F, 8),
    xx(ASGN, F, a),
    xx(ASGN, F, c),
    xx(ASGN, F, g),
    zz(ASGN, I),
    xx(ASGN, I, 1),
    xx(ASGN, I, 2),
    xx(ASGN, I, 4),
#ifdef SUPPORT_LL
    xx(ASGN, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    zz(ASGN, P),
    xx(ASGN, P, 4),
    /* V: no value/object of void type exists */
    zz(ASGN, B),

    /* indirection; F48acg I124(8) P4 V B
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_INDIR = OP_ASGN + (1 << OP_SOP),
    zz(INDIR, F),
    xx(INDIR, F, 4),
    xx(INDIR, F, 8),
    xx(INDIR, F, a),
    xx(INDIR, F, c),
    xx(INDIR, F, g),
    zz(INDIR, I),
    xx(INDIR, I, 1),
    xx(INDIR, I, 2),
    xx(INDIR, I, 4),
#ifdef SUPPORT_LL
    xx(INDIR, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    zz(INDIR, P),
    xx(INDIR, P, 4),
    zz(INDIR, V),
    zz(INDIR, B),

    /* supported conversions for 32-bit word:
     *
     *                   I1 I2  I1 I2
     *   F4 -             \ /    \ /
     *       = F8|a|c|g - I4 --- U4 - P4
     *   F8 -      \      /      /
     *              --- (I8) - (U8)
     *
     * one of F8|a|c|g is used depending on the target;
     * F8-Fa|c|g is not used when long double is F8
     */

    /* conversion from float/double(F4|8) to ldouble(F8|a|c|g) */
    OP_CVF = OP_INDIR + (1 << OP_SOP),
    zz(CVF, F),
    yy(CVF, F, 4, 8),
    yy(CVF, F, 4, a),
    yy(CVF, F, 4, c),
    yy(CVF, F, 4, g),
    yy(CVF, F, 8, a),
    yy(CVF, F, 8, c),
    yy(CVF, F, 8, g),

    /* conversion from ldouble(F8|a|c|g) to float/double(F4|8) */
    yy(CVF, F, 8, 4),
    yy(CVF, F, a, 4),
    yy(CVF, F, c, 4),
    yy(CVF, F, g, 4),
    yy(CVF, F, a, 8),
    yy(CVF, F, c, 8),
    yy(CVF, F, g, 8),

    /* conversion from ldouble(F8|a|c|g) to int/long(I4)/llong(I8) */
    zz(CVF, I),
    yy(CVF, I, 8, 4),
    yy(CVF, I, a, 4),
    yy(CVF, I, c, 4),
    yy(CVF, I, g, 4),
#ifdef SUPPORT_LL
    yy(CVF, I, 8, 8),
    yy(CVF, I, a, 8),
    yy(CVF, I, c, 8),
    yy(CVF, I, g, 8),
#endif    /* SUPPORT_LL */

    /* conversion from int/long(I4)/llong(I8) to ldouble(F8|a|c|g) */
    OP_CVI = OP_CVF + (1 << OP_SOP),
    zz(CVI, F),
    yy(CVI, F, 4, 8),
    yy(CVI, F, 4, a),
    yy(CVI, F, 4, c),
    yy(CVI, F, 4, g),
#ifdef SUPPORT_LL
    yy(CVI, F, 8, 8),
    yy(CVI, F, 8, a),
    yy(CVI, F, 8, c),
    yy(CVI, F, 8, g),
#endif    /* SUPPORT_LL */

    /* conversion from char(I1)/short(I2|4) to int/long(I4) */
    zz(CVI, I),
    yy(CVI, I, 1, 4),
    yy(CVI, I, 2, 4),

    /* conversion from int/long(I4) to char(I1)/short(I2|4) */
    yy(CVI, I, 4, 1),
    yy(CVI, I, 4, 2),

#ifdef SUPPORT_LL
    /* conversion between int/long(I4) and llong(I8) */
    yy(CVI, I, 4, 8),
    yy(CVI, I, 8, 4),
#endif    /* SUPPORT_LL */

    /* conversion from uchar(I1)/ushort(I2|4) to uint/ulong(U4) */
    zz(CVI, U),
    yy(CVI, U, 1, 4),
    yy(CVI, U, 2, 4),

    /* conversion from int/long(I4) to uint/ulong(U4) */
    yy(CVI, U, 4, 4),

#ifdef SUPPORT_LL
    /* conversion from llong(I8) to ullong(U8) */
    yy(CVI, U, 8, 8),
#endif    /* SUPPORT_LL */

    /* conversion from uint/ulong(U4) to uchar(I1)/ushort(I2|4) */
    OP_CVU = OP_CVI + (1 << OP_SOP),
    zz(CVU, I),
    yy(CVU, I, 4, 1),
    yy(CVU, I, 4, 2),

    /* conversion from uint/ulong(U4) to int/long(I4) */
    yy(CVU, I, 4, 4),

#ifdef SUPPORT_LL
    /* conversion from ullong(U8) to llong(I8) */
    yy(CVU, I, 8, 8),

    /* conversion between uint/ulong(U4) and ullong(U8) */
    zz(CVU, U),
    yy(CVU, U, 4, 8),
    yy(CVU, U, 8, 4),
#endif    /* SUPPORT_LL */

    /* conversion from uint/ulong(U4) to pointer(P4) */
    zz(CVU, P),
    yy(CVU, P, 4, 4),

    /* conversion from pointer(P4) to uint/ulong(U4) */
    OP_CVP = OP_CVU + (1 << OP_SOP),
    zz(CVP, U),
    yy(CVP, U, 4, 4),

    /* negation; F48acg I4(8) */
    OP_NEG = OP_CVP + (1 << OP_SOP),
    zz(NEG, F),
    xx(NEG, F, 4),
    xx(NEG, F, 8),
    xx(NEG, F, a),
    xx(NEG, F, c),
    xx(NEG, F, g),
    zz(NEG, I),
    xx(NEG, I, 4),
#ifdef SUPPORT_LL
    xx(NEG, I, 8),
#endif    /* SUPPORT_LL */
    /* U: negation of unsigned implemented by front-end */
    /* P: negation not applicable to pointers */
    /* V: no value of void type exists */
    /* B: negation not applicable to array/structs */

    /* function call; F48acg I4(8) B V;
       ASSUMPTION: signed/unsigned integers are compatible on the target;
       ASSUMPTION: pointers can be returned as an integer */
    OP_CALL = OP_NEG + (1 << OP_SOP),
    zz(CALL, F),
    xx(CALL, F, 4),
    xx(CALL, F, 8),
    xx(CALL, F, a),
    xx(CALL, F, c),
    xx(CALL, F, g),
    zz(CALL, I),
    xx(CALL, I, 4),
#ifdef SUPPORT_LL
    xx(CALL, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    /* P: I used instead */
    zz(CALL, B),
    zz(CALL, V),

    /* load; F48acg I124(8) U4(8) P4 B */
    OP_LOAD = OP_CALL + (1 << OP_SOP),
    zz(LOAD, F),
    xx(LOAD, F, 4),
    xx(LOAD, F, 8),
    xx(LOAD, F, a),
    xx(LOAD, F, c),
    xx(LOAD, F, g),
    zz(LOAD, I),
    xx(LOAD, I, 1),
    xx(LOAD, I, 2),
    xx(LOAD, I, 4),
#ifdef SUPPORT_LL
    xx(LOAD, I, 8),
#endif    /* SUPPORT_LL */
    zz(LOAD, U),
    xx(LOAD, U, 4),
#ifdef SUPPORT_LL
    xx(LOAD, U, 8),
#endif    /* SUPPORT_LL */
    zz(LOAD, P),
    xx(LOAD, P, 4),
    /* V: no value of void type exists */
    zz(LOAD, B),

    /* return; F48acg I4(8)
       ASSUMPTION: signed/unsigned integers are compatible on the target;
       ASSUMPTION: pointers can be returned as an integer */
    OP_RET = OP_LOAD + (1 << OP_SOP),
    zz(RET, F),
    xx(RET, F, 4),
    xx(RET, F, 8),
    xx(RET, F, a),
    xx(RET, F, c),
    xx(RET, F, g),
    zz(RET, I),
    xx(RET, I, 4),
#ifdef SUPPORT_LL
    xx(RET, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    /* P: I used instead */
    /* V: no value of void type exists */
    /* B: returning structs implemented by front-end */

    /* address (global); P4 */
    OP_ADDRG = OP_RET + (1 << OP_SOP),
    zz(ADDRG, P),
    xx(ADDRG, P, 4),

    /* address (parameter); P4 */
    OP_ADDRF = OP_ADDRG + (1 << OP_SOP),
    zz(ADDRF, P),
    xx(ADDRF, P, 4),

    /* address (local); P4 */
    OP_ADDRL = OP_ADDRF + (1 << OP_SOP),
    zz(ADDRL, P),
    xx(ADDRL, P, 4),

    /* addition; F48acg I4(8) U4(8) P4 */
    OP_ADD = OP_ADDRL + (1 << OP_SOP),
    zz(ADD, F),
    xx(ADD, F, 4),
    xx(ADD, F, 8),
    xx(ADD, F, a),
    xx(ADD, F, c),
    xx(ADD, F, g),
    zz(ADD, I),
    xx(ADD, I, 4),
#ifdef SUPPORT_LL
    xx(ADD, I, 8),
#endif    /* SUPPORT_LL */
    zz(ADD, U),
    xx(ADD, U, 4),
#ifdef SUPPORT_LL
    xx(ADD, U, 8),
#endif    /* SUPPORT_LL */
    zz(ADD, P),
    xx(ADD, P, 4),
    /* V: no value of void type exists */
    /* B: addition not applicable to array/structs */

    /* subtraction; F48acg I4(8) U4(8) P4 */
    OP_SUB = OP_ADD + (1 << OP_SOP),
    zz(SUB, F),
    xx(SUB, F, 4),
    xx(SUB, F, 8),
    xx(SUB, F, a),
    xx(SUB, F, c),
    xx(SUB, F, g),
    zz(SUB, I),
    xx(SUB, I, 4),
#ifdef SUPPORT_LL
    xx(SUB, I, 8),
#endif    /* SUPPORT_LL */
    zz(SUB, U),
    xx(SUB, U, 4),
#ifdef SUPPORT_LL
    xx(SUB, U, 8),
#endif    /* SUPPORT_LL */
    zz(SUB, P),
    xx(SUB, P, 4),
    /* V: no value of void type exists */
    /* B: subtraction not applicable to array/structs */

    /* left-shift; I4(8) U4(8) */
    OP_LSH = OP_SUB + (1 << OP_SOP),
    zz(LSH, I),       /* shift applicable only to integers */
    xx(LSH, I, 4),
#ifdef SUPPORT_LL
    xx(LSH, I, 8),
#endif    /* SUPPORT_LL */
    zz(LSH, U),
    xx(LSH, U, 4),
#ifdef SUPPORT_LL
    xx(LSH, U, 8),
#endif    /* SUPPORT_LL */

    /* remainder; I4(8) U4(8) */
    OP_MOD = OP_LSH + (1 << OP_SOP),
    zz(MOD, I),       /* remainder applicable only to integers */
    xx(MOD, I, 4),
#ifdef SUPPORT_LL
    xx(MOD, I, 8),
#endif    /* SUPPORT_LL */
    zz(MOD, U),
    xx(MOD, U, 4),
#ifdef SUPPORT_LL
    xx(MOD, U, 8),
#endif    /* SUPPORT_LL */

    /* right-shift; I4(8) U4(8);
       ASSUMPTION: right shift performs arithmetic shift on the target */
    OP_RSH = OP_MOD + (1 << OP_SOP),
    zz(RSH, I),       /* shift applicable only to integers */
    xx(RSH, I, 4),
#ifdef SUPPORT_LL
    xx(RSH, I, 8),
#endif    /* SUPPORT_LL */
    zz(RSH, U),
    xx(RSH, U, 4),
#ifdef SUPPORT_LL
    xx(RSH, U, 8),
#endif    /* SUPPORT_LL */

    /* bit-and; U4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_BAND = OP_RSH + (1 << OP_SOP),
    /* I: U used instead; bit-and applicable only to integers */
    zz(BAND, U),
    xx(BAND, U, 4),
#ifdef SUPPORT_LL
    xx(BAND, U, 8),
#endif    /* SUPPORT_LL */

    /* bit-complement; U4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_BCOM = OP_BAND + (1 << OP_SOP),
    /* I: U used instead; bit-complement applicable only to integers */
    zz(BCOM, U),
    xx(BCOM, U, 4),
#ifdef SUPPORT_LL
    xx(BCOM, U, 8),
#endif    /* SUPPORT_LL */

    /* bit-or; U4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_BOR = OP_BCOM + (1 << OP_SOP),
    /* I: U used instead; bit-or applicable only to integers */
    zz(BOR, U),
    xx(BOR, U, 4),
#ifdef SUPPORT_LL
    xx(BOR, U, 8),
#endif    /* SUPPORT_LL */

    /* bit-xor; U4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target */
    OP_BXOR = OP_BOR + (1 << OP_SOP),
    /* I: U used instead; bit-xor applicable only to integer types */
    zz(BXOR, U),
    xx(BXOR, U, 4),
#ifdef SUPPORT_LL
    xx(BXOR, U, 8),
#endif    /* SUPPORT_LL */

    /* division; F48acg I4(8) U4(8) */
    OP_DIV = OP_BXOR + (1 << OP_SOP),
    zz(DIV, F),
    xx(DIV, F, 4),
    xx(DIV, F, 8),
    xx(DIV, F, a),
    xx(DIV, F, c),
    xx(DIV, F, g),
    zz(DIV, I),
    xx(DIV, I, 4),
#ifdef SUPPORT_LL
    xx(DIV, I, 8),
#endif    /* SUPPORT_LL */
    zz(DIV, U),
    xx(DIV, U, 4),
#ifdef SUPPORT_LL
    xx(DIV, U, 8),
#endif    /* SUPPORT_LL */
    /* P: division not applicable to pointers */
    /* V: no value of void type exists */
    /* B: division not applicable to array/structs */

    /* multiplication; F48acg I4(8) U4(8) */
    OP_MUL = OP_DIV + (1 << OP_SOP),
    zz(MUL, F),
    xx(MUL, F, 4),
    xx(MUL, F, 8),
    xx(MUL, F, a),
    xx(MUL, F, c),
    xx(MUL, F, g),
    zz(MUL, I),
    xx(MUL, I, 4),
#ifdef SUPPORT_LL
    xx(MUL, I, 8),
#endif    /* SUPPORT_LL */
    zz(MUL, U),
    xx(MUL, U, 4),
#ifdef SUPPORT_LL
    xx(MUL, U, 8),
#endif    /* SUPPORT_LL */

    /* P: multiplication not applicable to pointers */
    /* V: no value of void type exists */
    /* B: multiplication not applicable to array/structs */

    /* equal to; F48acg I4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target;
       ASSUMPTION: pointers can be returned as an integer */
    OP_EQ = OP_MUL + (1 << OP_SOP),
    zz(EQ, F),
    xx(EQ, F, 4),
    xx(EQ, F, 8),
    xx(EQ, F, a),
    xx(EQ, F, c),
    xx(EQ, F, g),
    zz(EQ, I),
    xx(EQ, I, 4),
#ifdef SUPPORT_LL
    xx(EQ, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    /* P: I used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/structs */

    /* greater than or equal to; F48acg I4(8) U4(8);
       ASSUMPTION: pointers can be returned as an integer */
    OP_GE = OP_EQ + (1 << OP_SOP),
    zz(GE, F),
    xx(GE, F, 4),
    xx(GE, F, 8),
    xx(GE, F, a),
    xx(GE, F, c),
    xx(GE, F, g),
    zz(GE, I),
    xx(GE, I, 4),
#ifdef SUPPORT_LL
    xx(GE, I, 8),
#endif    /* SUPPORT_LL */
    zz(GE, U),
    xx(GE, U, 4),
#ifdef SUPPORT_LL
    xx(GE, U, 8),
#endif    /* SUPPORT_LL */
    /* P: U used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/structs */

    /* greater than; F48acg I4(8) U4(8);
       ASSUMPTION: pointers can be returned as an integer */
    OP_GT = OP_GE + (1 << OP_SOP),
    zz(GT, F),
    xx(GT, F, 4),
    xx(GT, F, 8),
    xx(GT, F, a),
    xx(GT, F, c),
    xx(GT, F, g),
    zz(GT, I),
    xx(GT, I, 4),
#ifdef SUPPORT_LL
    xx(GT, I, 8),
#endif    /* SUPPORT_LL */
    zz(GT, U),
    xx(GT, U, 4),
#ifdef SUPPORT_LL
    xx(GT, U, 8),
#endif    /* SUPPORT_LL */
    /* p: U used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/structs */

    /* less than or equal to; F48acg I4(8) U4(8);
       ASSUMPTION: pointers can be returned as an integer */
    OP_LE = OP_GT + (1 << OP_SOP),
    zz(LE, F),
    xx(LE, F, 4),
    xx(LE, F, 8),
    xx(LE, F, a),
    xx(LE, F, c),
    xx(LE, F, g),
    zz(LE, I),
    xx(LE, I, 4),
#ifdef SUPPORT_LL
    xx(LE, I, 8),
#endif    /* SUPPORT_LL */
    zz(LE, U),
    xx(LE, U, 4),
#ifdef SUPPORT_LL
    xx(LE, U, 8),
#endif    /* SUPPORT_LL */
    /* P: U used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/structs */

    /* less than; F48acg I4(8) U4(8);
       ASSUMPTION: pointers can be returned as an integer */
    OP_LT = OP_LE + (1 << OP_SOP),
    zz(LT, F),
    xx(LT, F, 4),
    xx(LT, F, 8),
    xx(LT, F, a),
    xx(LT, F, c),
    xx(LT, F, g),
    zz(LT, I),
    xx(LT, I, 4),
#ifdef SUPPORT_LL
    xx(LT, I, 8),
#endif    /* SUPPORT_LL */
    zz(LT, U),
    xx(LT, U, 4),
#ifdef SUPPORT_LL
    xx(LT, U, 8),
#endif    /* SUPPORT_LL */
    /* P: U used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/struct */

    /* not equal; F48acg I4(8);
       ASSUMPTION: signed/unsigned integers are compatible on the target;
       ASSUMPTION: pointers can be returned as an integer */
    OP_NE = OP_LT + (1 << OP_SOP),
    zz(NE, F),
    xx(NE, F, 4),
    xx(NE, F, 8),
    xx(NE, F, a),
    xx(NE, F, c),
    xx(NE, F, g),
    zz(NE, I),
    xx(NE, I, 4),
#ifdef SUPPORT_LL
    xx(NE, I, 8),
#endif    /* SUPPORT_LL */
    /* U: I used instead */
    /* P: I used instead */
    /* V: no value of void type exists */
    /* B: comparison not applicable to array/struct */

    /* jump; V */
    OP_JMP = OP_NE + (1 << OP_SOP),
    zz(JMP, V),    /* jumps always have void type */

    /* label */
    OP_LABEL = OP_JMP + (1 << OP_SOP),
    zz(LABEL, V),    /* labels always have void type */

    /* used only for expressions with no type suffix */
    OP_AND = OP_LABEL + (1 << OP_SOP),
    OP_NOT = OP_AND + (1 << OP_SOP),
    OP_OR = OP_NOT + (1 << OP_SOP),
    OP_COND = OP_OR + (1 << OP_SOP),
    OP_RIGHT = OP_COND + (1 << OP_SOP),
    OP_FIELD = OP_RIGHT + (1 << OP_SOP),

    /* used only to issue proper diagnostics;
       see COP() from tree.c for declaration order */
    OP_POS = OP_FIELD + (1 << OP_SOP),
    OP_INCR = OP_POS + (1 << OP_SOP),
    OP_DECR = OP_INCR + (1 << OP_SOP),
    OP_SUBS = OP_DECR + (1 << OP_SOP),
    OP_CADD = OP_SUBS + (1 << OP_SOP),
    OP_CSUB = OP_CADD + (1 << OP_SOP),
    OP_CLSH = OP_CSUB + (1 << OP_SOP),
    OP_CMOD = OP_CLSH + (1 << OP_SOP),
    OP_CRSH = OP_CMOD + (1 << OP_SOP),
    OP_CBAND = OP_CRSH + (1 << OP_SOP),
    OP_CBOR = OP_CBAND + (1 << OP_SOP),
    OP_CBXOR = OP_CBOR + (1 << OP_SOP),
    OP_CDIV = OP_CBXOR + (1 << OP_SOP),
    OP_CMUL = OP_CDIV + (1 << OP_SOP),

    /* used for code generation; P */
    OP_VREG = OP_CMUL + (1 << OP_SOP),
    zz(VREG, P),

    /* # of operands in BURS spec */
    OP_1 = OP_VREG + (1 << OP_SOP),
    OP_2,

    /* BURS non-terminals follow;
       OP_2 must immediately precede NTs; see cgr_ntidx() and b*.c */
};


#undef a
#undef c
#undef g

#undef xx
#undef yy
#undef zz


int op_sfx(const ty_t *);
int op_sfxs(const ty_t *);
ty_t *op_stot(int);
#ifndef NDEBUG
const char *op_name(int);
#endif    /* !NDEBUG */


/* helper mask; OP TY CV SZ */
#define OP_MOP (~((1U << OP_SOP) - 1))
#define OP_MTY (~((1U << OP_STY) - 1) & ~OP_MOP)
#define OP_MCV (~((1U << OP_SCV) - 1) & ~(OP_MOP|OP_MTY))
#define OP_MSZ ((1U << OP_SCV) - 1)

/* op_sfx() with widening size of small integer types;
   ASSUMPTION: int represents all small integers;
   ASSUMPTION: int is the word size used for function return/arguments;
   ASSUMPTION: signed types are used instead of their unsigned counterparts */
#define OP_SFXW(ty) (op_sfxs(TY_ISSMALLINT(ty)? ty_inttype: (ty)))

/* op_sfx() for conversions;
   result type+source size+result size */
#define OP_CVSFX(t1, t2) ((op_size(op_sfx(t1)) << OP_SCV) + op_sfx(t2))

/* checks if one of ADDRs */
#define OP_ISADDR(op) (op >= OP_ADDRG && op < OP_ADD)

/* checks if integer type */
#define OP_ISINT(op) (op_type(op) == OP_I || op_type(op) == OP_U)

/* checks if signed/unsigned integer type */
#define OP_ISSINT(op) (op_type(op) == OP_I)
#define OP_ISUINT(op) (op_type(op) == OP_U)

/* checks if conversion */
#define OP_ISCV(op) ((op) >= OP_CVF && (op) < OP_NEG)

/* checks if comparison */
#define OP_ISCMP(op) ((op) >= OP_EQ && (op) < OP_JMP)

/* constructs ADDRx;
   ASSUMPTION: all pointers are uniform (same representation) */
#define op_addr(x) (OP_ADDR##x+OP_P+(ty_voidptype->size << OP_SSZ))

#define op_index(op)   ((op) >> OP_SOP)               /* converts generic operation to index */
#define op_generic(op) ((op) & OP_MOP)                /* generic code of operation */
#define op_optype(op)  ((op) & (OP_MOP|OP_MTY))       /* generic code and type of operation */
#define op_type(op)    ((op) & OP_MTY)                /* result type of operation */
#define op_tyscode(op) ((op) & (OP_MTY|OP_MSZ))       /* type and size of operation result */
#define op_scode(op)   ((op) & OP_MSZ)                /* size code of operation result */
#define op_cvscode(op) ((op) & OP_MCV)                /* size code of source in conversion */
#define op_size(op)    (((op) & OP_MSZ) >> OP_SSZ)    /* size of operation result */
#define op_cvsize(op)  (((op) & OP_MCV) >> OP_SCV)    /* size of source in conversion */


#endif    /* OP_H */

/* end of op.h */

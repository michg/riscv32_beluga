/*
 *  machine description for riscv32
 */


/* common non-terminals */
tt(stmt)
tt(reg)
tt(con)

/* non-terminals */
tt(cons12)
tt(con5)
tt(lab)
tt(addr)


/* common rules */
rr(P(reg),  OP_INDIRI1 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRI2 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRI4 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRP4 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")

rr(P(stmt),  OP_ASGNI1 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNI2 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")

rr(P(con),  OP_CNSTI1,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTI2,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTI4,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTU4,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTP4,  0,  NULL,  "%a")

rr(P(stmt),  P(reg),  0,  NULL,  "")

/* op rules */
/* CNST */
rr(P(con5),   OP_CNSTU4,  0,  rangeu5,  "%a")
rr(P(cons12), OP_CNSTI4,  0,  ranges12,  "%a")

/* ARG */
rr(P(stmt),  OP_ARGI4 _ OP_1 _ P(reg),                   0,  NULL,  "# arg \n")
rr(P(stmt),  OP_ARGP4 _ OP_1 _ P(reg),                   0,  NULL,  "# arg \n")

/* ASGN */
rr(P(stmt),  OP_ASGNI1 _ OP_2 _ P(addr) _ P(reg),  1,  NULL,  "sb %1, %0\n")
rr(P(stmt),  OP_ASGNI2 _ OP_2 _ P(addr) _ P(reg),  1,  NULL,  "sh %1, %0\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _ P(addr) _ P(reg),  1,  NULL,  "sw %1, %0\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _ P(addr) _ P(reg),  1,  NULL,  "sw %1, %0\n")
rr(P(stmt),  OP_ASGNB _ OP_2 _ P(reg) _ OP_INDIRB _ OP_1 _ P(reg), 0,  NULL, "# asgnb\n")

/* INDIR */
rr(P(reg),   OP_INDIRI1 _ OP_1 _ P(addr),  1,  NULL,  "lb %R, %0\n")
rr(P(reg),   OP_CVII14 _ OP_1 _ OP_INDIRI1 _ OP_1 _ P(addr),  1,  NULL,  "lb %R, %0\n")
rr(P(reg),   OP_CVIU14 _ OP_1 _ OP_INDIRI1 _ OP_1 _ P(addr),  1,  NULL,  "lbu %R, %0\n")
rr(P(reg),   OP_INDIRI2 _ OP_1 _ P(addr),  1,  NULL,  "lh %R, %0\n")
rr(P(reg),   OP_CVII24 _ OP_1 _ OP_INDIRI2 _ OP_1 _ P(addr),  1,  NULL,  "lh %R, %0\n")
rr(P(reg),   OP_CVIU24 _ OP_1 _ OP_INDIRI2 _ OP_1 _ P(addr),  1,  NULL,  "lhu %R, %0\n")
rr(P(reg),   OP_INDIRI4 _ OP_1 _ P(addr),  1,  NULL,  "lw %R, %0\n")
rr(P(reg),   OP_INDIRP4 _ OP_1 _ P(addr),  1,  NULL,  "lw %R, %0\n")

/* CVII */
rr(P(reg),  OP_CVII14 _ OP_1 _ P(reg),  2,  NULL,  "slli %R, %0, 8*3\nsrai %R, %R, 8*3\n")
rr(P(reg),  OP_CVII24 _ OP_1 _ P(reg),  2,  NULL,  "slli %R, %0, 8*2\nsrai %R, %R, 8*2\n")
rr(P(reg),  OP_CVII41 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVII41 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")
rr(P(reg),  OP_CVII42 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVII42 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")

/* CVIU */
rr(P(reg),  OP_CVIU14 _ OP_1 _ P(reg),  2,  NULL,  "slli %R, %0, 8*3\nsrli %R, %R, 8*3\n")
rr(P(reg),  OP_CVIU24 _ OP_1 _ P(reg),  2,  NULL,  "slli %R, %0, 8*2\nsrli %R, %R, 8*2\n")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R,%0\n")

/* CVUI */
rr(P(reg),  OP_CVUI41 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUI41 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R, %0\n")
rr(P(reg),  OP_CVUI42 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUI42 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R, %0\n")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R, %0\n")

/* CVUU - not applicable */

/* CVUP */
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R, %0\n")

/* CVPU */
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_move,      "mv %R, %0\n")

/* NEG */
rr(P(reg),  OP_NEGI4 _ OP_1 _ P(reg),  1,  NULL,  "sub %R, x0, %0\n")

/* CALL */
rr(P(reg),  OP_CALLI4 _ OP_1 _ P(lab),  1,  NULL,  "jal x1, %0\n")
rr(P(reg),  OP_CALLI4 _ OP_1 _ P(lab),  1,  NULL,  "jal x1, %0\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(lab),  1,  NULL,  "jal x1, %0\n")
rr(P(reg),  OP_CALLI4 _ OP_1 _ P(reg),  1,  NULL,  "jalr x1, %0, 0\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(reg),  1,  NULL,  "jalr x1, %0, 0\n")
                                                      

/* LOAD */
rr(P(reg),  OP_LOADI1 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")
rr(P(reg),  OP_LOADI2 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")
rr(P(reg),  OP_LOADI4 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")
rr(P(reg),  OP_LOADU4 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")
rr(P(reg),  OP_LOADP4 _ OP_1 _ P(reg),  0,  gen_move,  "mv %R, %0\n")

/* RET */
rr(P(stmt),  OP_RETI4 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")

/* ADDR */
rr(P(lab),   OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(addr),   OP_ADDRFP4,  0,  NULL,  "%a(x8)")
rr(P(addr),   OP_ADDRLP4,  0,  NULL,  "%a(x8)")
rr(P(reg),   OP_ADDRFP4,  1,  NULL,  "addi %R, x8, %a\n")
rr(P(reg),   OP_ADDRLP4,  1,  NULL,  "addi %R, x8, %a\n")


/* ADD */
rr(P(reg),  OP_ADDI4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "add %R, %0, %1\n")
rr(P(reg),  OP_ADDU4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "add %R, %0, %1\n")
rr(P(reg),  OP_ADDP4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "add %R, %0, %1\n")

/* SUB */
rr(P(reg),  OP_SUBI4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "sub %R, %0, %1\n")
rr(P(reg),  OP_SUBU4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "sub %R, %0, %1\n")
rr(P(reg),  OP_SUBP4 _ OP_2 _ P(reg) _ P(reg),    1,  NULL,  "sub %R, %0, %1\n")

/* LSH */
rr(P(reg),  OP_LSHI4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "sll %R, %0, %1\n")
rr(P(reg),  OP_LSHU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "sll %R, %0, %1\n")
rr(P(reg),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con5),  0, rangeu5,  "slli %R, %0, %1\n")
rr(P(reg),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con5),  0, rangeu5,  "slli %R, %0, %1\n")                                                                                                                    

/* MOD */
rr(P(reg),  OP_MODI4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "rem %R, %0, %1\n")                                                           
rr(P(reg),  OP_MODU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "remu %R, %0, %1\n")

/* RSH */
rr(P(reg),  OP_RSHI4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "sra %R, %0, %1\n")
rr(P(reg),  OP_RSHU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "srl %R, %0, %1\n")
rr(P(reg),  OP_RSHI4 _ OP_2 _ P(reg) _ P(con5),  0, rangeu5,  "srai %R, %0, %1\n")
rr(P(reg),  OP_RSHU4 _ OP_2 _ P(reg) _ P(con5),  0, rangeu5,  "srli %R, %0, %1\n")

/* BAND */
rr(P(reg),  OP_BANDU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "and %R, %0, %1\n")
rr(P(reg),  OP_BANDU4 _ OP_2 _ P(reg) _ P(cons12),  0, ranges12,  "andi %R, %0, %1\n")

/* BCOM */
rr(P(reg),  OP_BCOMU4 _ OP_1 _ P(reg),  1,  NULL,  "xori %R, %0, -1\n")

/* BOR */
rr(P(reg),  OP_BORU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "or %R, %0, %1\n")
rr(P(reg),  OP_BORU4 _ OP_2 _ P(reg) _ P(cons12),  0, ranges12,  "ori %R, %0, %1\n")

/* BXOR */
rr(P(reg),  OP_BXORU4 _ OP_2 _ P(reg) _ P(reg),  1,  NULL,  "xor %R, %0, %1\n")
rr(P(reg),  OP_BXORU4 _ OP_2 _ P(reg) _ P(cons12),  0, ranges12,  "xori %R, %0, %1\n")

/* DIV */
rr(P(reg),  OP_DIVI4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "div %R, %0, %1\n")
rr(P(reg),  OP_DIVU4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "divu %R, %0, %1\n")

/* MUL */
rr(P(reg),  OP_MULI4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "mul %R, %0, %1\n")
rr(P(reg),  OP_MULU4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "mul %R, %0, %1\n")

/* EQ */
rr(P(stmt),  OP_EQI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "beq %0, %1, %a\n")

/* GE */
rr(P(stmt),  OP_GEI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bge %0, %1, %a\n")
rr(P(stmt),  OP_GEU4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bgeu %0, %1, %a\n")

/* GT */
rr(P(stmt),  OP_GTI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bgt %0, %1, %a\n")
rr(P(stmt),  OP_GTU4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bgtu %0, %1, %a\n")

/* LE */
rr(P(stmt),  OP_LEI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "ble %0, %1, %a\n")
rr(P(stmt),  OP_LEU4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bleu %0, %1, %a\n")

/* LT */
rr(P(stmt),  OP_LTI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "blt %0, %1, %a\n")
rr(P(stmt),  OP_LTU4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bltu %0, %1, %a\n")

/* NE */
rr(P(stmt),  OP_NEI4 _ OP_2 _ P(reg) _ P(reg),  4,  NULL,  "bne %0, %1, %a\n")

/* JMP */
rr(P(stmt),  OP_JMPV _ OP_1 _ P(lab),  1,  NULL,  "jal x0, %0\n")
rr(P(stmt),  OP_JMPV _ OP_1 _ P(reg),  1,  NULL,  "jalr x0, %0, 0\n")

/* LABEL */
rr(P(stmt),  OP_LABELV,  0,  NULL,  "%a:\n")

/* chain rules */
rr(P(reg),   P(lab),  1,  NULL,  "la %R, %0\n")
rr(P(reg),   P(con),  1,  NULL,  "li %R, %0\n")  
rr(P(addr),   P(reg),  0,  NULL,  "0(%0)")
//rr(P(reg),   P(addr),  0,  NULL,  "%0")

#undef tt
#undef rr

/* end of bx86t.r */

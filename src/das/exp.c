/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  EXP.C
 */

#include "defs.h"

Prototype int32_t ParseIntExp(char *);
Prototype char  *ParseExp(char *, Label **, int32_t *);
Prototype char  *ParseEffAddr(char *, EffAddr *);
Prototype char  *ParseRegList(char *, uword *);
Prototype char  *ParseBinaryConst(char *, int32_t *);
Prototype char  *ParseHexConst(char *, int32_t *);
Prototype char  *ParseIntConst(char *, int32_t *);
Prototype char  *ParseLabel(char *, Label **, int);
Prototype int   CheckRegister(char *);
Prototype void  InitAlNumAry(void);

char AlNumAry[256];
char LowerAry[256];

void
InitAlNumAry(void)
{
    short i;

    for (i = 0; i < 256; ++i)
        LowerAry[i] = i;

    for (i = 'a'; i <= 'z'; ++i)
        AlNumAry[i] = 1;
    for (i = 'A'; i <= 'Z'; ++i) {
        AlNumAry[i] = 1;
        LowerAry[i] = 'a' - 'A' + i;
    }
    for (i = '0'; i <= '9'; ++i)
        AlNumAry[i] = 1;
    AlNumAry['_'] = 1;
    AlNumAry['@'] = 1;

}

int32_t
ParseIntExp(str)
char *str;
{
    int32_t v, a=0;
    short op = -1, neg = 0, com = 0;

    while (*str && *str != ' ' && *str != 9) {
        if (op) {
            if (*str == '-') {
                neg = !neg;
                ++str;
                continue;
            } else if (*str == '~') {
                com = !com;
                ++str;
                continue;
            } else if (*str == '$') {
                str = ParseHexConst(str + 1, &v);
            } else if (*str >= '0' && *str <= '9') {
                str = ParseIntConst(str, &v);
            } else if (*str == '%') {
                str = ParseBinaryConst(str + 1, &v);
            } else {    /*  assume label    */
                Label *lab;
                char *base = str;
                str = ParseLabel(str, &lab, 1);
                if (lab)
                    v = lab->l_Value;
                if (base == str) {
                    cerror(EERROR_ILLEGAL_LABEL, base);
                    ++str;
                }
            }

            if (neg)
                v = -v;
            neg = 0;
            if (com)
                v = ~v;
            com = 0;

            switch(op) {
              case  -1: a = v;   break;
              case '>': a >>= v; break;
              case '<': a <<= v; break;
              case '|': a |= v;  break;
              case '&': a &= v;  break;
              case '^': a ^= v;  break;
              case '+': a += v;  break;
              case '-': a -= v;  break;
              case '*': a *= v;  break;
              case '/':
                if (v)
                  a /= v;
                break;
            }
            op = 0;
        } else {
            switch(*str) {
            case '>':
            case '<':
            case '|': 
            case '&':
            case '^':
            case '+':
            case '-':
            case '*':
            case '/':
                op = *str;
                break;
            default:
                cerror(EERROR_EXPECTED_OPERATOR, *str);
                break;
            }
            ++str;
        }
    }
    if (op)
        cerror(EERROR_EXPECTED_EXP);
    
    return(a);
}

/*
 *  Parses an expression filling in the appropriate fields in the
 *  effective address.  Does NOT have to handle registers, register lists,
 *  or addressing modes.
 */

char *
ParseExp(str, plab, poff)
char *str;
Label **plab;
int32_t  *poff;
{
    int32_t v,a = 0;
    short op = -1, neg = 0, com = 0;

    while (*str) {
        if (op) {
            if (*str == '-') {
                neg = !neg;
                ++str;
                continue;
            } else if (*str == '~') {
                com = !com;
                ++str;
                continue;
            } else if (*str == '$') {
                str = ParseHexConst(str + 1, &v);
            } else if (*str == '%') {
                str = ParseBinaryConst(str + 1, &v);
            } else if (*str >= '0' && *str <= '9') {
                str = ParseIntConst(str, &v);
            } else {    /*  assume label    */
                Label *lab;
                char *base = str;
                v = 0;
                str = ParseLabel(str, &lab, 1);
                if (lab) {
                    dbprintf(0, ("exp lab %s type %d val %ld\n", lab->Name, lab->l_Type, lab->l_Value));
                    switch(lab->l_Type) {
                    case LT_LOC:
                    case LT_EXT:
                        *plab = lab;
                        if (op == '-' || neg)
                            cerror(EERROR_NEG_LABEL_OFFSET);
                        break;
                    case LT_REG:
                        *plab = lab;
                        break;
                    case LT_INT:
                        v = lab->l_Value;
                        break;
                    }
                } else {
                    dbprintf(0, ("Label %s not found\n", str));
                }
                if (base == str) {
                    cerror(EERROR_ILLEGAL_LABEL, base);
                    ++str;
                }
            }

            if (neg)
                v = -v;
            neg = 0;
            if (com)
                v = ~v;
            com = 0;

            switch(op) {
              case  -1: a = v;   break;
              case '>': a >>= v; break;
              case '<': a <<= v; break;
              case '|': a |= v;  break;
              case '&': a &= v;  break;
              case '^': a ^= v;  break;
              case '+': a += v;  break;
              case '-': a -= v;  break;
              case '*': a *= v;  break;
              case '/':
                if (v)
                  a /= v;
                break;
            }
            op = 0;
        } else {
            switch(*str) {
            case '>':
            case '<':
            case '|':
            case '&':
            case '^':
            case '+':
            case '-':
            case '*':
            case '/':
                op = *str;
                break;
            case '.':
            case ',':
            case '(':
            case ')':
            case '[':
            case ']':
                *poff = a;
                return(str);
            default:
                cerror(EERROR_EXPECTED_OPERATOR, *str);
                break;
            }
            ++str;
        }
        while (*str == ' ' || *str == 9)    /*  XXX remove me? */
            ++str;
    }
    if (op)
        cerror(EERROR_EXPECTED_EXP);

    *poff = a;
    return(str);
}


/*
 *  Parse an addressing mode.
 *
 *      Dn
 *      An
 *      (An)
 *      (An)++
 *      --(An)
 *      exp(An)                 od([bd,An],Xn.size*SCALE)
 *      exp(An,Xn.size*SCALE)   od([bd,An,Xn.size*SCALE])
 *      exp[.size]
 *      exp(pc)
 *      exp(pc,Xn.size*SCALE)
 *      #immediate
 *      CCR
 *      SR
 *      USP
 *
 *      regs    reglist (A single register is a valid reglist)
 *      branch  branch  exp(pc) and exp[.size] (exp contains statement label)
 *                      are valid
 */

#define MF_MEMIND       0x0001
#define MF_MEMINDPOS    0x0004
#define MF_INDEX        0x0008
#define MF_INDEX020     0x0010
#define MF_BDWORD       0x0020
#define MF_BDLONG       0x0040
#define MF_ODWORD       0x0080
#define MF_ODLONG       0x0100

char *
ParseEffAddr(str, ea)
char *str;
EffAddr *ea;
{
    short c1 = (ubyte)str[0];
    short c2 = (ubyte)str[1];
    short c3 = (ubyte)str[2];
    short flags;
    char baseReg;
    char idxReg;

    /*
     *  handle simple cases
     */

    /*
     *  An
     */
    if (c1 == 'A' && (c2 >= '0' && c2 <= '7') && (c3 == 0 || c3 == ',')) {
        ea->Reg1 = RB_AREG - '0' + c2;
        ea->Mode1 = AB_AN;
        ea->Mode2 = AB_REGS;
        ea->ExtWord |= 1 << ea->Reg1;
        return(str + 2);
    }

    /*
     *  Dn
     */
    if (c1 == 'D' && (c2 >= '0' && c2 <= '7') && (c3 == 0 || c3 == ',')) {
        ea->Reg1 = RB_DREG - '0' + c2;
        ea->Mode1 = AB_DN;
        ea->Mode2 = AB_REGS;
        ea->ExtWord |= 1 << ea->Reg1;
        return(str + 2);
    }

    /*
     *  handle more complex cases
     */

    c1 = LowerAry[c1];
    c2 = LowerAry[c2];

    switch(c1) {
    case 'a':
        if (c2 >= '0' && c2 <= '7' && !AlNumAry[c3]) {
            ea->Reg1 = RB_AREG - '0' + c2;
            ea->Mode1 = AB_AN;
anregs:
            ea->Mode2 = AB_REGS;
            ea->ExtWord |= 1 << ea->Reg1;
            str += 2;
            if (c3 == '-' || c3 == '/') {
                ea->Mode1 = AB_REGS;
                ea->Reg1 = -1;
                str = ParseRegList(str-2, &ea->ExtWord);
            }
            return(str);
        }
        break;
    case 'd':
        if (c2 >= '0' && c2 <= '7' && !AlNumAry[c3]) {
            ea->Reg1 = RB_DREG - '0' + c2;
            ea->Mode1 = AB_DN;
            goto anregs;
        }
        break;
    case '(':   /*  (An)[+]     */
        if (c2 == '[')
            break;
        if (str[3] != ')')
            break;
        if (c2 == 's') {
            c2 = 'a';
            str[1] = 'a';
            str[2] = c3 = '7';
        }
        if (c2 != 'a' || c3 < '0' || c3 > '7')
            syntax(2);
        ea->Reg1 = RB_AREG - '0' + c3;
        if (str[4] == '+') {
            ea->Mode1 = AB_INDPP;
            return(str + 5);
        } else {
            ea->Mode1 = AB_INDAN;
            return(str + 4);
        }
        /* not reached */
    case '#':   /*  #exp    */
        ea->Mode1 = AB_IMM;
        ++str;
        break;
    case '-':   /*  -(An)  */
        if (c2 == '(') {
            c1 = str[2] | 0x20;
            c2 = str[3] | 0x20;
            if (c1 == 's' && c2 == 'p') {
                c1 = 'a';
                c2 = '7';
            }
            if (c1 != 'a' || str[4] != ')')
                syntax(3);
            ea->Mode1 = AB_MMIND;
            ea->Reg1 = c2 - '0' + RB_AREG;
            return(str + 5);
        }
        break;
    case 'c':   /*  CCR     */
        if (c2 == 'c' && (c3|0x20) == 'r' && !AlNumAry[(short)str[3]]) {
            ea->Mode1 = AB_CCR;
            return(str + 3);
        }
        break;
    case 's':   /*  SP,SR   */
        if (c2 == 'p' && !AlNumAry[c3]) {   /*  sp  */
            ea->Mode1 = AB_AN;
            ea->Mode2 = AB_REGS;
            ea->Reg1 = 7;
            ea->ExtWord = 1 << ea->Reg1;
            return(str + 2);
        }
        if (c2 == 'r' && !AlNumAry[c3]) {
            ea->Mode1 = AB_SR;
            return(str + 2);
        }
        break;
    case 'u':   /*  USP     */
        if (c2 == 's' && (c3|0x20) == 'p' && !AlNumAry[(short)str[3]]) {
            ea->Mode1 = AB_USP;
            return(str + 3);
        }
        break;
    }

    /*
     *  Some kind of expression.  ea->Mode1 might already be set to AB_IMM
     *
     *      68000 format                        68020 format
     *
     *      d16(An)                             (d16,An)
     *      d8(An,Xn[.size][*SCALE])            (d8,An,Xn.z*SCALE)
     *      exp[.size]
     *      d16(pc)                             (d16,PC)
     *      d8(pc,Xn[.size][*SCALE])            (d8,PC,Xn.z*SCALE)
     *      #exp
     *                                          (bd,An,Xn.z*SCALE)  (od redundant)
     *                                          (bd,PC,Xn.z*SCALE)  (od redundant)
     *                                          ([bd,PC],Xn.z*SCALE,od)
     *                                          ([bd,PC,Xn.z*SCALE],od)
     *                                                ^
     *                                              ZPC for program-acc/PC=0
     */

    c1 = *str;
    if (c1 != '(') {
        str = ParseExp(str, &ea->Label1, &ea->Offset1);    /* can be nil exp */
        c1 = *str;
    }

    if (c1 == '.') {
        if (ea->Mode1 == AB_IMM)
            syntax(4);

        switch(CToSize(str[1])) {
        case 1:
            ea->ISize |= ISF_BYTEB;
            break;
        case 2:
            ea->ISize |= ISF_ABSW;
            break;
        case 4:
            break;
        }
        str += 2;
        c1 = *str;
    }
    if (c1 != '(') {
        if (ea->Mode1 != AB_IMM) {
            Label *lab = ea->Label1;

            ea->Mode1 = (ea->ISize & ISF_ABSW) ? AB_ABSW    : AB_ABSL;
            ea->Mode2 = (ea->ISize & ISF_INSTBYTE)? AB_BBRANCH : AB_WBRANCH;

            /*
             *  but wait, exp could have been a reg label specifying
             *  a single register.. valid in normal opcodes.
             */

            if (lab && lab->l_Type == LT_REG) {
                if (lab->l_RegNo >= 0) {
                    ea->Mode1  = (lab->l_RegNo >= RB_AREG) ? AB_AN : AB_DN;
                    ea->Reg1   = lab->l_RegNo;
                    ea->Mode2  = AB_REGS;
                    ea->ExtWord= lab->l_Mask;
                } else {
                    ea->Reg1    = -1;
                    ea->Mode1   = AB_REGS;
                    ea->ExtWord = lab->l_Mask;
                }
            }
        }
        return(str);
    }

    /*
     *  general index format
     *
     *  ([bd,An],Xn ...)        note:   od, An, Xn are optional
     *  ([bd,An,Xn...])
     */

    flags = 0;

    ++str;
    if (*str == '[') {
        flags |= MF_MEMIND | MF_INDEX | MF_INDEX020;
        ++str;
    }

    /*
     *  optional base displacement (Label2,Offset2), or base register
     */

    baseReg = CheckRegister(str);
    idxReg = -1;

    if (baseReg < 0) {
        {
            str = ParseExp(str, &ea->Label2, &ea->Offset2);

            flags |= MF_BDLONG;
            if (ea->Label2 && *str != '.')
                cerror(EERROR_OUTER_DISP_WL);
            if (*str == '.') {
                if ((str[1] | 0x20) == 'w') {
                    flags |= MF_BDWORD;
                    flags &= ~MF_BDLONG;
                }
                str += 2;
            } else if (ea->Offset2 >= -32768 && ea->Offset2 < 32768) {
                if (ea->Offset2)
                    flags |= MF_BDWORD;
                flags &= ~MF_BDLONG;
            }
            if (*str == '(' && strnicmp(str, "(a4)", 4) == 0) {
                ea->ISize |= ISF_BDDREL;    /*  data-relative   */
                str += 4;
                if ((flags & (MF_BDWORD|MF_BDLONG)) == 0)
                    flags |= MF_BDWORD;
            }
        }
#ifdef NOTDEF
            str = ParseExp(str, &ea->Label1, &ea->Offset1);

            flags |= MF_ODLONG | MF_INDEX;
            if (ea->Label1 && *str != '.' && *str != '(')
                cerror(EERROR_OUTER_DISP_WL);

            if (*str == '.') {
                if ((str[1] | 0x20) == 'w') {
                    flags |= MF_ODWORD;
                    flags &= ~MF_ODLONG;
                }
                str += 2;
            } else if (ea->Offset1 >= -32768 && ea->Offset1 < 32768) {
                if (ea->Offset1)
                    flags |= MF_ODWORD;
                flags &= ~MF_ODLONG;
            }
            if (*str == '(' && strnicmp(str, "(a4)", 4) == 0) {
                ea->ISize |= ISF_ODDREL;    /*  data-relative   */
                str += 4;
                if ((flags & (MF_ODWORD|MF_ODLONG)) == 0)
                    flags |= MF_ODWORD;
            }
        }
#endif
        flags |= MF_INDEX | MF_INDEX020;
    } else {
        str += 2;
    }

    if (*str != ']' && baseReg < 0) {
        /*
         *  expect base register, but could be index register
         */

        if (*str == ',')
            ++str;
        else
            syntax(13);

        baseReg = CheckRegister(str);
        if (baseReg < 0)
            syntax(10);
        str += 2;
        if (baseReg < RB_AREG || *str == '*' || *str == '.') {
            idxReg = baseReg;
            baseReg = -1;
            goto skip;
        }
    }

    if (*str == ']') {
        flags |= MF_MEMINDPOS | MF_INDEX | MF_INDEX020;
        ++str;
    }

    /*
     *  optional index register
     */

    if (*str == ',') {
        idxReg = CheckRegister(str + 1);

        if (idxReg >= 0) {
            str += 3;
skip:
            flags |= MF_INDEX;

            if (*str == '.') {
                switch(str[1]) {
                case 'w':
                case 'W':
                    break;
                case 'l':
                case 'L':
                    ea->ExtWord |= EXTF_LWORD;
                    break;
                default:
                    syntax(11);
                }
                str += 2;
            }
            if (*str == '*') {
                flags |= MF_INDEX | MF_INDEX020;

                switch(str[1]) {
                case '1':
                    break;
                case '2':
                    ea->ExtWord |= 0x0200;
                    break;
                case '4':
                    ea->ExtWord |= 0x0400;
                    break;
                case '8':
                    ea->ExtWord |= 0x0600;
                    break;
                }
                str += 2;
            }
        }
    }
    if ((flags & (MF_MEMINDPOS|MF_MEMIND)) == MF_MEMIND) {
        if (*str != ']')
            syntax(16);
        ++str;
    }

    /*
     *  outer displacement (illegal if no ind)
     */

    if (*str == ',') {      /*  outer displacement  */
        if (!(flags & MF_MEMIND))
            cerror(EERROR_NO_BRACKET_NO_OD);

        flags |= MF_INDEX | MF_INDEX020;
        str = ParseExp(str + 1, &ea->Label1, &ea->Offset1);

        flags |= MF_ODLONG;
        if (ea->Label1 && *str != '.' && *str != '(')
            cerror(EERROR_OUTER_DISP_WL);

        if (*str == '.') {
            if ((str[1] | 0x20) == 'w') {
                flags |= MF_ODWORD;
                flags &= ~MF_ODLONG;
            }
            str += 2;
        } else if (ea->Offset1 >= -32768 && ea->Offset1 < 32768) {
            if (ea->Offset1)
                flags |= MF_ODWORD;
            flags &= ~MF_ODLONG;
        }
        if (*str == '(' && strnicmp(str, "(a4)", 4) == 0) {
            ea->ISize |= ISF_ODDREL;    /*  data-relative   */
            str += 4;
            if ((flags & (MF_ODWORD|MF_ODLONG)) == 0)
                flags |= MF_ODWORD;
        }
    }

    ea->Reg1 = baseReg;

    if ((flags & MF_INDEX) == 0) {
        if (baseReg == 16) {
            ea->Mode1 = AB_OFFPC;
            ea->Mode2 = (ea->ISize & ISF_INSTBYTE) ? AB_BBRANCH : AB_WBRANCH;
        } else {
            ea->Mode1 = AB_OFFAN;
        }
    } else {
        if (flags & (MF_MEMIND|MF_ODLONG|MF_ODWORD|MF_BDLONG|MF_BDWORD)) {
            ea->ExtWord |= EXTF_FULL;

            if (flags & MF_ODWORD)
                ea->ExtWord |= EXTF_ODWORD;
            else if (flags & MF_ODLONG)
                ea->ExtWord |= EXTF_ODLONG;
            else
                ea->ExtWord |= EXTF_ODNULL;

            if (flags & MF_BDWORD)
                ea->ExtWord |= EXTF_BDWORD;
            else if (flags & MF_BDLONG)
                ea->ExtWord |= EXTF_BDLONG;
            else
                ea->ExtWord |= EXTF_BDNULL;

            if ((flags & MF_MEMINDPOS) && idxReg >= 0)
                ea->ExtWord |= EXTF_MEMIND;
            if ((flags & MF_MEMIND) == 0)       /*  no memory indirect */
                ea->ExtWord &= ~EXTF_ODMASK;
        }
        if (baseReg == 16) {
            ea->Mode1 = AB_OFFIDXPC;
        } else if (baseReg == 17) {
            ea->Mode1 = AB_OFFIDXPC;
            ea->ExtWord |= EXTF_NOBREG | EXTF_FULL;     /*  ZPC */
        } else {
            ea->Mode1 = AB_OFFIDX;
            if (baseReg < 0)
                ea->ExtWord |= EXTF_NOBREG | EXTF_FULL;
            ea->Reg1 = baseReg;
        }
        if (idxReg < 0) {
            ea->ExtWord |= EXTF_NOIDX | EXTF_FULL;
        } else if (idxReg < RB_AREG) {
            ea->ExtWord |= idxReg << 12;
        } else {
            ea->ExtWord |= ((idxReg - RB_AREG) << 12) | EXTF_AREG;
        }
    }
    if ((flags & MF_INDEX020) && !MC68020)
        cerror(EWARN_68020);
    if (*str != ')')
        syntax(9);
    return(str + 1);
}

char *
ParseRegList(str, pmask)
char *str;
uword *pmask;
{
    int32_t mask = 0;
    short regno = -1;
    short range = 0;

    while (*str && *str != ' ' && *str != 9) {
        if (range) {
            short reg2 = -1;

            if (*str == 'D' || *str == 'd')
                reg2 = str[1] - '0' + RB_DREG;
            else if (*str == 'A' || *str == 'a')
                reg2 = str[1] - '0' + RB_AREG;
            else
                cerror(EERROR_BAD_REG_SPEC);
            while (regno <= reg2)
                mask |= 1 << regno++;
            regno = -1;
            range = 0;
            str += 2;
            if (*str == '/')
                ++str;
        } else if (regno < 0) {
            if (*str == ',')
                break;
            if (*str == 'D' || *str == 'd')
                regno = str[1] - '0' + RB_DREG;
            else if (*str == 'A' || *str == 'a')
                regno = str[1] - '0' + RB_AREG;
            else
                cerror(EERROR_BAD_REG_SPEC);
            str += 2;
        } else {
            if (*str == ',')
                break;
            if (*str == '/') {
                mask |= 1 << regno;
                regno = -1;
            } else if (*str == '-') {
                range = 1;
            } else {
                cerror(EERROR_BAD_REG_SPEC);
            }
            ++str;
        }
    }
    if (range == 1)
        cerror(EERROR_BAD_REG_SPEC);
    if (regno >= 0)
        mask |= 1 << regno;
    *pmask = mask;
    return(str);
}


char *
ParseBinaryConst(str, pv)
char *str;
int32_t *pv;
{
    int32_t v = 0;

    for (;;) {
        if (*str == '0' || *str == '1')
            v = (v << 1) | (*str - '0');
        else if (*str != '.')
            break;
        ++str;
    }
    *pv = v;
    return(str);
}

char *
ParseHexConst(str, pv)
char *str;
int32_t *pv;
{
    int32_t v = 0;

    for (;;) {
        if (*str >= '0' && *str <= '9') {
            v = (v << 4) + *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            v = (v << 4) + *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'F') {
            v = (v << 4) + *str - 'A' + 10;
        } else
            break;
        ++str;
    }
    *pv = v;
    return(str);
}

char *
ParseIntConst(str, pv)
char *str;
int32_t *pv;
{
    int32_t v = 0;
    while (*str >= '0' && *str <= '9')
        v = v * 10 + *str++ - '0';
    *pv = v;
    return(str);
}

char *
ParseLabel(str, plab, showerr)
char *str;
Label **plab;
int showerr;
{
    char *base = str;
    char c;
    Label *lab;

    while (*str == '@' || *str == '_' || (*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9'))
        ++str;
    c = *str;
    *str = 0;
    lab = FindLabel(base);
    if (lab == NULL && showerr)
        cerror(EERROR_UNDEFINED_LABEL, base);
    *str = c;
    *plab = lab;
    return(str);
}

int
CheckRegister(ptr)
char *ptr;
{
    int reg = 0;

    switch(*ptr) {
    case 'a':
    case 'A':
        reg = RB_AREG;
    case 'd':
    case 'D':
        reg += ptr[1] - '0';
        if (ptr[1] >= '0' && ptr[1] <= '7') {
almost:
            if (ptr[2] == 0 || ptr[2] == '.' || ptr[2] == ']' || ptr[2] == ')' || ptr[2] == ',')
                return(reg);
        }
        break;
    case 'p':
    case 'P':
        reg = 16;
        if (ptr[1] == 'c' || ptr[1] == 'C')
            goto almost;
        break;
    case 's':
    case 'S':
        reg = RB_AREG + 7;
        if (ptr[1] == 'p' || ptr[1] == 'P')
            goto almost;
        break;
    case 'z':
    case 'Z':
        reg = 17;
        ++ptr;
        if (strnicmp(ptr, "pc", 2) == 0)
            goto almost;
        break;
    }
    return(-1);
}


#ifdef NOTDEF

char *
ParseDumpString(mc, str, termc)
MachCtx *mc;
char *str;
char termc;
{
    char buf[64];
    short i = 0;

    while (*str && *str != termc) {
        buf[i++] = *str;
        if (i == sizeof(buf)) {
            DumpSectionData(mc->Sect, buf, i);
            i = 0;
        }
        ++str;
    }
    if (i)
        DumpSectionData(mc->Sect, buf, i);
    if (*str)
        return(str+1);
    cerror(EERROR_EXPECTED_TERMINATING, termc);
    return(str);
}
#endif

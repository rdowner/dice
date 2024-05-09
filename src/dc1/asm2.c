/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASM2.C
 *
 */

/*
**      $Filename: asm2.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:35 $
**      $Log: asm2.c,v $
 * Revision 30.326  1995/12/24  06:09:35  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:37:22  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:46  dice
 * .
 *
 * Revision 1.6  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.5  1993/09/11  22:35:28  jtoebes
 * Fixed BUG06009.
 * asm_cmp optimized a compare into a tst instruction for unsigned values.
 * Eliminated this improper optimization.
 *
 * Revision 1.4  1993/09/06  09:31:42  jtoebes
 * Fixed BUG06066 - Enforcer hit with float code.
 * CallAsmSupport did not check for the register being null.
 *
**/

#include "defs.h"
#include "asm.h"

#define Max(a,b)    (((a) > (b)) ? (a) : (b))

Prototype void asm_move(Exp *, Stor *, Stor *);
Prototype void asm_move_cast(Exp *, Stor *, Stor *);
Prototype int32_t ReverseOrder(short);
Prototype void asm_cmp(Exp *, Stor *, Stor *, short *);
Prototype int32_t asm_div(Exp *, Stor *, Stor *, Stor *, short);
Prototype int32_t asm_mul(Exp *, Stor *, Stor *, Stor *);
Prototype int32_t asm_mul_requires_call(int32_t);
Prototype void asm_shift(Exp *, int32_t, Stor *, Stor *, Stor *);
Prototype void CallAsmSupport(Exp *, char *, Stor *, Stor *, Stor *, short);
Prototype void asm_bfext(Exp *, Stor *, Stor *);
Prototype void asm_bfsto(Exp *, Stor *, Stor *);
Prototype void asm_bftst(Exp *, Stor *);
Prototype void asm_blockfill(Exp *, Stor *, int32_t, char);
Prototype void asm_illegal(void);
Prototype void asm_moveqAndSwap(int32_t, char, short);

Local void asm_blockop(Exp *, char *, short, Stor *, Stor *);
Local void OptimizeBitField(Stor *, Stor *);

/*
 *  asm_blockop()   is used for block moves and block compares.  When
 *                  used for block compares the loop breaks out when
 *                  a not-equal condition occurs.  The 'cond' passed
 *                  to the routine only effects the final branch, NOT
 *                  the loop-breakout condition.
 */

Local void
asm_blockop(Exp *exp, char *op, short cond, Stor *s, Stor *d)
{
    int32_t n = s->st_Size >> 2;
    int32_t i;
    int32_t o1 = s->st_Offset;
    int32_t o2 = d->st_Offset;
    int32_t label;
    int32_t breaklabel;
    Stor scnt, mcnt, asrc, adst;
    char *dbstr;
    short doRemainder = 1;

    /*
    printf("block %s %s (%d %d) (%d %d)\n",
        StorToString(s, NULL), StorToString(d, NULL), s->st_Size, d->st_Size,
        s->st_Type, d->st_Type
    );
    */

    breaklabel = AllocLabel();

    switch(cond) {
    case COND_EQ:
    case COND_NEQ:
        dbstr = "dbne";     /*  loop until not equal or done (eq)   */
        break;
    default:
        dbstr = "dbf";
        break;
    }


    if (s->st_Type < ST_RelReg || s->st_Type > ST_RegIndex)
        yerror(exp->ex_LexIdx, EERROR_BLOCK_OPERATION_SOURCE);
    if (d->st_Type < ST_RelReg || d->st_Type > ST_RegIndex)
        yerror(exp->ex_LexIdx, EERROR_BLOCK_OPERATION_DEST);

    /*
     *  In the default case we load a data register with the lw count
     *  and two address registers with the base addresses.
     */

    switch(n) {
    default:
        --n;
        LockStorage(s);
        LockStorage(d);
        if (n > 65534) {
            AllocDataRegister(&mcnt, 2);
            AllocDataRegister(&scnt, 4);
        } else {
            AllocDataRegister(&scnt, 2);
        }

        /*
         *  If int32_t aligned we can reuse s and d registers, but if not
         *  we need them!
         */

        if (s->st_Size & 3) {
            AllocAddrRegister(&asrc);
            AllocAddrRegister(&adst);
        } else {
            UnlockStorage(s);
            AllocAddrRegister(&asrc);
            UnlockStorage(d);
            LockStorage(s);
            AllocAddrRegister(&adst);
            LockStorage(d);
        }
        LockStorage(&asrc);
        LockStorage(&adst);
        asm_movei(exp, n, &scnt);
        if (n > 65534) {
            printf("\tmove.l\tD%d,D%d\n", scnt.st_RegNo, mcnt.st_RegNo);
            printf("\tswap\tD%d\n", mcnt.st_RegNo);
        }
        asm_lea(exp, s, 0, &asrc);
        asm_lea(exp, d, 0, &adst);
        UnlockStorage(s);
        UnlockStorage(d);
        asrc.st_Type = ST_RelReg;
        asrc.st_Offset = 0;
        adst.st_Type = ST_RelReg;
        adst.st_Offset = 0;

        label = AllocLabel();
        asm_label(label);
        printf("\t%s.l\t(A%d)+,(A%d)+\n", op, asrc.st_RegNo - RB_ADDR, adst.st_RegNo - RB_ADDR);
        printf("\t%s\tD%d,l%d\n", dbstr, scnt.st_RegNo, label);
        if (n > 65534) {
            if (cond)
                asm_condbra(COND_NEQ, breaklabel);
            printf("\t%s\tD%d,l%d\n", dbstr, mcnt.st_RegNo, label);
        }

        /*
         *  Handle tail-end case here.  Can't handle tail end case
         *  down below more because the structure might be larger then
         *  128 bytes.
         */

        if (s->st_Size & 3) {
            if (cond)
                asm_condbra(COND_NEQ, breaklabel);
            switch(s->st_Size & 3) {
            case 3:
            case 2:
                printf("\t%s.w\t(A%d)+,(A%d)+\n", op, asrc.st_RegNo - RB_ADDR, adst.st_RegNo - RB_ADDR);
                if ((s->st_Size & 1) == 0)
                    break;
                if (cond)
                    asm_condbra(COND_NEQ, breaklabel);
                /* fall through */
            case 1:
                printf("\t%s.b\t(A%d),(A%d)\n", op, asrc.st_RegNo - RB_ADDR, adst.st_RegNo - RB_ADDR);
                break;
            }
            doRemainder = 0;
        }

        UnlockStorage(&asrc);
        UnlockStorage(&adst);
        FreeRegister(&adst);
        FreeRegister(&asrc);
        FreeRegister(&scnt);
        if (n > 65534)
            FreeRegister(&mcnt);
        break;
    case 4:     /*  note, LGBO_SIZE (machine.h) */
    case 3:
    case 2:
    case 1:
        if (cond)
            AllocDataRegister(&mcnt, 4);

        for (i = n - 1; i >= 0; --i) {
            s->st_Offset = o1 + i * 4;
            d->st_Offset = o2 + i * 4;
            if (cond) {
                outop("move", 4, d, &mcnt);
                outop(op, 4, s, &mcnt);
            } else {
                outop(op, 4, s, d);
            }

            if (cond && (i || (s->st_Size & 3)))
                asm_condbra(COND_NEQ, breaklabel);
        }
        if (cond)
            FreeRegister(&mcnt);
        break;
    case 0:
        break;
    }

    /*
     *  do remainder.  s and d are guarenteed to contain source and
     *  destination bases only if (s->st_Size & 3) != 0
     */

    if (doRemainder) {
        s->st_Offset = o1 + (s->st_Size & ~3);
        d->st_Offset = o2 + (d->st_Size & ~3);

        switch(s->st_Size & 3) {
        case 0:
            break;
        case 3:     /*   2 + 1  */
        case 2:
            if (cond) {
                AllocDataRegister(&mcnt, 4);
                outop("move", 2, d, &mcnt);
                outop(op, 2, s, &mcnt);
                FreeRegister(&mcnt);
            } else {
                outop(op, 2, s, d);
            }
            if ((s->st_Size & 1) == 0)
                break;
            s->st_Offset += 2;
            d->st_Offset += 2;
        case 1:
            if (cond) {
                AllocDataRegister(&mcnt, 4);
                outop("move", 1, d, &mcnt);
                outop(op, 1, s, &mcnt);
                FreeRegister(&mcnt);
            } else {
                outop(op, 1, s, d);
            }
            break;
        default:
            Assert(0);
        }
    }
    if (cond)
        asm_label(breaklabel);

    s->st_Offset = o1;
    d->st_Offset = o2;
}

#ifdef NOTDEF
/*
 *  fill a block (d is a pointer) with a value, simple stupid
 *
 *  currently d must be register-relative
 */

void
asm_blockfill(exp, d, n, v)
Exp *exp;
Stor *d;
int32_t n;
char v;
{
    Stor t;
    Stor r;
    Stor adst;
    Stor dcnt;
    int32_t label;

    asm_getind(&VoidPtrType, d, &t, -1, -1, 0);

    switch(n) {
    case 0:
        break;
    case 6:
        t.st_Size = 2;
        asm_movei(exp, 0, &t);
        n -= 2;
        t.st_Offset += 2;
    case 1:
    case 2:
    case 4:
        t.st_Size = n;
        asm_movei(exp, 0, &t);
        break;
    case 8:
    case 12:
    case 16:
        AllocDataRegister(&r, 4);
        t.st_Size = 4;
        asm_movei(exp, 0, &r);
        while (n) {
            asm_move(exp, &r, &t);
            t.st_Offset += 4;
            n -= 4;
        }
        FreeRegister(&r);
        break;
    default:        /*  byte fill   */
        label = AllocLabel();

        AllocDataRegister(&r, 4);
        AllocDataRegister(&dcnt, 2);
        AllocAddrRegister(&adst);
        asm_movei(exp, 0, &r);
        asm_move(exp, d, &adst);
        asm_movei(exp, n-1, &dcnt);
        asm_label(label);
        printf("\tmove.b\tD%d,(A%d)+\n", r.st_RegNo, adst.st_RegNo - RB_ADDR);
        printf("\tdbf\tD%d,l%d\n", dcnt.st_RegNo, label);
        FreeRegister(&r);
        FreeRegister(&dcnt);
        FreeRegister(&adst);
        break;
    }
}

#endif

/*
 *  asm_move()  strictly moves objects of the same size
 */

void
asm_move(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    /*
    fprintf(stderr, "move %s %s (%d %d) (%d %d)\n",
        StorToString(s, NULL), StorToString(d, NULL), s->st_Size, d->st_Size,
        s->st_Type, d->st_Type
    );
    */

    if (d->st_Flags & SF_LEA)
        yerror(exp->ex_LexIdx, EFATAL_DEST_NOT_LVALUE);

    if (SameStorage(s, d))
        return;
    if (s->st_Type == ST_IntConst) {
        int value = s->st_IntConst;
        int swapit = 0;

        switch (d->st_Size) {
        case 1:     /*  e.g. unsigned 0xF0 -> char == moveq #-10    */
            value = (char)value;
            break;
        case 2:     /*  e.g. unsigned 0xFFF0 -> short == moveq #-10 */
            value = (short)value;
            break;
        }

        /*
         *  can we moveq or moveq/swap ?
         *
         *  note: bug in lattice C, comparison with 0x is done signed. 0x
         *  hex constant is supposed to be unsigned.
         */

        if ((uint32_t)value >= 0x10000 && (int32_t)value < -128 && (int32_t)value > 127) {
            swapit = 1;
            value = ((uint32_t)value >> 16) | ((uint32_t)value << 16);
        }
        if (value >= -128 && value < 128) {
            Stor stor;

            if (d->st_Type == ST_Reg) {
                if (d->st_RegNo < RB_ADDR) {
                    asm_moveqAndSwap(value, d->st_RegNo + '0', swapit);
#ifdef NOTDEF
                    printf("\tmoveq.l\t#%d,D%c\n", value, d->st_RegNo + '0');
                    if (swapit)
                        printf("\tswap\tD%c\n", d->st_RegNo + '0');
#endif
                    return;
                }

                /*
                 *  optimize for address, move #n,An sign extends to
                 *  address register.
                 */

                if (s->st_IntConst == 0) {  /*  value may be invalid due to swap */
                    outop("suba", 4, d, d);
                    return;
                }
                if (s->st_IntConst < 32768) {
                    outop("move", 2, s, d);
                    return;
                }
            }
            if (d->st_Size == 4) {
                AllocDataRegister(&stor, 4);
                asm_moveqAndSwap(value, stor.st_RegNo + '0', swapit);
#ifdef NOTDEF
                printf("\tmoveq.l\t#%d,D%c\n", value, stor.st_RegNo + '0');
                if (swapit)
                    printf("\tswap\tD%c\n", stor.st_RegNo + '0');
#endif
                outop("move", 0, &stor, d);
                FreeRegister(&stor);
                return;
            }
        }

        /*
         *  note: value may be invalid due to swap
         */

        outop("move", 0, s, d);
        return;
    }
    if (s->st_Type == ST_FltConst) {
        int32_t fltv[4];
        int32_t offset = d->st_Offset;
        int32_t size   = d->st_Size;
        short i;

        /* Make sure that we are not trying to do a move on something that we   */
        /* can't bump the offset for an indirect move.  In the case of a simple */
        /* 4 byte float, it is legal to have a direct register.  Also, when we  */
        /* Move to 68881 support, we will need to relax this check              */
        if ((d->st_Type < ST_RelReg || d->st_Type > ST_RegIndex) &&
            ((d->st_Type != ST_Reg) && (size != 4)))
        {
            dbprintf(("softerror, blockop struct, type d=%d\n", d->st_Type));
            Assert(0);
        }

        asm_fltconst(exp, s, fltv);

        d->st_Size = 4;
        for (i = 0; i < size; i += 4) {
            asm_movei(exp, fltv[i>>2], d);
            d->st_Offset += 4;
        }
        d->st_Offset = offset;
        d->st_Size   = size;
        return;
    }
    if (s->st_Size != d->st_Size)
    {
        dbprintf(("asm_move: size mismatch %d %d", s->st_Size, d->st_Size));
        Assert(0);
    }

    if (s->st_Flags & SF_LEA) {
        if (d->st_Size != 4)
        {
            dbprintf(("move ptr, dest size not 4"));
            Assert(0);
        }
        if (d->st_Type == ST_Reg && d->st_RegNo >= RB_ADDR) {
            outop("lea", 4, s, d);
        } else if (s->st_Type == ST_RelReg && s->st_Offset == 0) {
            s->st_Type = ST_Reg;
            outop("move", 4, s, d);
            s->st_Type = ST_RelReg;
        } else {
            Stor t;
            LockStorage(s);
            AllocAddrRegister(&t);
            UnlockStorage(s);
            outop("lea", 4, s, &t);
            outop("move", 0, &t, d);
            FreeRegister(&t);
        }
        return;
    }
    if (s->st_Size < d->st_Size)    /* XXX != */
        eprintf(1, "move: size mismatch %d %d\n", s->st_Size, d->st_Size);

    if (s->st_Size <= 4 && s->st_Size != 3) {
        Stor stor;
        /* We have to check for the case where we are doing a byte move from an */
        /* address register and handle it specially.                            */
        /* To accomplish this, we will need a data register.                    */
        if ((s->st_Size == 1) &&       /* Byte move                             */
            (s->st_Type == ST_Reg) &&  /* From a register                       */
            (s->st_RegNo >= RB_ADDR))  /* But it must be an address register    */
        {

           AllocDataRegister(&stor, 4);
           s->st_Size = 2;
           outop("move", 0, s, &stor);
           s->st_Size = 1;
           s = &stor;
        }
        outop("move", 0, s, d);
        return;
    }
    asm_blockop(exp, "move", 0, s, d);
}

/*
 *  This is a hack.. this combination occurs so often that using printf()s
 *  effects the speed of compilation!
 */

void
asm_moveqAndSwap(int32_t value, char dregC, short swapit)
{
    char *ptr = StorBuf[0];

    ptr[0] = '\t';
    ptr[1] = 'm';
    ptr[2] = 'o';
    ptr[3] = 'v';
    ptr[4] = 'e';
    ptr[5] = 'q';
    ptr[6] = '.';
    ptr[7] = 'l';
    ptr[8] = '\t';
    ptr[9] = '#';
    ptr += 10;
    if (value) {
        ptr = itodec(ptr, value);
    } else {
        *ptr = '0';
        ++ptr;
    }

    ptr[0] = ',';
    ptr[1] = 'D';
    ptr[2] = dregC;
    ptr[3] = '\n';
    ptr += 4;

    if (swapit) {
        ptr[0] = '\t';
        ptr[1] = 's';
        ptr[2] = 'w';
        ptr[3] = 'a';
        ptr[4] = 'p';
        ptr[5] = '\t';
        ptr[6] = 'D';
        ptr[7] = dregC;
        ptr[8] = '\n';
        ptr += 9;
    }
    fwrite(StorBuf[0], ptr - StorBuf[0], 1, stdout);
#ifdef NOTDEF
                printf("\tmoveq.l\t#%d,D%c\n", value, stor.st_RegNo + '0');
                if (swapit)
                    printf("\tswap\tD%c\n", stor.st_RegNo + '0');
#endif
}


/*
 *  asm_move_cast() works like asm_move() but only on integer objects.  Objects
 *  of different sizes may be moved.
 */

void
asm_move_cast(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    if (s->st_Size > d->st_Size) {
        int32_t off = s->st_Offset;
        int32_t size = s->st_Size;

        switch(s->st_Type) {
        case ST_PtrConst:
        case ST_RelReg:
        case ST_RelArg:
        case ST_RelName:
        case ST_RelLabel:
        case ST_RegIndex:
            s->st_Offset += s->st_Size - d->st_Size;
            break;
        }
        s->st_Size = d->st_Size;
        asm_move(exp, s, d);
        s->st_Offset = off;
        s->st_Size = size;
        return;
    }
    if (s->st_Size == d->st_Size) {
        asm_move(exp, s, d);
        return;
    }
    if (SameStorage(s, d)) {
        if (s->st_Flags & SF_UNSIGNED) {
            int32_t val;
            Stor con;

            if (s->st_Size == 1) {
                if (d->st_Size == 2)
                    val = 0x00FF;
                else
                    val = 0x000000FF;
            } else {
                val = 0x0000FFFF;
            }
            AllocConstStor(&con, val, &LongType);
            asm_and(exp, &con, d, d);
            return;
        } else if (s->st_Type == ST_Reg && s->st_RegNo < RB_ADDR) {
            if (s->st_Size == 1) {
                if (d->st_Size >= 2)
                    outop("ext", 2, NULL, d);
                if (d->st_Size == 4)
                    outop("ext", 4, NULL, d);
            } else {
                outop("ext", 4, NULL, d);
            }
            return;
        }
    }

    /*
     *  not a reg or not same storage or not a reg and same storage.
     */
    {
        Stor tmp;

        LockStorage(s);
        AllocDataRegister(&tmp, d->st_Size);
        UnlockStorage(s);
        if (s->st_Flags & SF_UNSIGNED) {
            asm_movei(exp, 0, &tmp);
            tmp.st_Size = s->st_Size;
            asm_move(exp, s, &tmp);
            tmp.st_Size = d->st_Size;
            asm_move(exp, &tmp, d);
        } else {
            tmp.st_Size = s->st_Size;
            asm_move(exp, s, &tmp);
            if (s->st_Size == 1)
                outop("ext", 2, NULL, &tmp);
            if (d->st_Size == 4)
                outop("ext", 4, NULL, &tmp);
            tmp.st_Size = d->st_Size;
            asm_move(exp, &tmp, d);
        }
        FreeRegister(&tmp);
    }
}

/*
 *  Reversing the opcode ordering in the compare is not the same as
 *  cond = -cond
 */

int32_t
ReverseOrder(short cond)
{
    switch(cond) {
    case COND_LT:
        return(COND_GT);
    case COND_LTEQ:
        return(COND_GTEQ);
    case COND_GT:
        return(COND_LT);
    case COND_GTEQ:
        return(COND_LTEQ);
    case COND_EQ:
        return(COND_EQ);
    case COND_NEQ:
        return(COND_NEQ);
    case CF_UNS|COND_LT:
        return(CF_UNS|COND_GT);
    case CF_UNS|COND_LTEQ:
        return(CF_UNS|COND_GTEQ);
    case CF_UNS|COND_GT:
        return(CF_UNS|COND_LT);
    case CF_UNS|COND_GTEQ:
        return(CF_UNS|COND_LTEQ);
    case CF_UNS|COND_EQ:
        return(CF_UNS|COND_EQ);
    case CF_UNS|COND_NEQ:
        return(CF_UNS|COND_NEQ);

    case -COND_LT:          /*  gteq    */
        return(COND_LTEQ);
    case -COND_LTEQ:        /*  gt      */
        return(COND_LT);
    case -COND_GT:          /*  lteq    */
        return(COND_GTEQ);
    case -COND_GTEQ:        /*  lt      */
        return(COND_GT);
    case -COND_EQ:          /*  neq     */
        return(COND_NEQ);
    case -COND_NEQ:         /*  eq      */
        return(COND_EQ);

    case -(CF_UNS|COND_LT):     /*  gteq    */
        return(CF_UNS|COND_LTEQ);
    case -(CF_UNS|COND_LTEQ):   /*  gt      */
        return(CF_UNS|COND_LT);
    case -(CF_UNS|COND_GT):     /*  lteq    */
        return(CF_UNS|COND_GTEQ);
    case -(CF_UNS|COND_GTEQ):   /*  lt      */
        return(CF_UNS|COND_GT);
    case -(CF_UNS|COND_EQ):     /*  neq     */
        return(COND_NEQ);
    case -(CF_UNS|COND_NEQ):    /*  eq      */
        return(COND_EQ);
    }
    dbprintf(("Unknown cond %d\n", cond));
    Assert(0);
    return(0);                  /* not reached */
}

void
asm_cmp(exp, s1, s2, pcond)
Exp *exp;
Stor *s1;
Stor *s2;
short *pcond;
{
    Stor stor1;
    Stor stor2;

    if ((s1->st_Flags | s2->st_Flags) & SF_BITFIELD)
    {
        dbprintf(("compare - bitfield"));
        Assert(0);
    }

    Assert(s1->st_Size == s2->st_Size);
    if (s2->st_Type == ST_Reg) {
        SWAPS(s1, s2);
        *pcond = ReverseOrder(*pcond);
    }
    if (ImmStorage(s1)) {
        SWAPS(s1, s2);
        *pcond = ReverseOrder(*pcond);
    }
    if (!(s1->st_Flags & SF_LEA) && s2->st_Type == ST_IntConst) {
        /*
         *  if cmp.l #int32_t and int32_t is in moveq range then use
         *  moveq/cmp instead.  But, if the constant is 0 then use
         *  a tst instead.  Since we know the branch condition we could
         *  also conceivably optimize compares with the constants 1 and
         *  -1 as well (a >= 1 === a > 0).
         */
        if (s2->st_IntConst == 0 &&
            (s1->st_Type != ST_Reg || s1->st_RegNo < RB_ADDR) &&
            ((*pcond & CF_UNS) == 0)) {
            outop("tst", 0, NULL, s1);
            return;
        }
        if (s1->st_Size == 4 && s2->st_IntConst >= -128 && s2->st_IntConst < 128) {
            LockStorage(s1);
            AllocDataRegister(&stor1, 4);
            UnlockStorage(s1);
            asm_move(exp, s2, &stor1);
            outop("cmp", 0, s1, &stor1);
            *pcond = ReverseOrder(*pcond);
            FreeRegister(&stor1);
            return;
        }
        outop("cmp", 0, s2, s1);
        return;


#ifdef NOTDEF
        if (s2->st_IntConst < -128 || s2->st_IntConst > 127 || s2->st_Size < 4) {
            if (s1->st_Flags & SF_LEA) {
                LockStorage(s2);
                AllocAddrRegister(&stor1);
                asm_move(exp, s1, &stor1);
                UnlockStorage(s2);
                outop("cmp", 0, &stor1, s2);
                FreeRegister(&stor1);                  xXX
                *pcond = ReverseOrder(*pcond);      XXX
                return;
            }
            outop("cmp", 0, s2, s1);
            return;
        }
#endif
    }

    if (s2->st_Flags & SF_LEA) {        /*  cmp ea,An   */
        SWAPS(s1, s2);
        *pcond = ReverseOrder(*pcond);
    }

    if ((s1->st_Flags | s2->st_Flags) & SF_LEA) {
        if (s1->st_Flags & SF_LEA) {
            LockStorage(s2);
            AllocAddrRegister(&stor1);
            asm_move(exp, s1, &stor1);
            UnlockStorage(s2);
        } else {
            stor1 = *s1;
        }
        if (s2->st_Flags & SF_LEA) {
            LockStorage(s1);
            AllocAddrRegister(&stor2);
            asm_move(exp, s2, &stor2);
            UnlockStorage(s1);
        } else {
            stor2 = *s2;
        }
        outop("cmp", 0, &stor2, &stor1);
        if (s1->st_Flags & SF_LEA)
            FreeRegister(&stor1);
        if (s2->st_Flags & SF_LEA)
            FreeRegister(&stor2);
        return;
    }
    if (s1->st_Type == ST_Reg) {
        outop("cmp", 0, s2, s1);
        return;
    }
    if (s1->st_Size > 4) {
        asm_blockop(exp, "cmp", *pcond, s1, s2);
    } else {
        LockStorage(s2);
        AllocDataRegister(&stor1, s2->st_Size);
        asm_move(exp, s1, &stor1);
        UnlockStorage(s2);
        outop("cmp", 0, s2, &stor1);
        FreeRegister(&stor1);
    }
}

/*
 *  asm_div()   d = s1 / s2     divs s2,s1  divs ea,Dn
 *              d = s1 % s2
 *
 *  returns (1) TRUE if we had to make a call
 */

int32_t
asm_div(Exp *exp, Stor *s1, Stor *s2, Stor *d, short mod)
{
    Stor ts1;
    Stor ts2;
    short ts1Flag = 0;
    short ts2Flag = 0;
    int opsize = 2;
    char *dop;

    if ((s1->st_Flags | s2->st_Flags) & SF_UNSIGNED)
        dop = (mod) ? "modu" : "divu";
    else
        dop = (mod) ? "mods" : "divs";

    /*
     *
     *  procedure call required?  means:    (1) if d int32_t and s1 or s2 int32_t
     *                                      (2) d not int32_t and s2 int32_t
     */

    /*printf("; s1,s2,d %d %d %d\n", s1->st_Size, s2->st_Size, d->st_Size);*/

    if (s2->st_Size == 4 || (d->st_Size == 4 && s1->st_Size == 4)) {
        if (MC68020Opt && !mod) {
            opsize = 4;
        } else {
            CallAsmSupport(exp, dop, s1, s2, d, 1);
            return(1);
        }
    }

    ts1 = *s1;
    ts2 = *s2;

    /*
     *  s1 directly addressable?  means:    (1) int32_t
     *                                      (2) in data reg
     *                                      (3) s1 == d
     *
     *  (also implies d)
     *
     *  optimization:   if d is a register attempt to divide directly to it
     */

    if ((s1->st_Type != ST_Reg || s1->st_RegNo >= RB_ADDR) || s1->st_Size != 4 || SameStorage(s1, d) == 0) {
        if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR && !SameRegister(s2, d)) {
            ts1 = *d;
            ts1.st_Size = 4;
            ts1.st_Flags = s1->st_Flags;
            asm_ext(exp, s1, &ts1, s1->st_Flags);
        } else {
            LockStorage(s2);
            AllocDataRegister(&ts1, 4);
            asm_ext(exp, s1, &ts1, s1->st_Flags);
            UnlockStorage(s2);
            ts1Flag = 1;
        }
    }

    /*
     *  s2 addressable?     means:  (1) word sized
     *                              (2) not in addr reg
     */

    if (s2->st_Size != opsize || (s2->st_Type == ST_Reg && s2->st_RegNo >= RB_ADDR)) {
        LockStorage(s1);
        AllocDataRegister(&ts2, opsize);
        asm_ext(exp, s2, &ts2, s2->st_Flags);
        UnlockStorage(s1);
        ts2Flag = 1;
    }

    /*
     *  output instruction
     */

    dop = ((s1->st_Flags | s2->st_Flags) & SF_UNSIGNED) ? "divu" : "divs";

    outop(dop, opsize, &ts2, &ts1);

    if (mod) {
        char c = ts1.st_RegNo + '0';
        if (d->st_Size == 4 && (d->st_Flags & SF_UNSIGNED))
            printf("\tclr.w\tD%c\n", c);
        printf("\tswap\tD%c\n", c);
        if (d->st_Size == 4 && !(d->st_Flags & SF_UNSIGNED))
            printf("\text.l\tD%c\n", c);
    } else {
        char c = ts1.st_RegNo + '0';
        if (d->st_Size == 4 && opsize != 4) {
            if (d->st_Flags & SF_UNSIGNED)
                printf("\tand.l\t#$FFFF,D%c\n", c);
            else
                printf("\text.l\tD%c\n", c);
        }
    }

    /*
     *  Handle temporaries
     */

    if (ts2Flag)
        FreeRegister(&ts2);
    if (ts1Flag) {
        FreeRegister(&ts1);
        ts1.st_Size = d->st_Size;
        asm_move(exp, &ts1, d);
    }
    return(0);      /*  no call required */
}

#ifdef NOTDEF

asm_div(exp, s1, s2, d, mod)
Exp *exp;
Stor *s1, *s2;
Stor *d;
short mod;
{
    short s2Size;
    short sv;
    int r;
    char *dop;
    char *dopc;

    if (s1->st_Flags & SF_LEA) {
        Stor t;
        LockStorage(s2);
        AllocAddrRegister(&t);
        asm_move(exp, s1, &t);
        UnlockStorage(s2);
        r = asm_div(&t, s2, d, mod);
        FreeRegister(&t);
        return(r);
    }
    if (s2->st_Flags & SF_LEA) {
        Stor t;
        LockStorage(s1);
        AllocAddrRegister(&t);
        asm_move(exp, s2, &t);
        UnlockStorage(s1);
        r = asm_div(s1, &t, d, mod);
        FreeRegister(&t);
        return(r);
    }

    if (s2->st_Type == ST_IntConst) {
        if (s2->st_IntConst == 1) {
            if (mod)
                asm_movei(exp, 0, d);
            else
                asm_move(exp, s1, d);
            return(0);
        }
        if (s2->st_IntConst == -1) {
            if (mod)
                asm_movei(exp, 0, d);
            else
                asm_neg(exp, s1, d);
            return(0);
        }
    }

    /*
     *  note the last condition... if d = s1/s2 where s1's size is not
     *  a int32_t (divs/divu always take a int32_t for the numerator), we
     *  must make it a int32_t whether or not it is already in a data reg.
     *  We cannot simply truncate the data reg since it might be a
     *  int32_t register variable.
     */

    if (!SameStorage(s1, d) || d->st_Type != ST_Reg || d->st_RegNo >= RB_ADDR || s1->st_Size == 2) {
        Stor t;
        Stor u;

        LockStorage(s2);
        AllocDataRegister(&t, s1->st_Size);
        t.st_Flags |= s1->st_Flags & SF_UNSIGNED;
        u = t;
        u.st_Size = d->st_Size;

        if (s1->st_Type == ST_IntConst) {
            t.st_Size = 4;
            asm_move(exp, s1, &t);
        } else {
            if ((t.st_Flags & SF_UNSIGNED) && t.st_Size != 4)
                asm_movei(exp, 0, &t);
            asm_move(exp, s1, &t);
            if ((t.st_Flags & SF_UNSIGNED) == 0 && t.st_Size != 4) {
                if (t.st_Size == 1)
                    outop("ext", 2, NULL, &t);
                if (t.st_Size == 2)
                    outop("ext", 4, NULL, &t);
            }
            t.st_Size = 4;
        }
        UnlockStorage(s2);
        r = asm_div(&t, s2, &u, mod);
        asm_move(exp, &u, d);
        FreeRegister(&t);
        return(r);
    }

    s2Size = s2->st_Size;
    if (s2->st_Type == ST_IntConst)
        s2->st_Size = SizeFitSU(s2->st_IntConst, s2);

    /*
     *  s1 = s1 / s2, s1 is a DReg.  note that s1 and d might
     *  have different sizes.  s2 determines sign.
     *          s1  2,4
     *          s2  2,4 s/u
     *          d   2,4
     *
     */

    Assert(s1->st_Size != 1 && s2->st_Size != 1 && d->st_Size != 1);

    sv = (~s1->st_Size & 2) | (s2->st_Size & 4) | ((d->st_Size & 4) << 1);
    if (s2->st_Flags & SF_UNSIGNED)
        ++sv;

    /*
     *  odd == unsigned divide
     *  msb 8 for d size == 32, lsb 8 for d size == 16
     *  & 4 : s2 size 32
     *  & 2 : s1 size 32
     */

    dop = (sv & 1) ? "divu" : "divs";
    if (mod)
        dopc = (sv & 1) ? "modu" : "mods";
    else
        dopc = dop;

    if (sv >= 4) {      /*  >= 8 32 bit dest,   else 16 bit dest    */
        r = 1;
        CallAsmSupport(exp, dopc, s1, s2, d, 1);
    } else {            /*  32/16 or 32%16  */
        r = 0;
        outop(dop, 2, s2, d);
        if (mod) {
            char c = d->st_RegNo + '0';
            if (d->st_Size == 4 && (d->st_Flags & SF_UNSIGNED))
                printf("\tclr.w\tD%c\n", c);
            printf("\tswap\tD%c\n", c);
            if (d->st_Size == 4 && !(d->st_Flags & SF_UNSIGNED))
                printf("\text.l\tD%c\n", c);
        } else {
            char c = d->st_RegNo + '0';
            if (d->st_Size == 4) {
                if (d->st_Flags & SF_UNSIGNED)
                    printf("\tand.l\t#$FFFF,D%c\n", c);
                else
                    printf("\text.l\tD%c\n", c);
            }
        }
    }
    s2->st_Size = s2Size;
}

#endif

/*
 *  asm_mul()   d = s1 * s2     d = s1 * #const
 *
 *  Returns (1) TRUE if we had to make a call.  asm_mul_requires_call()
 *  deals with simplistic cases, erroring on the side of returning TRUE
 *  sometimes even if no call will be made, but never returning FALSE
 *  and then making a call.
 */

int32_t
asm_mul_requires_call(offset)
int32_t offset;
{
    if (offset >= -1 && offset <= 1)
        return(0);
    if (offset < 0)
        return(1);
    if (PowerOfTwo(offset) >= 0)
        return(0);
    return(1);
}

/*
 *  d = s1 * s2     muls ea,Dn      muls s2,s1
 *
 *  returns (1) TRUE if we had to make a call
 */

int32_t
asm_mul(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2;
Stor *d;
{
    Stor ts1;
    Stor ts2;
    short ts1Flag = 0;
    short ts2Flag = 0;
    int opsize = 2;
    char *mop = ((s1->st_Flags | s2->st_Flags) & SF_UNSIGNED) ? "mulu" : "muls";

    /*
     *  optimize constant placement, want constant in s2
     *  optimize destination, want destination == s1
     */

    if (s1->st_Type == ST_IntConst)
        SWAPS(s1,s2);
    if (SameStorage(s2,d))
        SWAPS(s1,s2);

    /*
     *  optimize for constant multiply
     */

    ts1 = *s1;
    ts2 = *s2;

    if (s2->st_Type == ST_IntConst) {
        int32_t n;

        switch (s2->st_IntConst) {
        case -1:
            if (s1->st_Size != d->st_Size) {
                if (d->st_Type == ST_Reg && d->st_Type < RB_ADDR) {
                    asm_ext(exp, s1, d, s1->st_Flags);
                    asm_neg(exp, d, d);
                } else {
                    AllocDataRegister(&ts1, d->st_Size);
                    asm_ext(exp, s1, &ts1, s1->st_Flags);
                    asm_neg(exp, &ts1, d);
                    FreeRegister(&ts1);
                }
            } else {
                asm_neg(exp, s1, d);
            }
            return(0);
        case 0:
            asm_movei(exp, 0, d);
            return(0);
        case 1:
            asm_ext(exp, s1, d, s1->st_Flags);
            return(0);
        }

        n = PowerOfTwo(s2->st_IntConst);
        if (n >= 0) {
            AllocConstStor(&ts1, n, &LongType);
            asm_shift(exp, -1, s1, &ts1, d);
            return(0);
        }
        ts2.st_Size = SizeFitSU(s2->st_IntConst, s2);
    }

    /*
     *  procedure call required?  means:    (1) either source is int32_t
     */

    if (ts1.st_Size == 4 || ts2.st_Size == 4) {
        if (MC68020Opt) {
            opsize = 4;
        } else {
            CallAsmSupport(exp, mop, &ts1, &ts2, d, -1);
            return(1);
        }
    }

    /*
     *  s1 directly addressable? means:  (1) word
     *                                   (2) in data reg
     *                                   (3) s1 == d
     *  (also implies d)
     *
     *  optimization:   if d is a register attempt to multiply directly
     *                  to it
     */

    if ((s1->st_Type != ST_Reg || s1->st_RegNo >= RB_ADDR) || s1->st_Size != opsize || SameStorage(s1, d) == 0) {
        if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR && !SameRegister(s2, d)) {
            ts1 = *d;
            ts1.st_Size = opsize;
            ts1.st_Flags = s1->st_Flags;
            asm_ext(exp, s1, &ts1, s1->st_Flags);
        } else {
            LockStorage(s2);
            AllocDataRegister(&ts1, opsize);
            asm_ext(exp, s1, &ts1, s1->st_Flags);
            UnlockStorage(s2);
            ts1Flag = 1;
        }
    }

    /*
     *  s2 addressable?     means:  (1) word
     *                              (2) not in address register
     */

    if (s2->st_Size != opsize || (s2->st_Type == ST_Reg && s2->st_RegNo >= RB_ADDR)) {
        LockStorage(s1);
        AllocDataRegister(&ts2, opsize);
        asm_ext(exp, s2, &ts2, s2->st_Flags);
        UnlockStorage(s1);
        ts2Flag = 1;
    }

    /*
     *  output instruction, result is a longword
     */

    outop(mop, opsize, &ts2, &ts1);

    /*
     *  Handle temporaries
     */

    if (ts2Flag)
        FreeRegister(&ts2);
    if (ts1Flag) {
        FreeRegister(&ts1);
        ts1.st_Size = d->st_Size;
        asm_move(exp, &ts1, d);
    }
    return(0);      /*  no call required */
}


#ifdef NOTDEF

asm_mul(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2;
Stor *d;
{
    short s2Size;
    short sv;
    int r = 0;
    char *mop;

    if (s1->st_Flags & SF_LEA) {
        Stor t;
        LockStorage(s2);
        AllocAddrRegister(&t);
        asm_move(exp, s1, &t);
        UnlockStorage(s2);
        r = asm_mul(&t, s2, d);
        FreeRegister(&t);
        return(r);
    }
    if (s2->st_Flags & SF_LEA) {
        Stor t;
        LockStorage(s1);
        AllocAddrRegister(&t);
        asm_move(exp, s2, &t);
        UnlockStorage(s1);
        r = asm_mul(s1, &t, d);
        FreeRegister(&t);
        return(r);
    }

    if (SameStorage(s2,d))
        SWAPS(s1,s2);

    if (s1->st_Type == ST_IntConst)
        SWAPS(s1,s2);

    s2Size = s2->st_Size;
    if (s2->st_Type == ST_IntConst) {
        int n;
        switch(s2->st_IntConst) {
        case -1:
            asm_neg(s1, d);
            return(0);
        case 0:
            asm_movei(exp, 0, d);
            return(0);
        case 1:
            asm_move(exp, s1, d);
            return(0);
        }

        n = PowerOfTwo(s2->st_IntConst);
        if (n >= 0) {
            Stor con;
            AllocConstStor(&con, n, &LongType);
            asm_shift(exp, -1, s1, &con, d);
            return(0);
        }
        s2->st_Size = SizeFitSU(s2->st_IntConst, s2);
    }


    if (!SameStorage(s1, d) || d->st_Type != ST_Reg || d->st_RegNo >= RB_ADDR) {
        Stor t;
        Stor u;

        LockStorage(s2);
        AllocDataRegister(&t, s1->st_Size);
        u = t;
        u.st_Size = d->st_Size;

        asm_move(exp, s1, &t);
        UnlockStorage(s2);
        r = asm_mul(&t, s2, &u);
        asm_move(exp, &u, d);
        FreeRegister(&t);
        s2->st_Size = s2Size;
        return(r);
    }


    /*
     *  s1 = s1 * s2, s1 is a DReg.  note that s1 and d might
     *  have different sizes.  s2 determines sign.
     *          s1  2,4
     *          s2  2,4 s/u
     *          d   2,4
     *
     */

    Assert(s1->st_Size != 1 && s2->st_Size != 1 && d->st_Size != 1);

    sv = (~s1->st_Size & 2) | (s2->st_Size & 4) | ((d->st_Size & 4) << 1);
    if (s2->st_Flags & SF_UNSIGNED)
        ++sv;

    /*
     *  odd == unsigned multiply
     *  msb 8 for d size == 32, lsb 8 for d size == 16
     *  & 4 : s2 size 32
     *  & 2 : s1 size 32
     */

    mop = (sv & 1) ? "mulu" : "muls";

    if (sv <= 9) {
        /*  s/u  ? * ? -> 16    */
        Stor t = *s2;
        if (t.st_Type != ST_Reg && (sv & 4))
            t.st_Offset += 2;
        outop(mop, 2, &t, d);
        r = 0;
    } else {
        /*  s 32 * 32 -> 32   */
        /*  u 32 * 32 -> 32   */

        CallAsmSupport(exp, mop, s1, s2, d, -1);
        r = 1;
    }
    s2->st_Size = s2Size;
    return(r);
}

#endif

void
asm_shift(exp, dir, s1, s2, d)
Exp *exp;
int32_t dir;
Stor *s1;
Stor *s2;
Stor *d;
{
    char *str = "asl";
    static short Recur = 3;

    if (--Recur == 0)
    {
        dbprintf(("asm_shift recur"));
        Assert(0);
    }

    if (dir > 0) {
        if (s1->st_Flags & SF_UNSIGNED)
            str = "lsr";
        else
            str = "asr";
    }

    /*
     *  XXX optimize 1 << ?
     */


    if (!SameStorage(s1, d) || SameStorage(s2, d) || s1->st_Size != d->st_Size || (s1->st_Type == ST_Reg && s1->st_RegNo >= RB_ADDR)) {
        Stor nd;

wc:
        LockStorage(s2);
        AllocDataRegister(&nd, Max(s1->st_Size, d->st_Size));
        nd.st_Flags |= s1->st_Flags & SF_UNSIGNED;
        asm_move_cast(exp, s1, &nd);
        UnlockStorage(s2);
        asm_shift(exp, dir, &nd, s2, &nd);
        asm_move_cast(exp, &nd, d);
        FreeRegister(&nd);
        ++Recur;
        return;
    }
    /*
     *  s1 == d
     */
    if (s1->st_Type != ST_Reg) {
        if (s2->st_Type == ST_IntConst && s2->st_IntConst == 1 && d->st_Size == 2) {
            outop(str, 0, NULL, d);
            ++Recur;
            return;
        }
        /*puts("SNR");*/
        goto wc;
    }

    /*
     *  s1 == d == register, s2 integer in range?
     */
    if (s2->st_Type == ST_IntConst && s2->st_IntConst > 0 && s2->st_IntConst <= 8) {
        outop(str, 0, s2, d);
        ++Recur;
        return;
    }

    /*
     *  s2 out of range, stick in register too
     */

    if (s2->st_Type == ST_Reg && s2->st_RegNo < RB_ADDR) {
        outop(str, 0, s2, d);
    } else {
        Stor cnt;

        AllocDataRegister(&cnt, s2->st_Size);
        asm_move(exp, s2, &cnt);
        outop(str, 0, &cnt, d);
        FreeRegister(&cnt);
    }
    ++Recur;
}

/*
 *  Compiler Support call (passing simple data <= 4 bytes)
 *
 *      order=1     order=0     order=-1
 *  s1      D0        D1        either way
 *  s2      D1        D0
 *
 *  (call)
 *  D0->d
 *
 *  handle byte/word cases (ext into int32_t)
 */

void
CallAsmSupport(Exp *exp,char *mop, Stor *s1, Stor *s2, Stor *d, short orderReq)
{
    Stor rd0, rd1;
    Stor ss1;
    Stor ss2;
    uint32_t mask = 0;

    GenFlagCallMade();

    if (d)
    {
        if (d->st_Type == ST_Reg) {
            mask = 1 << d->st_RegNo;
        } else if (d->st_Type == ST_RegIndex) {
            if (((1 << d->st_RegNo) | (1 << d->st_RegNo2)) & REGSCRATCH)
            {
                dbprintf(("scratch overrun %08lx\n", mask));
                Assert(0);
            }
        }
    }

    if ((mask = ~mask & GetLockedScratch()) != 0) {
        asm_save_regs(mask);
        printf("; Saved locked: %08x\n", mask);
    }

    AllocDataRegisterAbs(&rd0, 4, RB_D0);
    AllocDataRegisterAbs(&rd1, 4, RB_D1);

    AddAuxSub(mop);

    if (s1 && s1->st_Type == ST_FltConst) {
        int32_t fpv[4];

        Assert(s1->st_Size == 4);
        asm_fltconst(exp, s1, fpv);
        AllocConstStor(&ss1, fpv[0], &LongType);
        s1 = &ss1;
    }
    if (s2 && s2->st_Type == ST_FltConst) {
        int32_t fpv[4];

        Assert(s2->st_Size == 4);
        asm_fltconst(exp, s2, fpv);
        AllocConstStor(&ss2, fpv[0], &LongType);
        s2 = &ss2;
    }

    if (s1 && s2) {
        if (SameRegister(s2, &rd0)) {       /*  s2 uses D0! */
            if (SameRegister(s1, &rd1)) {   /*  s1 uses D1! */
                /*
                 *  put in reversed registers
                 */

                asm_ext(exp, s1, &rd1, s1->st_Flags);
                asm_ext(exp, s2, &rd0, s2->st_Flags);
                if (orderReq > 0)
                    printf("\texg\tD0,D1\n");
            } else {
                if (orderReq > 0) {
                    asm_ext(exp, s2, &rd1, s2->st_Flags);
                    asm_ext(exp, s1, &rd0, s1->st_Flags);
                } else {
                    asm_ext(exp, s2, &rd0, s2->st_Flags);
                    asm_ext(exp, s1, &rd1, s1->st_Flags);
                }
            }
        } else {
            asm_ext(exp, s1, &rd0, s1->st_Flags);
            asm_ext(exp, s2, &rd1, s2->st_Flags);
            if (orderReq == 0)
                printf("\texg\tD0,D1\n");
        }
    } else if (s1) {
        asm_ext(exp, s1, &rd0, s1->st_Flags);
    } else if (s2) {
        asm_ext(exp, s2, &rd1, s2->st_Flags);
    }

    if (SmallCode)
        printf("\tjsr\t__%s(pc)\n", mop);
    else
        printf("\tjsr\t__%s\n", mop);

    if (d) {
        if (d->st_Type != ST_Reg || d->st_RegNo != RB_D0) {
            rd0.st_Size = d->st_Size;
            asm_move(exp, &rd0, d);
            /*printf("\tmove.l\tD0,%s\n", StorToString(d, NULL));*/
        }
    }

    FreeRegister(&rd0);
    FreeRegister(&rd1);

    asm_restore_regs(mask);
}

/*
 *  BITFIELDS
 */

void
asm_bfext(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    Type *t;
    Stor dtmp;
    Stor stmp;
    short tmpFlag;

    OptimizeBitField(s, &stmp);


    LockStorage(&stmp);
    if (d->st_Type==ST_Reg && d->st_RegNo<RB_ADDR && !SameRegister(&stmp, d)){
        dtmp = *d;
        tmpFlag = 0;
    } else {
        AllocDataRegister(&dtmp, d->st_Size);
        tmpFlag = 1;
    }
    UnlockStorage(&stmp);

    Assert(stmp.st_Size > 1);

    if (stmp.st_Flags & SF_UNSIGNED) {
        switch(stmp.st_Size) {
        case 2:
            t = &UShortType;
            break;
        default:
            t = &ULongType;
            break;
        }
    } else {
        switch(stmp.st_Size) {
        case 2:
            t = &ShortType;
            break;
        default:
            t = &LongType;
            break;
        }
    }

    dbprintf(("BitFieldExt %s[siz=%d] ->", StorToString(&stmp, NULL), stmp.st_Size));
    dbprintf(("%s (off=%d, siz=%d)\n", StorToString(&dtmp, NULL), stmp.st_BOffset, stmp.st_BSize));
    Assert(stmp.st_Flags & SF_BITFIELD);

    /*
     *  extract a bit field.  A 1 bit bitfield is extracted via a btst/scc/neg
     */

    if (stmp.st_BSize == 1) {
        Stor st;

        AllocConstStor(&st, 1 << stmp.st_BOffset, t);
        st.st_Size = stmp.st_Size;
        if (stmp.st_Flags & SF_UNSIGNED) {
            if (dtmp.st_Size != 1)
                asm_movei(exp, 0, &dtmp);
            asm_test_and(exp, &stmp, &st);
            asm_sccb(exp, &dtmp, COND_NEQ, 0);
        } else {
            asm_test_and(exp, &stmp, &st);
            asm_sccb(exp, &dtmp, COND_NEQ, 1);
            if (dtmp.st_Size != 1) {
                st = dtmp;
                st.st_Size = 1;
                asm_ext(exp, &st, &dtmp, 0);
            }
        }
    } else {
        Stor st;
        short d_size = dtmp.st_Size;
        int32_t mask;

        /*
         *  unsigned breakout optimization
         */

        dtmp.st_Size = stmp.st_Size;
        mask = ((uint32_t)-1 >> (32 - stmp.st_BSize)) << stmp.st_BOffset;

        /* if (stmp.st_Flags & SF_UNSIGNED) { */
            if (dtmp.st_Size != 4)
                asm_movei(exp, 0, &dtmp);
        /* } */
        AllocConstStor(&st, mask, t);               /*  mask source -> d */
        asm_and(exp, &stmp, &st, &dtmp);

        if (stmp.st_BOffset) {                      /*  shift it        */
            int32_t flags = dtmp.st_Flags;

            dtmp.st_Flags |= SF_UNSIGNED;
            AllocConstStor(&st, stmp.st_BOffset, t);
            asm_shift(exp, 1, &dtmp, &st, &dtmp);
            dtmp.st_Flags = flags;
        }

        /*
         *  if signed then sign-extend, else if unsigned done (0 fill
         *  above handles word case).  When signed, we must ensure that
         *  the entire result type is set negative, so we restore the
         *  destination size immediately.
         */

        if ((stmp.st_Flags & SF_UNSIGNED) == 0) {
            int32_t l1 = AllocLabel();

            AllocConstStor(&st, 1 << (stmp.st_BSize - 1), t);
            asm_test_and(exp, &st, &dtmp);
            asm_condbra(COND_EQ, l1);   /*  skip if not negative */
                                        /*  if negative, make so */
            AllocConstStor(&st, -1 << stmp.st_BSize, t);
            dtmp.st_Size = d_size;
            st.st_Size = d_size;
            asm_or(exp, &st, &dtmp, &dtmp);
            asm_label(l1);
        } else {
            dtmp.st_Size = d_size;
        }
    }
    if (tmpFlag) {
        asm_move(exp, &dtmp, d);
        FreeRegister(&dtmp);
    }
}

void
asm_bfsto(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    int32_t mask;
    Type *t;
    Stor dtmp;

    OptimizeBitField(d, &dtmp);

    mask = ((uint32_t)-1 >> (32 - dtmp.st_BSize)) << dtmp.st_BOffset;
    t = (dtmp.st_Size == 2) ? &UShortType : &ULongType;


    dbprintf(("BitFieldSto %s ->", StorToString(s, NULL)));
    dbprintf(("%s (off=%d, siz=%d)\n", StorToString(&dtmp, NULL), dtmp.st_BOffset, dtmp.st_BSize));
    Assert(dtmp.st_Flags & SF_BITFIELD);

    /*
     *  store into a bit field.
     */

    if (s->st_Type == ST_IntConst) {
        int32_t valu;
        Stor st;

        valu = mask & (s->st_IntConst << dtmp.st_BOffset);

        dbprintf(("valu %08lx\nmask %08lx\n", valu, mask));

        if (valu == 0) {
            AllocConstStor(&st, ~mask, t);
            asm_and(exp, &st, &dtmp, &dtmp);
        } else if (valu == mask) {
            AllocConstStor(&st, mask, t);
            asm_or(exp, &st, &dtmp, &dtmp);
        } else {
            AllocConstStor(&st, ~mask, t);
            asm_and(exp, &st, &dtmp, &dtmp);
            AllocConstStor(&st, valu, t);
            asm_or(exp, &st, &dtmp, &dtmp);
        }
    } else {
        Stor st;
        Stor tm;

        if (dtmp.st_BSize == 1) {     /*  test bit 0, bset/bclr   */
            int32_t l1 = AllocLabel();
            int32_t l2 = AllocLabel();

            AllocConstStor(&st, 1, t);
            st.st_Size = s->st_Size;
            asm_test_and(exp, &st, s);
            asm_condbra(COND_EQ, l1);
            AllocConstStor(&st, mask, t);
            asm_or(exp, &st, &dtmp, &dtmp);
            asm_branch(l2);
            asm_label(l1);
            AllocConstStor(&st, ~mask, t);
            asm_and(exp, &st, &dtmp, &dtmp);
            asm_label(l2);
        } else {
            Stor t2;
            AllocDataRegister(&tm, dtmp.st_Size);
            AllocDataRegister(&t2, dtmp.st_Size);

            if (s->st_Size < tm.st_Size) {
                asm_ext(exp, s, &tm, s->st_Flags);   /*  ext & mask      */
            } else {
                tm.st_Size = s->st_Size;
                asm_move(exp, s, &tm);
                tm.st_Size = dtmp.st_Size;
            }

            AllocConstStor(&st, mask >> dtmp.st_BOffset, t);
            asm_and(exp, &st, &tm, &tm);

            if (dtmp.st_BOffset) {                        /*  shift it        */
                AllocConstStor(&st, dtmp.st_BOffset, t);
                asm_shift(exp, 0, &tm, &st, &tm);
            }
            AllocConstStor(&st, ~mask, t);
            asm_and(exp, &st, &dtmp, &t2);                 /*  mask destination    */
            asm_or(exp, &tm, &t2, &dtmp);                  /*  or in changes       */

            FreeRegister(&t2);
            FreeRegister(&tm);
        }
    }
}

void
asm_bftst(exp, s)
Exp *exp;
Stor *s;
{
    Stor st;
    Stor stmp;
    int32_t mask;
    Type *t;

    OptimizeBitField(s, &stmp);
    t = (stmp.st_Size == 2) ? &UShortType : &ULongType;

    dbprintf(("BitFieldTst %s (off=%d, siz=%d)\n", StorToString(&stmp, NULL), stmp.st_BOffset, stmp.st_BSize));

    /*
     *  test a bit field.  A 1 bit bitfield is tested with btst
     */

    mask = ((uint32_t)-1 >> (32 - stmp.st_BSize)) << stmp.st_BOffset;
    AllocConstStor(&st, mask, t);
    asm_test_and(exp, &stmp, &st);
}

void
OptimizeBitField(s, d)
Stor *s;
Stor *d;
{
    *d = *s;

    if (d->st_Size == 4) {
        if (d->st_BOffset < 16 && d->st_BOffset + d->st_BSize < 16) {
            d->st_Size = 2;
            d->st_Offset += 2;
        } else if (d->st_BOffset > 16) {
            d->st_BOffset -= 16;
            d->st_Size = 2;
        }
    }
}

void
asm_illegal(void)
{
    puts("\tdc.w\t$4AFC");
}

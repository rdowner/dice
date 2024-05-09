/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASM1.C
 *
 *  Contains routine that actually generate assembly output.  Any given
 *  asm_ routine is only allow to allocate one data and one address register
 *
 *  Note: avoid storing a temporary into the destination and then running an
 *  operation on the destination when one of the sources uses the destination.
 *  e.g.    ptr = *ptr + 2
 */

/*
**      $Filename: asm1.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:34 $
**      $Log: asm1.c,v $
 * Revision 30.326  1995/12/24  06:09:34  dice
 * .
 *
 * Revision 30.156  1995/01/11  05:04:36  dice
 * Added GlobalRegReserved, which serves real function and is also used to
 * implement the MINIDICE hacker-did-the-obvious-thing thing.
 *
 * Revision 30.6  1994/08/04  04:49:26  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:37:20  dice
 * rco_move returned bad value
 *
 * Revision 30.0  1994/06/10  18:04:45  dice
 * .
 *
 * Revision 1.5  1994/04/15  21:16:17  jtoebes
 * Fixed code for dynamic stack allocations.
 *
 * Revision 1.4  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.3  1993/11/14  21:37:04  jtoebes
 * FIXED BUG01140 - DC1 is misaligning structure sizes.
 * Output a ds.w 0 to align strings to a word boundary.
 *
 *
**/

#include "defs.h"
#include "asm.h"

Prototype int32_t AsmState;
Prototype int32_t LabelReturn;
Prototype int32_t LabelProfBeg;
Prototype int32_t LabelProfEnd;
Prototype int32_t LabelRegsUsed;
Prototype int32_t RegReserved;
Prototype int32_t GlobalRegReserved;
Prototype char LastSectBuf[256];
Prototype char *LastSectName;
Prototype char *SegNames[];
Prototype char *SegTypes[];
Prototype short OutAlign;

char *SegNames[] = { "text", "data", "fardata", "bss", "farbss", "altdata", "altbss", "altcode", "combss", "farcombss" };
char *SegTypes[] = { "code", "data", "data",    "bss", "bss"   , "data",    "bss",    "code",    "common", "common" };

int32_t AsmState = 0;   /*  includes msb masking bits   */
int32_t  LabelStackUsed;
int32_t  LabelRegsUsed;
int32_t  LabelReturn;
int32_t  LabelRegMaskUsed;
int32_t  LabelArgsBytes;
int32_t  LabelProfBeg;
int32_t  LabelProfEnd;
int32_t  RegReserved;
int32_t  GlobalRegReserved = RF_A5|RF_A7;
char  LastSectBuf[256];
char *LastSectName;

short OutAlign;
short OAlgnReserved;

static Stor SOff  = { ST_RelReg, RB_SP, 4 };
static Stor SPush = { ST_Push, 0, 4 };
static Stor SCallerBlk = { ST_RelArg, RB_FP, 4 };

Prototype void asm_extern(Var *);
Prototype void asm_export(Symbol *);
Prototype void asm_procbegin(Var *);
Prototype void asm_regentry(Var *);
Prototype void asm_proclink(Var *);
Prototype void asm_procend(Var *, short);
Prototype void asm_returnstorage(Exp *);
Prototype void asm_dynamictag(Var *);   /* XXX */
Prototype void asm_segment(Var *);
Prototype void asm_align(int32_t);
Prototype void asm_ds(Symbol *, int32_t);
Prototype void asm_string(int32_t, char *, int32_t, int32_t, int32_t);
Prototype void asm_label(int32_t);
Prototype void asm_branch(int32_t);
Prototype void asm_condbra(short, int32_t);
Prototype int32_t asm_push(Exp *, Type *, Stor *);
Prototype void asm_push_mask(int32_t);
Prototype int32_t asm_stackbytes(Type *);
Prototype void asm_call(Exp *, Stor *, Type *, Stor *, int32_t, short);
Prototype void asm_pop(int32_t);
Prototype void asm_pop_mask(int32_t);
Prototype void asm_ext(Exp *, Stor *, Stor *, int32_t);
Prototype void asm_test(Exp *, Stor *);
Prototype void asm_test_scc(Exp *, int32_t, Stor *, short, Stor *);
Prototype void asm_cond_scc(Exp *, int32_t, Stor *, Stor *, short *, Stor *);
Prototype void asm_sccb(Exp *, Stor *, short, short);
Prototype void asm_clr(Exp *, Stor *);
Prototype void asm_movei(Exp *, int32_t, Stor *);
Prototype void asm_neg(Exp *, Stor *, Stor *);
Prototype void ThreeOp(Exp *, Stor *, Stor *, Stor *, void (*)(Exp *, Stor *, Stor *, Stor *));
Prototype void asm_add(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_sub(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_xor(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_and(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_or(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_test_and(Exp *, Stor *, Stor *);
Prototype void asm_switch(Exp *, int32_t, int32_t *, int32_t *, int32_t);
Prototype int32_t SizeFit(int32_t);
Prototype int32_t SizeFitSU(int32_t, Stor *);
Prototype void asm_end(void);
Prototype int32_t asm_rcomove(Exp *, char *, short, Stor *);
Prototype void asm_exg(Stor *, Stor *);

Prototype int32_t RegCallOrder(Type *, char *, char *);
Prototype void AutoAssignRegisteredArgs(Var **, short);

Local void asm_gen_logic_class(Exp *, char *, char *, int32_t, Stor *, Stor *, Stor *, int32_t);
Local void SortCases(Exp *, int32_t *, int32_t *, int32_t);
Local void SubDivideSwitch(Exp *, Stor *, Stor *, int32_t, int32_t, int32_t, int32_t, int32_t *, int32_t *);
Local void extop(int32_t, int32_t, Stor *);

void
asm_extern(var)
Var *var;
{
    if (var->var_Stor.st_Flags & SF_REGARGSUSED)
        printf("\txref\t@%s\n", SymToString(var->Sym));
    printf("\txref\t_%s\n", SymToString(var->Sym));
}

void
asm_export(sym)
Symbol *sym;
{
    printf("\txdef\t_%s\n", SymToString(sym));
}


void
asm_procbegin(var)
Var *var;
{
    char *prefix;

    LabelStackUsed  = AllocLabel();
    LabelReturn     = AllocLabel();
    LabelRegsUsed   = AllocLabel();
    LabelRegMaskUsed= AllocLabel();
    LabelArgsBytes  = AllocLabel();

    /*
     *  prefix, TF_REGCALL uses @
     *          TF_STKCALL uses _
     */

    if ((var->Flags & TF_REGCALL) && (var->Flags & TF_STKCALL))
    {
        dbprintf(("RegCall & StkCall!"));   /* XXX */
        Assert(0);
    }

    if (var->Flags & TF_REGCALL)
        prefix = "@";
    else
        prefix = "_";

    /*
     *  autoinit code for procedure
     */

    if (var->Flags & TF_AUTOINIT) {
        printf("\tsection\tautoinit1,code\n");
        printf("\tjsr\t_%s\n", SymToString(var->Sym));
        puts(LastSectBuf);
    } else if (var->Flags & TF_AUTOEXIT) {
        printf("\tsection\tautoexit1,code\n");
        printf("\tjsr\t_%s\n", SymToString(var->Sym));
        puts(LastSectBuf);
    }

    /*
     *  autoinit/exit code for profiling
     *
     *  Use address of label(s) after call to ID the routine(s)
     */

    if (ProfOpt && (var->Flags & (TF_NOPROF|TF_INTERRUPT)) == 0) {
        int32_t l1 = AllocLabel();

        LabelProfBeg = AllocLabel();
        LabelProfEnd = AllocLabel();

        AddAuxSub("ProfInit");      /*  ProfInit(rtname, pcbeg, pcend)  */
        AddAuxSub("ProfExec");      /*  ProfInit(rtname, pcbeg, pcend)  */
        printf("\tsection\tautoinit1,code\n");
        printf("\tlea\tl%d%s,A0\n", l1, (SmallData) ? "(A4)" : "");
        printf("\tjsr\t__ProfInit\n");

        printf("\tsection\tlibdata,data\n");
        printf("\tds.l\t0\n");

        printf("l%d\n", l1);
        printf("\tdc.l\t0\n");      /*  next pointer    */
        printf("\tdc.l\t0\n");      /*  siblings        */
        printf("\tdc.l\t0\n");      /*  our parent      */
        printf("\tdc.w\t%zd\n", (40 + strlen(prefix) + strlen(SymToString(var->Sym)) + (1 + 3)) & ~3);   /*  size of structure   */
        printf("\tdc.w\t0\n");
        printf("\tdc.l\t0\n");      /*  time stamp      */
        printf("\tdc.l\t0\n");      /*  accum time      */
        printf("\tdc.l\t0\n");      /*  total time      */
        printf("\tdc.l\t0\n");      /*  # of calls      */
        printf("\tdc.l\tl%d\n", LabelProfBeg);
        printf("\tdc.l\tl%d\n", LabelProfEnd);
        printf("\tdc.b\t\'%s%s\',0\n", prefix, SymToString(var->Sym));
        printf("\tds.l\t0\n");

        puts(LastSectBuf);
    }

    if (GenLinkOpt || ((GenStackOpt || (var->Flags & TF_STKCHECK)) && !(var->Flags & (TF_INTERRUPT|TF_NOSTKCHECK))))
        printf("\tprocstart\t0\n");     /*  enable opts except link/unlk */
    else
        printf("\tprocstart\n");        /*  enable optimizations */
    printf("\tds.w\t0\n");              /*  Align the code       */

    if (!(var->Flags & TF_STATIC))
        printf("\txdef\t%s%s\n", prefix, SymToString(var->Sym));

    printf("%s%s:\n", prefix, SymToString(var->Sym));

    if (ProfOpt && (var->Flags & (TF_NOPROF|TF_INTERRUPT)) == 0) {
        printf("\tjsr\t__ProfExec%s\n", (SmallCode) ? "(pc)" : "");
        printf("l%d\n", LabelProfBeg);
    }
    printf("\tmovem.l\tl%d,-(sp)\n", LabelRegMaskUsed);
}

/*
 *  REGCALLORDER()
 *
 *  Fill a register ordering array (ano) with the registers for a
 *  registerized procedure call.
 *
 *  Returns a mask of registers that conflict with currently allocated
 *  registers.  Would previously generate a conflict bit for byte sized
 *  address register quantities but no longer does so (has been fixed
 *  in GenRegArgs)
 */

int32_t
RegCallOrder(type, ano, prgno)
Type *type;
char *ano;
char *prgno;    /*  prefilled pragma regs or NULL   */
{
    short i;
    Var *var;
    int32_t regMask = 0;

    for (i = 0; i < type->Args && (var = type->Vars[i]); ++i) {
        short regNo = 0;

        if (prgno && prgno[i] != -1) {
            regNo = prgno[i];
        } else if (var->RegFlags & RF_REGISTER) {
            regNo = var->RegFlags & RF_REGMASK;
        } else {
            dbprintf(("RegFlags %d prgno %08lx prgno[i] %d\n",
                var->RegFlags,
                (int32_t)prgno,
                (prgno) ? prgno[i] : -1
            ));
            Assert(0);
        }
        regMask |= RegCallUseRegister(regNo);
        ano[i] = regNo;
#ifdef NOTDEF
        if (regNo >= RB_ADDR && *var->Type->Size == 1) {
            dbprintf(("; RegCallOrder ADREG/CHAR %d\n", regNo));
            regMask |= 1 << regNo;
        }
#endif
    }
    if (prgno) {
        short regNo;

        while (i < 16 && (regNo = prgno[i]) != -1) {
            ano[i] = regNo;
            regMask |= RegCallUseRegister(regNo);
            ++i;
        }
    }
    while (i < 16)
        ano[i++] = -1;
    return(regMask);
}

/*
 *  AUTOASSIGNREGISTEREDARGS()
 *
 *  When automatic registerized variables are enabled this routine
 *  will assign procedure arguments to registers.  This routine is
 *  also called for explicitly registerized procedure calls but
 *  in that case the registers are already assigned and we have an
 *  effective nop.
 */

void
AutoAssignRegisteredArgs(Var **vars, short args)
{
    short i;
    int32_t regMask = 0;
    Var *var;

    for (i = 0; i < args && (var = vars[i]); ++i) {
        Assert(i < 16);

        if ((var->RegFlags & RF_REGISTER) == 0) {
            int32_t pick = 0;

            if (var->Type->Id == TID_INT) {
                if ((regMask & RF_D0) == 0)
                    pick = RB_D0;
                else if ((regMask & RF_D1) == 0)
                    pick = RB_D1;
                else if ((regMask & RF_A0) == 0)
                    pick = RB_A0;
                else if ((regMask & RF_A1) == 0)
                    pick = RB_A1;
                else
                    Assert(0);
            } else {
                if ((regMask & RF_A0) == 0)
                    pick = RB_A0;
                else if ((regMask & RF_A1) == 0)
                    pick = RB_A1;
                else if ((regMask & RF_D0) == 0)
                    pick = RB_D0;
                else if ((regMask & RF_D1) == 0)
                    pick = RB_D1;
                else
                    Assert(0);
            }
            var->RegFlags = pick | RF_REGISTER;
        }
        regMask |= 1 << (var->RegFlags & RF_REGMASK);
    }
}

/*
 *  returns whether a register swap had to be done (so GEN.C can invalidate
 *  the variable as reservable since it has been effectively modified)
 */

int32_t
asm_rcomove(Exp *exp, char *ano, short i, Stor *d)
{
    Stor t;
    short swap = -1;
    short si;

    t.st_Flags= SF_TMP;
    t.st_Type = ST_Reg;
    t.st_RegNo= ano[i];
    t.st_Size = d->st_Size;

    if (d->st_Type != ST_Reg) {
        asm_move(exp, &t, d);
        ano[i] = 0;
        return(0);
    }

    if (d->st_RegNo != ano[i]) {
        for (si = 0; si < 16; ++si) {
            if (d->st_RegNo == ano[si]) {
                swap = si;
                break;
            }
        }
        if (swap < 0) {     /*  destination contents garbage    */
            outop("move", 4, &t, d);
        } else {            /*  destination contents precious   */
            outop("exg", 4, &t, d);
            ano[si] = t.st_RegNo;
        }
        ano[i] = -1;
    }
    return(swap);
}

void
asm_exg(s1, s2)
Stor *s1;
Stor *s2;
{
    if (SameStorage(s1, s2) == 0)
        outop("exg", 4, s1, s2);
}

#ifdef NOTDEF
/*
 *  redefine the movem label.. make the old one nil (effecively move
 *  the movem to *after* the registers are loaded)
 */

void
asm_regentry(var)
Var *var;
{
    char *nam = SymToString(var->Sym);

    printf("l%d\teqr\n", LabelRegMaskUsed);
    printf("\txdef\t@%s\n", nam);
    printf("@%s:\n", nam);

    LabelRegMaskUsed= AllocLabel();
    printf("\tmovem.l\tl%d,-(sp)\n", LabelRegMaskUsed);
}
#endif

void
asm_proclink(var)
Var *var;
{
    if (var->Flags & (TF_INTERRUPT|TF_GETA4)) {
        if (var->Flags & TF_GETA4) {
            RegReserved |= REGSDPTR;    /*  XXX remove me   */
            AddAuxSub("ABSOLUTE_BAS");
            if (PIOpt) {
                printf("\tlea\t__ABSOLUTE_BAS(pc),A4\n"
                       "\tlea\t32766(A4),A4\n"
                      );
            } else {
                printf("\tlea\t__ABSOLUTE_BAS+32766,A4\n");
                if (ResOpt)
                    yerror(var->LexIdx, EERROR_GETA4_ILLEGAL);
            }

        }
        /* int code? */
    }

    /*
     *  If -gs is specified do stack checking, otherwise just do a simple
     *  link instruction.
     *
     *  In order to do stack checking we have to simulate the link
     *  instruction and modify A5 instead of SP, then check A6, then
     *  EXG the two if we are still ok.  We cannot store the link
     *  equivalent into A7 until we know it is safe.
     */

    if ((GenStackOpt || (var->Flags & TF_STKCHECK)) && !(var->Flags & (TF_INTERRUPT|TF_NOSTKCHECK))) {
        int32_t l = AllocLabel();

        AddAuxSub("stk_base");
        AddAuxSub("stk_alloc");

        /*
         *  simulate link instruction but with A5 and A7 swapped
         */
        printf("\tmove.l\tA%d,-(sp)\n", RB_FP - RB_ADDR);
        printf("\tlea\t-l%d(sp),A%d\n", LabelStackUsed, RB_FP - RB_ADDR);
        printf("\tcmp.l\t__stk_base%s,A%d\n",
            ((SmallData) ? "(A4)" : ""),
            RB_FP - RB_ADDR
        );
        printf("\tbhi\tl%d\n", l);

        /*
         *  Call __stk_alloc() (name changed from __stack_alloc() to avoid
         *  incompatible library functions after dynamic stacks were fixed).
         *  __stk_alloc() requires the number of bytes of arguments and
         *  the number of bytes of saved registers in order to be able to
         *  properly allocate a new stack frame and copy the appropriate
         *  information.
         *
         *  __stk_alloc() will replace the return vector and saved frame
         *  pointer in the new frame to point to __stk_free(), which
         *  restores the old frame.
         *
         *  NOTE: The minimum argument size for K&R procedures is 256 bytes
         *  more then the specified arguments in order to accomodate
         *  var-args functions.  ANSI prototyped procedures are much more
         *  efficient, only copying the actual number of arguments, plus
         *  256 more bytes if the (, ...) form is used.
         */

        {
            BlockStmt *block = var->u.Block;
            int32_t argBytes = block->Frame.ArgsStackUsed;

            if (var->Flags & TF_PROTOTYPE) {
                if (var->Flags & TF_DOTDOTDOT)
                    argBytes += 256;
            } else {
                argBytes += 256;
            }
            if (argBytes < 32768)
                printf("\tpea\t%d.W\n", argBytes);
            else
                printf("\tpea\t%d\n", argBytes);
        }
        printf("\tpea\tl%d-8.W\n", LabelRegsUsed);
        printf("\tjsr\t__stk_alloc%s\n", (SmallCode) ? "(pc)" : "");

        /*
         *  exg A5, A7 to 'make it right'
         */
        printf("l%d\texg\tsp,A%d\n", l, RB_FP - RB_ADDR);
    } else {
        printf("\tlink\tA%d,#-l%d\n", RB_FP - RB_ADDR, LabelStackUsed);
    }
}

void
asm_procend(Var *var, short forceLink)
{
    BlockStmt *block = var->u.Block;
    short count;
    short dummy;

    int32_t stack = Align(block->Frame.DownStackUsed + block->Frame.StackUsed, StackAlign);
    int32_t mask = GetUsedRegisters() & ~RegReserved;

    if (var->Flags & (TF_INTERRUPT)) {
        if (SmallData != 2)
            mask |= REGSDPTR;
        if (block->Frame.Flags & FF_CALLMADE)
            mask |= REGSCRATCH;
    } else {
        mask &= ~REGSCRATCH;
    }
    if (var->Flags & TF_GETA4)
        mask |= REGSDPTR;

    if (stack > 32767)
        yerror(var->LexIdx, EERROR_TOO_MANY_AUTOS);

    /*
     *  return, unlk, restore regs, rts.
     */

    asm_label(LabelReturn);
    printf("\tunlk\tA%d\n", RB_FP - RB_ADDR);
    count = asm_restore_regs(mask);

    if (var->Flags & TF_INTERRUPT) {
        printf("\trte\n");
    } else {
        if (ProfOpt && (var->Flags & TF_NOPROF) == 0) {
            printf("\tjsr\t__ProfExec%s\n", (SmallCode) ? "(pc)" : "");
            printf("l%d\n", LabelProfEnd);
        }
        printf("\trts\n");
    }

    printf("l%d\treg\t%s\n", LabelRegMaskUsed, RegMaskToString(mask, &dummy));
    printf("l%d\tequ\t%d\n", LabelRegsUsed, 8 + count * 4);
    printf("l%d\tequ\t%d\n", LabelStackUsed, stack);
    printf("\tprocend");
    if (forceLink)
        printf(" forcelink");
    puts("");
}

void
asm_push_mask(mask)
int32_t mask;
{
    if (mask) {
        short n;
        char *ptr = RegMaskToString(mask, &n);

        printf("\tmove%s.l\t%s,-(sp)\n", ((n > 1) ? "m" : ""), ptr);
    }
}

void
asm_pop_mask(mask)
int32_t mask;
{
    if (mask) {
        short n;
        char *ptr = RegMaskToString(mask, &n);

        printf("\tmove%s.l\t(sp)+,%s\n", ((n > 1) ? "m" : ""), ptr);
    }
}

void
asm_returnstorage(exp)
Exp *exp;
{
    Stor sd0;
    Stor sd1;
    Stor sa0;
    Stor sa1;
    Stor *s = &exp->ex_Stor;

    switch(exp->ex_Type->Id) {
    case TID_STRUCT:
    case TID_UNION:
        {
            int label = AllocLabel();

            /*
             *  structures and unions are returned to storage allocated by
             *  the caller.  The caller passes a pointer to this storage
             *  as the first argument in the call, which is -4 relative
             *  ST_RelArg/fp (asubs.c will modify the basic RelArg offset
             *  so 0 always points to the first user arguments)
             *
             *  Note that if the caller expects no return data it will
             *  pass NULL for the pointer, in which case we do NOT
             *  copy the return structure.
             */

            /*
             *  used to allocate A0 but this is illegal -- A0 might already
             *  be used to hold the source of the copy.
             */

            AllocAddrRegister(&sa0);
            SCallerBlk.st_Offset = -4;
            asm_move(exp, &SCallerBlk, &sa0);

            asm_test(exp, &sa0);
            asm_condbra(COND_EQ, label);

            sa0.st_Offset = 0;
            sa0.st_Type = ST_RelReg;
            sa0.st_Size = s->st_Size;
            asm_move(exp, s, &sa0);
            FreeRegister(&sa0);
            asm_label(label);
        }
        break;
    case TID_FLT:
        AllocDataRegisterAbs(&sd0, 4, RB_D0);
        AllocDataRegisterAbs(&sd1, 4, RB_D1);

        if (s->st_Type == ST_FltConst) {
            int32_t ary[4];

            asm_fltconst(exp, s, ary);

            switch(s->st_Size) {
            case 16:
                AllocAddrRegisterAbs(&sa0, RB_A0);
                AllocAddrRegisterAbs(&sa1, RB_A1);
                asm_movei(exp, ary[3], &sa1);
                asm_movei(exp, ary[2], &sa0);
                FreeRegister(&sa0);
                FreeRegister(&sa1);
            case 8:
                asm_movei(exp, ary[1], &sd1);
            case 4:
                asm_movei(exp, ary[0], &sd0);
                break;
            default:
                Assert(0);
            }
        } else {
            char *sstr = StorToString(s, NULL);

            switch(s->st_Size) {
            case 4:
                printf("\tmove.l\t%s,D0\n", sstr);
                break;
            case 8:
                printf("\tmovem.l\t%s,D0/D1\n", sstr);
                break;
            case 16:
                AllocAddrRegisterAbs(&sa0, RB_A0);
                AllocAddrRegisterAbs(&sa1, RB_A1);
                printf("\tmovem.l\t%s,D0/D1/A0/A1\n", sstr);
                FreeRegister(&sa0);
                FreeRegister(&sa1);
                break;
            default:
                Assert(0);
            }
        }
        FreeRegister(&sd0);
        FreeRegister(&sd1);
        break;
    case TID_INT:               /*  size 4  D0      */
    case TID_PTR:
        Assert(s->st_Size == 4);
        AllocDataRegisterAbs(&sd0, 4, RB_D0);
        asm_move(exp, s, &sd0);
        FreeRegister(&sd0);
        break;
    default:
        yerror(exp->ex_LexIdx, EERROR_UNSUPPORTED_RETURN_TYPE);
        break;
    }
}

#ifdef DYNAMIC
/*
 *  geneate dynamic tag code, variable is a pointer to the procedure
 *  or variable we dynamically link in run-time.
 *
 *  procedure:  _GetHyperFunc("routine", "_stk_")
 *  variable:   _GetHyperVar("variable")
 *
 */

void
asm_dynamictag(var)
Var *var;
{
    int32_t l1 = AllocLabel();
    int32_t l2 = AllocLabel();
    int32_t l3 = AllocLabel();
    short isVar = (var->Type->SubType->Id != TID_PROC);

    /*
     *  autoinit code
     */

    printf("\tsection\tautoinit1,code\n");
    printf("\txref\t__AutoFail1\n");
    printf("\txref\t@_%s\n",
        ((isVar) ? "GetHyperVar" : "GetHyperFunc")
    );
    if (isVar == 0)
        printf("\tlea\tl%d(pc),A1\n", l2);
    printf("\tlea\tl%d(pc),A0\n", l1);
    printf("\tjsr\t@_%s%s\n",
        ((isVar) ? "GetHyperVar" : "GetHyperFunc"),
        ((PIOpt) ? "(pc)" : "")
    );
    printf("\tmove.l\tD0,_%s%s\n",
        SymToString(var->Sym),
        ((SmallData) ? "(A4)" : "")
    );
    printf("\tbeq\t__AutoFail1\n");
    printf("\tbra\tl%d\n", l3);
    printf("l%d\tdc.b\t'_%s',0\n", l1, SymToString(var->Sym));
    if (isVar == 0)
        printf("l%d\tdc.b\t'_stk_',0\n", l2);
    printf("\tds.w\t0\n");
    printf("l%d\n", l3);

    /*
     *  autoexit code to release dynamically referenced variable or routine
     */

    printf("\tsection\tautoexit1,code\n");
    printf("\txref\t@_%s\n",
        ((isVar) ? "RelsHyperVar" : "RelsHyperFunc")
    );
    printf("\tmove.l\t_%s%s,A0\n",
        SymToString(var->Sym),
        ((SmallData) ? "(A4)" : "")
    );
    printf("\tjsr\t@_%s%s\n",
        ((isVar) ? "RelsHyperVar" : "RelsHyperFunc"),
        ((PIOpt) ? "(pc)" : "")
    );

    puts(LastSectBuf);
}
#endif

void
asm_segment(var)
Var *var;
{
    int32_t mask;
    int32_t state;
    char *segName;
    char *segType;
    char *sn;

    if (var->Flags & TF_CHIP)
        mask = 0x40000000;
    else
        mask = 0;

    if (var->var_Stor.st_Flags & SF_CODE) {
        if (var->Flags & TF_AUTOINIT) {     /*  __autoinit const only   */
            segName = SegNames[7];
            segType = SegTypes[7];
        } else {
            segName = SegNames[0];
            segType = SegTypes[0];
        }
        state = ASM_CODE;
    } else {
        int32_t idx;

        if (var->u.AssExp)
            state = ASM_DATA;
        else
            state = ASM_BSS;

        if (var->Flags & TF_AUTOINIT) {
            if (var->u.AssExp)
                idx = 5;
            else
                idx = 6;
        } else if ((var->Flags & TF_FAR) || (SmallData == 0 && !(var->Flags & TF_NEAR))) {
            if (var->u.AssExp)
                idx = 2;
            else if (UnixCommonOpt && (var->Flags & TF_STATIC) == 0)
                idx = 9;
            else
                idx = 4;
        } else {
            if (var->u.AssExp)
                idx = 1;
            else if (UnixCommonOpt && (var->Flags & TF_STATIC) == 0)
                idx = 8;
            else
                idx = 3;
        }


        segName = SegNames[idx];
        segType = SegTypes[idx];
    }
    state |= mask;

    sn = LastSectBuf;

    if (AsmState != state || LastSectName != segName) {
        if (mask)
            sprintf(sn, "\tsection %s,%s,$%08x\n", segName, segType, mask);
        else
            sprintf(sn, "\tsection %s,%s\n", segName, segType);
        puts(sn);
        AsmState = state;
        LastSectName = segName;
        OutAlign = 1;
    }
}

void
asm_align(int32_t size)
{
    switch(size) {
    case 1:
        break;
    case 2:
        if (OutAlign & 1)
            printf("\tds.w 0\n");
        OutAlign = 2;
        break;
    case 4:
    default:
        if (OutAlign & 3)
            printf("\tds.l 0\n");
        OutAlign = 4;
        break;
    case 8:
        if (OutAlign & 7)
            printf("\tds.q 0; non-standard\n");
        OutAlign = 8;
        break;
    }
}

/*
 *  Generate a string ref.  An internationalizable reference generates a
 *  table reference (and the string is forced to be in a code section elsewhere
 *  in the code)
 */

void
asm_string(label, str, bytes, flags, iidx)
int32_t label;
char *str;
int32_t bytes;
int32_t flags;
int32_t iidx;
{
    short i = 0;
    char buf[128];
    char *p = buf;

    if (iidx >= 0) {
        int32_t newLabel = AllocLabel();

        printf("\tsection\tilocale,data\n");
        printf("l%d\tdc.l\tl%d,%d\n", label, newLabel, iidx);
        puts(LastSectBuf);
        label = newLabel;
    }

    if (flags & (TF_CONST | TF_SHARED))
        asm_segment(&DummyCodeVar);
    else
        asm_segment(&DummyDataVar);

    printf("\tds.w\t0\n");  /* Word align the string */

    *p++ = 'l';
    p = itodec(p, label);
    *p++ = '\n';

    while (bytes--) {
        if (i) {
            *p++ = ',';
        } else {
            strcpy(p, "\n\tdc.b\t");
            p += 7;
        }
        *p++ = '$';
        p = itohex(p, *str++);
        if (++i == 12) {
            fwrite(buf, p - buf, 1, stdout);
            p = buf;
            i = 0;
        }
    }
    *p++ = '\n';
    fwrite(buf, p - buf, 1, stdout);
}

void
asm_label(label)
int32_t label;
{
    char buf[32];
    char *ptr;

    buf[0] = 'l';
    ptr = itodec(buf + 1, label);
    *ptr = '\n';
    ++ptr;
    fwrite(buf, 1, ptr - buf, stdout);
}

/*
 *  BRANCHING.  NOTE NOTE NOTE.  Whenever we branch we must check
 *  for forced registers which must be restored.
 */

void
asm_branch(label)
int32_t label;
{
    printf("\tbra\tl%d\n", label);
}

void
asm_condbra(short cond, int32_t label)
{
    char *str = "bad";

    switch(cond) {
    case COND_LT:
        str = "blt";
        break;
    case COND_LTEQ:
        str = "ble";
        break;
    case COND_GT:
        str = "bgt";
        break;
    case COND_GTEQ:
        str = "bge";
        break;
    case COND_EQ:
        str = "beq";
        break;
    case COND_NEQ:
        str = "bne";
        break;
    case CF_UNS|COND_LT:
        str = "bcs";
        break;
    case CF_UNS|COND_LTEQ:
        str = "bls";
        break;
    case CF_UNS|COND_GT:
        str = "bhi";
        break;
    case CF_UNS|COND_GTEQ:
        str = "bcc";
        break;
    case CF_UNS|COND_EQ:
        str = "beq";
        break;
    case CF_UNS|COND_NEQ:
        str = "bne";
        break;

    case -COND_LT:
        str = "bge";
        break;
    case -COND_LTEQ:
        str = "bgt";
        break;
    case -COND_GT:
        str = "ble";
        break;
    case -COND_GTEQ:
        str = "blt";
        break;
    case -COND_EQ:
        str = "bne";
        break;
    case -COND_NEQ:
        str = "beq";
        break;
    case -(CF_UNS|COND_LT):
        str = "bcc";
        break;
    case -(CF_UNS|COND_LTEQ):
        str = "bhi";
        break;
    case -(CF_UNS|COND_GT):
        str = "bls";
        break;
    case -(CF_UNS|COND_GTEQ):
        str = "bcs";
        break;
    case -(CF_UNS|COND_EQ):
        str = "bne";
        break;
    case -(CF_UNS|COND_NEQ):
        str = "beq";
        break;
    case COND_BPL:
        str = "bpl";
        break;
    case COND_BMI:
        str = "bmi";
        break;
    default:
        dbprintf(("Unknown cond %d\n", cond));
        Assert(0);
        break;
    }
    printf("\t%s\tl%d\n", str, label);
}


/*
 *  push args onto stack, return bytes pushed.
 */

int32_t
asm_push(exp, type, s)
Exp *exp;
Type *type;
Stor *s;
{
    Stor t;

    if (s->st_Type == ST_IntConst) {
        if (s->st_IntConst >= -32768 && s->st_IntConst < 32768) {
            if (s->st_IntConst == 0)
                printf("\tclr.l\t-(sp)\n");
            else
                printf("\tpea\t%d.W\n", s->st_IntConst);
        } else {
            outop("move", 4, s, &SPush);
        }
        return(4);
    }

    if (s->st_Type == ST_FltConst) {
        int32_t fval[4];
        short i;

        asm_fltconst(exp, s, fval);
        for (i = (s->st_Size >> 2) - 1; i >= 0; --i) {
            int32_t v = fval[i];
            if (v >= -32768 && v < 32768)
                printf("\tpea\t$%08x.W\n", v);
            else
                printf("\tpea\t$%08x\n", v);
        }
        return(s->st_Size);
    }

    /*
     *  note, IntConst case already handled.
     */

    if (type->Id == TID_ARY || type->Id == TID_PROC || (s->st_Flags & SF_LEA)) {
        outop("pea", 4, NULL, s);
        return(4);
    }

    /*
     *  pointer, int32_t, 4 byte structure
     */

    if (s->st_Size == 4) {
        outop("move", 4, s, &SPush);
        return(4);
    }

    /*
     *  structure or fp number, guarenteed to be word aligned.  Push
     *  exact size.
     */

    if (type->Id != TID_INT) {
        int32_t bytes = s->st_Size;

        if (s->st_Size < 4) {
            outop("move", s->st_Size, s, &SPush);
            return(s->st_Size);
        }
        if ((s->st_Type < ST_RelReg || s->st_Type > ST_RegIndex) && s->st_Type != ST_PtrConst)
        {
            dbprintf(("bad type in asm_push: %d\n", s->st_Type));
            Assert(0);
        }

        if (bytes <= LGBO_SIZE && s->st_Offset >= -32768 + LGBO_SIZE && s->st_Offset < 32767 - LGBO_SIZE) {
            int32_t oldOffset = s->st_Offset;

            s->st_Offset += bytes;
            while (bytes >= 4) {
                bytes -= 4;
                s->st_Offset -= 4;
                outop("move", 4, s, &SPush);
            }
            s->st_Offset = oldOffset;
            if (bytes)
                outop("move", bytes, s, &SPush);
        } else {
            int32_t label = AllocLabel();
            Stor a;

            if (bytes & 1)
                yerror(exp->ex_LexIdx, EERROR_PASS_UNALIGNED_STRUCT);

            LockStorage(s);
            AllocDataRegister(&t, 4);
            AllocAddrRegister(&a);
            asm_lea(exp, s, bytes, &a);         /*  end of struct   */
            asm_movei(exp, bytes >> 2, &t);     /*  # of longwords  */
            if (bytes & 2)
                printf("\tmove.w\t-(A%c),-(sp)\n", a.st_RegNo - RB_ADDR + '0');
            asm_label(label);
            printf("\tmove.l\t-(A%c),-(sp)\n", a.st_RegNo - RB_ADDR + '0');
            printf("\tsubq.l\t#1,%s\n", StorToString(&t, NULL));
            asm_condbra(COND_NEQ, label);

            FreeRegister(&t);
            FreeRegister(&a);
            UnlockStorage(s);
        }
        return(s->st_Size);
    }

    /*
     *  < sizeof(int), optimizeo.  push an entire longword quantity even if
     *  the upper bits would be garbage.
     */

    Assert(s->st_Size < 4);

    if (s->st_Type == ST_Reg) {
        outop("move", 4, s, &SPush);
        return(4);
    }

    /*
     *  safe to push extranious garbage (assuming offset lw aligned)
     */

    if (s->st_Type == ST_RelArg || (s->st_RegNo == RB_FP && s->st_Type == ST_RelReg)) {
        int32_t offset = s->st_Offset + s->st_Size - 4;
        if ((offset & 1) == 0) {
            s->st_Offset = offset;
            outop("move", 4, s, &SPush);
            s->st_Offset -= s->st_Size - 4;
            return(4);
        }
    }

    /*
     *  note safe to push extranious garbage from before lvalue
     */

    switch(s->st_Size) {
    case 1:
        printf("\tsubq.l\t#4,sp\n");
        SOff.st_Offset = 3;
        outop("move", 1, s, &SOff);
        break;
    case 2:
        outop("move", 2, s, &SPush);
        printf("\tsubq.l\t#2,sp\n");
        break;
    default:
        dbprintf(("unexpected push size %d\n", s->st_Size));
        Assert(0);
    }
    return(4);
}

/*
 *  call s placing the return value into d.  d can be NULL
 *
 *  Note that d might also have been part of the registers saved,
 *  so we have to restore before moving the result.  Except D0 might
 *  have been part of the registers saved as well so in this special
 *  case we must move D0 into a temporary register before restoring.
 */

int32_t
asm_stackbytes(rtype)
Type *rtype;
{
    int32_t rtbytes = Align(*rtype->Size, StackAlign);

    if (rtbytes > 0 && rtbytes < 4)
        rtbytes = 4;
    return(rtbytes);
}

/*
 *  asm_call()  make subroutine call
 *
 *  autopush:   0x01    push result back onto stack
 *              0x02    regargs call
 *
 *  note that autopush & 1 is never set for structural return types
 */

void
asm_call(Exp *exp, Stor *s, Type *rtype, Stor *d, int32_t bytes, short autopush)
{
    int32_t rtbytes;

    if (rtype->Id == TID_STRUCT || rtype->Id == TID_UNION) {
        Stor t;

        if (d) {
            asm_getlea(exp, d, &t);
            asm_push(exp, &VoidPtrType, &t);
            FreeStorage(&t);
        } else {
            AllocConstStor(&t, 0, &LongType);
            asm_push(exp, &VoidPtrType, &t);
        }
        bytes += 4;
    }

    outop("jsr", -1, NULL, s);

    if (autopush & 1) {
        char *spnam;

        rtbytes = asm_stackbytes(rtype);

        if (bytes == rtbytes) {
            spnam = "(sp)";
        } else {
            spnam = "-(sp)";
            asm_pop(bytes);
        }

        switch(rtbytes) {
        case 4:
            printf("\tmove.l\tD0,");
            break;
        case 8:
            printf("\tmovem.l\tD0/D1,");
            break;
        case 16:
            printf("\tmovem.l\tD0/D1/A0/A1,");
            break;
        default:
            yerror(exp->ex_LexIdx, EERROR_UNSUPPORTED_RETURN_TYPE);
            break;
        }
        puts(spnam);
    } else {
        asm_pop(bytes);
    }

    if (d) {
        Stor sd0;
        Stor sd1;
        Stor sa0;   /*  int32_t double D0/D1/A0/A1  */
        Stor sa1;

        switch(rtype->Id) {
        case TID_STRUCT:
        case TID_UNION:
            /*
             *  structural return storage is written directly to the
             *  destination by the caller, so we need do nothing more.
             */
            break;
        case TID_FLT:
            if (d->st_Size > 4) {
                AllocDataRegisterAbs(&sd0, d->st_Size, RB_D0);
                AllocDataRegisterAbs(&sd1, d->st_Size, RB_D1);

                switch(d->st_Size) {
                case 8:
                    printf("\tmovem.l\tD0/D1,%s\n", StorToString(d, NULL));
                    break;
                case 16:
                    AllocAddrRegisterAbs(&sa0, RB_A0);
                    AllocAddrRegisterAbs(&sa1, RB_A1);
                    printf("\tmovem.l\tD0/D1/A0/A1,%s\n", StorToString(d, NULL));
                    FreeRegister(&sa0);
                    FreeRegister(&sa1);
                    break;
                default:
                    Assert(0);
                }

                FreeRegister(&sd0);
                FreeRegister(&sd1);
                break;
            }
            /* fall through single prec fp  */
        case TID_INT:
        case TID_PTR:
            AllocDataRegisterAbs(&sd0, d->st_Size, RB_D0);
            asm_move(exp, &sd0, d);
            FreeRegister(&sd0);
            break;
        default:
            yerror(exp->ex_LexIdx, EERROR_UNSUPPORTED_RETURN_TYPE);
            break;
        }
    }
}

void
asm_pop(n)
int32_t n;
{
    if (n) {
        if (n <= 8) {
            printf("\taddq.l\t#%d,sp\n", n);
            return;
        }
        if (n < 32768) {
            printf("\tlea\t%d(sp),sp\n", n);
            return;
        }
        printf("\taddi.l\t#%d,sp\n", n);
    }
}

/*
 *  signed/unsigned extend w/ major optimizeo.  Also handles truncation
 *
 *  NOTE: case with data register as destination assumed never to
 *  allocate temporary registers (by asm_div, because we use D0/D1
 *  without allocating them).
 */

void
asm_ext(exp, s, d, sflags)
Exp *exp;
Stor *s, *d;
int32_t sflags;
{
    Stor stor;
    Stor stor2;
    int32_t saveDSize = d->st_Size;

    sflags &= SF_UNSIGNED;
    if (s->st_Type == ST_IntConst) {
        int32_t v = s->st_IntConst;
        if (d->st_Size == 2)
            v &= 0xFFFF;
        if (d->st_Size == 1)
            v &= 0xFF;
        asm_movei(exp, v, d);
        return;
    }

    /*
     *  If source is an address register cannot use .b size.  Must move into
     *  data register.  However, if destination is .b sized and also a
     *  register, we can simply move it to the destination and be done
     *  with it.
     */

    if (d->st_Size == 1 && s->st_Type == ST_Reg && s->st_RegNo >= RB_ADDR) {

        /*
         *  If register-register we don't care if we use more of the
         *  destination register and this saves having to move the
         *  address register into a temporary
         */

        if (d->st_Type == ST_Reg) {
            int32_t saveSSize = s->st_Size;

            d->st_Size = s->st_Size = 2;
            asm_move(exp, s, d);
            s->st_Size = saveSSize;
            d->st_Size = saveDSize;
            return;
        }

        /*
         *  s is an address register, destination size is 1.  Use .w move
         *  since can't use .b move.  Since the destination is not a
         *  register we have to allocate one.
         */

        AllocDataRegister(&stor2, 2);
        s->st_Size = 2;
        asm_move(exp, s, &stor2);
        s->st_Size = 1;
        stor2.st_Size = 1;
        FreeRegister(&stor2);
        s = &stor2;
    }

    /*
     *  If destination is address register cannot use .b size.  Easy
     *  fix is to temporarily force destination size to 2.
     */

    if (d->st_Size == 1 && d->st_Type == ST_Reg && d->st_RegNo >= RB_ADDR)
        d->st_Size = 2;

    if (s->st_Size > d->st_Size) {

        stor = *s;

        switch(stor.st_Type) {
        case ST_PtrConst:
        case ST_RelArg:
        case ST_RelReg:
        case ST_RegIndex:
        case ST_RelLabel:
        case ST_RelName:
            stor.st_Offset += s->st_Size - d->st_Size;
            break;
        }
        stor.st_Size = d->st_Size;
        asm_move(exp, &stor, d);
    } else if (s->st_Size == d->st_Size) {
        asm_move(exp, s, d);
    } else if (SameStorage(s, d) && s->st_Type == ST_Reg) {
        if (s->st_RegNo >= RB_ADDR)
        {
            dbprintf(("asm_ext : ext An"));  /*  An  */
            Assert(0);
        }
        if (sflags) {
            AllocConstStor(&stor, (1 << 8*s->st_Size) - 1, &LongType);
            stor.st_Size = d->st_Size;
            outop("andi", d->st_Size, &stor, d);
        } else {
            extop(s->st_Size, d->st_Size, d);
        }
    } else if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR && !SameRegister(s, d)) {
        if (sflags) {
            int32_t size = d->st_Size;

            LockStorage(s);
            asm_movei(exp, 0, d);
            UnlockStorage(s);
            d->st_Size = s->st_Size;
            asm_move(exp, s, d);
            d->st_Size = size;
        } else {
            int32_t size = d->st_Size;
            d->st_Size = s->st_Size;
            asm_move(exp, s, d);
            d->st_Size = size;
            extop(s->st_Size, d->st_Size, d);
        }
    } else {
        if (sflags) {
            LockStorage(s);
            AllocDataRegister(&stor, 4);
            asm_movei(exp, 0, &stor);
            UnlockStorage(s);
            stor.st_Size = s->st_Size;
            asm_move(exp, s, &stor);
            stor.st_Size = d->st_Size;
            asm_move(exp, &stor, d);
            FreeRegister(&stor);
        } else {
            AllocDataRegister(&stor, s->st_Size);
            asm_move(exp, s, &stor);
            stor.st_Size = d->st_Size;

            extop(s->st_Size, d->st_Size, &stor);
            asm_move(exp, &stor, d);
            FreeRegister(&stor);
        }
    }
    d->st_Size = saveDSize;
}

void
extop(size1, size2, d)
int32_t size1;
int32_t size2;
Stor *d;
{
    if (size1 == 1 && size2 > 2) {
        if (MC68020Opt) {
            outop("extb", size2, NULL, d);
            return;
        }
        outop("ext", 2, NULL, d);
    }
    outop("ext", size2, NULL, d);
}


void
asm_test(exp, s)
Exp *exp;
Stor *s;
{
    if (s->st_Flags & SF_BITFIELD) {
        asm_bftst(exp, s);
        return;
    }

    if (s->st_Type == ST_IntConst)
    {
        dbprintf(("asm_test: integer constant %d", s->st_IntConst));
        Assert(0);
    }
    if ((s->st_Type == ST_Reg && s->st_RegNo >= RB_ADDR) || (s->st_Flags & SF_LEA) || ((s->st_Type == ST_RelLabel || s->st_Type == ST_RelName) && (s->st_Flags & SF_CODE))) {
        Stor stor;
        LockStorage(s);
        AllocDataRegister(&stor, s->st_Size);
        UnlockStorage(s);
        asm_move(exp, s, &stor);
        FreeRegister(&stor);
        return;
    }
    outop("tst", 0, NULL, s);
}

/*
 *  note: fp calls destroy scratch regs, so I have changed the moveq 0/scc
 *  to scc/ext
 */

void
asm_test_scc(Exp *exp, int32_t typeid, Stor *s, short cond, Stor *d)
{
    short isfp = (typeid == TID_FLT) ? 1 : 0;

    if (SameStorage(s, d) || SameRegister(s, d) || d->st_Type != ST_Reg || d->st_RegNo >= RB_ADDR) {
        Stor t;
        LockStorage(s);
        AllocDataRegister(&t, d->st_Size);
        UnlockStorage(s);

        if (isfp) {
            asm_fptest(exp, s);
        } else {
            if (d->st_Size != 1)
                asm_movei(exp, 0, &t);
            asm_test(exp, s);
        }
        asm_sccb(exp, &t, cond, 0);
        if (isfp)
            extop(1, d->st_Size, &t);

        FreeRegister(&t);
        asm_move(exp, &t, d);
    } else {
        if (isfp) {
            asm_fptest(exp, s);
        } else {
            if (d->st_Size != 1)
                asm_movei(exp, 0, d);
            asm_test(exp, s);
        }
        asm_sccb(exp, d, cond, 0);
        if (isfp)
            extop(1, d->st_Size, d);
    }
}

void
asm_cond_scc(exp, typeid, s1, s2, pcond, d)
Exp *exp;
int32_t typeid;
Stor *s1;
Stor *s2;
short *pcond;
Stor *d;
{
    short isfp = (typeid == TID_FLT) ? 1 : 0;

    if (SameStorage(s1, d) || SameStorage(s2, d) || d->st_Type != ST_Reg || d->st_RegNo >= RB_ADDR) {
        Stor t;
        LockStorage(s1);
        LockStorage(s2);
        AllocDataRegister(&t, d->st_Size);
        UnlockStorage(s1);
        UnlockStorage(s2);

        if (isfp) {
            asm_fpcmp(exp, s1, s2, pcond);     /*  if cmp reverses args it negates cond */
        } else {
            if (d->st_Size != 1)
                asm_movei(exp, 0, &t);
            asm_cmp(exp, s1, s2, pcond);     /*  if cmp reverses args it negates cond */
        }
        asm_sccb(exp, &t, *pcond, 0);
        if (isfp)
            extop(1, d->st_Size, &t);

        FreeRegister(&t);
        asm_move(exp, &t, d);
    } else {

        if (isfp) {
            asm_fpcmp(exp, s1, s2, pcond);   /*  if cmp reverses args it negates cond */
        } else {
            if (d->st_Size != 1)
                asm_movei(exp, 0, d);
            asm_cmp(exp, s1, s2, pcond);     /*  if cmp reverses args it negates cond */
        }
        asm_sccb(exp, d, *pcond, 0);
        if (isfp)
            extop(1, d->st_Size, d);
    }
}

void
asm_sccb(Exp *exp, Stor *d, short cond, short negative)
{
    char *str = "bad";
    int32_t size;

    switch(cond) {
    case COND_LT:
        str = "slt";
        break;
    case COND_LTEQ:
        str = "sle";
        break;
    case COND_GT:
        str = "sgt";
        break;
    case COND_GTEQ:
        str = "sge";
        break;
    case COND_EQ:
        str = "seq";
        break;
    case COND_NEQ:
        str = "sne";
        break;
    case CF_UNS|COND_LT:
        str = "scs";
        break;
    case CF_UNS|COND_LTEQ:
        str = "sls";
        break;
    case CF_UNS|COND_GT:
        str = "shi";
        break;
    case CF_UNS|COND_GTEQ:
        str = "scc";
        break;
    case CF_UNS|COND_EQ:
        str = "seq";
        break;
    case CF_UNS|COND_NEQ:
        str = "sne";
        break;

    case -COND_LT:
        str = "sge";
        break;
    case -COND_LTEQ:
        str = "sgt";
        break;
    case -COND_GT:
        str = "sle";
        break;
    case -COND_GTEQ:
        str = "slt";
        break;
    case -COND_EQ:
        str = "sne";
        break;
    case -COND_NEQ:
        str = "seq";
        break;
    case -(CF_UNS|COND_LT):
        str = "scc";
        break;
    case -(CF_UNS|COND_LTEQ):
        str = "shi";
        break;
    case -(CF_UNS|COND_GT):
        str = "sls";
        break;
    case -(CF_UNS|COND_GTEQ):
        str = "scs";
        break;
    case -(CF_UNS|COND_EQ):
        str = "sne";
        break;
    case -(CF_UNS|COND_NEQ):
        str = "seq";
        break;
    case COND_BPL:
        str = "spl";
        break;
    case COND_BMI:
        str = "smi";
        break;
    default:
        dbprintf(("Unknown cond %d\n", cond));
        Assert(0);
    }

    size = d->st_Size;
    d->st_Size = 1;
    outop(str, 0, NULL, d);         /*  result 0 or -1.B */

    if (negative == 0)              /*  want positive r. */
        outop("neg", 1, NULL, d);
    d->st_Size = size;
}

void
asm_clr(exp, d)
Exp *exp;
Stor *d;
{
    asm_movei(exp, 0, d);
}

void
asm_movei(exp, val, d)
Exp *exp;
int32_t val;
Stor *d;
{
    Stor stor;

    AllocConstStor(&stor, val, &LongType);
    stor.st_Size = d->st_Size;
    asm_move(exp, &stor, d);
}

/*
 *

void
asm_globlea(s, d)
Stor *s;
Stor *d;
{
    Assert(d->st_Type == ST_RelReg);

    printf("\tlea\t%s,A%c\n", StorToString(s, NULL), d->st_RegNo + '0' - RB_ADDR);
}
 *
 */


/*
 *  Arithmatic
 */

void
asm_neg(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    Stor stor;

    Assert(s->st_Size == d->st_Size);
    if (SameStorage(s, d)) {
        outop("neg", 0, NULL, d);
        return;
    }
    AllocDataRegister(&stor, d->st_Size);
    asm_move(exp, s, &stor);
    outop("neg", 0, NULL, &stor);
    FreeRegister(&stor);
    asm_move(exp, &stor, d);
}

/*
 *  Attempt to optimize a 3-op by moving one source to the destination and then
 *  doing the operation with the other source to the destination.  Only do this
 *  if the destination is a register (else if it were memory it would result in
 *  longer instructions OR screw ups if the memory is an IO location)
 *
 *  becareful when the destination is an address register and s2 indirects
 *  through that self-same register!
 */

void
ThreeOp(exp, s1, s2, d, op)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
void (*op)(Exp *, Stor *, Stor *, Stor *);
{
    if (d->st_Type == ST_Reg && !(d->st_RegNo == s2->st_RegNo && s2->st_Type == ST_RelReg)) {
        LockStorage(s2);
        asm_move(exp, s1, d);
        UnlockStorage(s2);
        (*op)(exp, d, s2, d);
    } else {
        Stor tmp;

        LockStorage(s2);
        AllocDataRegister(&tmp, s1->st_Size);
        asm_move(exp, s1, &tmp);
        UnlockStorage(s2);
        (*op)(exp, &tmp, s2, &tmp);
        asm_move(exp, &tmp, d);
        FreeRegister(&tmp);
    }
}

void
asm_add(exp, s1, s2, d)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
{
    if (ImmStorage(s2)) {
        SWAPS(s1, s2);
    }
    if (SameStorage(s1,d)) {
        SWAPS(s1, s2);
    }
    if (!SameStorage(s2,d)) {   /*  3 operand inst  */
        ThreeOp(exp, s1, s2, d, asm_add);
        return;
    }

    if (s1->st_Flags & SF_LEA) {
        Stor t;
        LockStorage(s2);
        AllocAddrRegister(&t);
        asm_move(exp, s1, &t);
        UnlockStorage(s2);
        asm_add(exp, &t, s2, d);
        FreeRegister(&t);
        return;
    }
    if (d->st_Flags & SF_LEA)
        yerror(exp->ex_LexIdx, EFATAL_DEST_NOT_LVALUE);

    if (ImmStorage(s1)) {
        if (s1->st_Type != ST_IntConst) {   /*  #label  */
            outop("add", 0, s1, d);
            return;
        } else {
            if (s1->st_IntConst == 0) {
                if (!SameStorage(s2,d))
                    asm_move(exp, s2, d);
                return;
            }
            if (s1->st_IntConst >= -8 && s1->st_IntConst <= 8) {
                if (s1->st_IntConst > 0) {
                    outop("addq", 0, s1, d);
                } else {
                    s1->st_IntConst = -s1->st_IntConst;
                    outop("subq", 0, s1, d);
                    s1->st_IntConst = -s1->st_IntConst;
                }
            } else if (d->st_Type == ST_Reg && d->st_RegNo >= RB_ADDR) {
                outop("adda", SizeFit(s1->st_IntConst), s1, d);
            } else {
                /*
                 *  use moveq/add if possible
                 */
                if (d->st_Size == 4 && s1->st_IntConst >= -128 && s1->st_IntConst < 128) {
                    goto moveadd;
                } else {
                    outop("add", 0, s1, d);
                }
            }
        }
    } else {    /*  not a constant  */
        Assert(s1->st_Size == d->st_Size);
        if ((s1->st_Type == ST_Reg && s1->st_RegNo < RB_ADDR) || d->st_Type == ST_Reg) {
            outop("add", 0, s1, d);
        } else {
            Stor stor;
moveadd:
            LockStorage(d);
            AllocDataRegister(&stor, d->st_Size);
            asm_move(exp, s1, &stor);
            UnlockStorage(d);
            asm_add(exp, &stor, d, d);
            FreeRegister(&stor);
        }
    }
}

void
asm_sub(exp, s1, s2, d)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
{
    if (!SameStorage(s1,d) && !SameStorage(s2,d)) {   /*  3 operand inst  */
        ThreeOp(exp, s1, s2, d, asm_sub);
        return;
    }

    if (s1->st_Flags & SF_LEA) {
        Stor t;

        LockStorage(s2);
        AllocAddrRegister(&t);
        asm_move(exp, s1, &t);
        UnlockStorage(s2);
        asm_sub(exp, &t, s2, d);
        FreeRegister(&t);
        return;
    }

    if (s2->st_Flags & SF_LEA) {
        Stor t;

        LockStorage(s1);
        AllocAddrRegister(&t);
        asm_move(exp, s2, &t);
        UnlockStorage(s1);
        asm_sub(exp, s1, &t, d);
        FreeRegister(&t);
        return;
    }



    if (s1->st_Type == ST_IntConst) {   /*  #val - s2   */
        Stor stor;

        Assert(SameStorage(s2,d));
        if (s2->st_Type == ST_Reg) {
            asm_neg(exp, d, d);
            asm_add(exp, s1, d, d);
            return;
        }
        LockStorage(s2);
        AllocDataRegister(&stor, d->st_Size);
        asm_move(exp, s1, &stor);
        UnlockStorage(s2);
        asm_sub(exp, &stor, s2, &stor);
        asm_move(exp, &stor, d);
        FreeRegister(&stor);
        return;
    }
    if (s2->st_Type == ST_IntConst) {   /*  s1 - #val   */
        Assert(SameStorage(s1,d));
        if (s2->st_IntConst == 0)
            return;
        if (s2->st_IntConst >= -8 && s2->st_IntConst <= 8) {
            if (s2->st_IntConst > 0) {
                outop("subq", 0, s2, d);
            } else {
                s2->st_IntConst = -s2->st_IntConst;
                outop("addq", 0, s2, d);
                s2->st_IntConst = -s2->st_IntConst;
            }
        } else if (d->st_Type == ST_Reg && d->st_RegNo >= RB_ADDR) {
            outop("suba", SizeFit(s2->st_IntConst), s2, d);
        } else {
            /*
             *  use moveq/sub if possible longword
             */
            if (d->st_Size == 4 && s2->st_IntConst >= -128 && s2->st_IntConst < 128) {
                goto movesub;
            } else {
                outop("sub", 0, s2, d);
            }
        }
        return;
    }
    if (SameStorage(s1,d)) {            /*  d = d - s2  */
        if (d->st_Type == ST_Reg || (s2->st_Type == ST_Reg && s2->st_RegNo < RB_ADDR)) {
            outop("sub", 0, s2, d);
        } else {
            Stor stor;
movesub:
            LockStorage(d);
            AllocDataRegister(&stor, d->st_Size);
            asm_move(exp, s2, &stor);
            UnlockStorage(d);
            asm_sub(exp, d, &stor, d);
            FreeRegister(&stor);
        }
    } else {                            /*  s2 == d:  d = s1 - d  */
        Stor stor;
        if (d->st_Type == ST_Reg) {
            asm_neg(exp, d, d);
            outop("add", 0, s1, d);
            return;
        }
        LockStorage(d);
        AllocDataRegister(&stor, d->st_Size);
        asm_move(exp, s1, &stor);
        UnlockStorage(d);
        asm_sub(exp, &stor, d, &stor);
        asm_move(exp, &stor, d);
        FreeRegister(&stor);
    }
    return;
}

/*
 *  eadnok  ==  0   Dn,<ea>
 *          ==  1   Dn,<ea> or <ea>,Dn
 */

Local void
asm_gen_logic_class(exp, str, bitstr, bitinverse, s1, s2, d, eadnok)
Exp *exp;
char *str;
char *bitstr;
Stor *s1;
Stor *s2;
Stor *d;
int32_t bitinverse;
int32_t eadnok;
{
    if ((s1->st_Flags & SF_LEA) || (s1->st_Type == ST_Reg && s1->st_RegNo >= RB_ADDR)) {
        Stor d1;

        LockStorage(s2);
        AllocDataRegister(&d1, 4);
        UnlockStorage(s2);
        asm_move(exp, s1, &d1);
        asm_gen_logic_class(exp, str, bitstr, bitinverse, &d1, s2, d, eadnok);
        FreeRegister(&d1);
        return;
    }
    if ((s2->st_Flags & SF_LEA) || (s2->st_Type == ST_Reg && s2->st_RegNo >= RB_ADDR)) {
        Stor d2;

        LockStorage(s1);
        AllocDataRegister(&d2, 4);
        UnlockStorage(s1);
        asm_move(exp, s2, &d2);
        asm_gen_logic_class(exp, str, bitstr, bitinverse, s1, &d2, d, eadnok);
        FreeRegister(&d2);
        return;
    }
    if (d->st_Type == ST_Reg && d->st_RegNo >= RB_ADDR) {   /*  addr reg is dest */
        Stor td;

        AllocDataRegister(&td, d->st_Size);
        asm_gen_logic_class(exp, str, bitstr, bitinverse, s1, s2, &td, eadnok);
        asm_move(exp, &td, d);
        FreeRegister(&td);
        return;
    }

    if (SameStorage(s2, d)) {
        SWAPS(s1,s2);
    }
    if (SameStorage(s1, d)) {
        if (s2->st_Type == ST_IntConst) {
            short n;
            if ((n = PowerOfTwo(s2->st_IntConst ^ bitinverse)) >= 0) {
                Stor con;
                int32_t size;
                AllocConstStor(&con, n, &LongType);
                if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR) {
                    outop(bitstr, 4, &con, d);
                } else {
                    int32_t bits = d->st_Size * 8;

                    con.st_IntConst &= 7;
                    Assert((d->st_Flags & SF_LEA) == 0);
                    switch(d->st_Type) {
                    case ST_PtrConst:
                    case ST_RelArg:
                    case ST_RelReg:
                    case ST_RegIndex:
                    case ST_RelLabel:
                    case ST_RelName:
                        if (n < bits) {
                            size = d->st_Size;
                            d->st_Size = 1;
                            d->st_Offset += (bits-1-n) >> 3;
                            outop(bitstr, 1, &con, d);
                            d->st_Offset -= (bits-1-n) >> 3;
                            d->st_Size = size;
                        } else {
                            yerror(exp->ex_LexIdx, EWARN_CONSTANT_OUT_OF_RANGE, n);
                        }
                        return;
                    default:
                        dbprintf(("Illegal destination type %d\n", d->st_Type));
                        Assert(0);
                    }
                }
                return;
            }

            /*
             *  s2 is an integer constant.  If the destination argument is a
             *  int32_t and s2 is in the moveq range it is faster and smaller to
             *  move the constant into a data register first.
             */

            if (d->st_Size == 4 && s2->st_IntConst >= -128 && s2->st_IntConst < 128) {
                Stor tmp;

                LockStorage(d);
                AllocDataRegister(&tmp, d->st_Size);
                UnlockStorage(d);
                asm_move(exp, s2, &tmp);
                outop(str, 0, &tmp, d);
                FreeRegister(&tmp);
            } else {
                outop(str, 0, s2, d);
            }
            return;
        } else if (s2->st_Type == ST_Reg && s2->st_RegNo < RB_ADDR) {
            outop(str, 0, s2, d);
            return;
        } else if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR) {
            if (eadnok) {
                outop(str, 0, s2, d);
                return;
            }
        }
    }
    if (d->st_Type == ST_Reg) {
        if (s1->st_Type == ST_Reg && s1->st_RegNo < RB_ADDR && !SameStorage(s1,d)) {
            LockStorage(s1);
            asm_move(exp, s2, d);
            UnlockStorage(s1);
            outop(str, 0, s1, d);
            return;
        } else if (s2->st_Type == ST_Reg && s2->st_RegNo < RB_ADDR && !SameStorage(s2,d)) {
            LockStorage(s2);
            asm_move(exp, s1, d);
            UnlockStorage(s2);
            outop(str, 0, s2, d);
            return;
        }
    }

    /*
     *  better to load the constant into a register, int32_t operations will
     *  benefit when we can use moveq.  We have to a move anyway so otherwise
     *  it does not matter whether the constant is moved or the other operand.
     */

    if (s1->st_Type == ST_IntConst)
        SWAPS(s1,s2);
    {
        Stor tmp;
        short useD;

        LockStorage(s1);
        if (d->st_Type == ST_Reg && d->st_RegNo < RB_ADDR && !SameRegister(s1, d)) {
            useD = 1;
            asm_move(exp, s2, d);
        } else {
            useD = 0;
            AllocDataRegister(&tmp, s2->st_Size);
            asm_move(exp, s2, &tmp);
        }
        UnlockStorage(s1);

        if ((s1->st_Type == ST_Reg && s1->st_RegNo >= RB_ADDR) || (eadnok == 0 && s1->st_Type != ST_Reg)) {
            Stor tmp2;
            AllocDataRegister(&tmp2, s1->st_Size);
            asm_move(exp, s1, &tmp2);
            if (useD)
                outop(str, 0, &tmp2, d);
            else
                outop(str, 0, &tmp2, &tmp);
            FreeRegister(&tmp2);
        } else {
            if (useD)
                outop(str, 0, s1, d);
            else
                outop(str, 0, s1, &tmp);
        }
        if (useD == 0) {
            asm_move(exp, &tmp, d);
            FreeRegister(&tmp);
        }
    }
}

void
asm_xor(exp, s1, s2, d)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
{
    if (s2->st_Type == ST_IntConst && s2->st_IntConst == -1) {
        Stor *t = s1;
        s1 = s2;
        s2 = t;
    }
    if (s1->st_Type == ST_IntConst && s1->st_IntConst == -1) {
        if (SameStorage(s2, d)) {
            outop("not", d->st_Size, NULL, d);
        } else {
            Stor t;

            AllocDataRegister(&t, s2->st_Size);
            asm_move(exp, s2, &t);
            outop("not", t.st_Size, NULL, &t);
            asm_move(exp, &t, d);
            FreeRegister(&t);
        }
        return;
    }
    asm_gen_logic_class(exp, "eor", "bchg", 0, s1, s2, d, 0);
}

void
asm_and(exp, s1, s2, d)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
{
    asm_gen_logic_class(exp, "and", "bclr", -1, s1, s2, d, 1);
}

void
asm_or(exp, s1, s2, d)
Exp *exp;
Stor *s1;
Stor *s2;
Stor *d;
{
    asm_gen_logic_class(exp, "or", "bset", 0, s1, s2, d, 1);
}

/*
 *  test / and optimization, note that btst doesn't work with address
 *  registers so can't optimize for that (asm_gen_logic.. will produce
 *  illegal output if we were to try)
 */

void
asm_test_and(exp, s1, s2)
Exp *exp;
Stor *s1;
Stor *s2;
{
    Stor d;

    if (s2->st_Type == ST_IntConst)
        SWAPS(s1,s2);
    if (s1->st_Type == ST_IntConst && PowerOfTwo(s1->st_IntConst) >= 0) {
        if (s2->st_Type != ST_Reg || s2->st_RegNo < RB_ADDR) {
            asm_gen_logic_class(exp, "tstand-bad-1", "btst", 0, s1, s2, s2, 1);
            return;
        }
    }
    if (s1->st_Size != s2->st_Size)
        yerror(exp->ex_LexIdx, EERROR_SIZE_MISMATCH, s1->st_Size, s2->st_Size);
    AllocDataRegister(&d, s1->st_Size);
    asm_gen_logic_class(exp, "and", "tstand-bad-2", 0, s1, s2, &d, 1);
    FreeRegister(&d);
}

/*
 *  Switch on storage s.    Cases are sorted.  Cases outside the
 *                          storage range of the storage are
 *                          deleted.
 *
 *  (1) Table lookup
 *  (2) Switch List
 *  (3) Binary search with switch list sub segments
 */

void
asm_switch(exp, num, cases, labels, deflabel)
Exp *exp;
int32_t num;
int32_t *cases;
int32_t *labels;
int32_t deflabel;
{
    int32_t minBound, maxBound;
    Stor con;
    Stor dtmp;
    Stor *s = &exp->ex_Stor;

    if (num == 0)
        return;

    minBound = 0x80000000;
    maxBound = 0x7FFFFFFF;

    if (s->st_Flags & SF_UNSIGNED) {
        if (s->st_Size == 1) {
            minBound = 0;
            maxBound = 255;
        } else if (s->st_Size == 2) {
            minBound = 0;
            maxBound = 65535;
        }
    } else {
        if (s->st_Size == 1) {
            minBound = -128;
            maxBound = 127;
        } else if (s->st_Size == 2) {
            minBound = -32768;
            maxBound = 32767;
        }
    }

    /*
     *  Sort the cases, delete out of bounds cases
     */

    SortCases(exp, cases, labels, num);

    while (num && cases[0] < minBound) {
        fprintf(stderr, "A %d %d %d\n", num, cases[0], minBound);
        yerror(exp->ex_LexIdx, EWARN_CONSTANT_OUT_OF_RANGE, cases[0]);
        --num;
        ++cases;
        ++labels;
    }
    while (num && cases[num-1] > maxBound) {
        fprintf(stderr, "B %d %d %d\n", num, cases[0], minBound);
        yerror(exp->ex_LexIdx, EWARN_CONSTANT_OUT_OF_RANGE, cases[num-1]);
        --num;
    }
    if (num == 0) {
        asm_branch(deflabel);
        return;
    }
    AllocConstStor(&con, 0, &LongType);

    if (num == 1) {
        short cond = COND_EQ;
        con.st_IntConst = cases[0];
        con.st_Size = s->st_Size;
        asm_cmp(exp, s, &con, &cond);
        asm_condbra(cond, labels[0]);
        asm_branch(deflabel);
        return;
    }

    /*
     *  If switching on an unsigned quantity convert the values to
     *  equivalent signed quantities because this is an assumption
     *  of the switch code.
     */

    if ((s->st_Flags & SF_UNSIGNED) && s->st_Size != 4) {
        int32_t i;

        for (i = 0; i < num; ++i) {
            if (s->st_Size == 1)
                cases[i] = (char)cases[i];
            else if (s->st_Size == 2)
                cases[i] = (short)cases[i];
        }
    }
    SortCases(exp, cases, labels, num);

    minBound = cases[0];        /*  boundry conditions  */
    maxBound = cases[num-1];

    if (num > 8 && (uint32_t)(maxBound - minBound) < num * 2) {
        int32_t label = AllocLabel();
        int32_t i;
        int32_t j;
        short cond;

        AllocDataRegister(&dtmp, s->st_Size);

        asm_move(exp, s, &dtmp);
        if (minBound) {
            con.st_IntConst = minBound;
            con.st_Size = s->st_Size;
            asm_sub(exp, &dtmp, &con, &dtmp);
        }

        /*
         *  range test
         */

        cond = COND_GT | CF_UNS;
        con.st_IntConst = maxBound - minBound;
        con.st_Size = s->st_Size;
        asm_cmp(exp, &dtmp, &con, &cond);
        asm_condbra(cond, deflabel);

        /*
         *  size = 1 case, then force size to 2. (can handle 8192 cases) XXX?
         */

        if (s->st_Size == 1) {
            con.st_IntConst = 0x00FF;
            outop("and", 2, &con, &dtmp);
        }
        dtmp.st_Size = 2;

        printf("\tasl.w\t#2,D%c\n", dtmp.st_RegNo + '0');
        printf("\tjmp\tl%d(pc,d%d.w)\n", label, dtmp.st_RegNo);

        asm_label(label);
        j = 0;
        for (i = minBound; i <= maxBound; ++i) {
            while (cases[j] != i) {
                printf("\tjmp\tl%d(pc)\n", deflabel);
                if (i == maxBound || j > num)
                {
                    dbprintf(("in switch, case [%d] %d", j, cases[j]));
                    Assert(0);
                }
                ++i;
            }
            printf("\tjmp\tl%d(pc)\n", labels[j++]);
        }
        if (j != num)
        {
            dbprintf(("in switch, aryend %d/%d", j, num));
            Assert(0);
        }
        FreeRegister(&dtmp);
    } else {
        /*
         *  subtract / branch
         */

        AllocDataRegister(&dtmp, s->st_Size);
        asm_move(exp, s, &dtmp);

        con.st_Size = s->st_Size;

        SubDivideSwitch(exp, &dtmp, &con, 0, 0, num, deflabel, cases, labels);
        FreeRegister(&dtmp);
    }

    /*
     *  If switching on an unsigned quantity we converted the quantities
     *  to signed for the switch op, but due to the possibility of
     *  retries we have to convert the objects back before returning.
     */

    if ((s->st_Flags & SF_UNSIGNED) && s->st_Size != 4) {
        int32_t i;

        for (i = 0; i < num; ++i) {
            if (s->st_Size == 1)
                cases[i] = (ubyte)cases[i];
            else if (s->st_Size == 2)
                cases[i] = (uword)cases[i];
        }
    }

}

/*
 *  SubDivideSwitch()
 *
 *  Values are signed quantities
 *
 *  This is used to execute a binary subdivision on a switch.  If the
 *  last case tested was j then our current zero is j.
 *
 *  To test case n- next we must add 5, bcs for higher cases and
 *  bcc for lower cases
 *
 *  To test case n+ next we must subtract 5, bcc for higher cases and
 *  bcs for lower cases
 *
 *              S
 *              .
 *              n-              5    -5
 *              .
 *              N  <---- j      10    0
 *              .
 *              n+              15   +5
 *              .
 *              X
 */

void
SubDivideSwitch(exp, s, c, j, is, ie, deflabel, cases, labels)
Exp *exp;
Stor *s;
Stor *c;
int32_t j;
int32_t is, ie, deflabel;
int32_t *cases;
int32_t *labels;
{
    int32_t i;
    int32_t ic;
    int32_t v;
    short high_cond;

    if (ie - is <= 4) {
        for (i = is; i < ie; ++i) {
            c->st_IntConst = (cases[i] - j);   /*  amount to subtract */
            if (c->st_IntConst)
                asm_sub(exp, s, c, s);
            else
                asm_test(exp, s);
            asm_condbra(COND_EQ, labels[i]);
            j += c->st_IntConst;
        }
        asm_branch(deflabel);
        return;
    }

    /*
     *  Current center point is j, find new center point.  If the absolute
     *  range is greater then the maximum positive signed quantity the
     *  storage can hold we use a CMP, otherwise we ADD or SUB to the
     *  new center point.
     *
     *  When we use compare we must use BGE style branches since we are
     *  doing a signed compare for signed ranging.  When we use an
     *  add or subtract we use BPL to get to the greater half (ignoring
     *  overflow) and are guarenteed the range will, by that time,
     *  be less then 2^(N-1)
     */

    ic = (is + ie) >> 1;                /*  center point (index)        */
    v = 1 << ((s->st_Size << 3) - 1);   /*  max pos integer + 1         */

    if ((uint32_t)(cases[ic] - cases[is]) >= (uint32_t)v || (uint32_t)(cases[ie-1] - cases[ic]) >= (uint32_t)v) {
        high_cond = COND_GTEQ;
        c->st_IntConst = cases[ic];
        asm_cmp(exp, s, c, &high_cond);
        /*
         *  j doesn't change (remains 0)
         */
    } else {
        c->st_IntConst = j - cases[ic];
        if (c->st_IntConst)
            asm_add(exp, s, c, s);
        else
            asm_test(exp, s);
        j = cases[ic];
        high_cond = COND_BPL;
    }

    asm_condbra(COND_EQ, labels[ic]);

    i = AllocLabel();

    asm_condbra(high_cond, i);

#ifdef NOTDEF
    if (high_cond == COND_GTEQ)
        printf("\tbpl\tl%d\n", i);
    else
        printf("\tbmi\tl%d\n", i);
#endif

    SubDivideSwitch(exp, s, c, j, is, ic, deflabel, cases, labels);
    asm_label(i);
    SubDivideSwitch(exp, s, c, j, ic+1, ie, deflabel, cases, labels);
}

#define SWAPL(l1,l2)    { int32_t lt = l1; l1 = l2; l2 = lt; }

Local void
SortCases(exp, cases, labels, num)
Exp *exp;
int32_t *cases;
int32_t *labels;
int32_t num;
{
    int32_t sv;
    int32_t si;
    int32_t sj;

loop:
    switch(num) {
    case -1:
    case 0:
    case 1:
        break;
    case 2:
        if (cases[0] > cases[1]) {
            SWAPL(cases[0], cases[1]);
            SWAPL(labels[0], labels[1]);
        }
        break;
    default:
        sv = cases[num>>1];     /*  split value */
        for (si = sj = 0; si < num; ++si) {
            if (cases[si] < sv) {
                SWAPL(cases[si], cases[sj]);
                SWAPL(labels[si], labels[sj]);
                ++sj;
            }
        }
        if (sj == 0) {          /*  sv smallest */
            if ((si = num >> 1) != 0) {
                SWAPL(cases[0], cases[si]);
                SWAPL(labels[0], labels[si]);
            }
            ++cases;
            ++labels;
            --num;
            goto loop;
        }
        if (sj == num) {        /*  sv largtest */
            if ((si = num >> 1) != num - 1) {
                SWAPL(cases[num-1], cases[si]);
                SWAPL(labels[num-1], labels[si]);
            }
            --num;
            goto loop;
        }
        SortCases(exp, cases, labels, sj);
        SortCases(exp, cases + sj, labels + sj, num - sj);
    }
    {
        int32_t *cp = cases;

        sv = *cp;
        for (si = num - 2; si >= 0; --si, ++cp) {
            sj = cp[1];
            if (sv == sj)
                break;
            sv = sj;
        }
    }
    if (si >= 0)
        yerror(exp->ex_LexIdx, EERROR_CASE_REPEATED, sv, sv);
}


int32_t
SizeFit(v)
int32_t v;
{
    if (v >= -32768 && v < 32768)
        return(2);
    return(4);
}

int32_t
SizeFitSU(v, s)
int32_t v;
Stor *s;
{
    if (s->st_Flags & SF_UNSIGNED) {
        if ((uint32_t)v < 65536)
            return(2);
    } else {
        if (v >= -32768 && v < 32768)
            return(2);
    }
    return(4);
}

void
asm_end()
{
    int32_t cp, ep;

    printf("\tEND\n");
    cp = ftell(stdout);
    fseek(stdout, 0L, 2);
    ep = ftell(stdout);
    if (cp > 0 && ep > 0 && cp < ep) {
        fseek(stdout, cp, 0);
        while (cp < ep) {
            fputc('\n', stdout);
            ++cp;
        }
    }
}

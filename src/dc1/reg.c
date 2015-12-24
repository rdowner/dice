/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  REG.C
 *
 *  Register and stack allocation.
 */

#include "defs.h"

Prototype   void    ResetRegAlloc(void);
Prototype   void    AllocExternalStorage(Symbol *, Stor *, Type *, int32_t);
Prototype   void    AllocStaticStorage(Symbol *, Stor *, Type *, int32_t);
Prototype   void    AllocConstStor(Stor *, int32_t, Type *);
Prototype   void    AllocFltConstStor(Stor *, char *, int32_t, Type *);
Prototype   void    AllocStackStorage(Stor *, Type *, int32_t);
Prototype   void    FreeStackStorage(Stor *);
Prototype   void    ReUseStackStorage(Stor *);
Prototype   void    AllocArgsStorage(Stor *, Type *, int, int32_t);
Prototype   int     AllocRegVarStorageReq(Var *, short, int32_t);
Prototype   int     AllocRegVarStorage(Var *);
Prototype   void    AllocAnyRegister(Stor *, Type *, Stor *);
Prototype   void    AllocTmpStorage(Stor *, Type *, Stor *);
Prototype   void    ReuseStorage(Stor *, Stor *);
Prototype   void    FreeStorage(Stor *);
Prototype   int     AllocDataRegister(Stor *, int32_t);
Prototype   int     AllocDataRegisterAbs(Stor *, int32_t, int);
Prototype   int     AllocAddrRegister(Stor *);
Prototype   int     AllocAddrRegisterAbs(Stor *, int);
Prototype   void    FreeRegister(Stor *);	/*  single registers only */
Prototype   void    LockStorage(Stor *);
Prototype   void    UnlockStorage(Stor *);
Prototype   void    RegDisableRegs(uint32_t);
Prototype   void    RegEnableRegs(void);
Prototype   void    UnscratchStorage(Exp *);
Prototype   int     RegInUse(short);
Prototype   int     AllocRegisterAbs(Stor *, short, short);
Prototype   int     AttemptAllocRegisterAbs(Stor *, short, short);
/*Prototype   void    TransferRegister(Stor *, short, short);*/

Prototype   uint32_t   GetAllocatedScratch(void);
Prototype   uint32_t   GetLockedScratch(void);
Prototype   uint32_t   GetUsedRegisters(void);
Prototype   uint32_t   RegCallUseRegister(short);
Prototype   int     TooManyRegs(void);
Prototype   int     CountDRegOver(void);
Prototype   int     CountARegOver(void);
Prototype   void    asm_save_regs(uint32_t);
Prototype   int     asm_restore_regs(uint32_t);
Prototype   void    RegFlagTryAgain(void);

Prototype   void    PushStackStorage(void);
Prototype   void    PopStackStorage(void);

Prototype uint32_t   RegAlloc;	/*  protos for debugging only	*/
Prototype uint32_t   RegLocked;
Prototype uint32_t   RegUsed;

uint32_t	RegAlloc;		/*  mask of allocated registers     */
uint32_t	RegLocked;		/*  (should be static)		    */
uint32_t	RegUsed;

static TmpStack TmpAry[TMP_STACK_MAX];

static short  DataRegCache = RB_D0; /*	cache of last free'd register   */
static short  AddrRegCache = RB_A0;
static short  TryAgainFlag = 0;
static short  Refs[32];
static short  Locked[32];
static short  CacheOnFree;
static short  TSIndex;
static short  TSMax;

/*
 *  Order of allocation of registers, ordering setup to avoid conflicts
 *  with shared library calls whenever possible (reduce the amount of stack
 *  swapping required).  Note that A6 (22) is last to facilitate library
 *  calls (future)
 */

static short RPAddrScan16[] = { 16, 17, 23, 21, 20, 19, 18, 22, 24, 25, 26, 27, 28, 29, 30, 31, -1 };
static short RPDataScan16[] = {  0,  1,  7,  6,  5,  4,  3,  2,  8,  9, 10, 11, 12, 13, 14, 15, -1 };

void
ResetRegAlloc()
{
    RegAlloc = RegReserved;
    RegUsed  = RegAlloc;
    RegLocked= 0;
    DataRegCache = RB_D0;
    AddrRegCache = RB_A0;
    TmpAry[0].ts_Size = 0;
    TSIndex = 0;
    TSMax = 0;

    TryAgainFlag = 0;
    setmem(Refs, sizeof(Refs), 0);

    Refs[RB_A4] = 1;	    /* XXX hack */
    Refs[RB_A5] = 1;
    Refs[RB_A7] = 1;

    setmem(Locked, sizeof(Locked), 0);
}

void
AllocExternalStorage(sym, stor, type, flags)
Symbol *sym;
Stor *stor;
Type *type;
int32_t flags;
{
    stor->st_Type = ST_RelName;
    stor->st_Name = sym;
    stor->st_Offset = 0;
    stor->st_Size = *type->Size;
    stor->st_Flags= SF_VAR | SF_NOSA;

    if (flags & TF_UNSIGNED)
	stor->st_Flags |= SF_UNSIGNED;
    if (flags & TF_NEAR) {
	stor->st_Flags |= SF_NEAR;
    } else if (flags & (TF_FAR | TF_CHIP)) {
	stor->st_Flags |= SF_FAR;
    }
    if (flags & TF_SHARED) {
	stor->st_Flags |= SF_CODE | SF_FAR;
    }
    if (flags & TF_CONST) {
	stor->st_Flags |= SF_CODE;
	if ((stor->st_Flags & (SF_NEAR|SF_FAR)) == 0) {
	    if (SmallCode == 0 || ((flags & TF_EXTERN) && ConstCode < 2))
		stor->st_Flags |= SF_FAR;
	    else
		stor->st_Flags |= SF_NEAR;
	}
    }
    if (type->Id == TID_PROC)
	stor->st_Flags |= SF_LEA | SF_CODE;
}

void
AllocStaticStorage(sym, stor, type, flags)
Symbol *sym;
Stor *stor;
Type *type;
int32_t flags;
{
    stor->st_Type = ST_RelLabel;
    stor->st_Label= AllocLabel();
    stor->st_Offset = 0;
    stor->st_Size = *type->Size;
    stor->st_Flags= SF_VAR | SF_NOSA;

    if (flags & TF_UNSIGNED)
	stor->st_Flags |= SF_UNSIGNED;
    if (flags & TF_NEAR) {
	stor->st_Flags |= SF_NEAR;
    } else if (flags & (TF_FAR | TF_CHIP)) {
	stor->st_Flags |= SF_FAR;
    }
    if (flags & TF_SHARED) {
	stor->st_Flags |= SF_CODE | SF_FAR;
	stor->st_Flags &= ~SF_NEAR;
    }
    if (flags & TF_CONST) {
	stor->st_Flags |= SF_CODE;
	if ((flags & (SF_NEAR|SF_FAR)) == 0) {
	    if (SmallCode)
		stor->st_Flags |= SF_NEAR;
	    else
		stor->st_Flags |= SF_FAR;
	}
    }
}

/*
 *  allocate an integer constant.  Note that the SF_NOSA flag is
 *  set.  Rightly this should be an expression flag.  In anycase,
 *  it prevents exp nodes from down copying storage (see GenEq()).
 */

void
AllocConstStor(s, val, type)
Stor *s;
int32_t val;
Type *type;
{
    s->st_Type = ST_IntConst;
    s->st_Size = *type->Size;
    s->st_Flags= SF_NOSA;
    s->st_IntConst = val;

    if (type->Flags & TF_UNSIGNED)
	s->st_Flags |= SF_UNSIGNED;
}

void
AllocFltConstStor(s, ptr, len, type)
Stor *s;
char *ptr;
int32_t len;
Type *type;
{
    s->st_Type = ST_FltConst;
    s->st_Size = *type->Size;
    s->st_Flags= SF_NOSA;
    s->st_FltConst = ptr;
    s->st_FltLen   = len;
}

/*
 *  allocate storage for argument
 */

void
AllocArgsStorage(Stor *stor, Type *type, int real, int32_t flags)
{
    Frame *frame = &CurGen->Frame;
    int32_t bytes;
    int32_t typesize = *type->Size;

    Assert(CurGen);

    switch(type->Id) {
    case TID_INT:
	bytes = Align(*type->Size, 4);
	break;
    case TID_ARY:
	bytes = PTR_SIZE;
	typesize = bytes;
	break;
    default:
	bytes = *type->Size;
	break;
    }
    frame->ArgsStackUsed += bytes;

    if (real) {
	stor->st_Type = ST_RelArg;
	stor->st_RegNo = RB_FP;
    } else {
	stor->st_Type = ST_RelArg;
	stor->st_RegNo = RB_SP;
    }
    stor->st_Size = typesize;
    stor->st_Offset = frame->ArgsStackUsed - typesize;
    stor->st_Flags = SF_VAR | SF_NOSA;
    if (flags & TF_UNSIGNED)
	stor->st_Flags |= SF_UNSIGNED;
}

/*
 *  Allocate the requested register var storage
 *
 *  Note that byte-wide variables cannot be allocated in address registers
 *  because byte assembly ops cannot be performed on address regs
 */

int
AllocRegVarStorageReq(Var *var, short reqNo, int32_t skipMask)
{
    uint32_t alMask;
    uint32_t usMask;
    int32_t r;

    Assert(CurGen);
    alMask = RegAlloc | ~REGREAL;
    if (CurGen->Frame.Flags & FF_CALLMADE)
	alMask |= REGSCRATCH;
    if (*var->Type->Size == 1 && reqNo >= RB_ADDR)
	alMask |= RF_AREG;

    /*
     *	Attempt to allocate the specific register in question
     */

    if ((alMask & (1 << reqNo)) == 0) {
	if (reqNo >= RB_ADDR)
	    AllocAddrRegisterAbs(&var->var_Stor, reqNo);
	else
	    AllocDataRegisterAbs(&var->var_Stor, *var->Type->Size, reqNo);
	var->var_Stor.st_Flags &= ~SF_TMP;
	var->var_Stor.st_Flags |= SF_VAR;
	if (var->Type->Flags & TF_UNSIGNED)
	    var->var_Stor.st_Flags |= SF_UNSIGNED;

	/*
	 *  so we don't save/restore passed register variables that are
	 *  never modified
	 */

	if ((var->RegFlags & RF_MODIFIED) == 0)
	    RegReserved |= 1 << reqNo;

	return(1);
    }

    /*
     *	Attempt to allocate a register not currently being used to pass
     *	a variable (reduces number of exchanges required)
     */

    alMask = RegAlloc;
    usMask = RegUsed;
    RegAlloc |= skipMask;
    RegUsed  |= skipMask;
    r = AllocRegVarStorage(var);

    /*
     *	Give up, normal allocation attempt
     */

    RegAlloc = alMask | (RegAlloc & ~skipMask);
    RegUsed  = usMask | (RegUsed  & ~skipMask);

    if (r == 0)
	r = AllocRegVarStorage(var);

    return(r);
}

/*
 *  Allocate register storage for variable.  Can only allocate real
 *  registers, cannot allocated used registers.  Can only allocate
 *  scratch regs if CurGen->Frame.Flags says no call made
 *
 *  flag another pass if unable to allocate a variable with more references
 *  then the worst case so far.
 */

int
AllocRegVarStorage(var)
Var *var;
{
    uint32_t mask;

    Assert(CurGen);
    mask = RegAlloc | ~REGREAL;
    if (CurGen->Frame.Flags & FF_CALLMADE)
	mask |= REGSCRATCH;

    if (mask != 0xFFFFFFFF) {
	short regno;
	short *regptr;

	if (var->Type->Id == TID_PTR || ((var->Flags & VF_ARG) && var->Type->Id == TID_ARY)) {
	    for (regptr = RPAddrScan16; (regno = *regptr) >= 0; ++regptr) {
		if ((mask & (1 << regno)) == 0) {
		    AllocAddrRegisterAbs(&var->var_Stor, regno);
		    var->var_Stor.st_Flags &= ~SF_TMP;
		    var->var_Stor.st_Flags |= SF_VAR;
		    if (var->Type->Flags & TF_UNSIGNED)
			var->var_Stor.st_Flags |= SF_UNSIGNED;
		    return(1);
		}
	    }
	} else {
	    for (regptr = RPDataScan16; (regno = *regptr) >= 0; ++regptr) {
		if ((mask & (1 << regno)) == 0) {
		    AllocDataRegisterAbs(&var->var_Stor, *var->Type->Size, regno);
		    var->var_Stor.st_Flags &= ~SF_TMP;
		    var->var_Stor.st_Flags |= SF_VAR;
		    if (var->Type->Flags & TF_UNSIGNED)
			var->var_Stor.st_Flags |= SF_UNSIGNED;
		    return(1);
		}
	    }
	}
    }
    return(0);
}

void
AllocAnyRegister(stor, type, cache)
Stor *stor;
Type *type;
Stor *cache;
{
    Assert (CurGen);
    Assert (type);

    if ((type->Id == TID_STRUCT) ||
        (type->Id == TID_UNION)  ||
        (*type->Size > 4))
    {
	Assert(0);
    }

    if (cache &&
        (cache->st_Type == ST_Reg    ||
         cache->st_Type == ST_RelReg ||
         cache->st_Type == ST_RegIndex))
    {
	Assert((unsigned short)cache->st_RegNo < 32);
	if (RegUsed & (1 << cache->st_RegNo)) {
	    if (cache->st_RegNo < RB_ADDR)
		DataRegCache = cache->st_RegNo;
	    else
		AddrRegCache = cache->st_RegNo;
	}
    }

    {
	int wantaddr = 0;   /* Assume that we will go for a data register */
	if (type->Id == TID_PTR)
	{
	    /* We prefer to have a pointer register here, but we need      */
	    /* to ensure that one is free before we will attempt to get it */
	    if (((RegAlloc | RegLocked) & 0x0FF0000) != 0xFF0000)
		wantaddr = 1;
	}
	else if (((RegAlloc | RegLocked) & 0x0FF) == 0xFF)
	{
	   /* we know that it is a data type item but there are no data    */
	   /* registers free so we will have to try for an address         */
	   /* register instead.                                            */
	   wantaddr = 1;
	}

	if (wantaddr)
	    AllocAddrRegister(stor);
	else
	{
	    AllocDataRegister(stor, *type->Size);

	    if (type->Flags & TF_UNSIGNED)
		stor->st_Flags |= SF_UNSIGNED;
	}
    }
}

/*
 *  AllocTmpStorage() is allowed to re-allocate half-freed storage.
 *
 *  Note that the size of the register is the same as that of the
 *  type.  Data registers do not necessarily contain just longs.
 */

void
AllocTmpStorage(stor, type, cache)
Stor *stor;
Type *type;
Stor *cache;
{
    short force_stk = 0;

    Assert (CurGen);
    Assert (type);

    if (type->Id == TID_STRUCT || type->Id == TID_UNION) {
	force_stk = 1;
    }

    if (cache && (cache->st_Type == ST_Reg || cache->st_Type == ST_RelReg || cache->st_Type == ST_RegIndex)) {
	Assert((unsigned short)cache->st_RegNo < 32);
	if (RegUsed & (1 << cache->st_RegNo)) {
	    if (cache->st_RegNo < RB_ADDR)
		DataRegCache = cache->st_RegNo;
	    else
		AddrRegCache = cache->st_RegNo;
	}
    }

    if (force_stk == 0) {
	if (type->Id == TID_PTR) {
	    AllocAddrRegister(stor);
	    return;
	} else if (*type->Size <= 4) {
	    AllocDataRegister(stor, *type->Size);
	    if (type->Flags & TF_UNSIGNED)
		stor->st_Flags |= SF_UNSIGNED;
	    return;
	}
    }
    {
	TmpStack *ts;
	int32_t alignment = *type->Align;

	if (type->Flags & TF_ALIGNED)
	    alignment = 4;

	for (ts = TmpAry; ts->ts_Size; ++ts) {
	    if (ts->ts_Refs)
		continue;
	    if ((ts->ts_Offset & (alignment - 1)) == 0 && ts->ts_Size == *type->Size) {
		++ts->ts_Refs;
		stor->st_Type = ST_RelReg;
		stor->st_RegNo = RB_FP;
		stor->st_Size = *type->Size;
		stor->st_Offset = ts->ts_Offset;
		stor->st_Flags = SF_TMP | SF_NOSA;
		if (type->Flags & TF_UNSIGNED)
		    stor->st_Flags |= SF_UNSIGNED;
		return;
	    }
	}
	if (ts == TmpAry + (TMP_STACK_MAX - 1))
	    zerror(EFATAL_MAX_TMP_EXCEEDED);	/* XXX */

	AllocStackStorage(stor, type, type->Flags);
	stor->st_Flags |= SF_TMP;
	stor->st_Flags &= ~SF_VAR;
	ts->ts_Offset = stor->st_Offset;
	ts->ts_Size = stor->st_Size;
	ts->ts_Refs = 1;
	(ts + 1)->ts_Size = 0;
	TSMax = (ts - TmpAry) + 1;
    }
}

void
ReuseStorage(s, d)
Stor *s, *d;
{
    *d = *s;

    if (s->st_Type == ST_RegIndex) {
	if (s->st_Flags & SF_TMP2) {
	    ++Refs[s->st_RegNo2];
	    RegAlloc |= 1 << s->st_RegNo2;
	    Assert((RegUsed | RegAlloc) == RegUsed);
	    /* RegUsed |= RegAlloc; */
	}
    }
    if (s->st_Flags & SF_TMP) {
	if (s->st_Type == ST_Reg || s->st_Type == ST_RelReg || s->st_Type == ST_RegIndex) {
	    if (s->st_RegNo == RB_FP) {
		ReUseStackStorage(s);
	    } else {
		++Refs[s->st_RegNo];
		RegAlloc |= 1 << s->st_RegNo;
		Assert((RegUsed | RegAlloc) == RegUsed);
		/* RegUsed |= RegAlloc; */
	    }
	}
    }
}

void
FreeStorage(stor)
Stor *stor;
{
    /*
     *	The RegIndex type holds two registers, either of which may actually
     *	be 'temporary' registers.
     */

    if (stor->st_Type == ST_RegIndex) {
	Stor x;

	x.st_Type = ST_Reg;
	if (stor->st_Flags & SF_TMP) {
	    if (stor->st_RegNo == RB_FP) {
		FreeStackStorage(stor);
	    } else {
		x.st_RegNo = stor->st_RegNo;
		FreeRegister(&x);
	    }
	}
	if (stor->st_Flags & SF_TMP2) {
	    x.st_RegNo = stor->st_RegNo2;
	    FreeRegister(&x);
	}
	return;
    }
    if (stor->st_Flags & SF_TMP) {
	if (stor->st_Type == ST_Reg || stor->st_Type == ST_RelReg) {
	    if (stor->st_RegNo == RB_FP) {
		FreeStackStorage(stor);
	    } else {
		CacheOnFree = 1;
		FreeRegister(stor);
		CacheOnFree = 0;
	    }
	    return;
	}
    }
}

int
RegInUse(short regno)
{
    return(Refs[regno]);
}

int
AllocDataRegister(stor, size)
Stor *stor;
int32_t size;
{
    short regno = DataRegCache;
    uword mask = (RegAlloc | RegLocked);

    if (mask & (1 << regno)) {
	regno = 0;
	if ((mask & 0x00FF) == 0x00FF) {
	    regno += 8;
	    mask >>= 8;
	}
	if ((mask & 0x000F) == 0x000F) {
	    regno += 4;
	    mask >>= 4;
	}
	if ((mask & 0x0001) == 0)
	    regno += 0;
	else if ((mask & 0x0002) == 0)
	    regno += 1;
	else if ((mask & 0x0004) == 0)
	    regno += 2;
	else if ((mask & 0x0008) == 0)
	    regno += 3;
	else
	    Assert(0);
    }

    if ((RegUsed & (1 << regno)) == 0) {
	RegUsed |= 1 << regno;
	if (regno >= 8) {
	    Assert(CurGen);
	    ++CurGen->Frame.CurDRegOver;
	}
    }
    RegAlloc |= 1 << regno;
    Assert((RegUsed | RegAlloc) == RegUsed);
    /* RegUsed |= RegAlloc; */

    ++Refs[regno];

    if (stor) {
	stor->st_Flags= SF_TMP;
	stor->st_Type = ST_Reg;
	stor->st_RegNo= regno;
	stor->st_Size = size;
    }
    return((int)regno);
}

int
AllocDataRegisterAbs(stor, size, regno)
Stor *stor;
int32_t size;
int regno;
{
    if ((RegUsed & (1 << regno)) == 0) {
	RegUsed |= 1 << regno;
	if (regno >= 8) {
	    Assert(CurGen);
	    ++CurGen->Frame.CurDRegOver;
	}
    }
    RegAlloc |= 1 << regno;
    Assert((RegUsed | RegAlloc) == RegUsed);
    /* RegUsed |= RegAlloc; */

    ++Refs[regno];

    if (stor) {
	stor->st_Flags= SF_TMP;
	stor->st_Type = ST_Reg;
	stor->st_RegNo= regno;
	stor->st_Size = size;
    }
    return((int)regno);
}

int
AllocAddrRegister(stor)
Stor *stor;
{
    short regno = AddrRegCache - RB_ADDR;
    uword mask = (RegAlloc | RegLocked) >> 16;

    Assert ((unsigned short)regno < 32);
    if (mask & (1 << regno)) {
	regno = 0;
	if ((mask & 0x00FF) == 0x00FF) {
	    regno += 8;
	    mask >>= 8;
	}
	if ((mask & 0x000F) == 0x000F) {
	    regno += 4;
	    mask >>= 4;
	}
	if ((mask & 0x0001) == 0)
	    regno += 0;
	else if ((mask & 0x0002) == 0)
	    regno += 1;
	else if ((mask & 0x0004) == 0)
	    regno += 2;
	else if ((mask & 0x0008) == 0)
	    regno += 3;
	else
	    Assert(0);
    }

    regno += 16;

    if ((RegUsed & (1 << regno)) == 0) {
	RegUsed |= 1 << regno;
	if (regno >= 24) {
	    Assert(CurGen);
	    ++CurGen->Frame.CurARegOver;
	}
    }
    RegAlloc |= 1 << regno;
    Assert((RegUsed | RegAlloc) == RegUsed);
    /* RegUsed |= RegAlloc; */

    ++Refs[regno];

    if (stor) {
	stor->st_Flags= SF_TMP;
	stor->st_Type = ST_Reg;
	stor->st_RegNo= regno;
	stor->st_Size = PTR_SIZE;
    }
    return((int)regno);
}

int
AllocAddrRegisterAbs(stor, regno)
Stor *stor;
int regno;
{
    if ((RegUsed & (1 << regno)) == 0) {
	RegUsed |= 1 << regno;
	if (regno >= 24) {
	    Assert(CurGen);
	    ++CurGen->Frame.CurARegOver;
	}
    }
    RegAlloc |= 1 << regno;
    if ((RegUsed | RegAlloc) != RegUsed) {
	dbprintf(("RegAll %d %08lx %08lx\n", regno, RegAlloc, RegUsed));
	Assert(0);
    }
    ++Refs[regno];

    if (stor) {
	stor->st_Flags= SF_TMP;
	stor->st_Type = ST_Reg;
	stor->st_RegNo= regno;
	stor->st_Size = PTR_SIZE;
    }
    return((int)regno);
}

int
AllocRegisterAbs(Stor *s, short regno, short size)
{
    if (regno >= RB_ADDR) {
	AllocAddrRegisterAbs(s, regno);
    } else {
	AllocDataRegisterAbs(s, size, regno);
    }
    return(s->st_RegNo);
}

/*
 *  Attempt to allocate the requested register.  If unable to do so then
 *  allocate some other register.  Will not allocate an address register
 *  for a byte-sized quantity.
 */

int
AttemptAllocRegisterAbs(Stor *s, short regno, short size)
{
    uint32_t mask = (RegAlloc | RegLocked);

#ifdef NOTDEF	/* removed, regargs gen handles the case now */
    if (regno >= RB_ADDR && size == 1) {
	AllocDataRegister(s, size);
    } else
#endif
    if (((1 << regno) & mask) == 0) {
	if (regno >= RB_ADDR)
	    AllocAddrRegisterAbs(s, regno);
	else
	    AllocDataRegisterAbs(s, size, regno);
    } else {
	/*
	 *  XXX hack.  Can't allocate address register if size == 1, also
	 *  addr register routines force size to 4, have to fix that...
	 */

	if (size != 1 || ((mask & RF_DREG) == RF_DREG && (mask & RF_AREG) != RF_AREG)) {
	    AllocDataRegister(s, size);
	} else {
	    AllocAddrRegister(s);
	    s->st_Size = size;
	}
    }
    return(s->st_RegNo);
}

#ifdef NOTDEF
void
TransferRegister(s, regno, size)
Stor *s;
short regno;
short size;
{
    Stor t;

    AttemptAllocRegisterAbs(&t, regno, 4);
    printf("\tmove.l\t%s,%s\n", StorToString(s, NULL), StorToString(&t, NULL));
    FreeStorage(s);
    *s = t;
}
#endif

void
FreeRegister(stor)
Stor *stor;
{
    switch(stor->st_Type) {
    case ST_Reg:
    case ST_RelReg:
	{
	    short regno = stor->st_RegNo;
	    uint32_t mask = 1 << regno;

	    Assert ((unsigned short)regno < 32);
	    if (--Refs[regno] < 0) {
		zerror(ESOFT_REG_NOT_ALLOCATED, regno);  /* XXX */
		Refs[regno] = 0;
	    }
	    if (Refs[regno] == 0) {
		if (CacheOnFree) {
		    if (regno >= RB_ADDR)
			AddrRegCache = regno;
		    else
			DataRegCache = regno;
		}
		if (RegAlloc & mask) {
		    RegAlloc &= ~mask;
		} else {
		    zerror(ESOFT_REG_NOT_ALLOCATED, regno); /* XXX */
		}
	    }
	}
	break;
    default:
	dbprintf(("Bad Free Type: %d\n", stor->st_Type));
	Assert(0);
	break;
    }
}

void
LockStorage(s)
Stor *s;
{
    if (s->st_Type == ST_Reg || s->st_Type == ST_RelReg || s->st_Type == ST_RegIndex) {
	if ((s->st_Flags & SF_TMP) && s->st_RegNo == RB_FP) {
	    ReUseStackStorage(s);
	    return;
	}
	{
	    short regno = s->st_RegNo;
	    Assert ((unsigned short)regno < 32);
	    if (Locked[regno]++ == 0)
		RegLocked |= 1 << regno;
	}
	if (s->st_Type == ST_RegIndex) {
	    short regno = s->st_RegNo2;
	    Assert ((unsigned short)regno < 32);
	    if (Locked[regno]++ == 0)
		RegLocked |= 1 << regno;
	}
    }
}

void
UnlockStorage(s)
Stor *s;
{
    if (s->st_Type == ST_Reg || s->st_Type == ST_RelReg || s->st_Type == ST_RegIndex) {
	if ((s->st_Flags & SF_TMP) && s->st_RegNo == RB_FP) {
	    FreeStackStorage(s);
	    return;
	}
	{
	    short regno = s->st_RegNo;
	    if (--Locked[regno] == 0)
		RegLocked &= ~(1 << regno);
	    if (Locked[regno] < 0) {
		dbprintf(("UnlockStorage: too many unls %d\n", regno));
		Assert(0);
		Locked[regno] = 0;
	    }
	}
	if (s->st_Type == ST_RegIndex) {
	    short regno = s->st_RegNo2;
	    if (--Locked[regno] == 0)
		RegLocked &= ~(1 << regno);
	    if (Locked[regno] < 0) {
		dbprintf(("UnlockStorage2: too many unls %d\n", regno));
		Assert(0);
		Locked[regno] = 0;
	    }
	}
    }
}

#ifdef NOTDEF
/*
 *  Disables the specified registers
 */

void
RegDisableRegs(mask)
uint32_t mask;
{
    DataRegCache = RB_D0;
    AddrRegCache = RB_A0;
    RegCantUse = mask;
    if (RegCantUse & (RegAlloc | RegLocked)) {
	if (GenPass > 1)
	{
	    dbprintf(("RegDisableRegs, regs in use: %08lx %08lx %08lx\n", RegCantUse, RegAlloc, RegLocked));
	    Assert(0);
	}
	++TryAgainFlag;
    }
}

/*
 *  Enables the specified registers
 */

void
RegEnableRegs()
{
    DataRegCache = RB_D0;
    AddrRegCache = RB_A0;
    RegCantUse = 0;
}

#endif

void
RegFlagTryAgain()
{
    if (TryAgainFlag == 0)
	dbprintf(("; TRY AGAIN\n"));
    TryAgainFlag = 1;
}

uint32_t
GetAllocatedScratch()
{
    return(RegAlloc & REGSCRATCH);
}

uint32_t
GetLockedScratch()
{
    return(RegLocked & REGSCRATCH);
}

uint32_t
GetUsedRegisters()
{
    return(RegUsed);
}

/*
 *  Returns register mask for those registers which must be saved and
 *  restored local to the call.
 *
 *  A4/A5 must be saved locally since they are place keepers (RegReserved)
 *
 * XXX
 */

uint32_t
RegCallUseRegister(short rno)
{
    int32_t mask = 1 << rno;

    if ((RegAlloc|RegReserved) & mask)
	return(mask);
    RegUsed |= mask;
    return(0);
}

int
TooManyRegs()
{
    if (TryAgainFlag || (RegUsed & 0xFF00FF00))
	return(1);
    return(0);
}

#ifdef NOTDEF
int
CountDRegOver()
{
    int cnt = 0;
    int32_t ru = RegUsed;

    if (ru & 0x0000FF00) {
	short i;
	for (i = 8; i < 16; ++i) {
	    if (ru & (1 << i))
		++cnt;
	}
    }
    return(cnt);
}

int
CountARegOver()
{
    int cnt = 0;
    int32_t ru = RegUsed;

    if (ru & 0xFF000000) {
	short i;
	for (i = 24; i < 32; ++i) {
	    if (ru & (1 << i))
		++cnt;
	}
    }
    return(cnt);
}
#endif

/*
 *  Save the registers specified by mask either by pusing them onto the
 *  stack or by allocating frame pointer space.
 */

void
asm_save_regs(mask)
uint32_t mask;
{
    short cnt = 0;
    char *regstr;

    if (mask) {
	regstr = RegMaskToString(mask, &cnt);
	if (cnt > 0) {
	    if (cnt == 1)
		printf("\tmove.l\t%s,-(sp)\n", regstr);
	    else
		printf("\tmovem.l\t%s,-(sp)\n", regstr);
	}
    }
}

/*
 *  restore registers specified from the stack or from the fp offset.
 */

int
asm_restore_regs(mask)
uint32_t mask;
{
    short cnt = 0;
    char *regstr;

    if (mask) {
	regstr = RegMaskToString(mask, &cnt);
	if (cnt > 0) {
	    if (cnt == 1)
		printf("\tmove.l\t(sp)+,%s\n", regstr);
	    else
		printf("\tmovem.l\t(sp)+,%s\n", regstr);
	}
    }
    return((int)cnt);
}

/*
 *  Ensure that the storage is not a scratch register.	All we are doing
 *  is changing the register, so we do a direct copy ignoring other
 *  parameters (e.g. 4(A0) -> 4(A2)  ==  move.l A0,A2)
 */

void
UnscratchStorage(exp)
Exp *exp;
{
    if (exp->ex_Stor.st_Type == ST_RegIndex) {
	if ((1 << exp->ex_Stor.st_RegNo) & REGSCRATCH) {
	    int32_t oldLocked = RegLocked;
	    Stor st;			    /*	new reg */
	    Stor sd = exp->ex_Stor;	    /*	old reg */

	    sd.st_Type = ST_Reg;
	    sd.st_Size = 4;
	    sd.st_Flags &= ~SF_LEA;
	    sd.st_RegNo = exp->ex_Stor.st_RegNo;

	    RegLocked |= REGSCRATCH;
	    if (exp->ex_Stor.st_RegNo < RB_ADDR) {
		AllocDataRegister(&st, 4);
	    } else {
		AllocAddrRegister(&st);
	    }
	    asm_move(exp, &sd, &st);
	    FreeStorage(&sd);
	    RegLocked = oldLocked;
	    exp->ex_Stor.st_RegNo = st.st_RegNo;
	}

	if ((1 << exp->ex_Stor.st_RegNo2) & REGSCRATCH) {
	    int32_t oldLocked = RegLocked;
	    Stor st;			    /*	new reg */
	    Stor sd = exp->ex_Stor;	    /*	old reg */

	    sd.st_Type = ST_Reg;
	    sd.st_Size = 4;
	    sd.st_Flags &= ~(SF_LEA|SF_TMP);
	    sd.st_RegNo = exp->ex_Stor.st_RegNo2;
	    if (sd.st_Flags & SF_TMP2)	    /*	ST_RegIndex->ST_Reg */
		sd.st_Flags |= SF_TMP;

	    RegLocked |= REGSCRATCH;
	    if (exp->ex_Stor.st_RegNo2 < RB_ADDR) {
		AllocDataRegister(&st, 4);
	    } else {
		AllocAddrRegister(&st);
	    }
	    asm_move(exp, &sd, &st);

	    FreeStorage(&sd);

	    RegLocked = oldLocked;
	    exp->ex_Stor.st_RegNo2 = st.st_RegNo;
	}
    }

    if (exp->ex_Stor.st_Type == ST_Reg || exp->ex_Stor.st_Type == ST_RelReg) {
	if ((1 << exp->ex_Stor.st_RegNo) & REGSCRATCH) {
	    int32_t oldLocked = RegLocked;
	    Stor st;
	    Stor sd = exp->ex_Stor;

	    sd.st_Type = ST_Reg;
	    sd.st_Size = 4;
	    sd.st_Flags &= ~SF_LEA;

	    RegLocked |= REGSCRATCH;
	    if (exp->ex_Stor.st_RegNo < RB_ADDR) {
		AllocDataRegister(&st, 4);
	    } else {
		AllocAddrRegister(&st);
	    }
	    asm_move(exp, &sd, &st);
	    FreeStorage(&sd);
	    RegLocked = oldLocked;
	    exp->ex_Stor.st_RegNo = st.st_RegNo;
	}
    }
}

/*
 *  These routines track stack temporaries.  All stack temporaries are
 *  4-byte aligned.
 *
 *  NOTE!  KNOWN BUG... If a dot operation on a temporary structure
 *  indexes past the end of the structure (i.e. you index an array
 *  with a constant past the end of the structure) a software error
 *  will occur because it will not be able to find the temporary
 *  memory block related to the original allocated structure
 */

void
AllocStackStorage(stor, type, flags)
Stor *stor;
Type *type;
int32_t flags;
{
    Frame *frame = &CurGen->Frame;
    int32_t alignment = *type->Align;
    int32_t size = *type->Size;


    /*
     *	fix alignment.	Note that auto char arrays must be aligned on a
     *	word boundry if they are auto-aggregate initialized.  I do it
     *	in general to cover the case
     */

    if ((flags & TF_ALIGNED) && alignment < 4)
	alignment = 4;
    if (*type->Size > 1 && alignment == 1)
	alignment = 2;
    Assert(CurGen);

    /*
     *	type alignment assumes alignment is a power of 2.  First add the
     *	size of the type to StackUsed.	Since the offset will eventually be
     *	negative this represents the minimum amount of space required to
     *	store the item.
     */

    frame->StackUsed += size;

    /*
     *	Additional space may be required to align the item.
     */

    {
	if (alignment > 1) {
	    short n = ((frame->StackParent + frame->StackUsed) & (alignment - 1));
	    if (n)
		frame->StackUsed += alignment - n;
	}
    }

    stor->st_Type = ST_RelReg;
    stor->st_RegNo = RB_FP;
    stor->st_Size = size;
    stor->st_Offset = -(frame->StackParent + frame->StackUsed);
    stor->st_Flags = SF_VAR | SF_NOSA;
    if (flags & TF_UNSIGNED)
	stor->st_Flags |= SF_UNSIGNED;

    dbprintf(("; AllocStackStorage(%ld)\n", stor->st_Offset));
}


void
ReUseStackStorage(stor)
Stor *stor;
{
    TmpStack *ts;

    dbprintf(("; ReUseStackStorage(%ld)\n", stor->st_Offset));

    for (ts = TmpAry; ts->ts_Size; ++ts) {
	/*if (stor->st_Offset == ts->ts_Offset && stor->st_Size <= ts->ts_Size) {*/
	if (stor->st_Offset >= ts->ts_Offset && stor->st_Offset < ts->ts_Offset + ts->ts_Size) {
	    ++ts->ts_Refs;
	    return;
	}
    }
    dbprintf(("reusetmp %08lx", stor->st_Offset));
    Assert(0);
}

/*
 *  find entry to free.
 */

void
FreeStackStorage(stor)
Stor *stor;
{
    TmpStack *ts;

    dbprintf(("; FreeStackStorage(%ld)\n", stor->st_Offset));

    for (ts = TmpAry; ts->ts_Size; ++ts) {
	/*if (stor->st_Offset == ts->ts_Offset) {*/

	if (stor->st_Offset >= ts->ts_Offset && stor->st_Offset < ts->ts_Offset + ts->ts_Size) {
	    --ts->ts_Refs;
	    if (ts->ts_Refs == 0)
		dbprintf(("; REFS = 0 OFFSET %04x\n", (uword)stor->st_Offset));
	    Assert(ts->ts_Refs >= 0);
	    return;
	}
    }
    dbprintf(("tmpstk %08lx", stor->st_Offset));
    Assert(0);
}

/*
 *  PushStackStorage invalidates any temporary storage beyond TSIndex.
 */

void
PushStackStorage(void)
{
    TmpStack *ts = TmpAry + TSIndex;

    Assert(TSIndex < TMP_STACK_MAX - 1);
    ts->ts_Size = -1;
    ts->ts_Offset = 0;
    ++ts;
    ++TSIndex;

    while (TSIndex < TSMax) {
	ts->ts_Size = 0;
	ts->ts_Offset = 0;
	--TSMax;
	++ts;
    }

    /*
     *	must guarentee a 0-size structure so other loops do not go beyond
     *	TSMax
     */

    ts->ts_Size = 0;
}

/*
 *  PopStackStorage() backs TSIndex up but does not delete the temporary
 *  stack cache due to possible return temporaries to the next level up.
 *
 *  The reason the cache must be cleared is because, unlike register
 *  allocation, normal stack allocation of variables does not take
 *  the cache into account.
 */

void
PopStackStorage(void)
{
    TmpStack *ts = TmpAry + (TSIndex - 1);

    while (ts >= TmpAry) {
	if (ts->ts_Size == -1) {
	    ts->ts_Size = 0;
	    --TSIndex;
	    return;
	} else {
	    /* ts->ts_Size = 0; */
	    --TSIndex;
	    --ts;
	}
    }
    Assert(0);
}


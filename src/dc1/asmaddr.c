/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASMADDR.C
 *
 *  These are the only routines that munge storage structures to get to sub
 *  types.  The only big thing we have to worry about is indirecting or
 *  indexing into arrays where the destination is an array... we must be
 *  sure to set the SF_LEA flag in these cases.
 */

#include "defs.h"
#include "asm.h"

Prototype void asm_getindex(Exp *, Type *, Stor *, Stor *, int32_t, Stor *, short, short);
Prototype void asm_getind(Exp *, Type *, Stor *, Stor *, short, short, short);
Prototype void asm_getlea(Exp *, Stor *, Stor *);
Prototype void asm_lea(Exp *, Stor *, int32_t, Stor *);

/*
 *  asm_getindex(type, s, off, mpx, d, sign, dexists)
 *
 *  index a pointer s by off, putting the resulting pointer into d.
 *
 *  This is the core code that modifies offsets and must ensure that
 *  offsets for RelReg do not go out of range (-32768 + 4 to 32767 - 4)
 *
 *  watch out for multiply
 */

void
asm_getindex(
    Exp *exp,
    Type *type,
    Stor *s,
    Stor *soff,
    int32_t mpx,
    Stor *d,
    short sign,
    short dexists	/*  destination exists	*/
) {
    int32_t flags = s->st_Flags | SF_UNSIGNED;
    short totmp = 0;	/*  can we modify to?	*/
    Stor toff;
    Stor td;

    if (type->Id == TID_ARY)
	Assert(flags & SF_LEA);

    if (soff->st_Type == ST_IntConst) {
	int32_t newOffset = soff->st_IntConst * mpx * sign;

	if (newOffset == 0) {
	    if (dexists)
		asm_move(exp, s, d);
	    else
		ReuseStorage(s, d);
	    return;
	}

	switch(s->st_Type) {
	case ST_IntConst:
	    if (dexists) {
		ReuseStorage(s, &td);
		td.st_IntConst += newOffset;
		td.st_Flags = flags;
		asm_move(exp, &td, d);
		FreeStorage(&td);
	    } else {
		ReuseStorage(s, d);
		d->st_IntConst += newOffset;
		d->st_Flags = flags;
	    }
	    return;
	case ST_PtrConst:	/*  e.g. *(int32_t *)4	*/
	    if (s->st_Flags & SF_LEA) {
		if (dexists) {
		    ReuseStorage(s, &td);
		    td.st_Offset += newOffset;
		    td.st_Flags = flags;
		    asm_move(exp, &td, d);
		    FreeStorage(&td);
		} else {
		    ReuseStorage(s, d);
		    d->st_Offset += newOffset;
		    d->st_Flags = flags;
		}
		return;
	    }
	    break;
	case ST_Reg:
	    if (s->st_RegNo >= RB_ADDR) {
		if (newOffset < -32768 + LGBO_SIZE || newOffset > 32767 - LGBO_SIZE)
		    break;
		if (dexists) {
		    ReuseStorage(s, &td);
		    td.st_Type = ST_RelReg;
		    td.st_Offset = newOffset;
		    td.st_Flags |= SF_LEA | SF_UNSIGNED;
		    td.st_Size = PTR_SIZE;
		    asm_move(exp, &td, d);
		    FreeStorage(&td);
		} else {
		    ReuseStorage(s, d);
		    d->st_Type = ST_RelReg;
		    d->st_Offset = newOffset;
		    d->st_Flags |= SF_LEA | SF_UNSIGNED;
		    d->st_Size = PTR_SIZE;
		}
		return;
	    }
	    break;
	case ST_RelArg:
	case ST_RelReg:
	    if (flags & SF_LEA) {
		if (newOffset + s->st_Offset < -32768 + LGBO_SIZE || newOffset + s->st_Offset > 32767 - LGBO_SIZE)
		    break;
		if (dexists) {
		    ReuseStorage(s, &td);
		    td.st_Flags = flags;
		    td.st_Offset += newOffset;
		    asm_move(exp, &td, d);
		    FreeStorage(&td);
		} else {
		    ReuseStorage(s, d);
		    d->st_Flags = flags;
		    d->st_Offset += newOffset;
		}
		return;
	    }
	    break;
	case ST_RegIndex:
	    if (flags & SF_LEA) {
		if (newOffset + s->st_Offset < -128 + LGBO_SIZE || newOffset + s->st_Offset > 127 - LGBO_SIZE)
		    break;
		if (dexists) {
		    ReuseStorage(s, &td);
		    td.st_Flags = flags;
		    td.st_Offset += newOffset;
		    asm_move(exp, &td, d);
		    FreeStorage(&td);
		} else {
		    ReuseStorage(s, d);
		    d->st_Flags = flags;
		    d->st_Offset += newOffset;
		}
		return;
	    }
	    break;
	case ST_RelLabel:
	case ST_RelName:
	    if (flags & SF_LEA) {
		if (dexists) {
		    ReuseStorage(s, &td);
		    td.st_Flags = flags;
		    td.st_Offset += newOffset;	/* no ran limit, large data model */
		    asm_move(exp, &td, d);
		    FreeStorage(&td);
		} else {
		    ReuseStorage(s, d);
		    d->st_Flags = flags;
		    d->st_Offset += newOffset;	/* no ran limit, large data model */
		}
		return;
	    }
	    break;
	default:
	    dbprintf(("bad st type asm_offset/const %d\n", s->st_Type));
	    Assert(0);
	}

	if (dexists == 0) {
	    AllocAddrRegister(d);
	    asm_move(exp, s, d);

	    if (newOffset < -32768 + LGBO_SIZE || newOffset > 32767 - LGBO_SIZE) {
		Stor con;
		AllocConstStor(&con, newOffset, &LongType);

		d->st_Type = ST_Reg;
		d->st_Offset= 0;
		d->st_Flags |= SF_UNSIGNED;
		asm_add(exp, d, &con, d);
	    } else {
		d->st_Type = ST_RelReg;
		d->st_Offset = newOffset;
		d->st_Flags |= SF_LEA | SF_UNSIGNED;
	    }
	} else {
	    Stor con;
	    AllocConstStor(&con, newOffset, &LongType);
	    asm_add(exp, s, &con, d);
	}

	return;
    }

    /*
     *	d = s + soff * mpx * sign, create d
     *
     *	may have to generate a temporary offset
     */

    LockStorage(s);

    /*
     *	to = offset, guarenteed in register and either int32_t or signed short.
     */

    if (soff->st_Type != ST_Reg || soff->st_Size == 1 || (soff->st_Size == 2 && (soff->st_Flags & SF_UNSIGNED))) {
	if (soff->st_Size == 1 || (soff->st_Size == 2 && !(soff->st_Flags & SF_UNSIGNED)))
	    AllocDataRegister(&toff, 2);
	else
	    AllocDataRegister(&toff, 4);
	asm_ext(exp, soff, &toff, soff->st_Flags);
	totmp = 1;		/*  we created it, can modify it */
    } else {
	ReuseStorage(soff, &toff);
    }

    /*
     *	Handle multipliers that cannot be handled inherently
     */

    if (sign < 0 || mpx != 1) {
	Stor con;
	AllocConstStor(&con, mpx * sign, &LongType);

	/*
	 *  should we check the type for a 'short' range?
	 */

	if (totmp) {
	    if (toff.st_Size == 2) {
		outop("ext", 4, NULL, &toff);
		toff.st_Size = 4;
	    }
	    asm_mul(exp, &con, &toff, &toff);
	} else {
	    FreeStorage(&toff);
	    AllocDataRegister(&toff, 4);
	    asm_ext(exp, soff, &toff, soff->st_Flags);
	    totmp = 1;		    /*	we created it, can modify it */
	    asm_mul(exp, &con, &toff, &toff);
	}
	mpx = 1;
	sign = 1;
    }

    /*
     *	&off(An)    d = to + s + offset
     */

    if (dexists) {
	if (toff.st_Size != d->st_Size) {   /*	i.e. toff.st_Size == 2 */
	    if (totmp) {
		outop("ext", 4, NULL, &toff);
		toff.st_Size = 4;
	    } else {
		FreeStorage(&toff);
		AllocDataRegister(&toff, d->st_Size);
		asm_ext(exp, soff, &toff, soff->st_Flags);
		totmp = 1;
	    }
	}
	asm_add(exp, s, &toff, d);
    } else {
	if ((s->st_Type == ST_Reg && s->st_RegNo >= RB_ADDR) || ((flags & SF_LEA) && s->st_Type == ST_RelReg && s->st_Offset >= -128 && s->st_Offset < 128)) {
	    ReuseStorage(s, d);
	    if (toff.st_Size == 2)
		d->st_Flags |= SF_IDXWORD;
	    if (toff.st_Flags & SF_TMP)
		d->st_Flags |= SF_TMP2;

	    d->st_RegNo2 = toff.st_RegNo;
	    d->st_Offset = 0;
	    if (s->st_Type == ST_RelReg)
		d->st_Offset = s->st_Offset;
	    d->st_Type = ST_RegIndex;
	    d->st_Flags |= SF_LEA | SF_UNSIGNED;
	    totmp = -1;     /*	now part of d	*/
	} else {
	    LockStorage(soff);
	    AllocAddrRegister(d);
	    UnlockStorage(soff);
	    if (toff.st_Size == 2) {
		asm_move(exp, s, d);
		outop("add", toff.st_Size, &toff, d);
	    } else {
		asm_add(exp, s, &toff, d);
	    }
	    d->st_Type = ST_RelReg;
	    d->st_Offset = 0;
	    d->st_Flags |= SF_LEA | SF_UNSIGNED;
	}
    }

    if (totmp != -1)
	FreeStorage(&toff);   /*  in case to is a register, can't use FreeReg*() */
    UnlockStorage(s);
}



/*
 *  asm_getind()
 *
 *	create a storage structure d that represents what s is pointing
 *	to.  The type of s is 'type'.  The size of the object being
 *	pointed to is *type->SubType->Size.
 *
 *	BITFIELDS:  If type->SubType->Id == TID_BITFIELD then
 *		    d->st_BOffset and d->st_BSize is preserved.
 *		    (use short access if possible)
 */

void
asm_getind(
    Exp *exp,
    Type *type,
    Stor *s,
    Stor *d,
    short boff,
    short bsize,
    short dexists
) {
    int32_t flags = s->st_Flags;
    short subtypeary __unused = 0;
    int32_t size;
    Type *subType;
    Stor td;

    if (type->Id != TID_ARY && type->Id != TID_PTR) {
	yerror(exp->ex_LexIdx, EERROR_BAD_IND_TYPE);
	subType = &LongType;
    } else {
	subType = type->SubType;
    }
    size = *subType->Size;

    if (type->Id == TID_ARY)
	Assert(flags & SF_LEA);

    if (subType->Id == TID_ARY)
	subtypeary = 1;

    switch(s->st_Type) {
    case ST_IntConst:
	ReuseStorage(s, &td);
	td.st_Type = ST_PtrConst;
	td.st_Offset= s->st_IntConst;
	goto done;
    case ST_PtrConst:
	if (s->st_Flags & SF_LEA) {
	    ReuseStorage(s, &td);
	    td.st_Flags &= ~SF_LEA;
	    goto done;
	}
	break;
    case ST_Reg:
	if (s->st_RegNo >= RB_ADDR) {
	    ReuseStorage(s, &td);
	    td.st_Type = ST_RelReg;
	    td.st_Offset = 0;
	    goto done;
	}
	break;
    case ST_RelArg:
    case ST_RelReg:
    case ST_RegIndex:
    case ST_RelLabel:
    case ST_RelName:
	if (flags & SF_LEA) {
	    ReuseStorage(s, &td);
	    goto done;
	}
	break;
    default:
	dbprintf(("bad st type asm_ind %d\n", s->st_Type));
	Assert(0);
    }

    AllocAddrRegister(&td);
    asm_move(exp, s, &td);
    td.st_Type = ST_RelReg;
    td.st_Offset = 0;
done:
    /*
     *	td is the result after indirection.  if dexists then it must be
     *	moved to d, else it becomes d.
     */

    td.st_Size = size;

    /*
     *	handle bit fields.  Note that GenStructInd/Str handles all non
     *	assignments of the bitfield storage type so only GenAss() really
     *	needs to understand bit fields.
     */

    if (dexists == 0) {
	td.st_Size = size;
	td.st_BOffset = boff;
	td.st_BSize   = bsize;
    } else if (bsize >= 0) {
	dbprintf(("bitfield/aa409"));
	Assert(0);
    }

    if (bsize == 0)
    {
	dbprintf(("bsize is 0 (asm_getind)"));
	Assert(0);
    }
    if (bsize > 0) {	       /*  bitfield    */
	td.st_Flags  |= SF_BITFIELD;
	td.st_Size    = 4;
	Assert(td.st_Type != ST_Reg);
    }

    if (subType->Id == TID_ARY) {
	td.st_Flags |= SF_LEA;
	td.st_Size = PTR_SIZE;
    } else {
	td.st_Flags &= ~SF_LEA;
    }

    if (subType->Flags & TF_UNSIGNED)
	td.st_Flags |= SF_UNSIGNED;
    else
	td.st_Flags &= ~SF_UNSIGNED;

    if (dexists) {
	FreeStorage(&td);
	asm_move(exp, &td, d);
    } else {
	*d = td;
    }
    if (d->st_Size == 0)
	yerror(exp->ex_LexIdx, EERROR_VOID_IND_TYPE);

}

/*
 *  asm_getlea(s, d)
 *
 *  allocates d, loads the address of s
 */

void
asm_getlea(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    switch(s->st_Type) {
    case ST_IntConst:
	yerror(exp->ex_LexIdx, EERROR_ADDRESS_INT_CONST);
	/* fall through */
    case ST_PtrConst:
	ReuseStorage(s, d);
	d->st_Type = ST_IntConst;
	d->st_Flags |= SF_UNSIGNED;
	d->st_Size = PTR_SIZE;
	d->st_IntConst = s->st_Offset;
	return;
    case ST_Reg:
	dbprintf(("ASM_GETLEA OF REGISTER!"));
	Assert(0);
	return;

    case ST_RelReg:
    case ST_RegIndex:
    case ST_RelArg:
    case ST_RelLabel:
    case ST_RelName:
	ReuseStorage(s, d);
	d->st_Flags |= SF_LEA | SF_UNSIGNED;
	d->st_Size = PTR_SIZE;
	break;
    default:
	dbprintf(("bad st type asm_lea %d\n", s->st_Type));
	Assert(0);
    }
}

/*
 *  asm_lea(s, offset, d)
 *
 *  load effective address of s + N into d, which must be an address
 *  register.
 */

void
asm_lea(exp, s, offset, d)
Exp *exp;
Stor *s, *d;
int32_t offset;
{
    if (s->st_Type == ST_RelReg) {
	Stor t = *s;

	t.st_Offset += offset;
	t.st_Size = PTR_SIZE;	/*  size of eff addr	*/
	t.st_Flags |= SF_LEA;
	if (t.st_Offset < -32768 || t.st_Offset > 32767) {
	    t.st_Offset = 0;
	    asm_move(exp, &t, d);
	    /*outop("lea", -1, &t, d);*/
	    t.st_Type = ST_Reg;
	    AllocConstStor(&t, s->st_Offset + offset, &LongType);
	    asm_add(exp, d, &t, d);
	} else {
	    asm_move(exp, &t, d);
	    /*outop("lea", -1, &t, d);*/
	}
    } else {
	int32_t  siz = s->st_Size;
	int32_t  flg = s->st_Flags;

	s->st_Flags |= SF_LEA;
	s->st_Size = PTR_SIZE;

	asm_move(exp, s, d);
	if (offset) {
	    Stor con;
	    AllocConstStor(&con, offset, &LongType);
	    asm_add(exp, d, &con, d);
	}
	s->st_Flags = flg;
	s->st_Size = siz;
    }
}


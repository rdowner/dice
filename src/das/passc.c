/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PASSC.C
 *
 *  OPTIMIZATION PASS.	Note that the machine list can be sent to PASSG
 *  without further modification whether or not this pass is run.  This
 *  pass does simple optimization (-O1):
 *
 *	(1) remove MOVEM instructions of REG equates for 0 registers
 *
 *	(2) convert MOVEM instructions of REG equates for 1 register to MOVE
 *
 *	(3) convert Bcc l1 .. l1 Bra L2  to Bcc l2 (note that if a short
 *	    branch is forced the opt will not occur if l2 is out of range)
 *
 *	(4) check sizes branches, set size (b,w) for unsized branches
 *	    (this is ok because our optimizations never make the code
 *	     larger than it currently is)
 *
 *	(5) convert JSR x(pc) into a BSR x(pc) if it can be reduced to a byte
 *	    offset.
 *
 *	note that link/unlk removal is not done in this pass, but in the
 *	full opt pass because removal of link/unlk kills debugging.
 *
 *	WARNING:    forward offsets for labels not correct since we modify
 *		    instructions, but can be considered a worst case range.
 *
 *	WARNING:    when using GetOpByName() remember that there may be
 *		    sequential opcodes with the same name and different
 *		    formats.
 */

#include "defs.h"

Prototype long	PassC(long addr);

long
PassC(long addr)
{
    MachCtx **pmc;
    MachCtx *mc;
    OpCod   *oc;
    long i;
    long lost = 0;	/*  # bytes of code *deleted* so far */
    MachCtx *lsb = NULL;
    EffAddr *ea;

    pmc = CurFile->fn_MBase;
    for (i = 0; i < CurFile->fn_MLines; ++i, ++pmc) {
	mc = *pmc;
	CurFile->fn_LineNo = mc->LineNo;
	if (mc->Label) {
	    if (lsb && mc->Label == lsb->Oper1.Label1 && addr == lsb->m_Addr + 2 && lsb->Oper1.Mode1 == AB_BBRANCH && lsb->OpCode->Id != OpBSR) {
		/*
		 *  branch to next location... backup, delete it, continue on (handles multiple deletes)
		 */
		if (Verbose)
		    cerror(EVERBOSE_REMOVED_BRANCH_NEXT);
		while (i >= 0 && lsb != *pmc) {
		    --i;
		    --pmc;
		}
		if (i < 0)
		    cerror(ESOFT_BRANCH_OPT);
		lsb->OpCode = NULL;
		addr = lsb->m_Addr;
		lost += 2;

		/*
		 *  find previous inst.. if a branch set lsb to that.
		 *  Ignore DEBUG pseudoops which do not insert any code
		 */
		{
		    MachCtx **ppmc = pmc;
		    long ii = i;
		    while (i >= 0 && ((*ppmc)->OpCode == NULL || (*ppmc)->OpCode->Id == OdDEBUG)) {
			--ii;
			--ppmc;
		    }
		    if (ii >= 0)
			lsb = *ppmc;
		}
		continue;
	    }
	    mc->Label->l_Offset = addr;
	}
	mc->m_Addr = addr;
	if ((oc = mc->OpCode) == NULL)
	    continue;
	if (oc->Id < 0) {
	    addr = ExecOpCodeC(mc, addr);
	    continue;
	}

	switch(oc->Id) {
	case OpMOVEM:				/*  MOVEM optimization */
	    if (mc->Oper1.Mode1 == AB_REGS)
		ea = &mc->Oper1;
	    else
		ea = &mc->Oper2;
	    if (ea->Label1 && ea->Label1->l_Type == LT_REG) {
		if (ea->ExtWord == 0) {
		    mc->OpCode = NULL;		/*  delete inst  */
		    lost += 4;
		    continue;
		}
		if (ea->Reg1 >= 0)  {   /*  just one reg.. -> MOVE */
		    if (ea->ExtWord & ~(1 << ea->Reg1))
			cerror(ESOFT_BAD_MASK_REG, ea->Reg1, ea->ExtWord);
		    mc->OpCode = oc = GetOpByName("MOVE");
		    if (ea->Reg1 >= RB_AREG) {
			if (ea == &mc->Oper2)
			    mc->OpCode = oc = GetOpByName("MOVEA");
			ea->Mode1 = AB_AN;
		    } else {
			ea->Mode1 = AB_DN;
		    }
		    lost += 2;
		    ea->Label1 = NULL;
		}
	    }
	    break;
	case OpJSR:				/*  JSR optimization	*/
	    /*
	     *	convert a pc-relative JSR to BSR.W (easy to do since no
	     *	change in instruction size)
	     */
	    if (mc->Oper1.Mode1 != AB_OFFPC)
		break;
	    oc -= 2;
	    mc->OpCode = oc;
	    mc->Oper1.Mode1 = AB_WBRANCH;
	    mc->OpSize = 0;
	    /* fall through */
	case OpBCC:				/*  BRANCH optimization */
	case OpBCS:
	case OpBEQ:
	case OpBGE:
	case OpBGT:
	case OpBHI:
	case OpBLE:
	case OpBLS:
	case OpBLT:
	case OpBMI:
	case OpBNE:
	case OpBPL:
	case OpBVC:
	case OpBVS:
	case OpBRA:
	case OpBSR:
	    if (mc->Oper1.Offset1 == 0) {
		Label *lab;
		long cnt = 20;	/*  handle loops */

		while ((lab = mc->Oper1.Label1) && lab->l_Type == LT_LOC && lab->Sect == CurSection && cnt--) {
		    MachCtx **ppmc = lab->MC;
		    MachCtx *xmc;

		    while (*ppmc && (*ppmc)->OpCode == NULL)    /*  skip nothings   */
			++ppmc;
		    if ((xmc = *ppmc) == NULL)                  /*  end of assembly */
			break;
		    if (xmc->Sect == CurSection && xmc->OpCode->Id == OpBRA) {            /*  branch to branch */
			if (mc == xmc)                          /*  bra to self..   */
			    break;
			mc->Oper1.Label1 = xmc->Oper1.Label1;
			if (Verbose)
			    cerror(EVERBOSE_BRANCH_BRANCH);
			continue;
		    }

		    /*
		     *	mc label is local.
		     *	    (1) remove branch entirely if it
		     *		branches to the next loc... (actually done
		     *		at top of loop)
		     *
		     *	    (2) optimize to byte branch if poss.  note: will not
		     *		know about bra-to-next-addr until later.
		     *
		     *
		     *	    When optimizing branches we must take into account
		     *	    the difference in PC and actual PC.. the 'lost'
		     *	    variable, due to removing previous instructions and
		     *	    the l_Offset's of later ones being wrong by 'lost'
		     *	    bytes.
		     */

		    if (mc->OpSize == 0) {
			long range = mc->Oper1.Label1->l_Offset - (addr + 2);

			if (mc->Oper1.Label1->l_Offset > (addr + 2))
			    range -= lost;

			/*
			 *  hack hack hack. need to convert to byte
			 *  branch for instruction deletion but can't
			 *  delete BSR's so must be sure we do not convert
			 *  to a BSR to the next instruction
			 */

			if (range >= -126 && range < 126 && range) {
			    if (oc->SModes & AF_WBRANCH) {
				if (range != 2 || oc->Id != OpBSR) {
				    mc->OpCode = ++oc;
				    mc->Oper1.Mode1 = AB_BBRANCH;
				    lost += 2;
				}
			    }
			} else {
			    if (oc->SModes & AF_BBRANCH) {
				mc->OpCode = --oc;
				mc->Oper1.Mode1 = AB_WBRANCH;
				lost -= 2;
			    }
			}
			lsb = mc;
		    }
		    break;
		}
	    }
	    break;
	case OpLINK:
	    if (Optimize >= 1) {
		if ((GetFlags() & ~MF_CALLMADE) == 0 && mc->Oper1.Label1 == NULL && mc->Oper1.Offset1 == 0) {
		    lost += 4;
		    mc->OpCode = NULL;		/*  delete inst  */
		    continue;
		} else {
		    SetA5UsedFlag();
		}
	    }
	    break;
	case OpUNLK:
	    if (Optimize >= 1) {
		if ((GetFlags() & ~MF_CALLMADE) == 0) {
		    lost += 2;
		    mc->OpCode = NULL;
		    continue;
		}
	    }
	    break;
	default:
	    lsb = NULL; /*  efficiency boost, not reqd */
	}
	addr += GetInstSize(mc);
    }
    return(addr);
}




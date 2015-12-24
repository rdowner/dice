/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DIRECT.C
 *
 *  Directives
 *
 */

/*
**      $Filename: direct.c $
**      $Author: dillon $
**      $Revision: 1.2 $
**      $Date: 1999/02/08 21:47:52 $
**      $Log: direct.c,v $
**      Revision 1.2  1999/02/08 21:47:52  dillon
**
**
**          Add double-quoted ( verses single-quoted ) strings for dc.b
**
**          Add DBRA, synonym for DBT.
**
**      Revision 1.1.1.1  1998/10/10 06:23:22  dillon
**
**
**          Initial import of DICE into CVS repository.  Logs for the thousands of
**          changes prior to this import have essentially been lost   -Matt.
**
 * Revision 30.5  1994/06/13  18:41:08  dice
 * byte ordering portability
 * align opcode was broken
 *
 * Revision 30.0  1994/06/10  18:07:43  dice
 * .
 *
 * Revision 1.2  1993/09/19  20:54:09  jtoebes
 * Fixed BUG06033 - DAS does not report original C source line numbers.
 * Handled setting a global variable to the current line being processed.
 *
 * Revision 1.1  1993/09/19  20:19:25  jtoebes
 * Initial revision
 *
**/

#include "defs.h"

Prototype   MachCtx *ProcCtx;
Prototype   short   MC68020;
Prototype   short   MC68881;
Prototype   int32_t    DebugLineNo;

Prototype   void    BadLabel(Label **);
Prototype   void    DeleteTrailingSpaces(char *);
Prototype   void    ExecOpCodeA(MachCtx *);
Prototype   int32_t    ExecOpCodeB(MachCtx *, int32_t);
Prototype   int32_t    ExecOpCodeC(MachCtx *, int32_t);
Prototype   int32_t    ExecOpCodeG(MachCtx *, int32_t);
Prototype   void    SetForcedLinkFlag(void);
Prototype   void    SetSubroutineCallFlag(void);
Prototype   void    SetA5UsedFlag(void);
Prototype   int32_t    GetFlags(void);
Prototype   void    StrToAlign(char *, int32_t *, int32_t *, int32_t);

MachCtx *ProcCtx;
short	MC68020;
short	MC68881;
int32_t    DebugLineNo;

void
BadLabel(plab)
Label **plab;
{
    *plab = GetLabelByName("____dummy____");
    cerror(EERROR_DIRECTIVE_REQUIRES_LABEL);
}

void
DeleteTrailingSpaces(str)
char *str;
{
    char *ptr = str + strlen(str) - 1;

    /* if (*str) check avoids bug in GNU-C -O/-O2 options */

    if (*str) {
	while (ptr >= str) {
	    if (*ptr == ' ' || *ptr == 9) {
		--ptr;
		continue;
	    }
	    break;
	}
    	++ptr;
    	*ptr = 0;
    }
}

/*
 *  Pass A
 *
 */

void
ExecOpCodeA(mc)
MachCtx *mc;
{
    OpCod *oc = mc->OpCode;
    char *str = mc->m_Operands;
    Label *lab;

    DeleteTrailingSpaces(str);
    switch(oc->Id) {
    case OdEQU:
	if ((lab = mc->Label) == NULL) {
	    BadLabel(&lab);
	}
	mc->Label = NULL;
	mc->m_XLabel = lab;
	lab->l_Type = LT_INT;
	lab->l_Value = ParseIntExp(str);
	dbprintf(0, ("label %s value %ld\n", lab->Name, lab->l_Value));
	break;
    case OdREG:     /*	RegList */
	if ((lab = mc->Label) == NULL)
	    BadLabel(&lab);
	mc->Label = NULL;
	mc->m_XLabel = lab;
	lab->l_Type = LT_REG;
	(void)ParseRegList(str, &lab->l_Mask);
	lab->l_RegNo =	OnlyOneRegister(lab->l_Mask);
	break;
    case OdXREF:
	{
	    Label *lab = GetLabelByName(str);
	    lab->l_Type = LT_EXT;
	    mc->m_XLabel = lab;
	}
	break;
    case OdXDEF:
	{
	    Label *lab = GetLabelByName(str);
	    mc->m_XLabel = lab;
	    /* label added to xdef list in passB */
	}
	break;
    case OdDS:
	/*
	 *  handle COMMON DS statements
	 */

	if (mc->Sect && mc->Sect->Type == SECT_COMMON) {
	    int32_t value;

	    if ((value = ParseIntExp(str)) != 0) {
		if ((lab = mc->Label) != NULL) {
		    mc->Label = NULL;
		    mc->m_XLabel = lab;
		    lab->l_Type = LT_EXT;
		    lab->l_Value = ParseIntExp(str);
		    dbprintf(0, ("common label %s value %ld\n", lab->Name, lab->l_Value));
		} else {
		    BadLabel(&lab);
		}
	    }
	}
    case OdDC:
	mc->m_SaveStr = str;
	break;
    case OdALIGN:
	mc->m_SaveStr = str;
	break;
    case OdSECTION:
	NewSection(str);
	mc->Sect = CurSection;
	break;
    case OdEND:
	break;
    case OdPROCSTART:
	if (*str)
	    Optimize = atoi(str);
	else if (Optimize == 0)
	    Optimize = 1;
	ProcCtx = mc;
	break;
    case OdPROCEND:
	if (*str)
	    SetForcedLinkFlag();
	ProcCtx = NULL;
	break;
    case OdMC68020:
	MC68020 = 1;
	break;
    case OdMC68881:
	MC68881 = 1;
	break;
    case OdDEBUG:
	DebugLineNo = strtol(str, NULL, 0);
        mc->m_DebugLine = DebugLineNo;
	break;
    case OdINCLUDE:
	PushFileNode(mc->m_FileNode = OpenFile(ParseFileName(str)));
	PassA();
	PopFileNode();
	break;
    default:
	cerror(ESOFT_BAD_DIRECTIVE);
    }
}

/*
 *  In pass B add XDEFd labels to the appropriate section list
 *
 *  m_Operands now invalid, contains m_Addr instead
 *
 */

int32_t
ExecOpCodeB(mc, addr)
MachCtx *mc;
int32_t addr;
{
    switch(mc->OpCode->Id) {
    case OdXDEF:
	{
	    Label *lab;
	    if ((lab = mc->m_XLabel) && lab->XDefLink == (void *)-1L)
	    {
	        if (lab->Sect == NULL)
	        {
	    	    lab->Sect = CurSection;
	    	    cerror(EERROR_UNDEFINED_LABEL, lab->Name);
	    	}
		lab->XDefLink = lab->Sect->XDefLab;
		lab->Sect->XDefLab = lab;
	    }
	}
	break;
    case OdSECTION:
	CurSection->Addr = addr;
	CurSection = mc->Sect;
	addr = CurSection->Addr;
	break;
    case OdDS:
        {
	    int32_t t;

	    if (mc->OpSize == 0)
		cerror(EERROR_DIRECT_REQUIRES_SIZE);
	    t = ParseIntExp(mc->m_SaveStr) * mc->OpSize;
	    if (t == 0)				/*  align. XXX align always? */
		t = Align(addr, mc->OpSize) - addr;
	    mc->Bytes = t;
	    addr += mc->Bytes;
	}
	break;
    case OdALIGN:
	{
	    int32_t align = 1;
	    int32_t alignVal = 0;

	    if (mc->OpSize == 0)
		cerror(EERROR_DIRECT_REQUIRES_SIZE);
	    StrToAlign(mc->m_SaveStr, &align, &alignVal, mc->OpSize);

	    mc->Bytes = Align(addr, align * mc->OpSize) - addr;
	    addr += mc->Bytes;
	}
	break;
    case OdDC:
	/*
	 *  Save expression string but count elements.
	 */
	{
	    char *str = mc->m_SaveStr;
	    int32_t t;

	    t = 0;

	    if (*str)
		++t;
	    while (*str) {
		if (*str == '\'') {
		    for (++str; *str && *str != '\''; ++str)
			++t;
		    if (*str == '\'')
			++str;
		    --t;
		} else if (*str == '"') {
		    for (++str; *str && *str != '"'; ++str)
			++t;
		    if (*str == '"')
			++str;
		    --t;
		}
		if (*str == ',')
		    ++t;
		++str;
	    }
	    mc->Bytes = (mc->OpSize == 0) ? t : (t * mc->OpSize);
	}
	addr += mc->Bytes;
	break;
    case OdPROCSTART:
	ProcCtx = mc;
	break;
    case OdPROCEND:
	ProcCtx = NULL;
	break;
    case OdMC68020:
	MC68020 = 1;
	break;
    case OdMC68881:
	MC68881 = 1;
	break;
    case OdDEBUG:
        DebugLineNo = mc->m_DebugLine;
	break;
    case OdINCLUDE:
	PushFileNode(mc->m_FileNode);
	addr = PassB(addr);
	PopFileNode();
    default:
	break;
    }
    return(addr);
}

void
SetForcedLinkFlag()
{
    if (ProcCtx)
	ProcCtx->Bytes |= MF_FORCELINK;
}

void
SetSubroutineCallFlag()
{
    if (ProcCtx)
	ProcCtx->Bytes |= MF_CALLMADE;
}

void
SetA5UsedFlag()
{
    if (ProcCtx)
	ProcCtx->Bytes |= MF_A5USED;
}

int32_t
GetFlags()
{
    if (ProcCtx)
	return(ProcCtx->Bytes);
    return(MF_CALLMADE|MF_A5USED);
}

int32_t
ExecOpCodeC(mc, addr)
MachCtx *mc;
int32_t addr;
{
    switch(mc->OpCode->Id) {
    case OdSECTION:
	CurSection->Addr = addr;
	CurSection = mc->Sect;
	addr = CurSection->Addr;
	break;
    case OdDS:
	addr += mc->Bytes;
	break;
    case OdALIGN:
	{
	    int32_t align = 1;
	    int32_t alignVal = 0;

	    StrToAlign(mc->m_SaveStr, &align, &alignVal, mc->OpSize);

	    mc->Bytes = Align(addr, align) - addr;
	    addr += mc->Bytes;
	}
	break;
    case OdDC:
	addr += mc->Bytes;
	break;
    case OdPROCSTART:
	ProcCtx = mc;
	break;
    case OdPROCEND:
	ProcCtx = NULL;
	break;
    case OdMC68020:
	MC68020 = 1;
	break;
    case OdMC68881:
	MC68881 = 1;
	break;
    case OdDEBUG:
	++CurSection->DebugLen;
	DebugLineNo = mc->m_DebugLine;
	break;
    case OdINCLUDE:
	PushFileNode(mc->m_FileNode);
	addr = PassC(addr);
	PopFileNode();
    default:
	break;
    }
    return(addr);
}


int32_t
ExecOpCodeG(mc, addr)
MachCtx *mc;
int32_t addr;
{
    switch(mc->OpCode->Id) {
    case OdSECTION:
	CurSection->Addr = addr;
	CurSection = mc->Sect;
	addr = CurSection->Addr;
	break;
    case OdDS:
	if (mc->Bytes)
	    DumpSectionData(mc->Sect, NULL, mc->Bytes);
	addr += mc->Bytes;
	break;
  case OdALIGN:
	{
	    int32_t align = 1;
	    int32_t alignVal = 0;
	    int32_t i;
	    char v_char;
	    short v_short;
	    int32_t v_long;

	    StrToAlign(mc->m_SaveStr, &align, &alignVal, mc->OpSize);

	    v_char = alignVal;
	    v_short = ToMsbOrderShort(alignVal);
	    v_long = ToMsbOrder(alignVal);

	    for (i = 0; i < mc->Bytes; i += mc->OpSize) {
		switch(mc->OpSize) {
		case 1:
		    {
			DumpSectionData(mc->Sect, &v_char, 1);
		    }
		    break;
		case 2:
		    {
			DumpSectionData(mc->Sect, &v_short, 2);
		    }
		    break;
		case 4:
		    {
			DumpSectionData(mc->Sect, &v_long, 4);
		    }
		    break;
		}
	    }
	}
	addr += mc->Bytes;
	break;
    case OdDC:
	/*
	 *  Parse m_SaveStr.
	 */
	{
	    EffAddr ea;
	    char *str = mc->m_SaveStr;
	    int32_t bytes = 0;	/*  bytes double check & offset track */
	    short hacksq = 0;	/*  hack-in-single-quote	      */

	    while (*str) {
		setmem(&ea, sizeof(ea), 0);

		if (hacksq) {
		    ea.Offset1 = *str;
		    ++str;
		    if (*str == hacksq) {
			hacksq = 0;
			++str;
		    }
		} else if (*str == '\'') {
		    hacksq = '\'';
		    ++str;
		    continue;
		} else if (*str == '"') {
		    hacksq = '"';
		    ++str;
		    continue;
		} else
		    str = ParseExp(str, &ea.Label1, &ea.Offset1);

		/*
		 *  ignore .W (which DC1 will generate)
		 */

		if (*str == '.' && (str[1] == 'w' || str[1] == 'W'))
		    str += 2;

		/*
		 *  XXX allow exp(pc)
		 *  XXX allow exp(An)	(data rel)
		 */

		if (ea.Label1) {
		    if (ea.Label1->l_Type == LT_LOC)
			ea.Offset1 += ea.Label1->l_Offset;
		    HandleInstReloc(mc->Sect, ea.Label1, addr + bytes, mc->OpSize, ((mc->OpSize == 2) ? RELOC_PCREL : 0));
		}

		switch(mc->OpSize) {
		case 0:
		    cerror(EERROR_DIRECT_REQUIRES_SIZE);
		    break;
		case 1:
		    {
			char c = ea.Offset1;
			DumpSectionData(mc->Sect, &c, 1);
		    }
		    break;
		case 2:
		    {
			short c = ToMsbOrderShort(ea.Offset1);
			DumpSectionData(mc->Sect, &c, 2);
		    }
		    break;
		case 4:
		    {
			int32_t c = ToMsbOrder(ea.Offset1);
			DumpSectionData(mc->Sect, &c, 4);
		    }
		    break;
		}
		if (hacksq == 0 && *str) {
		    if (*str != ',')
			cerror(EERROR_EXPECTED_GOT, *str, *str);
		    ++str;
		}
		bytes += mc->OpSize;
	    }
	    if (bytes != mc->Bytes)
		cerror(ESOFTWARN_DIRECT_BYTES_MISMATCH, mc->Bytes, bytes);
	}
	addr += mc->Bytes;
	break;
    case OdDEBUG:
	{
	    Sect *sect = CurSection;

	    if (sect->DebugAry == NULL)
		sect->DebugAry = zalloc(sizeof(DebugNode) * sect->DebugLen);
	    if (sect->DebugIdx >= sect->DebugLen)
		cerror(ESOFT_DEBUG_INTERNAL_ERROR);
	    sect->DebugAry[sect->DebugIdx].db_Line = mc->m_DebugLine;
	    sect->DebugAry[sect->DebugIdx].db_Offset = addr;
	    ++sect->DebugIdx;
	}
	DebugLineNo = mc->m_DebugLine;
	break;
    case OdINCLUDE:
	PushFileNode(mc->m_FileNode);
	addr = PassG(addr);
	PopFileNode();
    default:
	break;
    }
    return(addr);
}


void
StrToAlign(str, palign, pvalue, size)
char *str;
int32_t *palign;
int32_t *pvalue;
int32_t size;
{
    EffAddr ea;

    setmem(&ea, sizeof(ea), 0);

    str = ParseExp(str, &ea.Label1, &ea.Offset1);
    *palign = ea.Offset1 * size;

    if (*str == ',') {
	ParseExp(str + 1, &ea.Label1, &ea.Offset1);
	*pvalue = ea.Offset1;
    }
}



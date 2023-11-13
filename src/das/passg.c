/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PASSG.C
 *
 *  Generate the code...
 *
 *  Note that mc->OpSize may be 0.  By this point, we take it to mean the
 *  instruction is unsized or single-sized.  This really only effects the
 *  immediate addressing mode.
 */

#include "defs.h"

Prototype int32_t       PassG(int32_t addr);
Prototype int32_t       GenerateInstruction(MachCtx *);
Prototype int32_t       RegMaskForInst(short, int32_t);
Prototype int32_t       GenerateEA(short, MachCtx *, OpCod *, EffAddr *, uword *);

int32_t
PassG(int32_t addr)
{
    MachCtx **pmc;
    MachCtx *mc;
    OpCod   *oc;
    int32_t i;

    pmc = CurFile->fn_MBase;
    for (i = 0; i < CurFile->fn_MLines; ++i) {
        mc = *pmc++;
        CurFile->fn_LineNo = mc->LineNo;
        if (mc->Label && mc->Label->l_Offset != mc->m_Addr)
            cerror(EERROR_LABEL_ADDR_MISMATCH, mc->Label->Name, mc->Label->l_Offset, mc->m_Addr);
        if ((oc = mc->OpCode) == NULL)
            continue;
        if (mc->m_Addr != addr)
            cerror(EERROR_ADDR_MISMATCH, mc->m_Addr, addr);
        if (oc->Id < 0) {
            addr = ExecOpCodeG(mc, addr);
            continue;
        }
        addr += GenerateInstruction(mc);
    }
    return(addr);
}

static uword Code[16];

int32_t
GenerateInstruction(mc)
MachCtx *mc;
{
    OpCod *oc = mc->OpCode;
    uword opcode = oc->Template;
    uword numWords = 1;

    /*
     *  Step one, the instruction word itself
     */

    {
        short spec;

        if (oc->B_Siz >= 0) {   /*  bits in inst word for size  */
            if (mc->OpSize == 2)
                opcode |= 1 << oc->B_Siz;
            if (mc->OpSize == 4)
                opcode |= 2 << oc->B_Siz;
        }
        if (oc->B_Rs >= 0) {    /*  source register */
            if ((spec = RegCode[(short)mc->Oper1.Mode1]) >= 0)
                opcode |= spec << oc->B_Rs;
            else
                opcode |= (mc->Oper1.Reg1 & 7) << oc->B_Rs;
        }
        if (oc->B_EAs >= 0)     /*  source eff addr */
            opcode |= EACode[(short)mc->Oper1.Mode1] << oc->B_EAs;

        if (oc->B_Rd >= 0) {    /*  dest register   */
            if ((spec = RegCode[(short)mc->Oper2.Mode1]) >= 0)
                opcode |= spec << oc->B_Rd;
            else
                opcode |= (mc->Oper2.Reg1 & 7) << oc->B_Rd;
        } else if (oc->DModes & AF_MULDREGS) {
            Code[1] = ToMsbOrderShort(((mc->Oper2.Reg1 & 7) << 12) | 0x0800);
            numWords = 2;
        } else if (oc->DModes & AF_MULDREGU) {
            Code[1] = ToMsbOrderShort(((mc->Oper2.Reg1 & 7) << 12) | 0x0000);
            numWords = 2;
        }
        if (oc->B_EAd >= 0)     /*  dest eff addr   */
            opcode |= EACode[(short)mc->Oper2.Mode1] << oc->B_EAd;

        /*
         *  MOVEM instruction special case, mask always comes next
         */

        if (oc->Id == OpMOVEM)
            numWords = 2;

        /*
         *  note, in-instruction immediate data further on in routine.  Also
         *  absolute word/int32_t determined below.
         */
    }

    Code[0] = ToMsbOrderShort(opcode);

    numWords = GenerateEA(numWords, mc, oc, &mc->Oper1, Code + numWords);
    numWords = GenerateEA(numWords, mc, oc, &mc->Oper2, Code + numWords);

    dbprintf(0, ("line %ld bytes %d operModes %d %d\n", CurFile->fn_LineNo, numWords * 2, mc->Oper1.Mode1, mc->Oper2.Mode1));

    DumpSectionData(mc->Sect, Code, numWords*2);
    return(numWords*2);
}

int32_t
RegMaskForInst(short mode, int32_t mask)
{
    int32_t rvs = 0;
    short i;

    if (mode != AB_MMIND)
        return(mask);

    for (i = 0; i < 16; ++i) {
        if (mask & (1 << i))
            rvs |= 1 << (15-i);
    }
    return(rvs);
}

/*
 *  returns number of extension words added
 */

int32_t
GenerateEA(short numWords, MachCtx *mc, OpCod *oc, EffAddr *ea, uword *ext)
{
    int numBytes = numWords * 2;

    /*
     *  relocation
     */

    {
        Label *lab;

        /*
         *  fixup for local labels, base displacement
         */

        if ((lab = ea->Label2) && lab->l_Type == LT_LOC) {
            ea->Offset2 += lab->l_Offset;

            switch(ea->Mode1) {
            case AB_OFFIDXPC:
            case AB_OFFPC:
            case AB_BBRANCH:
            case AB_WBRANCH:
                if (lab->Sect == CurSection) {
                    ea->Offset2 -= (mc->m_Addr + numWords*2);
                    ea->Label2 = NULL;
                }
                break;
            }
        }

        /*
         *  fixup for local labels, offset or index value (but not outer
         *  displacement since it is not PC relative)
         */

        if ((lab = ea->Label1) && lab->l_Type == LT_LOC) {
            ea->Offset1 += lab->l_Offset;

            switch(ea->Mode1) {
            case AB_OFFIDXPC:
                if ((ea->ExtWord & EXTF_ODMASK) == EXTF_ODWORD || (ea->ExtWord & EXTF_ODMASK) == EXTF_ODLONG) {
                    break;
                }
            case AB_OFFPC:
            case AB_BBRANCH:
            case AB_WBRANCH:
                if (lab->Sect == CurSection) {
                    ea->Offset1 -= (mc->m_Addr + numWords*2);
                    ea->Label1 = NULL;
                }
            }
        }

    }

    /*
     *  mode and extension words
     */

    switch(ea->Mode1) {
    case 0:
    case AB_DN:
    case AB_AN:
    case AB_INDAN:
    case AB_INDPP:
    case AB_MMIND:
        break;
    case AB_OFFAN:
        if (ea->Label1)
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 2, RELOC_DATAREL);
        ++numWords;
        ext[0] = ToMsbOrderShort(ea->Offset1);
        if (ea->Offset1 < -32768 || ea->Offset1 > 32767)
            cerror(EERROR_WORD_OFFSET, ea->Offset1);
        break;
    case AB_OFFIDX:
    case AB_OFFIDXPC:
        if (ea->ExtWord & EXTF_FULL) {
            int i = 1;

            switch(ea->ExtWord & EXTF_BDMASK) {
            case EXTF_BDWORD:
                if (ea->Label2) {
                    int32_t reloc_type = (ea->Mode1 == AB_OFFIDXPC) ? RELOC_PCREL : ((ea->ISize & ISF_BDDREL) ? RELOC_DATAREL : 0);

                    HandleInstReloc(mc->Sect, ea->Label2, mc->m_Addr + numBytes + 2, 2, reloc_type);
                    /*
                     *  address will be relocated relative to label instead
                     *  of to extension word, must compensate
                     */
                    ea->Offset2 += 2;
                }
                ext[i++] = ToMsbOrderShort(ea->Offset2);
                break;
            case EXTF_BDLONG:
                if (ea->Label2) {
                    int32_t reloc_type = (ea->Mode1 == AB_OFFIDXPC) ? RELOC_PCREL : ((ea->ISize & ISF_BDDREL) ? RELOC_DATAREL : 0);

                    HandleInstReloc(mc->Sect, ea->Label2, mc->m_Addr + numBytes + 2, 4, reloc_type);
                    ea->Offset2 += 2;
                }
                ext[i++] = ToMsbOrderShort(ea->Offset2 >> 16);
                ext[i++] = ToMsbOrderShort(ea->Offset2);
                break;
            }
            switch(ea->ExtWord & EXTF_ODMASK) {
            case EXTF_ODWORD:
                if (ea->Label1) {
                    int32_t reloc_type = (ea->ISize & ISF_ODDREL) ? RELOC_DATAREL : 0;
                    HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes + i * 2, 2, reloc_type);
                }
                ext[i++] = ToMsbOrderShort(ea->Offset1);
                break;
            case EXTF_ODLONG:
                if (ea->Label1) {
                    int32_t reloc_type = (ea->ISize & ISF_ODDREL) ? RELOC_DATAREL : 0;
                    HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes + i * 2, 4, reloc_type);
                }
                ext[i++] = ToMsbOrderShort(ea->Offset1 >> 16);
                ext[i++] = ToMsbOrderShort(ea->Offset1);
                break;
            }
            numWords += i;
            ext[0] = ToMsbOrderShort(ea->ExtWord);
        } else {
            if (ea->Label1)
                HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes + 1, 1, 0);
            if (ea->Offset1 < -128 || ea->Offset1 > 127)
                cerror(EERROR_BYTE_OFFSET, ea->Offset1);
            ext[0] = ToMsbOrderShort(ea->ExtWord | (ea->Offset1 & 0xFF));
            ++numWords;
        }
        break;
    case AB_ABSW:
        if (ea->Label1)
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 2, 0);
        ++numWords;
        ext[0] = ToMsbOrderShort(ea->Offset1);
        break;
    case AB_ABSL:
        if (ea->Label1)
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 4, 0);
        numWords += 2;
        ext[0] = ToMsbOrderShort(ea->Offset1 >> 16);
        ext[1] = ToMsbOrderShort(ea->Offset1);
        break;
    case AB_OFFPC:
        if (ea->Label1)
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 2, RELOC_PCREL);
        ++numWords;
        ext[0] = ToMsbOrderShort(ea->Offset1);
        if (ea->Offset1 < -32768 || ea->Offset1 > 32767)
            cerror(EERROR_WORD_OFFSET, ea->Offset1);
        break;
    case AB_IMM:
        switch(oc->Special) {
        case IMM07:
            if (ea->Label1)
                cerror(EERROR_BAD_RELOC);
            Code[0] |= ToMsbOrderShort((ea->Offset1 & 7) << 9);
            if (ea->Offset1 < 0 || ea->Offset1 > 7)
                cerror(EERROR_7_OFFSET, ea->Offset1);
            break;
        case IMM18:
            if (ea->Label1)
                cerror(EERROR_BAD_RELOC);
            Code[0] |= ToMsbOrderShort((ea->Offset1 & 7) << 9);
            if (ea->Offset1 < 1 || ea->Offset1 > 8)
                cerror(EERROR_8_OFFSET, ea->Offset1);
            break;
        case IMM0F:
            if (ea->Label1)
                HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + 1, 1, 0);
            Code[0] |= ToMsbOrderShort(ea->Offset1 & 15);
            if (ea->Offset1 < 0 || ea->Offset1 > 15)
                cerror(EERROR_15_OFFSET, ea->Offset1);
            break;
        case IMM256:
            if (ea->Label1)
                HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + 1, 1, 0);
            Code[0] |= ToMsbOrderShort(ea->Offset1 & 0xFF);
            if (ea->Offset1 < -128 || ea->Offset1 > 127)
                cerror(EERROR_BYTE_OFFSET, ea->Offset1);
            break;
        case IMMB:
            if (ea->Label1)
                HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes + 1, 1, 0);
            ext[0] = ToMsbOrderShort(ea->Offset1 & 0xFF);
            ++numWords;
            break;
        default:
            if (mc->OpSize == 0) {
                if (oc->Sizes & S_B)
                    mc->OpSize = 1;
                if (oc->Sizes & S_W)
                    mc->OpSize = 2;
                if (oc->Sizes & S_L)
                    mc->OpSize = 4;
            }

            switch(mc->OpSize) {
            case 1:
                if (ea->Label1)
                    HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes + 1, 1, 0);
                ext[0] = ToMsbOrderShort(ea->Offset1 & 0xFF);
                ++numWords;
                break;
            case 2:
                if (ea->Label1)
                    HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 2, RELOC_PCREL);
                ext[0] = ToMsbOrderShort(ea->Offset1);
                ++numWords;
                break;
            case 4:
                if (ea->Label1)
                    HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + numBytes, 4, 0);
                ext[0] = ToMsbOrderShort(ea->Offset1 >> 16);
                ext[1] = ToMsbOrderShort(ea->Offset1);
                numWords += 2;
                break;
            }
        }
        break;
    case AB_REGS:
        Code[1] = ToMsbOrderShort(RegMaskForInst(mc->Oper2.Mode1, ea->ExtWord));
        break;
    case AB_BBRANCH:
        /*
         *  since byte branches are relative to the base of the
         *  instruction word, NOT the address of the offset, we
         *  have to compensate -1
         */

        if (ea->Label1) {
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + 1, 1, 0);
            Code[0] |= ToMsbOrderShort((ea->Offset1 - 1) & 0xFF);
        } else {
            Code[0] |= ToMsbOrderShort((ea->Offset1) & 0xFF);
        }

        if (ea->Label1 == NULL && ea->Offset1 == 0)
            cerror(EERROR_SHORT_BRANCH);
        if (ea->Offset1 < -128 || ea->Offset1 > 127)
            cerror(EERROR, "byte branch offset too large");
        break;
    case AB_WBRANCH:
        if (ea->Label1)
            HandleInstReloc(mc->Sect, ea->Label1, mc->m_Addr + 2, 2, RELOC_PCREL);

        ++numWords;
        ext[0] = ToMsbOrderShort(ea->Offset1);
        if (ea->Offset1 < -32768 || ea->Offset1 > 32767)
            cerror(EERROR_WORD_OFFSET, ea->Offset1);
        break;
    case AB_CCR:
    case AB_SR:
    case AB_USP:
        break;
    default:
        cerror(ESOFT_BAD_SRCAB, ea->Mode1);
        break;
    }
    return(numWords);
}


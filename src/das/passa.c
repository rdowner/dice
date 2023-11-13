/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PASSA
 *
 *  Scan and resolve all label types & opcodes. fields:
 *      Label
 *      OpCode
 *      OpSize  (0 == unspecified)
 *
 *  Creates appropriate MachCtx structures
 */

#include "defs.h"

Prototype void  PassA(void);

void
PassA()
{
    char *ptr = CurFile->fn_AsBuf;
    char c;
    MachCtx *mc;
    int32_t lineno = 0;

    while ((c = *ptr) != 0) {
        CurFile->fn_LineNo = ++lineno;
        if (c == '\n') {
            ++ptr;
            continue;
        }
        mc = AllocStructure(MachCtx);
        mc->Sect = CurSection;
        mc->LineNo = lineno;
        CurFile->fn_MBase[CurFile->fn_MLines++] = mc;

        if (c != ' ' && c != '\t') {      /*  label   */
            char *skip = ptr;
            Label *label;
            while (c != '\n' && c != ' ' && c != '\t' && c != ':')
                c = *++ptr;
            *ptr = 0;
            mc->Label = label = GetLabelByName(skip);
            if (label->Sect && label->Sect->Type != SECT_COMMON) {
                cerror(EERROR_LABEL_MULTIPLY_DEFINED, label->Name);
            } else {
                label->Sect = CurSection;
                label->MC = CurFile->fn_MBase + CurFile->fn_MLines - 1;
            }
            if (c == ':')
                c = *++ptr;

            /*
            if (++label->l_Cre > 1)
                xxxcerror(EERROR, "Duplicate label: %s", label->Name);
            */
        }
        while (c == ' ' || c == '\t')
            c = *++ptr;
        if (c != '\n') {         /*  opcode/directive    */
            char *skip = ptr;
            while (c != '\n' && c != ' ' && c != '\t' && c != '.') {
                if (c >= 'a' && c <= 'z')
                    *ptr &= ~0x20;     /*  -> upper case */
                c = *++ptr;
            }
            *ptr = 0;
            mc->OpCode = GetOpByName(skip);

            if (c == '.') {
                mc->OpSize = CToSize(*++ptr);
                c = *++ptr;
            }


            if (mc->OpCode == NULL)
                cerror(EERROR_UNKNOWN_DIRECTIVE, skip);

            if (c != '\n') {
                while (c == ' ' || c == '\t')
                    c = *++ptr;
                mc->m_Operands = ptr;
            } else {
                mc->m_Operands = "";
            }
            while (c != '\n')
                c = *++ptr;
        }
        *ptr++ = 0;
        if (mc->OpCode && mc->OpCode->Id < 0)
            ExecOpCodeA(mc);
    }
    CurFile->fn_MBase[CurFile->fn_MLines] = NULL;
}


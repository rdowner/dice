/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

// ************************************************************************

Local BOOL              RefreshRegister(char *regName, ULONG current, ULONG last);
Prototype char          *StateText(ULONG staten);
Local void              DisplayFlags(void);
Prototype WORD          RefreshRegisters(WORD maxLines, BOOL fullRefresh);
Prototype long          *RegisterAddress(char *name);


// ************************************************************************

ULONG           programState = STATE_EXITED;
UWORD           programSR;
ULONG           programPC;
ULONG           programD0;
ULONG           programD1;
ULONG           programD2;
ULONG           programD3;
ULONG           programD4;
ULONG           programD5;
ULONG           programD6;
ULONG           programD7;
ULONG           programA0;
ULONG           programA1;
ULONG           programA2;
ULONG           programA3;
ULONG           programA4;
ULONG           programA5;
ULONG           programA6;
ULONG           programA7;

ULONG           lastState = STATE_EXITED;
UWORD           lastSR;
ULONG           lastPC;
ULONG           lastD0;
ULONG           lastD1;
ULONG           lastD2;
ULONG           lastD3;
ULONG           lastD4;
ULONG           lastD5;
ULONG           lastD6;
ULONG           lastD7;
ULONG           lastA0;
ULONG           lastA1;
ULONG           lastA2;
ULONG           lastA3;
ULONG           lastA4;
ULONG           lastA5;
ULONG           lastA6;
ULONG           lastA7;

Local BOOL      RefreshRegister(char *regName, ULONG current, ULONG last) {
        ScrPlain();
        ScrPrintf("%s: ", regName);
        if (current != last)ScrHighlight();
        ScrPrintf("$%08X ", current);
}

struct {
        ULONG   state;
        char    *text;
} stateTable[] = {
        { 0,    "RUNNING             " },
        { 1,    "HALTED              " },
        { 2,    "BUS_ERROR           " },
        { 3,    "ADDRESS_ERROR       " },
        { 4,    "ILLEGAL_INSTRUCTION " },
        { 5,    "ZERO_DIVIDE         " },
        { 6,    "CHK                 " },
        { 7,    "TRAPV               " },
        { 8,    "PRIVILEGE_VIOLATION" },
        { 9,    "TRACE               " },
        { 10,   "LINEA               " },
        { 11,   "LINEF               " },
        { 64,   "RESET               " },
        { 65,   "EXITED              " },
        { 66,   "STEPPED             " },
        { 67,   "STEPPEDOVER         " },
        { 68,   "BREAKPOINT HIT      " },
        { 69,   "GOSTEP              " },
        { 512,  "UNDEFINED           " },
};

char    *StateText(ULONG state) {
        short   i;

        for (i=0; stateTable[i].state != state && stateTable[i].state != 512; i++);
        return stateTable[i].text;
}

Local void      DisplayFlags(void) {
        ScrPlain();
//      ScrPrintf("T=%c ", (programSR & 0x8000) ? '1' : '0');
//      ScrPrintf("S=%c ", (programSR & 0x2000) ? '1' : '0');
        ScrPrintf("X=%c ", (programSR & 0x0010) ? '1' : '0');
        ScrPrintf("N=%c ", (programSR & 0x0008) ? '1' : '0');
        ScrPrintf("Z=%c ", (programSR & 0x0004) ? '1' : '0');
        ScrPrintf("V=%c ", (programSR & 0x0002) ? '1' : '0');
        ScrPrintf("C=%c ", (programSR & 0x0001) ? '1' : '0');
//      ScrPrintf("IM=%d ",  (programSR>>8)&7);
}

WORD    RefreshRegisters(WORD maxLines, BOOL fullRefresh) {
        WORD    count = 0;

    if(CurDisplay->ds_RegFlag) {

        RefreshRegister("D0", programD0, lastD0);
        RefreshRegister("D1", programD1, lastD1);
        RefreshRegister("D2", programD2, lastD2);
        RefreshRegister("D3", programD3, lastD3);
        count++; 
        Newline(); 
        maxLines--; 
        if (!maxLines) return count;

        RefreshRegister("D4", programD4, lastD4);
        RefreshRegister("D5", programD5, lastD5);
        RefreshRegister("D6", programD6, lastD6);
        RefreshRegister("D7", programD7, lastD7);
        count++; Newline(); maxLines--; if (!maxLines) return count;

        RefreshRegister("A0", programA0, lastA0);
        RefreshRegister("A1", programA1, lastA1);
        RefreshRegister("A2", programA2, lastA2);
        RefreshRegister("A3", programA3, lastA3);
        count++; Newline(); maxLines--; if (!maxLines) return count;

        RefreshRegister("A4", programA4, lastA4);
        RefreshRegister("A5", programA5, lastA5);
        RefreshRegister("A6", programA6, lastA6);
        RefreshRegister("A7", programA7, lastA7);
        count++; Newline(); maxLines--; if (!maxLines) return count;

        RefreshRegister("PC", programPC, lastPC);
        ScrPlain();
        ScrPrintf("SR: ");
        if (programSR != lastSR)ScrHighlight();
        ScrPrintf("$%04X ", programSR);
        DisplayFlags();
        ScrPlain();
        ScrPrintf("STATE: ");
        if (programState != lastState)
            ScrHighlight();
        ScrPrintf("%s", StateText(programState));
        count++; Newline(); maxLines--; if (!maxLines) return count;
    }
    return count;
}

/*
 *  RegisterAddress()   - Given a valid two character register specification
 *                        return a pointer to the storage are for the reg.
 *
 *                        return NULL if the two char specification is
 *                        illegal.
 */

long   *RegisterAddress(char *name)
{
        char c = name[1];

        switch(name[0]) {
        case 'a':
        case 'A':
            switch(c) {
            case '0':
                return(&programA0);
            case '1':
                return(&programA1);
            case '2':
                return(&programA2);
            case '3':
                return(&programA3);
            case '4':
                return(&programA4);
            case '5':
                return(&programA5);
            case '6':
                return(&programA6);
            case '7':
                return(&programA7);
            }
            break;
        case 'd':
        case 'D':
            switch(c) {
            case '0':
                return(&programD0);
            case '1':
                return(&programD1);
            case '2':
                return(&programD2);
            case '3':
                return(&programD3);
            case '4':
                return(&programD4);
            case '5':
                return(&programD5);
            case '6':
                return(&programD6);
            case '7':
                return(&programD7);
            }
            break;
        case 'p':
        case 'P':
            if (c == 'c' || c == 'C')
                return(&programPC);
            break;
        case 's':
        case 'S':
            if (c == 'p' || c == 'P')
                return(&programA7);
            break;
        }
        return(NULL);
}


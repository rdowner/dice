/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

// ************************************************************************

Prototype WORD          RefreshWatchpoints(WORD maxLines, BOOL fullRefresh);
Prototype void          InitWatchpoints(void);
Local WORD              FindWP(char *expression);
Prototype BOOL          IsWatchpoint(char *expression);
Prototype BOOL          SetWatchpoint(char *expression, ULONG type);
Prototype BOOL          ClearWatchpoint(char *expression);
Prototype BOOL          ShowWatchTitle(BOOL fullRefresh, WORD *count, WORD *maxLines);
// ************************************************************************

WP      wpTable[MAXWP];

WORD    RefreshWatchpoints(WORD maxLines, BOOL fullRefresh) {
        WORD    count = 0;
        WORD    i, undef;
        ULONG   val;
        BOOL    flag = FALSE;


        for (i=0; i<MAXWP && maxLines > 0; i++) {
                switch (wpTable[i].type) {
                        case WP_UNSET:          
                                break;
                        case WP_BYTES:
                                val = ParseExp(wpTable[i].expression, &undef, strlen(wpTable[i].expression));
                                if (undef)
                                        ScrPrintf("%-24.24s *UNDEFINED*", wpTable[i].expression);
                                else {
                                        WORD    j = (CurDisplay->ds_ScrCols-35)/3;
                                        UBYTE   *ps = (UBYTE *)val;

                                        if (!flag) {
                                            flag = ShowWatchTitle(fullRefresh,&count,&maxLines);
                                        }

                                        ScrPrintf("%-24.24s %08X  ", wpTable[i].expression, val);
                                        while (j > 0) {
                                                ScrPrintf("%02X ", *ps++);
                                                --j;
                                        }
                                }
                                count++; 
                                Newline(); 
                                maxLines--;

                                break;
                        case WP_WORDS:
                                val = ParseExp(wpTable[i].expression, &undef, strlen(wpTable[i].expression));
                                if (undef)
                                        ScrPrintf("%-24.24s *UNDEFINED*", wpTable[i].expression);
                                else {
                                        WORD    j = (CurDisplay->ds_ScrCols-35)/5;
                                        UWORD   *ps = (UWORD *)val;

                                        if (!flag) {
                                            flag = ShowWatchTitle(fullRefresh,&count,&maxLines);
                                        }
                                        ScrPrintf("%-24.24s %08X  ", wpTable[i].expression, val);
                                        while (j > 0) {
                                                ScrPrintf("%04X ", *ps++);
                                                --j;
                                        }
                                }
                                count++; Newline(); maxLines--;
                                break;
                        case WP_LONGS:
                                val = ParseExp(wpTable[i].expression, &undef, strlen(wpTable[i].expression));
                                if (undef)
                                        ScrPrintf("%-24.24s *UNDEFINED*", wpTable[i].expression);
                                else {
                                        WORD    j = (CurDisplay->ds_ScrCols-35)/9;
                                        ULONG   *ps = (ULONG *)val;

                                        if (!flag) {
                                            flag = ShowWatchTitle(fullRefresh,&count,&maxLines);
                                        }
                                        ScrPrintf("%-24.24s %08X  ", wpTable[i].expression, val);
                                        while (j > 0) {
                                                ScrPrintf("%08X ", *ps++);
                                                --j;
                                        }
                                }
                                count++; 
                                Newline(); 
                                maxLines--;
                                break;
                }
        }

        // add a newline to end if at top of screen
        if(flag && !CurDisplay->ds_RegFlag) {
            Newline(); 
            count++;
            maxLines++;
        }

        return count;
}

void    InitWatchpoints(void) {
        WORD    i;

        for (i=0; i<MAXWP; i++) wpTable[i].type = WP_UNSET;
}

Local WORD      FindWP(char *expression) {
        WORD    i, undef;
        ULONG   value, value2;

        value = ParseExp(expression, &undef, strlen(expression));
        if (undef) return FALSE;
        for (i=0; i<MAXWP; i++) {
                if (wpTable[i].type != WP_UNSET) {
                        if (!strcmp(expression, wpTable[i].expression)) return i;
                        value2 = ParseExp(wpTable[i].expression, &undef, strlen(wpTable[i].expression));
                        if (value == value2) return i;
                }
        }
        return -1;
}

BOOL    IsWatchpoint(char *expression) {
        return (FindWP(expression) != -1);
}

BOOL    SetWatchpoint(char *expression, ULONG type) {
        WORD    i, undef;
        ULONG   value;

        value = ParseExp(expression, &undef, strlen(expression));
        if (undef) return FALSE;
        for (i=0; i<MAXWP; i++) {
                if (wpTable[i].type == WP_UNSET) {
                        wpTable[i].type = type;
                        strcpy(wpTable[i].expression, expression);
                        return TRUE;
                }
        }
        ScrStatus("*** Watchpoint table full!!");
        return FALSE;
}

BOOL    ClearWatchpoint(char *expression) {
        WORD    i = FindWP(expression);
        if (i == -1) {
                ScrStatus("*** Watchpoint not in table");
                return FALSE;
        }
        wpTable[i].type = WP_UNSET;
        return TRUE;
}


BOOL ShowWatchTitle(BOOL fullRefresh, WORD *count, WORD *maxLines) 
{
    if(fullRefresh != -1) {
        if(CurDisplay->ds_RegFlag) {
            Newline(); 
            (*count)++;
            (*maxLines)++;
        }
    }

    ScrPlain();
    ScrInverse();
    ScrPuts("WATCHPOINT               ADDRESS   CONTENTS");
//           xxxxxxxxxxxxxxxxxxxxxxxx xxxxxxxx  xx
    Newline(); 
    (*count)++;
    (*maxLines)++;

    return TRUE;
}
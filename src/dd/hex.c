/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

#define     HEX_OVERHEAD    (8+4)   /*  address, overhead spacing       */
#define     HEX_LIMIT       16      /*  16 columns limit                */

#define     BYTE_COLUMNS    (3+1)   /*  xx<space> plus 1 for ascii char */
#define     WORD_COLUMNS    (5+1)   /*  xxxx<space> plus 2 ascii chars  */
#define     LONG_COLUMNS    (9+4)   /*  x8<space> plus 4 ascii chars    */

// ************************************************************************

Prototype WORD          RefreshBytes(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype WORD          RefreshWords(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype WORD          RefreshLongs(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype BOOL          UpBytes(void);
Prototype BOOL          UpWords(void);
Prototype BOOL          UpLongs(void);
Prototype BOOL          PageUpHex(void);
//Prototype BOOL                PageUpBytes(void);
//Prototype BOOL                PageUpWords(void);
//Prototype BOOL                PageUpLongs(void);
Prototype BOOL          DownBytes(void);
Prototype BOOL          DownWords(void);
Prototype BOOL          DownLongs(void);
Prototype BOOL          PageDownHex(void);

// ************************************************************************

BOOL    UpBytes(void) {

        CurDisplay->ds_WindowTop -= ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT);
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrolldown();
        RefreshBytes(1, -1, CurDisplay->ds_WindowTop);

        return TRUE;
}

BOOL    UpWords(void) {

        CurDisplay->ds_WindowTop -= ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT) * 2;
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrolldown();
        RefreshWords(1, -1, CurDisplay->ds_WindowTop);

        return TRUE;
}

BOOL    UpLongs(void) {

        CurDisplay->ds_WindowTop -= ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT) * 4;
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrolldown();
        RefreshLongs(1, -1, CurDisplay->ds_WindowTop);

        return TRUE;
}

BOOL    DownBytes(void) {

        CurDisplay->ds_WindowTop += ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT);
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrollup();
        RefreshBytes(1, -1, CurDisplay->ds_WindowTop + (ScrMainBodyRange(NULL,NULL) - 1) * ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT));

        return TRUE;
}

BOOL    DownWords(void) {

        CurDisplay->ds_WindowTop += ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT) * 2;
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrollup();
        RefreshWords(1, -1, CurDisplay->ds_WindowTop + (ScrMainBodyRange(NULL,NULL) - 1) * ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT) * 2);

        return TRUE;
}

BOOL    DownLongs(void) {

        CurDisplay->ds_WindowTop += ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT) * 4;
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrollup();
        RefreshLongs(1, -1, CurDisplay->ds_WindowTop + (ScrMainBodyRange(NULL,NULL) - 1) * ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT) * 4);

        return TRUE;
}

BOOL    PageUpHex(void) {
        ULONG address = CurDisplay->ds_WindowTop;
        int lines = CalcDisplayLines();

        if(lines > 0) {
            switch(CurDisplay->ds_DisplayMode) {
                case DISPLAY_BYTES:
                        address -= (lines * ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT));
                        break;
                case DISPLAY_WORDS:
                        address -= (2 * lines * ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT));
                        break;
                case DISPLAY_LONGS:
                        address -= (4 * lines * ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT));
                        break;
            }
            CurDisplay->ds_WindowTop = address;
            CurDisplay->ds_WindowTopLine = 0;
            RefreshWindow(1);
        }
        return TRUE;
}


BOOL    PageDownHex(void) {
        ULONG address = CurDisplay->ds_WindowTop;
        int lines = CalcDisplayLines();

        if(lines > 0) {
            switch(CurDisplay->ds_DisplayMode) {
                case DISPLAY_BYTES:
                        address += (lines * ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT));
                        break;
                case DISPLAY_WORDS:
                        address += (2 * lines * ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT));
                        break;
                case DISPLAY_LONGS:
                        address += (4 * lines * ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT));
                        break;
            }
            CurDisplay->ds_WindowTop = address;
            CurDisplay->ds_WindowTopLine = 0;
            RefreshWindow(1);
        }
        return TRUE;
}


WORD    RefreshBytes(WORD maxLines, BOOL fullRefresh, ULONG addr) {
        WORD    count = 0;
        UBYTE   *address = (UBYTE *)addr;
        WORD    i;
        WORD    columns = ScrColumns(HEX_OVERHEAD, BYTE_COLUMNS, HEX_LIMIT);

        do_scroller();
        if (fullRefresh != -1) {
            SetTitle(NULL,NULL);
            
#if 0
            ScrInverse();
            ScrPuts("HEX BYTES");
#endif
            // add a seperation between register display and source

            if(CurDisplay->ds_RegFlag) {
                Newline();
                ++count;
                if (--maxLines == 0)return count;
            }
        }

        while (maxLines > 0) {
            address = (UBYTE *)ValidMemCheck((ULONG)address);
            ScrInverse();
            ScrPrintf("%08X ", address);
            ScrPlain();
            if(address) {
                for (i = 0; i < columns; ++i)ScrPrintf(" %02X", address[i]);
                ScrPrintf("  ");
                for (i = 0; i < columns; ++i)
                        ScrPutChar((address[i] >= ' ' && address[i] <= 0x7F) ? address[i] : '.');

            }
            ++count;
            ScrPutNewline();
            if (--maxLines == 0)break;
            address += columns;
        }
        return count;
}

WORD    RefreshWords(WORD maxLines, BOOL fullRefresh, ULONG addr) {
        WORD    count = 0;
        UBYTE   *address = (UBYTE *)addr;
        WORD    i;
        WORD    columns = ScrColumns(HEX_OVERHEAD, WORD_COLUMNS, HEX_LIMIT);

        do_scroller();
        if (fullRefresh != -1) {
            SetTitle(NULL,NULL);
#if 0
            ScrInverse();
            ScrPuts("HEX WORDS");
#endif
            // add a seperation between register display and source
            if(CurDisplay->ds_RegFlag) {
                ++count;
                Newline();
                if (--maxLines == 0)return count;
            }
        }


        while (maxLines > 0) {
            address = (UBYTE *)ValidMemCheck((ULONG)address);
            ScrInverse();
            ScrPrintf("%08X ", address);
            ScrPlain();
            if(address) {
                for (i = 0; i < columns * 2; i += 2)
                        ScrPrintf(" %04X", (address[i] << 8) | address[i+1]);
                ScrPrintf("  ");
                for (i = 0; i < columns * 2; ++i)
                        ScrPutChar((address[i] >= ' ' && address[i] <= 0x7F) ? address[i] : '.');
            }
            ++count;
            ScrPutNewline();
            if (--maxLines == 0)break;
            address += columns * 2;
        }
        return count;
}

WORD    RefreshLongs(WORD maxLines, BOOL fullRefresh, ULONG addr) {
        WORD    count = 0;
        UBYTE   *address = (UBYTE *)addr;
        WORD    i;
        WORD    columns = ScrColumns(HEX_OVERHEAD, LONG_COLUMNS, HEX_LIMIT);

        do_scroller();
        if (fullRefresh != -1) {
            SetTitle(NULL,NULL);
#if 0
            ScrInverse();
            ScrPuts("HEX LONGS");
#endif
            // add a seperation between register display and source
            if(CurDisplay->ds_RegFlag) {
                ++count;
                Newline();
                if (--maxLines == 0)return count;
            }
        }

        while (maxLines > 0) {
            address = (UBYTE *)ValidMemCheck((ULONG)address);
            ScrInverse();
            ScrPrintf("%08X ", address);
            ScrPlain();
            if(address) {
                for (i = 0; i < columns * 4; i += 4)
                        ScrPrintf(" %08X", (address[i] << 24) | (address[i+1] << 16) | (address[i+2] << 8) | address[i+3]);
                ScrPrintf("  ");
                for (i = 0; i < columns * 4; ++i)
                        ScrPutChar((address[i] >= ' ' && address[i] <= 0x7F) ? address[i] : '.');
            }
            ++count;
            ScrPutNewline();
            if (--maxLines == 0)break;
            address += columns * 4;
        }
        return count;
}


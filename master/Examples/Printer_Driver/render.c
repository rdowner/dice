
/*
 *  RENDER.C
 *
 *  David Berezowski - March/88.
 *  Modified for DICE - May/91  Matthew Dillon
 *
 *  Copyright (c) 1988  Commodore-Amiga, Inc.
 *  (c)Copyright 1991 Matthew Dillon
 */

#include "defs.h"

Prototype __geta4 long Render(long, long, long, long);

#define NUMSTARTCMD     8       /* # of cmd bytes before binary data */
#define NUMENDCMD       1       /* # of cmd bytes after binary data */
#define NUMTOTALCMD (NUMSTARTCMD + NUMENDCMD)   /* total of above */
#define NUMLFCMD        4       /* # of cmd bytes for linefeed */
#define MAXCOLORBUFS    4       /* max # of color buffers */

#define STARTLEN        16
#define PITCH           1
#define CONDENSED       2
#define LMARG           8
#define RMARG           11
#define DIREC           15

__geta4 long
Render(ct, x, y, status)
long ct, x, y, status;
{
    static UWORD RowSize, ColorSize, BufSize, TotalBufSize;
    static UWORD dataoffset, dpi_code;
    static UWORD colors[MAXCOLORBUFS];      /* color ptrs */
    static UWORD colorcodes[MAXCOLORBUFS];  /* printer color codes */
    static UWORD NumColorBufs;              /* actually number of color buffers req. */

    /*
            00-01   \003P           set pitch (10 or 12 cpi)
            02-02   \022            set condensed fine (on or off)
            03-05   \033W\000       enlarge off
            06-08   \033ln          set left margin to n
            09-11   \033Qn          set right margin to n
            12-12   \015            carriage return
            13-15   \033U1          set uni-directional mode
    */

    static UBYTE StartBuf[STARTLEN] =
            "\033P\022\033W\000\033ln\033Qn\015\033U1";
    UBYTE *ptr, *ptrstart, *ptr2, *ptr2start;
    long i, err;

    err = PDERR_NOERR;

    switch(status) {
    case 0:     /* Master Initialization */
        /*
         *      ct      - pointer to IODRPReq structure.
         *      x       - width of printed picture in pixels.
         *      y       - height of printed picture in pixels.
         */
        RowSize = x * 3;
        ColorSize = RowSize + NUMTOTALCMD;
        if (PD->pd_Preferences.PrintShade == SHADE_COLOR) {
            NumColorBufs = MAXCOLORBUFS;
            colors[0] = ColorSize * 3; /* Black */
            colors[1] = ColorSize * 0; /* Yellow */
            colors[2] = ColorSize * 1; /* Magenta */
            colors[3] = ColorSize * 2; /* Cyan */
            colorcodes[0] = 4; /* Yellow */
            colorcodes[1] = 1; /* Magenta */
            colorcodes[2] = 2; /* Cyan */
            colorcodes[3] = 0; /* Black */
        } else {        /* grey-scale or black&white */
            NumColorBufs = 1;
            colors[0] = ColorSize * 0; /* Black */
            colorcodes[0] = 0; /* Black */
        }
        BufSize = ColorSize * NumColorBufs + NUMLFCMD;
        TotalBufSize = BufSize * 2;
        PD->pd_PrintBuf = AllocMem(TotalBufSize, MEMF_PUBLIC);
        if (PD->pd_PrintBuf == NULL)
            err = PDERR_BUFFERMEMORY;
        else {
            dataoffset = NUMSTARTCMD;
            /*
             *      This printer prints graphics within its
             *      text margins.  This code makes sure the
             *      printer is in 10 cpi and then sets the
             *      left and right margins to their minimum
             *      and maximum values (respectively).  A
             *      carriage return is sent so that the
             *      print head is at the leftmost position
             *      as this printer starts printing from
             *      the print head's position.  The printer
             *      is put into unidirectional mode to
             *      reduce wavy vertical lines.
             */
            StartBuf[PITCH] = 'P'; /* 10 cpi */
            StartBuf[CONDENSED] = '\022'; /* off */

            /* left margin of 1 */
            StartBuf[LMARG] = 0;
            /* right margin of 80 or 136 */
            StartBuf[RMARG] = PD->pd_Preferences.
                    PaperSize == W_TRACTOR ? 136 : 80;
            /* uni-directional mode */
            StartBuf[DIREC] = '1';
            err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
        }
        break;
    case 1: /* Scale, Dither and Render */
            /*
             *      ct      - pointer to PrtInfo structure.
             *      x       - 0.
             *      y       - row # (0 to Height - 1).
             */
        Transfer((void *)ct, y, &PD->pd_PrintBuf[dataoffset], colors);
        break;
    case 2: /* Dump Buffer to Printer */
        /*
         *      ct      - 0.
         *      x       - 0.
         *      y       - # of rows sent (1 to NumRows).
         */
        /* white-space strip */

        ptrstart = &PD->pd_PrintBuf[dataoffset];
        ptr2start = ptr2 = ptrstart - NUMSTARTCMD;
        x = 0;                      /* flag no transfer required yet */
        for (ct=0; ct<NumColorBufs; ct++, ptrstart += ColorSize) {
            i = RowSize;
            ptr = ptrstart + i - 1;
            while (i > 0 && *ptr == 0) {
                i--;
                ptr--;
            }
            if (i != 0) {   /* if data */
                /* convert to # of pixels */
                i = (i + 2) / 3;
                ptr = ptrstart - NUMSTARTCMD;
                *ptr++ = 27;
                *ptr++ = 'r';
                *ptr++ = colorcodes[ct]; /* color */
                *ptr++ = 27;
                *ptr++ = '*';
                *ptr++ = dpi_code;      /* density */
                *ptr++ = i & 0xff;
                *ptr++ = i >> 8;        /* size */
                i *= 3; /* back to # of bytes used */
                *(ptrstart + i) = 13;   /* cr */
                i += NUMTOTALCMD;
                                        /* if must transfer data */
                if (x != 0) {
                                        /* get src start */
                    ptr = ptrstart - NUMSTARTCMD;
                                        /* xfer and update dest ptr */
                    do {
                            *ptr2++ = *ptr++;
                    } while (--i);
                }
                else {      /* no transfer required */
                            /* update dest ptr */
                    ptr2 += i;
                }
            }
            /* if compacted or 0 */
            if (i != RowSize + NUMTOTALCMD) {
                /* we need to transfer next time */
                x = 1;
            }
        }
        *ptr2++ = 13;   /* cr */
        *ptr2++ = 27;
        *ptr2++ = 'J';
        *ptr2++ = y;    /* y/180 lf */
        err = (*(PD->pd_PWrite))(ptr2start, ptr2 - ptr2start);
        if (err == PDERR_NOERR) {
            dataoffset = (dataoffset == NUMSTARTCMD ?
                BufSize : 0) + NUMSTARTCMD;
        }
        break;
    case 3:     /* Clear and Init Buffer */
        /*
         *      ct      - 0.
         *      x       - 0.
         *      y       - 0.
         */
        ptr = &PD->pd_PrintBuf[dataoffset];
        i = BufSize - NUMTOTALCMD - NUMLFCMD;
        do {
            *ptr++ = 0;
        } while (--i);
        break;
    case 4: /* Close Down */
        /*
         *      ct      - error code.
         *      x       - io_Special flag from IODRPReq.
         *      y       - 0.
         */

        /* if user did not cancel print */
        if (ct != PDERR_CANCEL) {
            /* restore preferences pitch and margins */
            if (PD->pd_Preferences.PrintPitch == ELITE) {
                StartBuf[PITCH] = 'M'; /* 12 cpi */
            } else if (PD->pd_Preferences.PrintPitch == FINE) {
                StartBuf[CONDENSED] = '\017'; /* on */
            }
            StartBuf[LMARG] = PD->pd_Preferences.PrintLeftMargin - 1;
            StartBuf[RMARG] = PD->pd_Preferences.PrintRightMargin;
            StartBuf[DIREC] = '0'; /* bi-directional */
            err = (*(PD->pd_PWrite))(StartBuf, STARTLEN);
        }
        /* wait for both buffers to empty */
        (*(PD->pd_PBothReady))();
        if (PD->pd_PrintBuf != NULL)
            FreeMem(PD->pd_PrintBuf, TotalBufSize);
        break;
    case 5: /* Pre-Master Initialization */
        /*
         *      ct      - 0 or pointer to IODRPReq structure.
         *      x       - io_Special flag from IODRPReq.
         *      y       - 0.
         *
         *      Kludge for weak power supplies.
         *      FANFOLD - use all 24 pins (default).
         *      SINGLE  - use only 16 pins.
         */
        PED->ped_NumRows = ((PD->pd_Preferences.PaperType == SINGLE) ? 16 : 24);
        dpi_code = SetDensity(x & SPECIAL_DENSITYMASK);
        break;
    }
    return(err);
}


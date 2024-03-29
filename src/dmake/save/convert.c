
/*
 *  CONVERT.C
 */

#include "defs.h"

#define MAXLEVELS   10

Prototype int  WildConvert(char *, char *, char *, char *);

/*
 *  Run srcBuf through the srcMat pattern matcher and if it matches
 *  convert it via dstMat into dstBuf, else dstBuf \0 len.
 */


int
WildConvert(srcBuf, dstBuf, srcMat, dstMat)
char *srcBuf;
char *dstBuf;
char *srcMat;
char *dstMat;
{
    short r = 0;
    long i;
    static short Index;
    static short SubLen[MAXLEVELS];
    static char *SubStr[MAXLEVELS];

    if (Index == MAXLEVELS)
        error(FATAL, "maximum recursion reached in WildConvert");

    db4printf(("WildConvert %-15s (%s -> %s)\n", srcBuf, srcMat, (dstMat) ? dstMat : "..."));

    /*
     *  skip non-wildcards, srcBuf must match srcMat
     */

    while (*srcMat && *srcMat != '*' && *srcMat != '?') {
        if (*srcBuf != *srcMat)
            return(-1);
        ++srcBuf;
        ++srcMat;
    }

    switch(*srcMat) {
    case '\0':                      /*  match end, terminating case */
        if (*srcBuf)                /*  buf srcBuf not exhausted!   */
            r = -1;
        break;
    case '?':                       /*  match 1 */
        if (*srcBuf == 0)           /*  match failed against srcbuf  */
            return(-1);
        SubStr[Index] = srcBuf;
        SubLen[Index] = 1;
        ++Index;
        r = WildConvert(srcBuf + 1, NULL, srcMat + 1, NULL);
        --Index;
        break;
    case '*':                       /*  match any   */
        /*
         *  strangeness in loop is so \0 (nil string) is checked for,
         *  it is perfectly valid for the remainder to be nil.
         *
         *  note: bug in NeXT's GCC -O/-O2, had to reorder r == -1 to
         *  the right side of the &&
         */

        r = -1;
        for (i = 0; (i == 0 || srcBuf[i-1]) && r == -1; ++i) {
            SubStr[Index] = srcBuf;
            SubLen[Index] = i;
            ++Index;
            r = WildConvert(srcBuf + i, NULL, srcMat + 1, NULL);
            --Index;
        }
        break;
    }
    if (r == 0 && dstMat) {
        short k = 0;
        short n;

        while (*dstMat) {
            switch(*dstMat) {
            case '%':
                n = (dstMat[1] - '1');
            case '*':
            case '?':
                if (*dstMat == '%')
                    ++dstMat;
                else
                    n = k++;

                if (n >= 0 && n < MAXLEVELS) {
                    movmem(SubStr[n], dstBuf, SubLen[n]);
                    dstBuf += SubLen[n];
                }
                break;
            default:
                *dstBuf++ = *dstMat;
                break;
            }
            ++dstMat;
        }
        *dstBuf = 0;
    }
    db4printf((" r = %d\n", r));
    return(r);
}



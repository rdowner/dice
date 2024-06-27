/*
 * Copyright (c) 2003-2011,2023 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@backplane.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
        short n = -1;

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
                    bcopy(SubStr[n], dstBuf, SubLen[n]);
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
    return(r);
}



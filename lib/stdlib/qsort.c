
/*
 *  QSORT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Well, I actually implement a merge sort here, because I have no idea
 *  how much stack is available for a quick sort.
 */

#include <stdlib.h>
#include <string.h>

void
qsort(base, elms, elmSize, cmpfunc)
void *base;
size_t elms;
size_t elmSize;
int (*cmpfunc)(const void *, const void *);
{
    long i, j;
    void *tmp = malloc(elmSize);

    if (tmp == NULL)
        return;

    for (i = 2; (i >> 1) < elms; i *= 2) {         /*  merge sets of 2,4,8.. */
        for (j = 0; j < elms; j += i) {
            long i1 = i >> 1;
            long i2 = i1;
            char *p1;
            char *p2;

            if (elmSize == 4) {
                p1 = (char *)base + j * 4;
                p2 = p1 + i1 * 4;
            } else {
                p1 = (char *)base + j * elmSize;
                p2 = p1 + i1 * elmSize;
            }

            if (j + i1 + i2 > elms)
                i2 = elms - i1 - j;

            /*
             *  Merge p2/i2 into p1/i1 until p2/i2 is exausted.  Note that
             *  i2 might start out negative (in which case i1 is invalid,
             *  but the segment is already sorted anyway)
             */

            while (i2 > 0 && i1 > 0) {
                /*
                 *  compare current p1/i1 with next p2/i2.  Skip p1 while
                 *  p1 < p2.  Otherwise, merge the p2/i2 element into
                 *  p1/i1.
                 */

                if ((*cmpfunc)(p1, p2) <= 0) {
                    --i1;
                    p1 += elmSize;
                } else {
                    if (elmSize == 4) {
                        *(long *)tmp = *(long *)p2;
                        movmem(p1, p1 + 4, i1 * 4);
                        *(long *)p1 = *(long *)tmp;
                    } else {
                        movmem(p2, tmp, elmSize);
                        movmem(p1, p1 + elmSize, i1 * elmSize);
                        movmem(tmp, p1, elmSize);
                    }
                    --i2;
                    p2 += elmSize;
                    p1 += elmSize;      /*  shifted p1, i1 stays    */
                }
            }
        }
    }
    free(tmp);
}


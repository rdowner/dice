
/*
 *  BSEARCH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  BSearch() - binary search of an array
 *
 *  note that aryEnd is the number of elements in the array and therefore
 *  base[aryEnd * objSize] is an INVALID element and not included in the
 *  search (e.g. just past the end of the array).
 */

#include <stdlib.h>
#include <string.h>

void *
bsearch(key, base, aryEnd, objSize, compare)
void *key;
void *base;
size_t aryEnd;
size_t objSize;
int (*compare)(const void *key, const void *elm);
{
    static short Power = -1;    /*  power of 2 optimization */
    size_t aryBeg = 0;
    long   r;
    short power = Power;

    if (power < 0 || (1 << power) != objSize) {
        for (power = 0; (r = (1 << power)) && (unsigned long)r < objSize; ++power)
            ;
        if (r != objSize)
            power = -1;
        Power = power;
    }
    if (power >= 0) {
        while (aryBeg < aryEnd) {
            size_t i = (aryBeg + aryEnd) >> 1;
            void *obj = (void *)((char *)base + (i << power));

            r = compare(key, obj);
            if (r < 0)              /*  we are over the object */
                aryEnd = i;
            else if (r > 0)         /*  we are under the object  */
                aryBeg = i + 1;
            else
                return(obj);
        }
    } else {
        while (aryBeg < aryEnd) {
            size_t i = (aryBeg + aryEnd) >> 1;
            void *obj = (void *)((char *)base + i * objSize);

            r = compare(key, obj);
            if (r < 0)              /*  we are over the object */
                aryEnd = i;
            else if (r > 0)         /*  we are under the object  */
                aryBeg = i + 1;
            else
                return(obj);
        }
    }
    return(NULL);
}


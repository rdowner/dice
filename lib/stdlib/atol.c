
/*
 *  int ATOL(str)
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdlib.h>

long
atol(str)
const char *str;
{
    return(atoi(str));
}


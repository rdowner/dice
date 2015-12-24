
/*
 *  ISALNUM.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */


int
isalnum(c)
int c;
{
    c;
#define CTYPE_NEAR
#include <ctype.h>
    return(isalnum(c));
}


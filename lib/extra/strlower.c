
/*
 *  Convert string to lower case
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#ifndef HYPER
#define HYPER(x) x
#endif

char *
HYPER(strlower)(str)
char *str;
{
    char c;
    char *base = str;

    while (c = *str) {
	if (c >= 'A' && c <= 'Z')
	    *str = c + ('a' - 'A');
	++str;
    }
    return(base);
}



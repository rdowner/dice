
/*
 *  GETS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  buf = gets(buf)
 */

#include <stdio.h>

char *
gets(buf)
char *buf;
{
    int c;
    int cnt = 0;
    char *base = buf;

    while ((c = getc(stdin)) != EOF) {
	*buf++ = c;
	if (c != '\n' && cnt < BUFSIZ) {
	    ++cnt;
	    continue;
	}
	buf[-1] = 0;
	return(base);
	break;
    }
    if (buf == base)
	return(NULL);
    buf[0] = 0;
    return(base);
}


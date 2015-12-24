
/*
 *  STRTOK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>

char *
strtok(cmd, toks)
char *cmd;
const char *toks;
{
    static char *Cmd;
    char *ptr;

    if (cmd == NULL) {
	cmd = Cmd;
	if (cmd == NULL)
	    return(NULL);
    }

    /*
     *	skip whitespace
     */

    while (*cmd && strchr(toks, *cmd))
	++cmd;

    if (*cmd == 0)
	return(NULL);

    if (ptr = strpbrk(cmd, toks))
	*ptr++ = 0;
    Cmd = ptr;
    return(cmd);
}


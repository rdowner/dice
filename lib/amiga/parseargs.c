
/*
 *  PARSEARGS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <lib/misc.h>

/*
 *  Returns argc for arguments.  Does NOT include arg0
 */

int
_parseargs1(copy, len)
char *copy;
int len;
{
    int ac = 0;

    for (;;) {
	while (*copy == ' ' || *copy == 9)
	    ++copy;
	if (*copy == 0 || *copy == 0x0A)
	    break;
	++ac;
	if (*copy == '\"') {
	    do {
		++copy;
	    } while (*copy && *copy != '\"');
	} else {
	    while (*copy && *copy != 0x0A && *copy != ' ' && *copy != 9)
		++copy;
	}
	if (*copy == 0)
	    break;
	*copy++ = 0;
    }
    return(ac);
}

/*
 *  Dumps 'ac' arguments into av beginning at index 0.
 */

void
_parseargs2(copy, av, ac)
char *copy;
char **av;
int ac;
{
    while (ac) {
	while (*copy == ' ' || *copy == 9)
	    ++copy;
	if (*copy == '\"')
	    ++copy;
	*av++ = copy;
	while (*copy)
	    ++copy;
	++copy;
	--ac;
    }
}


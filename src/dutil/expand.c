/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  EXPAND.C
 *
 *  EXPAND [tpl] wildcard
 */

#include <stdio.h>
#include <stdlib.h>

#include <lib/version.h>

#ifdef _DCC
IDENT("expand",".2");
DCOPYRIGHT;
#endif


main(ac, av)
int   ac;
char *av[];
{
    int   i;
    char  *tpl;

    switch(ac) {
    case 1:
        puts("Expand \"fmtstring\" wildcards...");
        puts("Expand %s #?");
        exit(1);
    case 2:
        tpl = "%s";
        break;
    default:
        tpl = av[1];
        ++av;
        --ac;
    }
#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 1; i < ac; ++i) {
        char *p = av[i];
        printf(tpl, p, p, p, p, p, p, p, p, p, p);
        puts("");
    }
    return(0);
}


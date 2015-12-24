/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CAT.C
 */

#include <stdio.h>
#include <stdlib.h>

#include <lib/version.h>

IDENT("cat",".2");
DCOPYRIGHT;

int _DiceCacheEnable = 1;

main(ac, av)
int   ac;
char **av;
{
    int   i;
    char buf[256];

    Ident;		/* references so GNU-C does not complain */
    DCopyright;

#ifdef AMIGA
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 1; i < ac; ++i) {
	FILE *fi = fopen(av[i], "r");
	if (fi == NULL) {
	    fprintf(stderr, "Unable to open %s\n", av[i]);
	    continue;
	}
	while (fgets(buf, sizeof(buf), fi))
	    fputs(buf, stdout);
	fclose(fi);
    }
    if (ac == 1) {
	short c;
	while ((c = getc(stdin)) != EOF)
	    putc(c, stdout);
    }
    return(0);
}


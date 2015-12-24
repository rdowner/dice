/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  HEAD.C
 *
 *
 *  print first 10 lines of a file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

IDENT("head",".3");
DCOPYRIGHT;

void head(FILE *);

int
main(ac, av)
int   ac;
char **av;
{
    int   i;

#ifdef AMIGA
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 1; i < ac; ++i) {
	FILE *fi;

	if ((fi = fopen(av[i], "r")) != NULL) {
	    if (ac > 2)
		printf("--- %s ---\n", av[i]);
	    head(fi);
	    fclose(fi);
	    puts("");
	} else {
	    printf ("--- %s --- (unable to open)\n", av[i]);
	}
    }
    return(0);
}

void
head(fi)
FILE *fi;
{
    char buf[256];
    short i;

    for (i = 0; i < 10; ++i) {
	if (fgets(buf, sizeof(buf), fi) == NULL)
	    break;
	fputs(buf, stdout);
    }
}


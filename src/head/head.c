/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  HEAD.C
 *
 *  print first 10 lines of a file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/version.h>

#ifdef _DCC
IDENT("head",".3");
DCOPYRIGHT;
#endif

void head(FILE *);

int
main(ac, av)
int   ac;
char **av;
{
    int   i;

#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 1; i < ac; ++i) {
	FILE *fi;

	if (fi = fopen(av[i], "r")) {
	    if (ac > 2)
	    {
		fputs("--- ", stdout);
		fputs(av[i],  stdout);
		puts(" ---");
//		printf("--- %s ---\n", av[i]);
	    }
	    head(fi);
	    fclose(fi);
	    puts("");
	} else {
	    fputs("--- ", stdout);
	    fputs(av[i],  stdout);
	    puts(" --- (unable to open)");
//	    printf ("--- %s --- (unable to open)\n", av[i]);
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


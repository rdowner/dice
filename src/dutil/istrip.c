/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ISTRIP.C
 *
 *  ISTRIP destprefix wildcards
 *
 *
 *  Strips the files specified by the wildcard and generates files by
 *  prefixing each file with the destprefix.
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

IDENT("istrip",".3");
DCOPYRIGHT;

void StripFile(char *, char *);

int
main(int ac, char **av)
{
    int   i;

    if (ac == 1) {
	puts(Ident);
	puts(DCopyright);
	puts("istrip destprefix wildcards");
	exit(1);
    }

#ifdef AMIGA
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 2; i < ac; ++i) {
	char buf[256];
	sprintf(buf, "%s%s", av[1], av[i]);
	StripFile(av[i], buf);
	puts("");
    }
    return(0);
}

void
StripFile(sname, dname)
char *sname;
char *dname;
{
    FILE *fi;
    FILE *fo;
    short c;
    short firstComment = 1;

    printf("%-20s %-20s ", sname, dname);
    fflush(stdout);
    fi = fopen(sname, "r");
    if (fi == NULL) {
	puts("unable to open source");
	return;
    }
    fo = fopen(dname, "w");
    if (fo == NULL) {
	puts("unable to open dest");
	fclose(fi);
	return;
    }
    while ((c = getc(fi)) != EOF) {
	switch(c) {
	case '/':       /*  look for comment    */
	    c = getc(fi);
	    if (c != '*') {
		fputc('/', fo);
		fputc(c, fo);
		continue;
	    }
	    if (c == '*' && firstComment == 1) {
		firstComment = 0;
		fputc('/', fo);
		fputc(c, fo);
		continue;
	    }
	    for (;;) {
		if (c == EOF)
		    break;
		if (c == '*') {
		    c = getc(fi);
		    if (c != '/')
			continue;
		    break;
		} else {
		    c = getc(fi);
		}
	    }
	    break;
	case ' ':
	case 9:
	    while (c == 9 || c == ' ')
		c = getc(fi);
	    if (c != EOF)
		ungetc(c, fi);
	    putc(' ', fo);
	    break;
	default:
	    putc(c, fo);
	    break;
	}
    }
    fclose(fi);
    fclose(fo);
}


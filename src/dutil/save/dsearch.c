
/*
 *  DSEARCH.C
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 *
 *  DSEARCH string files ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <lib/version.h>

#ifdef _DCC
IDENT("dsearch", ".3");
DCOPYRIGHT;
#endif

static char Tmp[256];
static long LastLine = -1;	/*  offset of base of last printed line */

short QuietOpt = 0;

void look(char *, char *, FILE *);
void print_line(FILE *, long, char *, char *, char *);

main(ac, av)
int   ac;
char **av;
{
    int i;

    if (ac == 1) {
	puts(Ident);
	puts("DSEARCH srch-string files");
	puts("can use AmigaDOS wildcards for files");
	exit(1);
    }
    {
	char *ptr;

	if ((ptr = strrchr(av[0], '/')) == NULL && (ptr = strrchr(av[0], ':')) == NULL)
	    ptr = av[0];
	else
	    ++ptr;
    }

#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif

    for (i = 2; i < ac; ++i) {
	FILE *fi;

	if (fi = fopen(av[i], "r")) {
	    if (ac > 2 && !QuietOpt)
		printf("--- %s ---\n", av[i]);
	    LastLine = -1;
	    look(av[1], av[i], fi);
	    fclose(fi);
	} else {
	    if (QuietOpt == 0)
		printf ("--- %s --- (unable to open)\n", av[i]);
	}
    }
    return(0);
}

char Buf[8192];

void
look(str, fileName, fi)
char *str;
char *fileName;
FILE *fi;
{
    int len = strlen(str);
    long off = 0;
    short n;
    short nn;
    char *ptr;

    while ((nn = fread(Buf, 1, sizeof(Buf), fi)) > 0) {
	n = nn;
	for (ptr = Buf; n > 0; ++ptr, --n) {
	    if (((*str ^ *ptr) & ~0x20) == 0) {
		if (n < len) {
		    if (strnicmp(ptr, str, n) == 0) {	/*  doesn't happen */
			fseek(fi, off + nn - n, 0);	/*  often, I hope. */
			break;
		    }
		} else {
		    if (strnicmp(ptr, str, len) == 0) {
			if (QuietOpt && LastLine == -1)
			    printf("--- %s ---\n", fileName);

			print_line(fi, off, Buf, ptr, Buf + nn);
		    }
		}
	    }
	}
	off = ftell(fi);
    }
}

void
print_line(fi, off, base, ptr, endptr)
FILE *fi;
long off;
char *base;
char *ptr;
char *endptr;
{
    long savpos = ftell(fi);
    short sook = 0;		    /*	nobody said I 'aught to use good 'nglish!   */

    /*
     *	search backwards for newline
     */

    while (*ptr != '\n') {
	if (ptr <= base) {	    /*	doesn't happen often */
	    if (off == 0)	    /*	beginning of file */
		break;
	    off -= sizeof(Tmp);
	    if (off < 0)
		off = 0;
	    fseek(fi, off, 0);
	    sook = 1;
	    base = Tmp;
	    endptr = base + fread(Tmp, 1, sizeof(Tmp), fi);
	    ptr = endptr;
	}
	--ptr;
    }
    if (LastLine != off + (endptr - ptr)) {
	LastLine = off + (endptr - ptr);

       /*
	*  Search forwards for newline or EOF
	*/

	do {
	    ++ptr;
	    if (ptr >= endptr) {    /*	doesn't happen often */
		off += endptr - base;
		fseek(fi, off, 0);
		sook = 1;
		base = Tmp;
		endptr = base + fread(Tmp, 1, sizeof(Tmp), fi);
		ptr = Tmp;
		if (endptr <= base) {
		    putchar('\n');
		    break;
		}
	    }
	    putchar(*ptr);
	} while (*ptr != '\n');
    }

    if (sook)
	fseek(fi, savpos, 0);
}


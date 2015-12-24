/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  AUTOREFS refsfile docfile docfile docfile
 *
 *  Given one or more autodoc or .H files (e.g. intuition.doc) or include
 *  files (e.g. exec/types.h), this program appends to a dme.refs file
 *  <refsfile> appropriate lines.
 *
 *  AUTOREFS determines the file type from the extension (.h for header
 *  files, otherwise assumed to be a doc file).
 */

#include <stdio.h>
#include <string.h>

char *SetSName(char *, char *);

main(xac, xav)
int xac;
char *xav[];
{
    short i;
    FILE *fi;
    FILE *fo;
    int ac;
    char **av;

    expand_args(xac, xav, &ac, &av);

    if (ac == 1) {
	puts("autorefs outfile docfile docfile ...");
	puts("AmigaDOS wildcarding works too, btw");
	exit(1);
    }
    fo = fopen(av[1], "a");
    if (!fo) {
	printf("unable to open %s for append\n", av[1]);
	exit(1);
    }
    for (i = 2; i < ac; ++i) {
	char *file = av[i];
	short len = strlen(file);
	short doth = 0;

	if (len >= 2 && (file[len-1] == 'h' || file[len-1] == 'H') && file[len-2] == '.')
	    doth = 1;

	fi = fopen(file, "r");
	if (fi) {
	    if (doth) {
		printf("Scanning .H  file: %s\n", file);
		scanhfile(fi, fo, file);
	    } else {
		printf("Scanning DOC file: %s\n", file);
		scandocfile(fi, fo, file);
	    }
	    fclose(fi);
	} else {
	    printf("Unable to read %s\n", file);
	}
    }
    return(0);
}

/*
 *  Find the headers for each function entry and generate a DME.REFS
 *  entry for it.  The @@<N> is a short form seek position (this field
 *  normally holds a search string).
 */

scandocfile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    char buf[256];
    long pos = 0;
    short lastLineFF = 0;

    while (fgets(buf, 256, fi)) {
	short len = strlen(buf) - 1;
	char *ptr = buf + len;
	char *bas = buf;
	char *header, *tail;

	buf[len] = 0;
	while (ptr != buf && ptr[-1] != ' ' && ptr[-1] != 9)
	    --ptr;
	while (bas < ptr && *bas == 12)
	    ++bas;
	if (ptr != bas && *ptr && strncmp(bas, ptr, strlen(ptr)) == 0) {
	    if (buf[0] == 12) {
		++pos;
		buf[0] = 0;
	    }
	    header = ptr;
	    for (ptr = buf + len; ptr != buf && IsAlphaNum(ptr[-1]); --ptr);
	    tail = ptr;
	    fprintf(fo, "%-20s (^l) %s @@%ld\n", tail, filename, pos);
	} else if (ptr == bas && *ptr && lastLineFF) {
	    if (buf[0] == 12) {
		++pos;
		buf[0] = 0;
	    }
	    for (ptr = buf + len; ptr != buf && IsAlphaNum(ptr[-1]); --ptr);
	    fprintf(fo, "%-20s (^l) %s @@%ld\n", ptr, filename, pos);
	}
	if (buf[0] == ('l'&0x1F))
	    lastLineFF = 1;
	else
	    lastLineFF = 0;
	pos = ftell(fi);
    }
}

/*
 *  Find each structure definition (stupid search, assume struct on left
 *  hand side) then generate dme.refs entry from the end point of the
 *  previous structure to the beginning of the next structure.	That is,
 *  the reference refers to the structure and all fields before and after
 *  it until the next structure (before and after).
 */

scanhfile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    static char buf[256];
    static char sname[128];
    static char lname[128];
    long lin  = 1;
    long lin1;
    long lin2 = 1;
    long pos  = 0;
    long pos1;
    long pos2 = 0;
    short snameisvalid = 0;
    short newsname = 0;

    while (fgets(buf, 256, fi)) {
	char *ptr = buf;

	if ((ptr = strstr(buf, "struct")) || (ptr = strstr(buf, "union"))) {
	    if (ptr[0] == 's')
		++ptr;
	    ptr += 5;

	    ptr = SetSName(lname, ptr);

	    /*
	     *	search for '{'
	     */

	    {
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == 12)
		    ++ptr;
		if (*ptr == 0) {
		    short c = ' ';
		    long savpos = ftell(fi);
		    while (c == ' ' || c == '\t' || c == '\n' || c == 12)
			c = getc(fi);
		    ptr[0] = c;
		    ptr[1] = 0;
		    fseek(fi, savpos, 0);
		}
	    }

	    if (*ptr == '{' && lname[0]) {
		if (snameisvalid)
		    fprintf(fo, "%-20s %3ld %s @@%ld\n", sname, lin-lin1, filename, pos1);
		strcpy(sname, lname);
		snameisvalid = 0;
		newsname = 1;
		pos1 = pos2;
		lin1 = lin2;
	    }
	}
	pos = ftell(fi);
	++lin;

	if (strstr(buf, "}")) {
	    pos2 = pos;
	    lin2 = lin;
	    snameisvalid = newsname;
	}
    }
    if (snameisvalid)
	fprintf(fo, "%-20s %3ld %s @@%ld\n", sname, lin-lin1, filename, pos1);
}

char *
SetSName(buf, ptr)
char *buf, *ptr;
{
    while (*ptr == ' ' || *ptr == 9)
	++ptr;
    while (*ptr && *ptr != '\n' && *ptr != ' ' && *ptr != 9 && *ptr != 12)
	*buf++ = *ptr++;
    *buf = 0;
    return(ptr);
}

IsAlphaNum(c)
char c;
{
    if ((c >= 'a' && c <= 'z') ||
	(c >= 'A' && c <= 'Z') ||
	(c >= '0' && c <= '9') ||
	(c == '_') || (c == '(') || (c == ')')
    )
	return(1);
    return(0);
}


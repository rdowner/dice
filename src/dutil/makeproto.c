/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MAKEPROTO.C
 *
 *  MAKEPROTO [-o outfile] [-f field] file1 file2 ... fileN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef AMIGA
#include <exec/types.h>
#include <exec/lists.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lib/version.h>
#else
#include <suplib/lists.h>
#include <include/lib/version.h>
#endif

typedef struct List List;
typedef struct Node Node;

void ScanFile(char *);
void help(int);
void xprintf(const char *, ...);
void DumpNodeList(void);
int EndsWithSlash(const char *);

#ifdef AMIGA
#ifdef _DCC
IDENT("makeproto",".4");
DCOPYRIGHT;
#endif
#endif

char	*Field = "Prototype";
char	*OutFile;
int	FieldLen = 9;
int	_DiceCacheEnable = 1;
List	OutList;

int
main(int ac, char **av)
{
    short i;

    NewList(&OutList);

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];
	if (*ptr != '-') {
	    ScanFile(ptr);
	    continue;
	}
	ptr += 2;
	switch(ptr[-1]) {
	case 'f':
	    if (*ptr == 0)
		ptr = av[++i];
	    Field = ptr;
	    FieldLen = strlen(ptr);
	    break;
	case 'o':
	    if (*ptr == 0)
		ptr = av[++i];
	    OutFile = ptr;
	    xprintf("\n/* MACHINE GENERATED */\n\n");
	    break;
	default:
	    printf("illegal option: -%c\n", ptr[-1]);
	    help(1);
	}
    }
    if (ac == 1)
	help(0);
    else
	DumpNodeList();
    return(0);
}

void
help(int code)
{
    fputs("MAKEPROTO - Scans for 'Prototype' lines in source files\n", stderr);
    fputs("makeproto [-o outfile] file1 file2... fileN\n", stderr);
    exit(code);
}

void
ScanFile(file)
char *file;
{
    FILE *fi = fopen(file, "r");
    char buf[512];

    if (fi == NULL) {
	fprintf(stderr, "makeproto: couldn't open %s\n", file);
	return;
    }
    xprintf("\n/* %-20s */\n\n", file);
    while (fgets(buf, sizeof(buf), fi)) {
	char *ptr = buf;

	if (*ptr == ';')
	    ++ptr;
	if (strncmp(ptr, Field, FieldLen) == 0) {
	    xprintf("%s", ptr);
	    while (EndsWithSlash(ptr)) {
		if (fgets(buf, sizeof(buf), fi) == NULL)
		    break;
		if (*ptr == ';')
		    ++ptr;
		xprintf("%s", ptr);
	    }
	}
    }
    fclose(fi);
}

/*
 *  xprintf() - write to a node list
 */

void
xprintf(const char *ctl, ...)
{
    va_list va;
    static char Buf[256];
    short i;
    Node *node;

    va_start(va, ctl);
    i = vsprintf(Buf, ctl, va);
    va_end(va);

    node = malloc(sizeof(Node) + i + 1);
    node->ln_Name = (char *)(node + 1);
    strcpy(node->ln_Name, Buf);
    AddTail(&OutList, node);
}

void
DumpNodeList(void)
{
    FILE *fo;
    Node *node;
    short i;

    if (OutFile && (fo = fopen(OutFile, "r"))) {
	for (node = OutList.lh_Head; node->ln_Succ; node = node->ln_Succ) {
	    for (i = 0; node->ln_Name[i] && (unsigned char)node->ln_Name[i] == getc(fo); ++i)
		;
	    if (node->ln_Name[i])
		break;
	}

	/*
	 *  no change to file, do not update (this allows us to have a
	 *  DMakefile dependancy on the prototype file to force, for
	 *  example, precompiled defs to be recreated)
	 */

	if (node->ln_Succ == NULL && getc(fo) == EOF) {
	    fclose(fo);
	    return;
	}
	fclose(fo);
    }
    if (OutFile)
	fo = fopen(OutFile, "w");
    else
	fo = stdout;

    if (fo) {
	while ((node = RemHead(&OutList)) != NULL)
	    fwrite(node->ln_Name, 1, strlen(node->ln_Name), fo);
    } else {
	fprintf(stderr, "Unable to create %s\n", OutFile);
	exit(20);
    }

    if (fo && OutFile)
	fclose(fo);
}

int
EndsWithSlash(const char *ptr)
{
    if ((ptr = strrchr(ptr, '\\')) != NULL) {
	for (++ptr; *ptr == ' ' || *ptr == '\t'; ++ptr)
	    ;
	if (*ptr == 0 || *ptr == '\n')
	    return(1);
    }
    return(0);
}


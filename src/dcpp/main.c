/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MAIN.C
 *
 *  CPP inputfile <options>
 *
 */

#include "defs.h"
#include "DCPP_rev.h"

static char *DCopyright =
"Copyright (c) 1992,1993,1994 Obvious Implementations Corp., Redistribution & Use under DICE-LICENSE.TXT." VERSTAG;


Prototype FILE *Fo;
Prototype char *OutName;
Prototype char *DefAmigaDir;
Prototype short DDebug;
Prototype short ExitCode;
Prototype short NoDefaultIncs;
Prototype short UndefSyms;
Prototype short TriGraph;
Prototype short GlobalStringize;
Prototype short SlashSlashOpt;
Prototype short UnixOpt;
Prototype short FFPDef;
Prototype short ForceLineSpec;
Prototype short ErrorOpt;
Prototype FILE	*ErrorFi;
Prototype short StripOpt;
Prototype PreCompNode *PreCompBase;
Prototype char VersionId[];
Prototype short AnyPrecomps;

Prototype int main(int, char **);
Prototype void ParseOpts(short, char **, short);
Prototype void help(int);
Prototype char *mergestr(char *s1, char *s2);

#ifdef _DCC
char VersionId[] = { VERSION, REVISION, 0 };
#else
char VersionId[] = { "" };
#endif

FILE *Fo;
char *InFile;
char *OutFile;
char *OutName;
char *DefIncludePath;
char *DefAmigaDir;
char Prefix[64];
short DDebug;
short ExitCode;
short NoDefaultIncs;
short UndefSyms;
short TriGraph = 1;
short GlobalStringize;
short SlashSlashOpt;
short UnixOpt;
short FFPDef;
short ForceLineSpec;
short AnyPrecomps;
short IncPri = 8;	    /*	command line -I options go before defaults */
short ErrorOpt;
FILE  *ErrorFi;
short StripOpt;
PreCompNode *PreCompBase;


int _DiceCacheEnable = 1;

int _bufsiz = 8192;

int
main(ac, av)
int ac;
char *av[];
{
    FILE *fi;

    if (ac == 1)
	help(1);

    {
	char *ptr;
	char *p2;

	if ((ptr = strrchr(av[0], '/')) || (ptr = strrchr(av[0], ':')))
	    ++ptr;
	else
	    ptr = av[0];
	if ((p2 = strchr(ptr, '_')) == NULL)
	    p2 = ptr;
	else
	    ++p2;
	sprintf(Prefix, "%.*s", (int)(p2 - ptr), ptr);
    }

#ifdef AMIGA
    DefIncludePath = mergestr(Prefix, "dinclude:");
#else
    if (getenv("DINCLUDE")) {
	DefIncludePath = getenv("DINCLUDE");
    } else {
	DefIncludePath = mergestr(INSTDIR, Prefix);
	DefIncludePath = mergestr(DefIncludePath, "include/");
    }
#endif
    DefAmigaDir = mergestr(DefIncludePath, "amiga");
    dbprintf(("DefAmigaDir: %s\n", DefAmigaDir));
    dbprintf(("DefIncludePath: %s\n", DefIncludePath));

    InitPrecomp();
    InitCpp();

    AddInclude(DefIncludePath, 5);
    AddInclude(mergestr(DefIncludePath, "pd"), 5);
    AddInclude(DefAmigaDir, 5);

    ParseOpts(ac, av, 1);

    IncPri = 10;     /*  any #pragma -I options go before everything */

    if (InFile == NULL)
	cerror(EFATAL_INPUT_FILE_REQUIRED);

    fi = fopen(InFile, "r");
    if (fi == NULL) {
	ErrorOpenFailed(InFile, 0);
	exit(20);
    }

    if (OutFile) {
	Fo = fopen(OutFile, "w+");
	if (Fo == NULL)
	    cerror(EFATAL_CANT_CREATE_FILE, OutFile);
	OutName = OutFile;
    } else {
	Fo = stdout;
    }

    InitDefines(InFile);
    InitDirective();

    cpp(0, 0, InFile, fi, NULL, 0);

    fclose(Fo);
    if (ExitCode > 5) {
	if (OutName)
	    remove(OutName);
    }
    return(ExitCode);
}

void
help(int code)
{
    printf("%s\n%s\n", VSTRING, DCopyright);
    puts("DCPP infile [-o outfile] <other-options>");
    exit(code);
}

#ifdef NOTDEF

extern int32_t *__MemList;

void
du(ptr)
int32_t *ptr;
{
    char buf[64];

    sprintf(buf, "%08lx %08lx %d\n", ptr, ptr[0], ptr[1]);
    Write(Output(), buf, strlen(buf));
}

#endif

void
ParseOpts(short ac, char **av, short cppOnly)
{
    short i;

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];

	if (*ptr != '-') {
	    /*
	     *	If from a #pragma (cppOnly == 0) then do not parse any
	     *	new file names, they could be part of previous options
	     *	that were not meant for us.
	     */

	    if (cppOnly) {
		InFile = ptr;
		AddSourceDirInclude(InFile);
	    }
	    continue;
	}
	ptr += 2;
	switch(ptr[-1]) {
	case '1':       /*  -1.x    */
	case '2':       /*  -2.x    */
	case '3':       /*  -3.x    */
	case '4':       /*  -4.x    */
	case '5':       /*  -5.x    */
	    if (ptr[0] != '.')
		help(1);
	    {
		static char ABuf[256];

#ifdef AMIGA
		sprintf(ABuf, "%sdinclude:amiga%c%c",
		    Prefix,
		    ptr[-1],
		    ptr[1]
		);
#else
		snprintf(ABuf, sizeof(ABuf), INSTDIR "%sinclude/amiga%c%c",
		    Prefix,
		    ptr[-1],
		    ptr[1]
		);
#endif
		if (DefAmigaDir)
		    RemInclude(DefAmigaDir);
		DefAmigaDir = ABuf;
		AddInclude(DefAmigaDir, 5);
		dbprintf(("change DefAmigaDir: %s\n", DefAmigaDir));
	    }
	    break;
	case 'n':
	    if (strcmp(ptr, "otri") == 0) {
		TriGraph = 0;
		break;
	    } else if (strcmp(ptr, "opragma") == 0) {
		/*PragmaWarn = 0;*/
		break;
	    }
	    if (cppOnly)
		cppOnly = -1;
	    break;
	case 'd':
	    if (*ptr)
		DDebug = atoi(ptr);
	    else
		DDebug = 1;
	    break;
	case 'f':
	    if (strcmp(ptr, "fp") == 0)
		FFPDef = 1;
	    break;
	case 'o':
	    if (*ptr)
		OutFile = ptr;
	    else
		OutFile = av[++i];
	    break;
	case 'D':
	    if (*ptr == 0)
		ptr = av[++i];
	    DefineOptSymbol(ptr);
	    break;
	case 'H':   /*  -H<path>=<header> - precompiled header spec */
	    AnyPrecomps = 1;
	    if (*ptr == 0)
		ptr = av[++i];
	    {
		PreCompNode *pcn = malloc(sizeof(PreCompNode));
		pcn->pn_Next = PreCompBase;
		pcn->pn_OutName = ptr;
		if ((pcn->pn_HeadName = strchr(ptr, '=')) != NULL)
		    *pcn->pn_HeadName++ = 0;
		else
		    help(1);
		PreCompBase = pcn;
	    }
	    break;
	case 'I':
	    if (ptr[0] == '0' && ptr[1] == 0) {
		NoDefaultIncs = 1;
		RemAllIncludes();
		AddSourceDirInclude(InFile);
		DefAmigaDir = NULL;
		break;
	    }
	    if (*ptr)
		AddInclude(ptr, IncPri);
	    else
		AddInclude(av[++i], IncPri);
	    break;
	case 'u':
	    if (strcmp(ptr, "nix") == 0) {
		UnixOpt = 1;
	    }
	    break;
	case 'U':
	    UndefSyms = 1;
	    break;
	case 'F':
	    ErrorOpt = 1;
	    if (*ptr == 'F')
		ErrorOpt = 2;
	    ptr = av[++i];
	    ErrorFi = fopen(ptr, "a");
	    break;
	case '/':
	    if (strcmp(ptr-1, "//") == 0) {
		SlashSlashOpt = 1;
	    } else {
		if (cppOnly)
		    cppOnly = -1;
	    }
	    break;
	case 's':
	    if (strcmp(ptr, "trip") == 0) {
		StripOpt = 1;
	    }
	    break;
	default:
	    if (cppOnly)
		cppOnly = -1;
	    break;
	}
	if (cppOnly < 0)
	    break;
    }
    if (cppOnly < 0)
	help(1);
}

char *
mergestr(char *s1, char *s2)
{
    char *ptr = malloc(strlen(s1) + strlen(s2) + 1);
    sprintf(ptr, "%s%s", s1, s2);
    return(ptr);
}


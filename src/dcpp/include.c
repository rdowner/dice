/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  INCLUDE.C
 *
 */

#include "defs.h"

#ifndef O_BINARY
#define O_BINARY    0
#endif

Prototype void AddSourceDirInclude(char *);
Prototype void AddInclude(char *, short);
Prototype void RemInclude(char *);
Prototype void RemAllIncludes(void);
Prototype void do_include(char *, int, int32_t *);

Prototype short PreCompFlag;

static MinList IncList = { (MinNode *)&IncList.mlh_Tail, NULL, (MinNode *)&IncList.mlh_Head };
static short MaxIncLen;

short PreCompFlag;

void
AddSourceDirInclude(buf)
char *buf;
{
    char *ptr;
    char c;

    for (ptr = buf + strlen(buf); ptr >= buf; --ptr) {
	if (*ptr == '/' || *ptr == ':')
	    break;
    }
    ++ptr;
    c = *ptr;
    *ptr = 0;
    AddInclude(buf, 99);
    *ptr = c;
}

void
AddInclude(char *dir, short pri)
{
    short len = strlen(dir);
    Node *node = malloc(sizeof(Node) + len + 2);

    node->ln_Name = (char *)(node + 1);
    node->ln_Pri = pri;
    strcpy(node->ln_Name, dir);

    if (len && dir[len-1] != ':' && dir[len-1] != '/') {
	strcat(node->ln_Name, "/");
	++len;
    }
    Enqueue((List *)&IncList, node);
    if (MaxIncLen < len)
	MaxIncLen = len;
}

/*
 *  Remove default includes by name
 */

void
RemInclude(dir)
char *dir;
{
    Node *node;

    if ((node = FindName((List *)&IncList, dir)) != NULL) {
	Remove(node);
	free(node);
    }
}

void
RemAllIncludes(void)
{
    Node *node;

    while ((node = (Node *)IncList.mlh_Head) != (Node *)&IncList.mlh_Tail)
	RemInclude(node->ln_Name);
}

/*
 *  note:   pidx is a pointer to the index at the current level.  This allows
 *	    macro's to extend beyond the include back into the higher level
 *	    even though this feature is probably not standard.
 */

void
do_include(buf, max, pidx)
char *buf;
int max;
int32_t *pidx;
{
    int i;
    int b;
    int32_t pos;
    short done = 0;
    FILE *fi = NULL;
    char *tmp;
    Node *inc;
    PreCompNode *pcn;
    short len;

    /* First skip over any leading white space */
    i = 0;
    while((i < max) && WhiteSpace[(ubyte)buf[i]]) i++;

    /* Do we have a symbol to deal with here first? */
    if ((i < max) && SymbolChar[(ubyte)buf[i]])
    {
	/* We have a symbol.  We need to expand it in place in the buffer */
	int32_t b = i;
	Sym *node;

	i = ExtSymbol(buf, i, max);
	if ((node = FindSymbol(buf + b, i - b)) != NULL) {
	    buf = node->Text;
	    max = node->TextLen;
	    if (node->NumArgs >= 0) {
		cerror(EERROR_PARSING_INCLUDE);
		return;
	    }
	} else {
	    cerror(EERROR_PARSING_INCLUDE);
	    return;
	}
    }

    for (i = 0; i < max && buf[i] != '\"' && buf[i] != '<'; ++i)
        ;
    if (i == max) {
	cerror(EERROR_PARSING_INCLUDE);
	return;
    }
    for (b = ++i; i < max && buf[i] != '\"' && buf[i] != '>'; ++i)
        ;
    if (i == max) {
	cerror(EERROR_PARSING_INCLUDE);
	return;
    }
    len = i - b;
    if (len <= 0 || len > 512)
	cerror(EERROR_PARSING_INCLUDE);
    tmp = zalloc(MaxIncLen + len + 8);

    strncpy(tmp, buf + b, len);
    tmp[len] = 0;

    /*
     *	check against precompiled to-include list.  If found check existance
     *	of precompiled include & version.
     */

    for (pcn = PreCompBase; pcn; pcn = pcn->pn_Next) {
	if (stricmp(pcn->pn_HeadName, tmp) == 0) {
	    int fd;
	    PreCompHdr pch;

	    if ((fd = open(pcn->pn_OutName, O_RDONLY|O_BINARY)) >= 0) {
		if (read(fd, &pch, sizeof(pch)) == sizeof(pch)) {
		    if (pch.pc_Magic == PCH_MAGIC && strcmp(pch.pc_Version, VersionId) == 0) {
			LoadPrecompiledHeader(pcn->pn_OutName, &pch, fd);
			done = 1;
		    }
		}
		close(fd);
	    }
	    break;
	}
    }

    if (done == 0) {
	/*
	 *  normal include.
	 *
	 *  If an include file is including itself, we use sameFi to
	 *  remember the fact and 'skip over' it to try to find another
	 *  include file at a deeper level of the same name.  If we do
	 *  not we revert to the skipped over include.
	 */

	FILE *sameFi = NULL;

	fi = fopen(tmp, "r");
#ifdef _DCC
	if (fi == NULL && UnixOpt) {
	    char *unixPath = UnixToAmigaPath(tmp);
	    if (strcmp(unixPath, tmp) != 0)
		fi = fopen(unixPath, "r");
	}
#endif
	for (inc = (Node *)IncList.mlh_Head; fi == NULL && inc != (Node *)&IncList.mlh_Tail; inc = inc->ln_Succ) {
	    sprintf(tmp, "%s%.*s", inc->ln_Name, len, buf + b);

	    fi = fopen(tmp, "r");
#ifdef _DCC
	    if (fi == NULL && UnixOpt) {
		char *unixPath = UnixToAmigaPath(tmp);
		if (strcmp(unixPath, tmp) != 0) {
		    strcpy(tmp, unixPath);
		    fi = fopen(unixPath, "r");
		}
	    }
#endif
	    /*
	     *	If this is the same include files as the current one,
	     *	that is an include including itself, attempt to find
	     *	another.  If we can't, we stick with this one.
	     */

	    if (fi &&
		PushBase &&
		PushBase->FileName &&
		strcmp(tmp, PushBase->FileName) == 0
	    ) {
		if (sameFi) {
		    fclose(fi);
		} else {
		    sameFi = fi;
		}
		fi = NULL;
	    }
	    dbprintf(("try: %s (%08lx)\n", tmp, (uint32_t)(uintptr_t)fi));
	}

	/*
	 *  If we didn't find an include and don't have one to back off
	 *  to, we error out.  Otherwise we revert to the back off
	 *  include (an include including itself).  If we did find an
	 *  include we throw away the back off include.
	 */

	if (fi == NULL) {
	    if (sameFi == NULL) {
		ErrorOpenFailed(buf + b, len);
		return;
	    }
	    fi = sameFi;
	    strcpy(tmp, PushBase->FileName);
	    sameFi = NULL;
	}
	if (sameFi) {
	    fclose(sameFi);
	    sameFi = NULL;
	}

	/*
	 *  If pch != NULL we must remember our current place in the output
	 *  file and after cpp() runs create a precompiled file as appropriate.
	 */

	{
	    FILE *foSave = NULL;

	    if (pcn) {
		pos = ftell(Fo);
		if (pos < 0)
		    cerror(EFATAL_SEEK_OUTPUT);
		++SymGroup;
		foSave = Fo;
		if ((Fo = fopen(pcn->pn_OutName, "w+")) == NULL)
		    cerror(EFATAL_CANT_CREATE_FILE, pcn->pn_OutName);
		DumpPrecompiledPrefix(pcn);
		++PreCompFlag;
	    }

	    *pidx = cpp(*pidx, PushBase->Level + 1, tmp, fi, NULL, 0);

	    if (pcn) {
		DumpPrecompiledPostfix(pcn, foSave);
		--PreCompFlag;
		fclose(Fo);
		Fo = foSave;
		/*
		DumpPrecompiledHeader(pcn, Fo, pos, ftell(Fo));
		*/
	    }
	}
    }
    if (PushBase) {
	fprintf(Fo, "\n# %d \"%s\" %d\n",
		PushBase->LineNo, PushBase->FileName, PushBase->Level);
    }
}


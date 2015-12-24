
/*
 *  CMDLIST.c
 *
 */

#include "defs.h"

Prototype void InitCmdList(void);
Prototype void PutCmdListChar(List *, char);
Prototype void PutCmdListSym(List *, char *, short *);
Prototype void CopyCmdList(List *, List *);
Prototype void AppendCmdList(List *, List *);
Prototype int  PopCmdListSym(List *, char *, long);
Prototype int  PopCmdListChar(List *);
Prototype void CopyCmdListBuf(List *, char *);
Prototype void CopyCmdListNewLineBuf(List *, char *);
Prototype long CmdListSize(List *);
Prototype long CmdListSizeNewLine(List *);
Prototype void CopyCmdListConvert(List *, List *, char *, char *);
Prototype long ExecuteCmdList(DepNode *, List *);

List CmdFreeList;
__aligned char CmdTmp1[256];
__aligned char CmdTmp2[256];

void
InitCmdList()
{
    NewList(&CmdFreeList);
}

void
PutCmdListChar(List *list, char c)
{
    CmdNode *node;

    if ((node = GetTail(list)) == NULL || (node->cn_Idx == node->cn_Max)) {
	if ((node = RemHead(&CmdFreeList)) == NULL) {
	    node = malloc(sizeof(CmdNode) + 64);
	    node->cn_Node.ln_Name = (char *)(node + 1);
	    node->cn_Max = 64;
	}
	node->cn_Node.ln_Type = 0;
	node->cn_Idx = 0;
	node->cn_RIndex = 0;
	AddTail(list, &node->cn_Node);
    }
    node->cn_Node.ln_Name[node->cn_Idx++] = c;
}

void
PutCmdListSym(list, buf, pspace)
List *list;
char *buf;
short *pspace;
{
    if (*buf) {
	if (pspace) {
	    if (*pspace)
		PutCmdListChar(list, ' ');
	    *pspace = 1;
	}
	/*if ((node = GetTail(list)) && node->cn_Idx && node->cn_Node.ln_Name[node->cn_Idx-1] != ' ')*/
	while (*buf) {
	    PutCmdListChar(list, *buf);
	    ++buf;
	}
    }
}

void
CopyCmdList(fromList, toList)
List *fromList;
List *toList;
{
    CmdNode *from;
    long n;
    long i;

    for (from = GetHead(fromList); from; from = GetSucc(&from->cn_Node)) {
	CmdNode *copy = NULL;

	dbprintf(("COPYFROM %*.*s\n", from->cn_Idx, from->cn_Idx, from->cn_Node.ln_Name));

	for (n = 0; n < from->cn_Idx; ) {
	    if ((copy = RemHead(&CmdFreeList)) == NULL) {
		copy = malloc(sizeof(CmdNode) + 64);
		copy->cn_Max = 64;
		copy->cn_Node.ln_Name = (char *)(copy + 1);
	    }
	    i = (copy->cn_Max < from->cn_Idx - n) ? copy->cn_Max : from->cn_Idx - n;
	    copy->cn_Node.ln_Type = 0;
	    copy->cn_Idx = i;
	    copy->cn_RIndex = 0;
	    movmem(from->cn_Node.ln_Name + n, copy->cn_Node.ln_Name, i);
	    AddTail(toList, &copy->cn_Node);
	    n += i;
	}
	if (copy)
	    copy->cn_Node.ln_Type = from->cn_Node.ln_Type;
    }
}

void
AppendCmdList(fromList, toList)
List *fromList;
List *toList;
{
    CmdNode *from;

    while (from = RemHead(fromList))
	AddTail(toList, &from->cn_Node);
}


/*
 *  pop a symbol (symbols are separated by white space)
 */

int
PopCmdListSym(cmdList, buf, max)
List *cmdList;
char *buf;
long max;
{
    short c;
    short i = 0;

    --max;

    while ((c = PopCmdListChar(cmdList)) == ' ' || c == '\t' || c == '\n')
	;
    while (c != EOF && c != ' ' && c != '\t' && c != '\n' && i < max) {
	buf[i++] = c;
	c = PopCmdListChar(cmdList);
    }
    buf[i] = 0;
    return((i) ? 0 : -1);
}

int
PopCmdListChar(cmdList)
List *cmdList;
{
    CmdNode *node;
    short c = EOF;

    while (node = GetHead(cmdList)) {
	if (node->cn_RIndex != node->cn_Idx)
	    return((ubyte)node->cn_Node.ln_Name[node->cn_RIndex++]);
	Remove((struct Node *)node);
	AddTail(&CmdFreeList, &node->cn_Node);
    }
    return(c);
}

void
CopyCmdListBuf(list, buf)
List *list;
char *buf;
{
    CmdNode *node;
    long n = 0;

    while (node = RemHead(list)) {
	movmem(node->cn_Node.ln_Name + node->cn_RIndex, buf, node->cn_Idx - node->cn_RIndex);
	buf += node->cn_Idx;
	AddTail(&CmdFreeList, &node->cn_Node);
    }
    buf[0] = 0;
}

void
CopyCmdListNewLineBuf(list, buf)
List *list;
char *buf;
{
    CmdNode *node;
    long i;

    while (node = RemHead(list)) {
	for (i = node->cn_RIndex; i < node->cn_Idx && node->cn_Node.ln_Name[i] != '\n'; ++i)
	    *buf++ = node->cn_Node.ln_Name[i];
	if (i != node->cn_Idx) {
	    node->cn_RIndex = i + 1;
	    AddHead(list, &node->cn_Node);
	    break;
	}
	AddTail(&CmdFreeList, &node->cn_Node);
    }
    *buf = 0;
}

long
CmdListSize(list)
List *list;
{
    CmdNode *node;
    long n = 0;

    for (node = GetHead(list); node; node = GetSucc(&node->cn_Node))
	n += node->cn_Idx - node->cn_RIndex;
    return(n);
}

long
CmdListSizeNewLine(list)
List *list;
{
    CmdNode *node;
    long n = 0;
    long i;

    for (node = GetHead(list); node; node = GetSucc(&node->cn_Node)) {
	for (i = node->cn_RIndex; i < node->cn_Idx && node->cn_Node.ln_Name[i] != '\n'; ++i)
	    ;
	n += i - node->cn_RIndex;
	if (i != node->cn_Idx)
	    break;
    }
    return(n);
}

void
CopyCmdListConvert(fromList, toList, srcMat, dstMat)
List *fromList;
List *toList;
char *srcMat;
char *dstMat;
{
    List tmpList;
    short space = 0;

    dbprintf(("fromlist %08lx copyconvert '%s' -> '%s'\n", GetHead(fromList), srcMat, dstMat));
    srcMat = ExpandVariable(srcMat, NULL);
    dstMat = ExpandVariable(dstMat, NULL);

    NewList(&tmpList);
    CopyCmdList(fromList, &tmpList);

    if (!*srcMat)
    {
       if (!GetHead(&tmpList))
       {
          PutCmdListSym(toList, dstMat, &space);
          return;
       }
       /* We need to replace the src and destination with meaningful wildcards */
       srcMat = dstMat = "*";
    }

    /*
     *	run each symbol through the conversion
     */

    while (PopCmdListSym(&tmpList, CmdTmp1, sizeof(CmdTmp1)) == 0)
    {
	if (WildConvert(CmdTmp1, CmdTmp2, srcMat, dstMat) == 0)
	{
	    PutCmdListSym(toList, CmdTmp2, &space);
	}
    }
}

/*
 *  The command list is executed by making a duplicate of it then reparsing
 *  it resolving variable references
 *
 */

long
ExecuteCmdList(dep, list)
DepNode *dep;
List *list;
{
    List tmpSrc;
    List tmpDst;
    short c;
    long r = 0;

    NewList(&tmpSrc);
    NewList(&tmpDst);
    CopyCmdList(list, &tmpSrc);

    while ((c = PopCmdListChar(&tmpSrc)) != EOF) {
	if (c == '$' || c == '%') {
	    short c0 = c;

	    c = PopCmdListChar(&tmpSrc);
	    if (c == '(') {     /*  Variable Ref */
		char *spec = AllocPathBuffer();

		/*
		 *  copy variable specification into a buffer then resolve
		 *  it to tmpDst
		 */
		spec[0] = c0;
		spec[1] = c;
		for (c0 = 2; (c = PopCmdListChar(&tmpSrc)) != EOF && c != ')' && c0 < PBUFSIZE - 3; ++c0) {
		    if (c == '\"') {
			spec[c0++] = c;
			while ((c = PopCmdListChar(&tmpSrc)) != EOF && c != '\"' && c0 < PBUFSIZE - 3)
			    spec[c0++] = c;
		    }
		    spec[c0] = c;
		}
		if (c != ')')
		    error(FATAL, "bad variable spec in command list for %s", dep->dn_Node.ln_Name);
		spec[c0++] = c;
		spec[c0] = 0;
		ExpandVariable(spec, &tmpDst);
		FreePathBuffer(spec);
		continue;
	    }
	    if (c != c0)		/*  $$, %% escape   */
		PutCmdListChar(&tmpDst, c0);
	}
	PutCmdListChar(&tmpDst, c);
    }

    /*
     *	pop into a command buffer for execution
     */

    {
	long n;

	while (r <= 5 && (n = CmdListSizeNewLine(&tmpDst))) {
	    short allocated;
	    short quiet = 0;
	    short ignore= 0;
	    char *cmd;

	    if (n >= sizeof(CmdTmp1) - 2) {	/*  avoid malloc    */
		allocated = 1;
		cmd = (char *)malloc(n + 2);
	    } else {
		allocated = 0;
		cmd = CmdTmp1;
	    }
	    while ((c = PopCmdListChar(&tmpDst)) != EOF && (c == ' ' || c == '\t'))
		--n;
	    if (c == '@') {
		quiet = 1;
		c = PopCmdListChar(&tmpDst);
		--n;
	    }
	    if (c == '-') {
		ignore = 1;
		c = PopCmdListChar(&tmpDst);
		--n;
	    }
	    cmd[0] = c;
	    CopyCmdListNewLineBuf(&tmpDst, cmd + 1);

	    if (c) {
		cmd[n] = 0;

		if (quiet == 0)
		    printf("    %s\n", cmd);
		if (NoRunOpt == 0 && cmd[0] != '#') {
		    r = Execute_Command(cmd, ignore);
		    if (r < 0)
			r = 20;
		    if (ExitCode < r)
			ExitCode = r;
		}
	    }
	    if (allocated)
		free(cmd);
	}
    }
    return(r);
}

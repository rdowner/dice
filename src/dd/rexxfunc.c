/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include	"defs.h"
#include	"dbug_protos.h"


Prototype BOOL		RXGetPC(char *args);
Prototype BOOL		RXGetLine(char *args);
Prototype BOOL		RXGetInfo(char *args);
Prototype BOOL		RXGetDismLine(char *args);
Prototype BOOL 		RXGetEval(char *args);
Prototype BOOL 		RXGetBytes(char *args);
Prototype BOOL		RXGetWords(char *args);
Prototype BOOL		RXGetLongs(char *args);

Prototype BOOL		RXGetCommand(char *args);

Prototype BOOL		RXStartList(char *args);
Prototype BOOL		RXEndList(char *args);
Prototype BOOL		RXPutList(char *args);



BOOL	RXGetPC(char *args)
{
    sprintf(RexxReplyString, "%08X",programPC);
    return TRUE;
}



BOOL	RXGetEval(char *args) {
long val;
short undef = TRUE;


    if(*args) {
        val = ParseExp(args, &undef, strlen(args));
    }
    if(!undef) {
	char	buf[128];
        OffsetAddressBuf(val, buf);
	sprintf(RexxReplyString,"%d %08lx  %s",val, val, buf);
	return TRUE;
    }
    sprintf(RexxReplyString,"UNDEFINED");
    return 0;
}


BOOL	RXGetLine(char *args) {
ULONG addr, line = 0;
long	info[2];    //	info[0] == lineBeg, info[1] == lineNo
WORD type, i;
short undef = TRUE;


    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
    type = CurrentMixedLine(&addr, &line, info);
    if(type == MIXTYPE_SOURCE) {
	char *lineStr = (char *)info[0];
	for (i = 0; lineStr[i] && lineStr[i] != '\n'; ++i)
   	 ;
        sprintf(RexxReplyString,"%6d. ", info[1]); 
	strncat(RexxReplyString, lineStr, i);
    }
    else {
	sprintf(RexxReplyString,"%08X",addr);
	strcat(RexxReplyString," ");
	i=strlen(RexxReplyString);
	Disassemble(addr, addr, &RexxReplyString[i]);
    }
    return TRUE;
}

BOOL	RXGetDismLine(char *args) {
ULONG addr;
short undef = TRUE, i;


    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
	sprintf(RexxReplyString,"%08X",addr);
	strcat(RexxReplyString," ");
	i=strlen(RexxReplyString);
	Disassemble(addr, addr, &RexxReplyString[i]);
    return TRUE;
}

BOOL	RXGetBytes(char *args) {
ULONG addr;
UBYTE	*address;
short undef = TRUE, i = 0, j;


    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
    address = (UBYTE *)addr;
    for (i = 0; i < 7; ++i) {
	j=strlen(RexxReplyString);
	sprintf(&RexxReplyString[j]," %02X", address[i]);
    }
    return TRUE;
}


BOOL	RXGetLongs(char *args) {
ULONG addr;
UBYTE	*address;
short undef = TRUE, i = 0, j;


    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
    address = (UBYTE *)addr;
    for (i = 0; i < 7*4; i += 4) {
	j =strlen(RexxReplyString);
	sprintf(&RexxReplyString[j]," %08X", (address[i] << 24) | (address[i+1] << 16) | (address[i+2] << 8) | address[i+3]);
    }
    return TRUE;
}

BOOL	RXGetWords(char *args) {
ULONG addr;
UBYTE	*address;
short undef = TRUE, i = 0, j;


    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
    address = (UBYTE *)addr;
    for (i = 0; i < 7*2; i += 2) {
	j =strlen(RexxReplyString);
	sprintf(&RexxReplyString[j]," %04X", (address[i] << 8) | address[i+1]);
    }
    return TRUE;
}


// RXGetCommand

BOOL RXGetCommand(char *args) {

    commandLine[commandEnd] = '\0';
    strcpy(RexxReplyString,commandLine);
    InitCommand();		// necessary to clear command line
    ScrCursoff();
    RefreshCommand(1);
    ScrCurson();

    return TRUE;


}

/* RxGetInfo function
 * returns a string in the following format
 * breakpoint status   program state
 * source file name (or NOSOURCE)   address range for source
 * line number range for source
 *
 */
BOOL	RXGetInfo(char *args) {
ULONG addr, line = 0, startaddr, endaddr;
long	info[2];    //	info[0] == lineBeg, info[1] == lineNo
int first = 0, last = 0, type;
short undef = TRUE, i;
DEBUG	*debug;

    if(*args) {
        addr = ParseExp(args, &undef, strlen(args));
    }
    if(undef)addr = programPC;
    debug = FindNearestDebug(addr);

    // give breakpoint info and program status first
    if(IsBreakpoint(addr))sprintf(RexxReplyString,"BP ");
    sprintf(RexxReplyString,"NOBP ");

    strcat(RexxReplyString,StateText(programState));

    if(debug) {	// we have source

        // find end of this source range
        type = CurrentMixedLine(&addr,&line,info);
	if (type == 0)type = NextMixedLine(&addr, &line, info);

        if(type == MIXTYPE_SOURCE) {
	    first = info[1];
	    // find end of range for the target range
	    while(NextMixedLine(&addr,&line,info) == MIXTYPE_SOURCE);
	    last = info[1];
	}

	// now find the address range for this line
	startaddr = endaddr = addr;
	while(NextMixedLine(&addr,&line,info) == MIXTYPE_DISM)endaddr=addr;

	// give source range, then address range
	i = strlen(RexxReplyString);
	sprintf(&RexxReplyString[i]," %d. %d. %08X %08X",first,last,startaddr,endaddr);

	// finally, give the file name
	strcat(RexxReplyString," ");
	strcat(RexxReplyString,DirBuf);
	strcat(RexxReplyString,debug->sourceName);
    }
    else {
	strcat(RexxReplyString," NOSOURCE 0. 0. 0 0 NOSOURCE"); 
    }
    return TRUE;
}

// Arexx List building commands
BOOL	RXStartList(char *args) {
	LIST	*lp = &CurDisplay->ds_List;

	FreeDLIST(lp);
	SetDisplayMode(DISPLAY_REXXLIST,0);

	return TRUE;
}

BOOL	RXEndList(char *args) {

	SetDisplayMode(DISPLAY_REXXLIST,0);
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	RXPutList(char *args) {
	SprintfDLIST(&CurDisplay->ds_List, DTYPE_REXXLIST, "%s", args);
	return TRUE;

}
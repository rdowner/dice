
/*
 *  SETENV.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  sets a local enviroment variable under 2.0, global env var under 1.3
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libraries/dos.h>

#if INCLUDE_VERSION >= 36
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#endif

typedef struct Library Library;

extern Library *SysBase;

int
setenv(varName, varStr)
const char *varName;
const char *varStr;
{
#if INCLUDE_VERSION >= 36
    if (SysBase->lib_Version >= 37) {
	SetVar(varName, varStr, -1, GVF_GLOBAL_ONLY);
    } else
#endif
    {
	long fh;
	BPTR lock;

	if (lock = Lock("ENV:", SHARED_LOCK)) {
	    lock = CurrentDir(lock);
	    if (fh = Open(varName, 1006)) {
		Write(fh, varStr, strlen(varStr));
		Close(fh);
	    }
	    UnLock(CurrentDir(lock));
	}
    }
    return(0);
}




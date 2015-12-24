
/*
 *  GETENV.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if INCLUDE_VERSION >= 36
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#endif

typedef struct Library Library;

typedef struct Sym {
    struct Sym *sm_Next;
    char       *sm_VarName;
    char       *sm_Data;
    long	sm_DataLen;
} Sym;

Sym *_Env_SymBase = NULL;

extern Library *SysBase;

char *
getenv(varName)
const char *varName;
{
    Sym *sym;

    for (sym = _Env_SymBase; sym; sym = sym->sm_Next) {
	if (stricmp(varName, sym->sm_VarName) == 0)
	    break;
    }
    if (sym == NULL) {
	if (sym = malloc(sizeof(Sym) + strlen(varName) + 1)) {
	    sym->sm_DataLen = -1;
	    sym->sm_VarName = (char *)(sym + 1);
	    strcpy(sym->sm_VarName, varName);

#if INCLUDE_VERSION >= 36
	    if (SysBase->lib_Version >= 37) {
		char buf[2];
		long len;

		if (GetVar(varName, buf, sizeof(buf) - 1, 0) >= 0) {
		    len = IoErr();
		    if (sym->sm_Data = malloc(len + 1)) {
			if ((len = GetVar(varName, sym->sm_Data, len + 1, 0)) >= 0) {
			    sym->sm_DataLen = len;
			} else {
			    free(sym->sm_Data);
			}
		    }
		}
	    } else
#endif
	    {
		char *ptr = malloc(strlen(varName) + 8);
		long fh;

                if (ptr)
                {
                    strcpy(ptr, "ENV:");
                    strcat(ptr, varName);
                    if (fh = Open(ptr, 1005)) {
                        long len;

                        free(ptr);
                        Seek(fh, 0L, 1);
                        if ((len = Seek(fh, 0L, 0)) >= 0) {
                            Seek(fh, 0L, -1);
                            if( (sym->sm_Data = malloc(len + 1)) != NULL)
                            {
                                if ((len = Read(fh, sym->sm_Data, len)) >= 0) {
                                    sym->sm_DataLen = len;
                                } else {
                                    free(sym->sm_Data);
                                }
                            }
                        }
                        Close(fh);
                    } else {
                        free(ptr);
                    }
                }
	    }
	    {
		long len;

		if ((len = sym->sm_DataLen) >= 0) {
		    sym->sm_Next = _Env_SymBase;
		    _Env_SymBase = sym;
		    if (len && sym->sm_Data[len-1] == '\n')
			--len;
		    sym->sm_Data[len] = 0;
		} else {
		    free(sym);
		    sym = NULL;
		}
	    }
	}
    }
    if (sym)
	return(sym->sm_Data);
    return(NULL);
}



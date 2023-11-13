/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  VAR.C
 */

#include "defs.h"
#ifdef AMIGA
#include <dos/dos.h>
#endif

Prototype void InitVar(void);
Prototype Var *MakeVar(char *, char);
Prototype Var *FindVar(char *, char);
Prototype void AppendVar(Var *, char *, long);

List VarList;

void
InitVar(void)
{
    NewList(&VarList);
}

/*
 *  create a variable, deleting any previous contents
 */

Var *
MakeVar(char *name, char type)
{
    Var *var;

    for (var = GetHead(&VarList); var; var = GetSucc(&var->var_Node)) {
        if ((char)var->var_Node.ln_Type == type && strcmp(var->var_Node.ln_Name, name) == 0) {
            while (PopCmdListChar(&var->var_CmdList) != EOF)
                ;
            return(var);
        }
    }
    var = malloc(sizeof(Var) + strlen(name) + 1);
    clrmem(var, sizeof(Var));

    var->var_Node.ln_Name = (char *)(var + 1);
    var->var_Node.ln_Type = type;
    strcpy(var->var_Node.ln_Name, name);
    NewList(&var->var_CmdList);
    AddTail(&VarList, &var->var_Node);
    return(var);
}

Var *
FindVar(char *name, char type)
{
    Var *var;

    for (var = GetHead(&VarList); var; var = GetSucc(&var->var_Node)) {
        if ((char)var->var_Node.ln_Type == type && strcmp(var->var_Node.ln_Name, name) == 0)
            break;
    }

    /*
     *  check for local & env variable(s).  local variables under 2.04
     *  or later only.
     */

    if (var == NULL || var->var_Node.ln_Type == '0') {
#ifdef AMIGA
        if (Running2_04()) {
            char *ptr;
            long len;

            if (GetVar(name, (char *)&ptr, 2, 0) >= 0) {
                len = IoErr();
                ptr = malloc(len + 1);
                if (GetVar(name, ptr, len + 1, 0) >= 0) {
                    var = MakeVar(name, '0');
                    AppendVar(var, ptr, strlen(ptr));
                }
                free(ptr);
            }
        } else {
            BPTR lock;
            long fh;
            long size;

            if (lock = Lock("ENV:", SHARED_LOCK)) {
                if (fh = Open(name, 1005)) {
                    Seek(fh, 0L, 1);
                    if ((size = Seek(fh, 0L, -1)) >= 0) {
                        char *ptr = malloc(size + 1);

                        Read(fh, ptr, size);
                        ptr[size] = 0;

                        var = MakeVar(name, '0');
                        AppendVar(var, ptr, strlen(ptr));
                        free(ptr);
                    }
                    Close(fh);
                }
                UnLock(lock);
            }
        }
#else
        {
            char *ptr;

            if ((ptr = getenv(name)) != NULL) {
                var = MakeVar(name, '0');
                AppendVar(var, ptr, strlen(ptr));
            }
        }
#endif
    }
    return(var);
}


void
AppendVar(var, buf, len)
Var *var;
char *buf;
long len;
{
    while (len--)
        PutCmdListChar(&var->var_CmdList, *buf++);
}


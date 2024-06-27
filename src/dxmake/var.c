/*
 * Copyright (c) 2003-2011,2023 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@backplane.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "defs.h"

Prototype void InitVar(void);
Prototype Var *MakeVar(char *, char);
Prototype Var *FindVar(char *, char);
Prototype void AppendVar(Var *, char *, long);
Prototype void InsertVar(Var *, char *, long);

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
    bzero(var, sizeof(Var));

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
        char *ptr;

        if ((ptr = getenv(name)) != NULL) {
            var = MakeVar(name, '0');
            AppendVar(var, ptr, strlen(ptr));
        }
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

void
InsertVar(var, buf, len)
Var *var;
char *buf;
long len;
{
    buf += len;
    while (len--)
        InsCmdListChar(&var->var_CmdList, *--buf);
}


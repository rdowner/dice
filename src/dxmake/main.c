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

void help(int);
void InitStuff(void);
const char *SkipAss(const char *);

Prototype List  DoList;
Prototype short DDebug;
Prototype short NoRunOpt;
Prototype short ExitCode;
Prototype short DoAll;
Prototype short SomeWork;
Prototype short NParallel;

List    DoList;
short   DDebug;
short   NoRunOpt;
short   QuietOpt;
short   DoAll;
short   SomeWork;
char    *XFileName = "DMakefile";
short   FileSpecified = 0;
short   ExitCode;
short   NParallel = 1;

void
myexit(void)
{
    if (QuietOpt == 0) {
        if (SomeWork) {
            printf("DMAKE Done.\n");
        } else {
            printf("All Targets up to date.\n");
        }
    }
}

int
main(int ac, char **av)
{
    short i;
    int r = 0;
    List tmpList;
    Var *var;
    NewList(&tmpList);

    InitStuff();

    /*printf("ARGS= %d\n", ac);*/
    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];
        char *p2;

        /*printf("ARG[%d]= %d:%s\n", i, strlen(av[i]), av[i]);*/

        if (*ptr != '-') {
            CreateDepRef(&DoList, ptr);
            continue;
        }
        ptr += 2;
        switch(ptr[-1]) {
        case 'F':   /*  fast opt    */
            break;
        case 'f':
            XFileName = (*ptr) ? ptr : av[++i];
            break;
        case 'n':
            NoRunOpt = 1;
            break;

        case 'D':
            ptr = (*ptr) ? ptr : av[++i];
            if ((p2 = strchr(ptr, '=')) != NULL)
                *p2++ = 0;
            var = MakeVar(ptr, '$');
            if (p2) {
                ExpandVariable((ubyte *)p2, &tmpList);
                AppendCmdList(&tmpList, &var->var_CmdList);
            }
            break;

        case 'd':
            DDebug = (*ptr) ? atoi(ptr) : 1;
            break;
        case 'a':
            DoAll = 1;
        case 'q':
            QuietOpt = 1;
            break;
        case 'j':
            NParallel = (*ptr) ? atoi(ptr) : 2;
            break;
        case 'h':
        default:
            help(1);
        }
    }

    if (i > ac)
        error(FATAL, "Expected argument to command line option");

    /*
     *  resolve dependancies requested by the user.  If none requested
     *  the resolve the first one
     */

    (void)MakeVar("TOPDIR", '$');

    {
        DepRef *node;

        ParseFile(XFileName);

        if (GetHead(&DoList) == NULL) {
            CreateDepRef(&DoList, "all");
#if 0
            if ((node = GetHead(&DepList)) != NULL)
                CreateDepRef(&DoList, ((DepNode *)node)->dn_Node.ln_Name);
#endif
        }

        while ((node = RemHead(&DoList)) != NULL) {
            if ((node->rn_Dep->dn_Node.ln_Type != NT_RESOLVED) &&
               (GetHead(&node->rn_Dep->dn_DepCmdList) == NULL))
            {
                error(FATAL, "Unable to find %s", node->rn_Node.ln_Name);
                break;
            }

            if ((r = ExecuteDependency(NULL, node, ED_WAIT)) < 0)
                break;
        }
    }
    if (r < 0 && ExitCode < 20)
        ExitCode = 20;
    return(ExitCode);
}

void
InitStuff()
{
    static int Initialized;

    atexit(myexit);
    if (Initialized == 0) {
        Initialized = 1;
        NewList(&DoList);
        InitCommand();
        InitCmdList();
        InitVar();
        InitDep();
        InitParser();
    }
}

void
help(code)
int code;
{
    puts("DMAKE V2.12 (c)Copyright 1991-2003 Matthew Dillon, All Rights Reserved");
    puts("DMAKE [-f file] [-n] [-Dvariable] [-d] [-a] [-q] [-h] [-j n]");
    exit(code);
}

const char *
SkipAss(ptr)
const char *ptr;
{
    while (*ptr && *ptr != '=')
        ++ptr;
    if (*ptr == '=') {
        for (++ptr; *ptr == ' ' || *ptr == '\t'; ++ptr)
            ;
    }
    return(ptr);
}


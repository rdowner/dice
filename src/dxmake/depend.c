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

Prototype void InitDep(void);
Prototype DepRef  *CreateDepRef(List *, char *);
Prototype DepCmdList *AllocDepCmdList(void);
Prototype DepRef  *DupDepRef(DepRef *);
Prototype void    IncorporateDependency(DepRef *, DepRef *, List *);
Prototype int     ExecuteDependency(DepNode *parent, DepRef *lhs, int how);

Prototype List DepList;

List DepList;

void
InitDep(void)
{
    NewList(&DepList);      /*  master list */
}

DepRef *
CreateDepRef(list, name)
List *list;
char *name;
{
    DepRef *ref;
    DepNode *dep;

    for (dep = GetTail(&DepList); dep; dep = GetPred(&dep->dn_Node)) {
        if (strcmp(name, dep->dn_Node.ln_Name) == 0)
            break;
    }
    if (dep == NULL) {
        dep = malloc(sizeof(DepNode) + strlen(name) + 1);
        bzero(dep, sizeof(DepNode));
        dep->dn_Node.ln_Name = (char *)(dep + 1);
        NewList(&dep->dn_DepCmdList);
        strcpy(dep->dn_Node.ln_Name, name);
        AddTail(&DepList, &dep->dn_Node);
    }

    ref = malloc(sizeof(DepRef));
    bzero(ref, sizeof(DepRef));

    ref->rn_Node.ln_Name = dep->dn_Node.ln_Name;
    ref->rn_Dep = dep;
    AddTail(list, &ref->rn_Node);
    return(ref);
}


DepRef *
DupDepRef(ref0)
DepRef *ref0;
{
    DepRef *ref = malloc(sizeof(DepRef));

    bzero(ref, sizeof(DepRef));
    ref->rn_Node.ln_Name = ref0->rn_Node.ln_Name;
    ref->rn_Dep = ref0->rn_Dep;
    return(ref);
}

void
IncorporateDependency(lhs, rhs, cmdList)
DepRef *lhs;
DepRef *rhs;
List *cmdList;
{
    DepNode *dep = lhs->rn_Dep;     /*  left hand side master */
    DepCmdList *depCmdList = GetHead(&dep->dn_DepCmdList);

    /* 
     * Attempt to match against existing lhs:rhs command lists
     */

    while (depCmdList) {
        if (depCmdList->dc_CmdList == cmdList)
            break;
        depCmdList = GetSucc(&depCmdList->dc_Node);
    }
    if (depCmdList == NULL) {
        depCmdList = malloc(sizeof(DepCmdList));
        bzero(depCmdList, sizeof(DepCmdList));
        NewList(&depCmdList->dc_RhsList);
        depCmdList->dc_CmdList = cmdList;
        AddTail(&dep->dn_DepCmdList, &depCmdList->dc_Node);
    }

    if (rhs)
        AddTail(&depCmdList->dc_RhsList, &rhs->rn_Node);

    db3printf(("Incorporate: %s -> %s\n", dep->dn_Node.ln_Name, (rhs) ? rhs->rn_Node.ln_Name : ""));

}

/*
 * ExecuteDependancy()
 *
 *  Execute a dependency.  Return the appropriate DN_ code.  We are passed
 *  (parent : lhs)  (lhs is one of possibly several right hand sides to 
 *  parent).  We must resolve 'lhs' by running through its own right hand
 *  sides and then aggregating the result into parent.
 *
 *  The dependancy 'parent : lhs' is executed.  We take lhs in the context
 *  of its own dependancies (which is why we call it lhs).
 */

int
ExecuteDependency(DepNode *parent, DepRef *lhs, int how)
{
    DepNode *lhsDep = lhs->rn_Dep;
    DepRef *rhsRef;
    DepCmdList *depCmdList;
    int index = 0;
    int parStRes;
    int lhsStRes;
    int runCmds = 0;
    struct stat parSt;
    struct stat lhsSt;
    int yr = DN_NOCHANGE;
    static int tab;

    /*
     * parStRes reflects the existance of the parent dependancy.
     * e.g. if lib depends on a.o, b.o, c.o, the parent is 'lib'
     * and we run through lhs->rn_Dep (a.o, b.o, c.o).
     */
    if (parent != NULL) {
        parStRes = stat(parent->dn_Node.ln_Name, &parSt);
    } else {
        parStRes = -1;
        bzero(&parSt, sizeof(parSt));
    }

    lhsStRes = stat(lhsDep->dn_Node.ln_Name, &lhsSt);

    /*
     *  If this lhs has no dependancies, compare the parent file against
     *  the file represented by this lhs to calculate the return value.
     */
    if (GetHead(&lhsDep->dn_DepCmdList) == NULL) {
        if (DoAll)
            return(DN_CHANGED);

        if (parent == NULL)     /* XXX */
            return(DN_CHANGED);

        if (lhsStRes < 0) {
            printf("The file %s could not be found\n", lhsDep->dn_Node.ln_Name);
            return(DN_FAILED);
        }
        if (parStRes < 0)
            return(DN_CHANGED);
        if ((int)parSt.st_mtime - (int)lhsSt.st_mtime > 0)
            return(DN_NOCHANGE);
        return(DN_CHANGED);
    }

    /*
     * Scan the right hand side's dependancies.  e.g. if
     * we are lib : a.o b.o c.o then this scan is testing
     * a.o : a.c,  b.o : b.c,  and c.o : c.c.
     */

    lhsDep->dn_Result = DN_NOCHANGE;
    lhsDep->dn_Node.ln_Type = NT_RESOLVED;
    lhsDep->dn_Flags |= DNF_VIRTUAL;

    for (depCmdList = GetHead(&lhsDep->dn_DepCmdList); 
        lhsDep->dn_Result > DN_FAILED && depCmdList; 
        depCmdList = GetSucc(&depCmdList->dc_Node)
    ) {
        int xr = DN_NOCHANGE;

        ++index;

        if (GetHead(depCmdList->dc_CmdList) != NULL)
            lhsDep->dn_Flags &= ~DNF_VIRTUAL;

        /*
         * Scan and run the rhs that our lhs depends on to determine 
         * whether the command list for our lhs must be run.
         *
         * Handle the special case where there are no rhs dependancies.
         * In this case we must return 0 to execute the command list.
         *
         * Handle the special case where the lhs does not exist.  In this
         * case we must return 0 to execute the command list.
         *
         * Handle the special case where an rhs dependancy is the same as
         * the lhs, causing us to test for file existance.
         */
        if (GetHead(&depCmdList->dc_RhsList) == NULL)
            xr = DN_CHANGED;

        for (rhsRef = GetHead(&depCmdList->dc_RhsList); 
            xr > DN_FAILED && rhsRef; 
            rhsRef = GetSucc(&rhsRef->rn_Node)
        ) {
            int r;

            if (lhsDep == rhsRef->rn_Dep) {
                /*
                 * file:file or dir:dir
                 */
                struct stat st;

                if (stat(lhsDep->dn_Node.ln_Name, &st) == 0)
                    r = DN_NOCHANGE;
                else
                    r = DN_CHANGED;
            } else {
                /*
                 * Run the dependancy
                 */
                tab += 4;
                r = ExecuteDependency(lhsDep, rhsRef, ED_WAIT);
                tab -= 4;
            }

            /*
             * If we won't trivially fail due to the rhs/lhs combo having to
             * have been run or the parent not existing anyway, and this
             * baby is not a virtual node (i.e. one that has no command list
             * and does not exist as a file), then we still have to see if
             * we are out of date relative to the rhs.  
             *
             * However, if the rhs is a directory we simply check for
             * existance.
             *
             * for example, lib depends on .o depends on .c.  If the .o is
             * resolved fine but the library is out of date, we have to
             * return a failure to generate the lib even though all our
             * .o->.c dependancies succeeded.  This is 'yr'.
             *
             * Additionally, if the lhs does not exist at all we need to
             * return a failure to generate the result (the else clause).
             * This can occur in the situation:
             *
             *          install: target1 target2
             *
             *          target1: objectX
             *
             *          target2: objectX
             *
             * If we do not handle the case where target2 does not exist we
             * will end up not running target2's command list due to target1
             * having caused objectX to resolve.
             */

            if (parStRes == 0 &&
                r >= DN_NOCHANGE_TOUCH &&
                (lhsDep->dn_Flags & DNF_VIRTUAL) == 0
            ) {
                struct stat st2;
                if (stat(lhsDep->dn_Node.ln_Name, &st2) == 0) {
                    if ((int)parSt.st_mtime - (int)st2.st_mtime < 0) {
                        if (!S_ISDIR(st2.st_mode))
                            yr = DN_CHANGED;
                    }
                } else {
                    yr = DN_CHANGED;
                }
            }

            /*
             * If the parent doesn't exist at all and it is not virtual, we
             * will want to return that it has changed.
             */
            if (parStRes < 0 && r > DN_CHANGED && 
                (parent == NULL || (parent->dn_Flags & DNF_VIRTUAL) == 0)
            ) {
                yr = DN_CHANGED;
            }

            if (xr > r)
                xr = r;

            dbprintf((
                "%*.*sRUNDEPEND lhs=\"%s\" rhs=\"%s\"\n"
                "%*.*s--------- r=%d cumulative=%d\n",
                tab, tab, "",
                lhsDep->dn_Node.ln_Name,
                rhsRef->rn_Node.ln_Name,
                tab, tab, "",
                r,
                xr
            ));
        }
        /*
         *  The DoAll flag forces the command list to be run
         */
        if (xr > DN_CHANGED && DoAll)
            xr = DN_CHANGED;

        /*
         *  If our result is 0 then something had to be run in the 
         *  subdependancies, so we have to run this dependency's
         *  command list.
         *
         *  [re]create %(left) and %(right) variables
         */
        if (xr == DN_CHANGED)
            runCmds = 1;
        if (lhsDep->dn_Result > xr)
            lhsDep->dn_Result = xr;
    }

    /*
     * If runCmds was set, do another run through and execute all the
     * related commands.
     */
    for (depCmdList = GetHead(&lhsDep->dn_DepCmdList); 
        runCmds && lhsDep->dn_Result > DN_FAILED && depCmdList; 
        depCmdList = GetSucc(&depCmdList->dc_Node)
    ) {
        DepRef  *rhsRef;
        int xr = DN_CHANGED;

        if (GetHead(depCmdList->dc_CmdList) != NULL) {
            Var *var;

            dbprintf(("%*.*sRUNCMDLIST \"%s\" index=%d\n", 
                tab, tab, "",
                lhsDep->dn_Node.ln_Name, index));

            if ((var = MakeVar("left", '%')) != NULL) {
                PutCmdListSym(&var->var_CmdList, lhsDep->dn_Node.ln_Name, NULL);
            }
            if ((var = MakeVar("right", '%')) != NULL) {
                short space = 0;

                for (
                    rhsRef = GetHead(&depCmdList->dc_RhsList); 
                    rhsRef; 
                    rhsRef = GetSucc(&rhsRef->rn_Node)
                ) {
                    PutCmdListSym(&var->var_CmdList, rhsRef->rn_Node.ln_Name, &space);
                }
            }
            SomeWork = 1;
            if (ExecuteCmdList(lhsDep, depCmdList->dc_CmdList) > EXIT_CONTINUE)
                xr = DN_FAILED;
        }
        if (lhsDep->dn_Result > xr)
            lhsDep->dn_Result = xr;
    }

    /*
     * If we ran commands and the left hand side result is marked as having
     * changed, and the left hand side represents a pre-existing file, 
     * check to see if the file has been updated.  Normally the file will
     * have been updated by the commands that were run but in certain
     * cases, such as when generating prototypes or dependancies, it is 
     * quite possible that no changes were made and the commands explicitly
     * did not rewrite the left hand side file because of that.
     */
    if (runCmds && lhsStRes == 0 && lhsDep->dn_Result == DN_CHANGED) {
        struct stat newSt;

        if (stat(lhsDep->dn_Node.ln_Name, &newSt) == 0
            && lhsSt.st_mtime == newSt.st_mtime
            && lhsSt.st_size == newSt.st_size 
#if defined(__FreeBSD__)
            && lhsSt.st_gen == newSt.st_gen
#endif
        ) {
            lhsDep->dn_Result = DN_NOCHANGE_TOUCH;
        }
    }

    /*
     * yr overrides the final result, indicating that our target is out of
     * date.
     */
    if (lhsDep->dn_Result > yr)
        lhsDep->dn_Result = yr;

    /*
     * If the result code is DN_NOCHANGE_TOUCH we have to touch the 
     * pre-existing left hand side file so the next dmake run does not
     * go through this whole mess again.  This will cause DN_NOCHANGE_TOUCH
     * to propogate so, for example, if a source module is changed but the
     * prototypes generation dependancy does not change the prototype file,
     * the prototype file, object modules, and library files will be touched
     * so the next dmake run doesn't have to regenerate the prototypes 
     * again.
     */
    if (lhsDep->dn_Result == DN_NOCHANGE_TOUCH) {
        int fd;

#if 0
        printf("TOUCHFILE %s\n", lhsDep->dn_Node.ln_Name);
#endif
        if ((fd = open(lhsDep->dn_Node.ln_Name, O_RDWR)) >= 0) {
            char c;
            if (read(fd, &c, 1) == 1) {
                lseek(fd, 0, SEEK_SET);
                write(fd, &c, 1);
            }
            close(fd);
        }
    }

    /*
     * If the parent does not exist as a file and we are not a virtual
     * dependancy, mark the parent as having changed.
     */
    yr = lhsDep->dn_Result;
    if (parStRes < 0 &&
        lhsDep->dn_Result > DN_FAILED &&
        (lhsDep->dn_Flags & DNF_VIRTUAL) == 0
    ) {
        yr = DN_CHANGED;
    }

    /*
     * Debugging
     */
    {
        const char *name = parent ? parent->dn_Node.ln_Name : "?";
        dbprintf(("%*.*sFINAL lhs=%s parStRes=%d(%s) r=%d\n",
            tab, tab, "", lhsDep->dn_Node.ln_Name, parStRes,
            name, lhsDep->dn_Result));
    }
    return(yr);
}


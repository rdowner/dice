
/*
 *  DEPEND.C
 */

#include "defs.h"

Prototype void InitDep();
Prototype DepRef  *CreateDepRef(List *, char *);
Prototype DepCmdList *AllocDepCmdList(void);
Prototype DepRef  *DupDepRef(DepRef *);
Prototype void    IncorporateDependency(DepRef *, DepRef *, List *);
Prototype int     ExecuteDependency(DepRef *, time_t *);

Prototype List DepList;

List DepList;

void
InitDep()
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
        clrmem(dep, sizeof(DepNode));
        dep->dn_Node.ln_Name = (char *)(dep + 1);
        NewList(&dep->dn_DepCmdList);
        strcpy(dep->dn_Node.ln_Name, name);
        AddTail(&DepList, &dep->dn_Node);
    }

    ref = malloc(sizeof(DepRef));
    clrmem(ref, sizeof(DepRef));

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

    clrmem(ref, sizeof(DepRef));
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
    DepNode *dep = lhs->rn_Dep;     /*  source master */
    DepCmdList *depCmdList = GetHead(&dep->dn_DepCmdList);

    if (depCmdList == NULL || depCmdList->dc_CmdList != cmdList) {
        depCmdList = malloc(sizeof(DepCmdList));
        clrmem(depCmdList, sizeof(DepCmdList));
        NewList(&depCmdList->dc_RhsList);
        depCmdList->dc_CmdList = cmdList;
        AddHead(&dep->dn_DepCmdList, &depCmdList->dc_Node);
    }

    if (rhs)
        AddTail(&depCmdList->dc_RhsList, &rhs->rn_Node);

    db3printf(("Incorporate: %s -> %s\n", dep->dn_Node.ln_Name, (rhs) ? rhs->rn_Node.ln_Name : ""));

}

/*
 *  Execute dependency returning an error and time completion code (time
 *  completion code is 0 if object does not exit or had to be 'run')
 */

int
ExecuteDependency(ref, pt)
DepRef *ref;
time_t *pt;
{
    DepNode *dep = ref->rn_Dep;
    int r = 0;

    dbprintf(("Go %s:\n", dep->dn_Node.ln_Name));

    if (dep->dn_Node.ln_Type != NT_RESOLVED) {
        DepCmdList *depCmdList;
        short statok = 0;
        short masterForce = 0;
        struct stat sbuf;

        dep->dn_Node.ln_Type = NT_RESOLVED;

        /*
         *  sub-dependency groups are handled individually
         */

        if (stat(dep->dn_Node.ln_Name, &sbuf) == 0) {
            statok = 1;
            dep->dn_Time = sbuf.st_mtime;
        }

        for (depCmdList = GetHead(&dep->dn_DepCmdList); r == 0 && depCmdList; depCmdList = GetSucc(&depCmdList->dc_Node)) {
            short force;

            /*
             *  A lower level dependency that gets hit but has no command
             *  list will force the next higher level dependency to get
             *  hit.
             */

            if (masterForce == 2) {
                masterForce = 1;
                force = 1;
            } else {
                force = DoAll;
            }

            *pt = 0;

            for (ref = GetHead(&depCmdList->dc_RhsList); r == 0 && ref; ref = GetSucc(&ref->rn_Node)) {
                time_t t;

                if ((r = ExecuteDependency(ref, &t)) < 0)
                    break;
                if (t == 0)
                    force = 1;
                if (*pt == 0 || (long)(t - *pt) > 0)
                    *pt = t;        /*  latest  */
                dbprintf(("LHS %s stok=%d rhs=%s time=%08lx\n",
                        dep->dn_Node.ln_Name,
                        statok,
                        ref->rn_Node.ln_Name,
                        t
                    ));
            }
            if (r == 0) {
                if (statok) {
                    /*
                     *  if the file exists check the time agains the
                     *  collected sub-dependency/  If the sub-dep is
                     *  newer we force
                     *
                     *  if *pt is NULL we check to see if there were any
                     *  sub-dependancies or commands.  If so we force, other
                     *  wise we load *pt with the file time
                     */

                    if (*pt) {
                        if ((long)(*pt - sbuf.st_mtime) > 0)
                            force = 1;
                    } else if (GetHead(&depCmdList->dc_RhsList) || GetHead(depCmdList->dc_CmdList)) {
                        force = 1;
                    } else {
                        *pt = sbuf.st_mtime;

                    }
                } else {
                    /*
                     *
                     */
                    /* Who knows why this code is here.  Matt seems to remember */
                    /* a bug that had existed at one time.  This probably used  */
                    /* to say something different.                              */
                    /* if (GetHead(&depCmdList->dc_RhsList)) { ********DEAD******/
                        force = 1;
                    /* } else {                                ********DEAD******/
                    /* force = 1;                              ********DEAD******/
                    /* }                                       ********DEAD******/
                }

                dbprintf(("LHS %s stok=%d time=%08lx force= %d\n",
                        dep->dn_Node.ln_Name,
                        statok,
                        *pt,
                        force
                    ));

                /*
                 *  run command list if necessary.  If run then force result
                 *  to caller by setting *pt to 0
                 *
                 *  [re]create %(left) and %(right) variables
                 */

                if (force) {
                    dbprintf(("FORCE %s\n", dep->dn_Node.ln_Name));

                    masterForce = 1;
                    if (GetHead(depCmdList->dc_CmdList)) {
                        Var *var;

                        if (var = MakeVar("left", '%')) {
                            PutCmdListSym(&var->var_CmdList, dep->dn_Node.ln_Name, NULL);
                        }
                        if (var = MakeVar("right", '%')) {
                            short space = 0;

                            for (ref = GetHead(&depCmdList->dc_RhsList); r == 0 && ref; ref = GetSucc(&ref->rn_Node))
                                PutCmdListSym(&var->var_CmdList, ref->rn_Node.ln_Name, &space);
                        }
                        SomeWork = 1;
                        if (ExecuteCmdList(dep, depCmdList->dc_CmdList) > 5) {
                            r = -1;
                        }
                    } else {
                        masterForce = 2;
                    }
                }
                if (dep->dn_Time == 0 || (long)(*pt -  dep->dn_Time) > 0)
                    dep->dn_Time = *pt;
            }
        }
        if (GetHead(&dep->dn_DepCmdList) == NULL) {
            if (statok)
                *pt = sbuf.st_mtime;
        }
        if (masterForce)
            *pt = 0;
        dep->dn_Time = *pt;
        dbprintf(("FINAL %s statok=%d time=%08lx\n", dep->dn_Node.ln_Name, statok, *pt));
    } else {
        *pt = dep->dn_Time;
        dbprintf(("DONE-ALREADY %s time=%08lx\n", dep->dn_Node.ln_Name, *pt));
    }
    return(r);
}


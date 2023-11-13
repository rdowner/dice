/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PRAGMA.C
 */

/*
**      $Filename: pragma.c $
**      $Author: dice $
**      $Revision: 30.0 $
**      $Date: 1994/06/10 18:04:52 $
**      $Log: pragma.c,v $
 * Revision 30.0  1994/06/10  18:04:52  dice
 * .
 *
 * Revision 1.2  1993/09/06  09:56:47  jtoebes
 * Fixed BUG65021 - #pragma libcall required 00 instead of just 0
 *
**/

#include "defs.h"

Prototype void ProcessLibCallPragma(char *, short);
Prototype char *plcsym(char *, char **, short *);
Prototype PragNode *TestPragmaCall(Var *, char *);

#define HSIZE   256
#define HMASK   (HSIZE-1)


PragNode *PragHash[HSIZE];

/*
 *  DOSBase Open 1E 2102
 *
 *  Tag existing prototype with __libcall and store pragma in separate
 *  semantic symbol table.  On procedure call this table will be searched
 *  and the pragma decoded at that time
 */

#ifdef REGISTERED

void
ProcessLibCallPragma(char *buf, short syscall)
{
    PragNode *pn = zalloc(sizeof(PragNode));

    {
        char *ptr = buf;

        if (syscall) {
            pn->pn_Sym = PragmaSymbol("SysBase", 7);
        } else {
            short len;
            char *name;

            ptr = plcsym(ptr, &name, &len);
            pn->pn_Sym = PragmaSymbol(name, len);
        }
        ptr = plcsym(ptr, &pn->pn_Func, &pn->pn_FuncLen);
        ptr = plcsym(ptr, &pn->pn_Off, &pn->pn_OffLen);
        ptr = plcsym(ptr, &pn->pn_Ctl, &pn->pn_CtlLen);

        /*

        if (pn->pn_Sym->Data == NULL) {
            Exp *exp = zalloc(sizeof(Exp) + sizeof(Stor));
            pn->pn_Sym->Data = exp;
            exp->ex_Func = FuncPragExp;
            exp->ex_Type = &VoidPtrType;
            AllocExternalStorage(pn->pn_Sym, (Stor *)(exp + 1), &VoidPtrType, (SmallData) ? TF_NEAR : TF_FAR);
            printf("\txref\t_%.*s\n", pn->pn_Sym->Len, pn->pn_Sym->Name);
        }

        */
    }
    {
        PragNode **ppn = PragHash + (hash(pn->pn_Func, pn->pn_FuncLen) & HMASK);

        pn->pn_Next = *ppn;
        *ppn = pn;
    }
}

char *
plcsym(ptr, pbase, plen)
char *ptr;
char **pbase;
short *plen;
{
    while (*ptr == ' ' || *ptr == '\t')
        ++ptr;
    *pbase = ptr;
    while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr)
        ++ptr;
    *plen = ptr - *pbase;
    return(ptr);
}

#endif

PragNode *
TestPragmaCall(var, prgno)
Var *var;       /*  variable (procedure name) to test..          */
char *prgno;    /*  array of 16 chars to be filled if 1 returned */
{
    PragNode *pn;

    {
        Symbol *sym = var->Sym;
        for (pn = PragHash[hash(sym->Name, sym->Len) & HMASK]; pn; pn = pn->pn_Next) {
            if (sym->Len == pn->pn_FuncLen && cmpmem(sym->Name, pn->pn_Func, sym->Len) == 0)
                break;
        }
    }
    if (pn) {
        short i;
        short j;
        short narg = strtol(pn->pn_Ctl + pn->pn_CtlLen - 1, NULL, 16);

        if (pn->pn_Ctl[pn->pn_CtlLen-2] != '0')
        {
            if (pn->pn_Ctl[pn->pn_CtlLen-2] != ' ')
                yerror(var->LexIdx, EFATAL_PRAGMA_RETURN_D0, pn->pn_FuncLen, pn->pn_Func);
            pn->pn_Ctl[pn->pn_CtlLen-2] = '0';
        }

        if (var->Type->Args != narg)
            yerror(var->LexIdx, EFATAL_PRAGMA_CONFLICT_ARGS, pn->pn_FuncLen, pn->pn_Func);

        /*printf(";narg = %d\n", narg);*/
        if (narg > 15)
            narg = 15;

        if (pn->pn_Offset == 0)
            pn->pn_Offset = strtol(pn->pn_Off, NULL, 16);

        /*ptr = pn->pn_Ctl;*/
        for (i = 0, j = pn->pn_CtlLen - 3; i < narg; ++i, --j) {
            short regNo = (j >= 0) ? pn->pn_Ctl[j] : '0';

            switch(regNo) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                regNo -= '0';
                break;
            case '8':
            case '9':
                regNo = regNo - '8' + 16;
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                regNo = regNo - 'a' + 18;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                regNo = regNo - 'A' + 18;
                break;
            default:
                regNo = 0;
            }
            prgno[i] = regNo;
            /*printf(";argno %d regno %d\n", i, regNo);*/
        }
        prgno[i++] = RB_A6;

        while (i < 16) {
            prgno[i] = -1;
            ++i;
        }

        /*
         *  If our special pragma exp node has not been appended,
         *  append one.
         */

        return(pn);
    } else {
        return(NULL);
    }
}


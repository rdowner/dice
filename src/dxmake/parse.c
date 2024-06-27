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

Prototype void InitParser(void);
Prototype void ParseFile(char *);
Prototype token_t ParseAssignment(char *, token_t, int, char);
Prototype token_t ParseDependency(char *, token_t);
Prototype token_t GetElement(int ifTrue, int *expansion);
Prototype void    ParseVariable(List *, short);
Prototype char   *ParseVariableBuf(List *, ubyte *, short);
Prototype char   *ExpandVariable(ubyte *, List *);
Prototype token_t GetToken(void);
Prototype void expect(token_t, token_t);
Prototype void error(short, const char *, ...);


Prototype char SymBuf[256];
Prototype long LineNo;

char SpecialChar[256];
char SChars[] = { ":=()\n\\\" \t\r\014" };
char SymBuf[256];
char AltBuf[256];
char AltBuf2[256];
long LineNo = 1;
char *FileName = "";
FILE *Fi;
List CCList;

#ifdef unix
static const char *ccmd(int cno, const char *cmd, const char *arg1);
#endif

void
InitParser()
{
    short i;
    for (i = 0; SChars[i]; ++i)
        SpecialChar[(ubyte)SChars[i]] = 1;
    NewList(&CCList);
}

/*
 *  Parse lines as follows:
 *
 *  symbol = ...
 *  symbol ... : symbol ...
 *      (commands, begin with tab or space)
 *      (blank line)
 *
 *
 */

void
ParseFile(fileName)
char *fileName;
{
    FILE *fi;
    token_t t;
    IfNode *ifBase = NULL;
    int ifTrue = 1;
    int expansion;
    List topdirList;

    NewList(&topdirList);

    /*
     * Open the file
     */
    {
        Var *var = FindVar("TOPDIR", '$');
        char *tfileName;
        int len;
        List list;

        NewList(&list);
        CopyCmdList(&var->var_CmdList, &list);
        CopyCmdList(&var->var_CmdList, &topdirList);
        len = CmdListSize(&list);
        tfileName = malloc(len + strlen(fileName) + 1);
        CopyCmdListBuf(&list, tfileName);
        strcpy(tfileName + len, fileName);

        if ((fi = fopen(tfileName, "r")) == NULL)
            error(FATAL, "Unable to open %s", tfileName);
        free(tfileName);
    }
    FileName = strdup(fileName);
    Fi = fi;
    LineNo = 1;

    /*
     * Adjust TOPDIR based on include path
     */
    {
        const char *p;

        if ((p = strrchr(fileName, '/')) != NULL) {
                Var *var = FindVar("TOPDIR", '$');
                AppendVar(var, fileName, p - fileName + 1);
        }
    }

    /*
     * Parse the file
     */
    for (t = GetElement(ifTrue, &expansion); t; ) {
        switch(t) {
        case TokNewLine:
            t = GetElement(ifTrue, &expansion);
            break;
        case TokSym:
            strcpy(AltBuf2, SymBuf);

            if (expansion == 0 && SymBuf[0] == '.' && 
                SymBuf[1] != '.' && SymBuf[1] != '/'
            ) {
                if (ifTrue && strcmp(SymBuf, ".export") == 0) {
                    char *data;
                    Var *var;

                    while ((t = GetElement(ifTrue, &expansion)) != TokNewLine) {
                        int maxl;

                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .export!");

                        if ((var = FindVar(SymBuf, '$')) == NULL) {
                            error(
                                FATAL, 
                                "export %s failed, variable not found", 
                                SymBuf
                            );
                        }
                        {
#if USE_PUTENV
                            char *assBuf;
#endif
                            static List CmdList = { 
                                (Node *)&CmdList.lh_Tail,
                                NULL, 
                                (Node *)&CmdList.lh_Head
                            };
                            CopyCmdList(&var->var_CmdList, &CmdList);
                            maxl = CmdListSize(&CmdList) + 1;
                            data = malloc(maxl);
                            CopyCmdListBuf(&CmdList, data);
#if USE_PUTENV
                            assBuf = malloc(strlen(SymBuf) + strlen(data) + 2);
                            sprintf(assBuf, "%s=%s", SymBuf, data);
                            putenv(assBuf);
                            free(assBuf);
#else
                            setenv(SymBuf, data, 1);
#endif
                            free(data);
                        }
                    }
                } else if (ifTrue && strcmp(SymBuf, ".include") == 0) {
                    FILE *saveFi = Fi;
                    char *saveFileName = FileName;
                    int saveLine = LineNo;
                    char *path;

                    t = GetElement(ifTrue, &expansion);
                    if (t != TokSym)
                        error(FATAL, "Expected a symbol for .include!");
                    path = strdup(SymBuf);
                    ParseFile(path);
                    free(path);

                    LineNo = saveLine;
                    Fi = saveFi;
                    FileName = saveFileName;

                    t = GetElement(ifTrue, &expansion);
                    if (t != TokNewLine)
                        error(FATAL, "Expected newline after .include filename");
                } else if (strcmp(SymBuf, ".else") == 0) {
                    if (ifBase == NULL)
                        error(FATAL, ".else without .if*");
                    ifTrue = elseIf(&ifBase);
                } else if (strcmp(SymBuf, ".ifuser") == 0) {
#ifndef unix
                    error(FATAL, ".ifuser directive not available on non-UNIX-like ports of dxmake");
#else
                    if (ifTrue) {
                        struct passwd *pw;

                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .ifuser!");
                        if ((pw = getpwuid(getuid())) == NULL)
                            error(FATAL, "getpwuid(getuid()) failed");
                        if (strcmp(SymBuf, pw->pw_name) == 0)
                            ifTrue = pushIf(&ifBase, 1);
                        else
                            ifTrue = pushIf(&ifBase, 0);
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
#endif
                } else if (strcmp(SymBuf, ".ifdef") == 0) {
                    if (ifTrue) {
                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .ifdef!");
                        if (FindVar(SymBuf, '$')) {
                            ifTrue = pushIf(&ifBase, 1);
                        } else {
                            ifTrue = pushIf(&ifBase, 0);
                        }
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
                } else if (strcmp(SymBuf, ".ifhost") == 0) {
#ifndef unix
                    error(FATAL, ".ifhost directive not available on non-UNIX-like ports of dxmake");
#else
                    if (ifTrue) {
                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .ifhost!");
                        if (strcasecmp(SymBuf, ccmd(1, "hostname", NULL)) == 0) {
                            ifTrue = pushIf(&ifBase, 1);
                        } else {
                            ifTrue = pushIf(&ifBase, 0);
                        }
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
#endif
                } else if (strcmp(SymBuf, ".ifos") == 0) {
                    if (ifTrue) {
                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .ifos!");
#ifdef unix
                        const char *osName = ccmd(2, "uname", "-s");
#else
                        const char *osName = "AmigaOS";
#endif
                        if (strcasecmp(SymBuf, osName) == 0) {
                            ifTrue = pushIf(&ifBase, 1);
                        } else {
                            ifTrue = pushIf(&ifBase, 0);
                        }
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
                } else if (strcmp(SymBuf, ".ifarch") == 0) {
                    if (ifTrue) {
                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .ifarch!");
#ifdef unix
                        const char *arch = ccmd(2, "uname", "-m");
#else
                        const char *arch = "m68k";
#endif
                        if (strcasecmp(SymBuf, arch) == 0) {
                            ifTrue = pushIf(&ifBase, 1);
                        } else {
                            ifTrue = pushIf(&ifBase, 0);
                        }
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
                } else if (strcmp(SymBuf, ".iffile") == 0) {
                    if (ifTrue) {
                        struct stat st;

                        t = GetElement(ifTrue, &expansion);
                        if (t != TokSym)
                            error(FATAL, "Expected a symbol for .iffile!");
                        if (stat(SymBuf, &st) == 0) {
                            ifTrue = pushIf(&ifBase, 1);
                        } else {
                            ifTrue = pushIf(&ifBase, 0);
                        }
                    } else {
                        ifTrue = pushIf(&ifBase, 0);
                    }
                } else if (strcmp(SymBuf, ".endif") == 0) {
                    if (ifBase == NULL)
                        error(FATAL, ".endif without .if");
                    ifTrue = popIf(&ifBase);
                } else if (ifTrue) {
                    error(FATAL, "unknown '.' directive");
                }

                /*
                 * End of special directive handling
                 */
                while (t && t != TokNewLine)
                    t = GetElement(ifTrue, &expansion);
                continue;
            }

            /*
             * Ignore .if'd out code
             */
            if (ifTrue == 0) {
                while (t && t != TokNewLine)
                    t = GetElement(ifTrue, &expansion);
                continue;
            }

            /*
             *  check for '=' -- assignment
             */
            t = GetElement(ifTrue, &expansion);
            if (t == TokQuestion) {
                t = GetElement(ifTrue, &expansion);
                if (t == TokEq)
                    t = ParseAssignment(AltBuf2, t, 1, '$');
                else
                    error(FATAL, "Expected '?=' got '?'");
            } else if (t == TokEq) {
                t = ParseAssignment(AltBuf2, t, 0, '$');
            } else {
                t = ParseDependency(AltBuf2, t);
            }
            break;
        case TokColon:
            /*
             * Ignore .if'd out code
             */
            if (ifTrue == 0) {
                while (t && t != TokNewLine)
                    t = GetElement(ifTrue, &expansion);
                continue;
            }
            t = ParseDependency(NULL, t);
            break;
        default:
            /*
             * Ignore .if'd out code
             */
            if (ifTrue == 0) {
                while (t && t != TokNewLine)
                    t = GetElement(ifTrue, &expansion);
                continue;
            }
            error(FATAL, "Expected a symbol!");
            break;
        }
    }
    if (ifBase != NULL)
        error(FATAL, "Dangling .if's at EOF");

    /*
     * Restore TOPDIR
     */
    {
        Var *var = FindVar("TOPDIR", '$');
        FreeCmdList(&var->var_CmdList);
        AppendCmdList(&topdirList, &var->var_CmdList);
    }
}

/*
 *  Parse an asignment.  Parsed as-is (late eval)
 *
 *  t contains TokEq, ignore
 */

token_t
ParseAssignment(char *varName, token_t t, int cond, char type)
{
    Var *var;
    int newVar = 0;
    long len;
    short done;
    short eol = 1;
    List tmpList;

    if (cond == 0 || FindVar(varName, type) == NULL) {
        newVar = 1;
    }

    NewList(&tmpList);

    while (fgets(AltBuf, sizeof(AltBuf), Fi)) {
        len = strlen(AltBuf);

        if (eol && AltBuf[0] == '#') {
            ++LineNo;
            continue;
        }
        if (len && AltBuf[len-1] == '\n') {
            ++LineNo;
            --len;
            if (len && AltBuf[len-1] == '\\') {
                --len;
                done = 0;
            } 
            else {
                done = 1;
            }
            eol = 1;
        } 
        else {
            done = 0;
            eol = 0;
        }

        AltBuf[len] = 0;
        {
            long i;

            for (i = 0; i < len && (AltBuf[i] == ' ' || AltBuf[i] == '\t'); ++i)
                ;
            if(i && ((AltBuf[i-1] == '\t') || (AltBuf[i] == ' ')))--i;
            for (     ; i < len; ++i)
                PutCmdListChar(&tmpList, AltBuf[i]);

        }
        if (done > 0)break;
    }

    /*
     *  Now, load temp list into buffer and expand into the variable
     */

    {
        char *buf = malloc(CmdListSize(&tmpList) + 1);
        CopyCmdListBuf(&tmpList, buf);
        if (newVar) {
            ExpandVariable((ubyte *)buf, &tmpList);
            var = MakeVar(varName, type);
            AppendCmdList(&tmpList, &var->var_CmdList);
        }
        free(buf);
    }
    return(GetElement(1, NULL));
}

/*
 *  Parse a dependency
 */

token_t
ParseDependency(char *firstSym, token_t t)
{
    DepRef  *lhs;
    DepRef  *rhs;
    List    lhsList;
    List    rhsList;
    List    *cmdList = malloc(sizeof(List));
    int    nlhs = 0;
    int    nrhs = 0;
    short   ncol = 0;

    NewList(cmdList);
    NewList(&lhsList);
    NewList(&rhsList);

    if (firstSym) {
        ++nlhs;
        CreateDepRef(&lhsList, firstSym);
    }

    while (t != TokColon) {
        expect(t, TokSym);
        CreateDepRef(&lhsList, SymBuf);
        ++nlhs;
        t = GetElement(1, NULL);
    }
    t = GetElement(1, NULL);
    if (t == TokColon) {
        ++ncol;
        t = GetElement(1, NULL);
    }

    while (t != TokNewLine) {
        expect(t, TokSym);
        CreateDepRef(&rhsList, SymBuf);
        ++nrhs;
        t = GetElement(1, NULL);
    }

    /*
     *  parse command list
     */

    {
        short c;
        short blankLine = 1;
        short ws = 0;           /*  white space skip    */

        while ((c = getc(Fi)) != EOF) {
            if (c == '\n') {
                ++LineNo;
                if (blankLine)
                    break;
                PutCmdListChar(cmdList, '\n');
                blankLine = 1;
                ws = 0;
                continue;
            }
            if (c == '.' && blankLine && ws == 0) {
                ungetc(c, Fi);
                break;
            }

            switch(c) {
            case ' ':
            case '\t':
                if (blankLine) {    /*  remove all but one ws after nl */
                    ws = 1;
                    continue;
                }
                PutCmdListChar(cmdList, c);
                break;
            case '\\':
                if (ws) {
                    PutCmdListChar(cmdList, ' ');
                    ws = 0;
                }
                c = getc(Fi);
                if (c == '\n') {
                    blankLine = 1;
                    ++LineNo;
                    continue;
                }
                PutCmdListChar(cmdList, '\\');
                PutCmdListChar(cmdList, c);
                break;
            default:
                if (ws) {
                    PutCmdListChar(cmdList, ' ');
                    ws = 0;
                }
                PutCmdListChar(cmdList, c);
                break;
            }
            blankLine = 0;
        }
    }

    /*
     *  formats allowed:
     *
     *      X :: Y      each item depends on all items (X x Y dependancies)
     *      1 : N       item depends on items
     *      N : N       1:1 map item to item
     *      N : 1       items depend on item
     */

    if (ncol == 1) {
        while ((lhs = RemHead(&lhsList)) != NULL) {
            if (GetHead(&lhsList)) {
                for (rhs = GetHead(&rhsList); rhs; rhs = GetSucc(&rhs->rn_Node))
                    IncorporateDependency(lhs, DupDepRef(rhs), cmdList);
            } else {
                while ((rhs = RemHead(&rhsList)) != NULL)
                    IncorporateDependency(lhs, rhs, cmdList);
            }
            IncorporateDependency(lhs, NULL, cmdList);
            free(lhs);
        }
    } else if (nlhs == 1) {
        lhs = RemHead(&lhsList);
        while ((rhs = RemHead(&rhsList)) != NULL)
            IncorporateDependency(lhs, rhs, cmdList);
        IncorporateDependency(lhs, NULL, cmdList);
        free(lhs);
    } else if (nrhs == 1) {
        rhs = RemHead(&rhsList);
        while ((lhs = RemHead(&lhsList)) != NULL) {
            IncorporateDependency(lhs, rhs, cmdList);
            free(lhs);
        }
    } else if (nlhs == nrhs) {
        while ((lhs = RemHead(&lhsList)) && (rhs = RemHead(&rhsList))) {
            IncorporateDependency(lhs, rhs, cmdList);
            free(lhs);
        }
    } else {
        error(FATAL, "%d items on the left, %d on the right of colon!", nlhs, nrhs);
    }
    return(t);
}

/*
 *  GetElement()    - return a token after variable/replace parsing
 */

token_t
GetElement(int ifTrue, int *expansion)
{
    static List CmdList = { (Node *)&CmdList.lh_Tail, NULL, (Node *)&CmdList.lh_Head };
    token_t t;
    short c;

top:
    if (PopCmdListSym(&CmdList, SymBuf, sizeof(SymBuf)) == 0) {
        if (expansion)
            *expansion = 1;
        return(TokSym);
    }
    if (expansion)
        *expansion = 0;

    t = GetToken();
swi:
    switch(t) {
    case TokDollar:
    case TokPercent:
        c = fgetc(Fi);
        if (c == '(' && ifTrue) {
            ParseVariable(&CmdList, (t == TokPercent) ? '%' : '$');

            /*
             *  XXX how to handle dependancies verses nominal string concat?
             */

            while ((c = fgetc(Fi)) != ' ' && c != '\t' && c != '\n' && c != ':') {
                if (c == EOF)
                    break;
                if (c == '$') {
                    t = TokDollar;
                    goto swi;
                }
                if (c == '%') {
                    t = TokPercent;
                    goto swi;
                }
                PutCmdListChar(&CmdList, c);
            }
            if (c != EOF)
                ungetc(c, Fi);
            goto top;
        }
        ungetc(c, Fi);
        /* fall through */
    default:
        break;
    }
    return(t);
}

/*
 *  ParseVariable() - parse a variable reference, expanding it into a
 *  command list.  Fi begins at the first character in the variable name
 *
 *  $(NAME)
 *  $(NAME:"from":"to")
 *
 */

void
ParseVariable(List *cmdList, short c0)
{
    short c;
    short i = 0;
    Var *var;

    /*
     *  variable name
     */

    while ((c = getc(Fi)) != EOF && !SpecialChar[c])
        AltBuf[i++] = c;
    AltBuf[i] = 0;

    var = FindVar(AltBuf, c0);
    if (var == NULL)
        error(FATAL, "Variable %s does not exist", AltBuf);

    /*
     *  now, handle modifiers
     */

    if (c == ')') {
        CopyCmdList(&var->var_CmdList, cmdList);
        return;
    }
    if (c != ':')
        error(FATAL, "Bad variable specification after name");

    /*
     *  source operation
     */

    c = fgetc(Fi);
    if (c == '\"') {
        ungetc(c, Fi);
        expect(GetToken(), TokStr);
        c = fgetc(Fi);
    } else {
        i = 0;
        while (c != ')' && c != ':' && c != EOF) {
            SymBuf[i++] = c;
            c = fgetc(Fi);
        }
        SymBuf[i] = 0;
    }

    strcpy(AltBuf, SymBuf);

    /*
     *  destination operation
     */

    if (c == ')') {
        CopyCmdListConvert(&var->var_CmdList, cmdList, AltBuf, AltBuf);
        return;
    }

    if (c != ':')
        error(FATAL, "Bad variable replacement spec: %c", c);

    c = fgetc(Fi);
    if (c == '\"') {
        ungetc(c, Fi);
        expect(GetToken(), TokStr);
        c = fgetc(Fi);
    } else {
        i = 0;
        while (c != ')' && c != ':' && c != EOF) {
            SymBuf[i++] = c;
            c = fgetc(Fi);
        }
        SymBuf[i] = 0;
    }

    if (c != ')')
        error(FATAL, "Bad variable replacement spec: %c", c);

    CopyCmdListConvert(&var->var_CmdList, cmdList, AltBuf, SymBuf);
}

/*
 *  Since this is recursively called we have to save/restore oru temporary
 *  bufferse (SymBuf & AltBuf).  the buf pointer may itself be pointing
 *  into these but we are ok since it is guarenteed >= our copy destination
 *  as we index through it.
 */


char *
ParseVariableBuf(List *cmdList, ubyte *buf, short c0)
{
    short c;
    short i = 0;
    Var *var;
    char *symBuf = AllocPathBuffer();
    char *altBuf = AllocPathBuffer();

    /*
     *  variable name
     */

    while ((c = *buf++) && !SpecialChar[c])
        altBuf[i++] = c;
    altBuf[i] = 0;

    var = FindVar(altBuf, c0);
    if (var == NULL)
        error(FATAL, "Variable %s does not exist", altBuf);

    /*
     *  now, handle modifiers
     */

    if (c == ')') {
        CopyCmdList(&var->var_CmdList, cmdList);
        FreePathBuffer(symBuf);
        FreePathBuffer(altBuf);
        return((char *)buf);
    }
    if (c != ':')
        error(FATAL, "Bad variable specification after name");

    /*
     *  source operation
     */

    c = *buf++;

    if (c == '\"') {
        i = 0;
        while ((c = *buf++) && c != '\"')
            symBuf[i++] = c;
        if (c == '\"')
            c = *buf++;
    } else {
        i = 0;
        while (c && c != ')' && c != ':') {
            symBuf[i++] = c;
            c = *buf++;
        }
    }

    symBuf[i] = 0;
    strcpy(altBuf, symBuf);

    /*
     *  destination operation
     */

    if (c == ')') {
        CopyCmdListConvert(&var->var_CmdList, cmdList, altBuf, symBuf);
        FreePathBuffer(symBuf);
        FreePathBuffer(altBuf);
        return((char *)buf);
    }

    if (c != ':')
        error(FATAL, "Bad variable replacement spec: %c", c);

    c = *buf++;

    if (c == '\"') {
        i = 0;
        while ((c = *buf++) && c != '\"')
            symBuf[i++] = c;
        if (c == '\"')
            c = *buf++;
    } else {
        i = 0;
        while (c && c != ')' && c != ':') {
            symBuf[i++] = c;
            c = *buf++;
        }
    }
    symBuf[i] = 0;

    if (c != ')')
        error(FATAL, "Bad variable replacement spec: %c", c);

    CopyCmdListConvert(&var->var_CmdList, cmdList, altBuf, symBuf);
    FreePathBuffer(symBuf);
    FreePathBuffer(altBuf);
    return((char *)buf);
}

char *
ExpandVariable(buf, list)
ubyte *buf;
List *list;
{
    short c;
    short n = 0;
    short tmpListValid;
    short keepInList;
    List tmpList;
    static int Levels;

    if (++Levels == 20)
        error(FATAL, "Too many levels of variable recursion");

    if (list) {
        keepInList = 1;
        tmpListValid = 1;
    } else {
        keepInList = 0;
        tmpListValid = 0;
        list = &tmpList;
        NewList(list);
    }

    while ((c = buf[n]) != 0) {
        if (c == '$' || c == '%') {
            if (buf[n+1] == '(') {
                if (tmpListValid == 0) {
                    int i;

                    for (i = 0; i < n; ++i)
                        PutCmdListChar(list, buf[i]);
                    tmpListValid = 1;
                }
                n = (ubyte *)ParseVariableBuf(list, buf + n + 2, c) - buf;
            } else if (buf[n+1] == c) {
                if (tmpListValid)
                    PutCmdListChar(list, c);
                n += 2;
            } else {
                if (tmpListValid)
                    PutCmdListChar(list, c);
                ++n;
            }
        } else {
            if (tmpListValid)
                PutCmdListChar(list, c);
            ++n;
        }
    }
    if (keepInList == 0) {
        if (tmpListValid) {
            buf = malloc(CmdListSize(list) + 1);
            CopyCmdListBuf(list, (char *)buf);
        }
    }
    --Levels;
    return((char *)buf);
}


#ifdef NOTDEF

    short c;
    short i;
    Var *var;

    /*
     *  variable name
     */

    while (c = *buf++ && !SpecialChar[c])
        AltBuf[i++] = c;

    AltBuf[i] = 0;

    var = FindVar(AltBuf, c0);
    if (var == NULL)
        error(FATAL, "Variable %s does not exist", AltBuf);

    /*
     *  now, handle modifiers
     */

    if (c == ')') {
        CopyCmdList(&var->var_CmdList, cmdList);
        return;
    }
    if (c != ':')
        error(FATAL, "Bad variable specification after name");

    /*
     *  source operation
     */

    c = fgetc(Fi);
    if (c == '\"') {
        ungetc(c, Fi);
        expect(GetToken(), TokStr);
        c = fgetc(Fi);
    } else {
        i = 0;
        while (c != ')' && c != ':' && c != EOF) {
            SymBuf[i++] = c;
            c = fgetc(Fi);
        }
    }
    SymBuf[i] = 0;

    strcpy(AltBuf, SymBuf);

    /*
     *  destination operation
     */

    if (c == ')') {
        CopyCmdListConvert(&var->var_CmdList, cmdList, AltBuf, SymBuf);
        return;
    }

    if (c != ':')
        error(FATAL, "Bad variable replacement spec: %c", c);


    c = fgetc(Fi);
    if (c == '\"') {
        ungetc(c, Fi);
        expect(GetToken(), TokStr);
        c = fgetc(Fi);
    } else {
        i = 0;
        while (c != ')' && c != ':' && c != EOF) {
            SymBuf[i++] = c;
            c = fgetc(Fi);
        }
    }

    if (c != ')')
        error(FATAL, "Bad variable replacement spec: %c", c);

    CopyCmdListConvert(&var->var_CmdList, cmdList, AltBuf, SymBuf);
}

#endif


/*
 *  GetToken()  - return a single token
 */

/*
token_t
GetToken()
{
    token_t t;
    printf("get ");
    fflush(stdout);
    t = XGetToken();
    printf("token %d\n", t);
    return(t);
}
*/

token_t
GetToken()
{
    short c;
    short i;

    for (;;) {
        switch(c = getc(Fi)) {
        case EOF:
            return(0);
        case ':':
            return(TokColon);
        case '=':
            return(TokEq);
        case '?':
            return(TokQuestion);
        case '\n':
            ++LineNo;
            return(TokNewLine);
        case '(':
            return(TokOpenParen);
        case ')':
            return(TokCloseParen);
        case '$':
            return(TokDollar);
        case '%':
            return(TokPercent);
        case ' ':
        case '\t':
        case '\014':
        case '\r':
            break;
        case '#':
            while ((c = getc(Fi)) != EOF) {
                if (c == '\n') {
                    ++LineNo;
                    break;
                }
            }
            break;
        case '\"':
            for (i = 0; i < sizeof(SymBuf) - 1 && (c = fgetc(Fi)) != EOF; ++i) {
                if (c == '\n')
                    error(FATAL, "newline in control string");
                if (c == '\"')
                    break;
                if (c == '\\')
                    c = fgetc(Fi);
                SymBuf[i] = c;
            }
            SymBuf[i] = 0;
            if (i == sizeof(SymBuf) - 1)
                error(FATAL, "Symbol overflow: %s", SymBuf);
            if (c != '\"')
                error(FATAL, "Expected closing quote");
            return(TokStr);
        case '\\':
            c = fgetc(Fi);
            if (c == '\n') {
                ++LineNo;
                break;
            }
            /* fall through */
        default:
            SymBuf[0] = c;

            for (i = 1; i < sizeof(SymBuf) - 1 && (c = getc(Fi)) != EOF; ++i) {
                if (SpecialChar[c]) {
                    ungetc(c, Fi);
                    break;
                }
                SymBuf[i] = c;
            }
            SymBuf[i] = 0;
            if (i == sizeof(SymBuf) - 1)
                error(FATAL, "Symbol overflow: %s", SymBuf);
            return(TokSym);
        }
    }
}

void
expect(token_t tgot, token_t twant)
{
    if (tgot != twant)
        error(FATAL, "Unexpected token");
}

void
error(short type, const char *ctl, ...)
{
    static char *TypeString[] = { "Fatal", "Warning", "Debug" };
    static char ExitAry[] = { 1, 0, 0 };
    va_list va;

    printf("%s: %s Line %ld: ", FileName, TypeString[type], LineNo);
    va_start(va, ctl);
    vprintf(ctl, va);
    va_end(va);
    puts("");
    if (ExitAry[type])
        exit(20);
}

#ifdef unix

/*
 * ccmd() - execute a UNIX command and cache the result.
 */

typedef struct CCNode {
    Node        cc_Node;
    int         cc_CNumber;
    char        *cc_Cmd;
    char        *cc_Arg1;
    char        *cc_Result;
} CCNode;

static const char *
ccmd(int cno, const char *cmd, const char *arg1)
{
    CCNode *ccn;
    int pfd[2];
    int status;
    pid_t pid;
    char buf[256];
    int i;
    int n;

    for (ccn = GetHead(&CCList); ccn; ccn = GetSucc(&ccn->cc_Node)) {
        if (cno == ccn->cc_CNumber)
            return(ccn->cc_Result);
    }

    if (pipe(pfd) < 0) {
        perror("pipe");
        exit(30);
    }
    if ((pid = fork()) == 0) {
        dup2(pfd[1], 1);
        close(pfd[0]);
        close(pfd[1]);
        execlp(cmd, cmd, arg1, NULL);
        _exit(30);
    }
    if (pid < 0) {
        perror("fork");
        exit(30);
    }
    close(pfd[1]);
    i = 0;
    while (i < sizeof(buf) - 1) {
        n = read(pfd[0], buf + i, sizeof(buf) - 1 - i);
        if (n <= 0)
            break;
        i += n;
    }
    close(pfd[0]);
    while (waitpid(pid, &status, 0) != pid)
        ;
    if (WEXITSTATUS(status)) {
        fprintf(stderr, "Unable to execute command %s %s\n", 
            cmd,
            (arg1 ? arg1 : "")
        );
        exit(30);
    }
    if (i && buf[i-1] == '\n')
        --i;
    buf[i] = 0;
    ccn = malloc(sizeof(CCNode));
    ccn->cc_Cmd = strdup(cmd);
    ccn->cc_Result = strdup(buf);
    ccn->cc_CNumber = cno;
    AddTail(&CCList, &ccn->cc_Node);
    return(ccn->cc_Result);
}

#endif

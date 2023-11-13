/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PARSE.C
 *
 *  Parse the next token or tokens
 */

#include "defs.h"
#include <stdarg.h>
#ifndef AMIGA
#include <suplib/string.h>
#endif

Prototype void InitParser(void);
Prototype void ParseFile(char *);
Prototype token_t ParseAssignment(char *, token_t);
Prototype token_t ParseDependency(char *, token_t);
Prototype token_t GetElement(void);
Prototype token_t XGetElement(void);
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

void
InitParser()
{
    short i;
    for (i = 0; SChars[i]; ++i)
        SpecialChar[(ubyte)SChars[i]] = 1;
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

    if ((fi = fopen(fileName, "r")) == NULL)
        error(FATAL, "Unable to open %s", fileName);
    FileName = strdup(fileName);
    Fi = fi;

    for (t = GetElement(); t; ) {
        switch(t) {
        case TokNewLine:
            t = GetElement();
            break;
        case TokSym:
            strcpy(AltBuf2, SymBuf);

            /*
             *  check for '=' -- assignment
             */

            t = GetElement();
            if (t == TokEq)
                t = ParseAssignment(AltBuf2, t);
            else
                t = ParseDependency(AltBuf2, t);
            break;
        default:
            error(FATAL, "Expected a symbol!");
            break;
        }
    }
}

/*
 *  Parse an asignment.  Parsed as-is (late eval)
 *
 *  t contains TokEq, ignore
 */

token_t
ParseAssignment(char *varName, token_t t)
{
    Var *var = MakeVar(varName, '$');
    long len;
    short done;
    short eol = 1;
    List tmpList;

    NewList(&tmpList);

#ifdef NOTDEF
    {
        short c;
        while ((c = fgetc(Fi)) == ' ' || c == '\t')
            ;
        if (c != EOF)
            ungetc(c, Fi);
    }
#endif

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
            /*
             * allow one whitespace in
             */
            if (i && ((AltBuf[i-1] == '\t') || (AltBuf[i] == ' ')))
                --i;
            for (; i < len; ++i)
                PutCmdListChar(&tmpList, AltBuf[i]);

        }
        if (done > 0)
            break;
    }

    /*
     *  Now, load temp list into buffer and expand into the variable
     */

    {
        char *buf = malloc(CmdListSize(&tmpList) + 1);
        CopyCmdListBuf(&tmpList, buf);
        ExpandVariable(buf, &tmpList);
        AppendCmdList(&tmpList, &var->var_CmdList);
        free(buf);
    }
    return(GetElement());
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
    long    nlhs = 0;
    long    nrhs = 0;
    short   ncol = 0;

    NewList(cmdList);
    NewList(&lhsList);
    NewList(&rhsList);

    ++nlhs;
    for (CreateDepRef(&lhsList, firstSym); t != TokColon; t = GetElement()) {
        expect(t, TokSym);
        CreateDepRef(&lhsList, SymBuf);
        ++nlhs;
    }
    t = GetElement();
    if (t == TokColon) {
        ++ncol;
        t = GetElement();
    }

    while (t != TokNewLine) {
        expect(t, TokSym);
        CreateDepRef(&rhsList, SymBuf);
        ++nrhs;
        t = GetElement();
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
                if (blankLine)break;
                PutCmdListChar(cmdList, '\n');
                blankLine = 1;
                continue;
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
    dbprintf(("parse: %d : %d\n", nlhs, nrhs));

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

#ifdef NOTDEF
token_t
GetElement(void)
{
    token_t t = XGetElement();
    return(t);
}
#endif

token_t
GetElement(void)
{
    static List CmdList = { (Node *)&CmdList.lh_Tail, NULL, (Node *)&CmdList.lh_Head };
    token_t t;
    short c;

top:
    if (PopCmdListSym(&CmdList, SymBuf, sizeof(SymBuf)) == 0) {
        return(TokSym);
    }

    t = GetToken();
swi:
    switch(t) {
    case TokDollar:
    case TokPercent:
        c = fgetc(Fi);
        if (c == '(') {
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

    dbprintf(("ParseVariable: (%c:%c) %s\n", c0, c, AltBuf));

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

    dbprintf(("ParseVariableBuf: (%c) %s\n", c0, buf));
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
        return(buf);
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
        return(buf);
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

    dbprintf(("CopyConvert to %s %s (%s) %08lx\n", altBuf, symBuf, var->var_Node.ln_Name, GetHead(&var->var_CmdList)));

    CopyCmdListConvert(&var->var_CmdList, cmdList, altBuf, symBuf);
    FreePathBuffer(symBuf);
    FreePathBuffer(altBuf);
    return(buf);
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
            CopyCmdListBuf(list, buf);
        }
    }
    --Levels;
    return(buf);
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

#ifdef NOTDEF
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
#endif

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

    printf("%s: %s Line %d: ", FileName, TypeString[type], (int)LineNo);
    va_start(va, ctl);
    vprintf(ctl, va);
    va_end(va);
    puts("");
    if (ExitAry[type])
        exit(20);
}


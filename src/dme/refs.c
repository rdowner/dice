/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  REFS.C
 *
 *  Bringup a cross reference editor window.  The file S:dme.refs and
 *  dme.refs in the current directory are searched for the reference.
 *  If found, the file specified is searched and the segment specified
 *  loaded as a new file in a new window.
 */

#include "defs.h"
#include <libraries/dos.h>

Prototype void do_addpath (void);
Prototype void do_rempath (void);
Prototype void do_ctags (void);
Prototype void do_refs (void);
Prototype int searchref (char *, char *, char **, char **, int *, char **);
Prototype int dirpart (char *);

Prototype MLIST     PBase;


#define PEN    struct _PEN

PEN {
    MNODE   Node;
    char    *path;
};

MLIST   PBase;          /*  special DME paths   */

/*
 *  Special DME paths for REF and CTAGS
 */

#ifndef NO_DO2

void
do_addpath()
{
    PEN *pen;
    short len = strlen(av[1]);

    for (pen = (PEN *)PBase.mlh_Head; pen->Node.mln_Succ; pen = (PEN *)pen->Node.mln_Succ) {
        if (strcmp(av[1], pen->path) == 0)
            return;
    }
    if (pen = malloc(sizeof(PEN)+len+2)) {
        pen->path = (char *)(pen + 1);
        strcpy(pen->path, av[1]);
        switch(pen->path[len-1]) {
        case ':':
        case '/':
            break;
        default:
            strcat(pen->path, "/");
        }
    }
    AddTail((LIST *)&PBase, (NODE *)pen);
}

void
do_rempath()
{
    PEN *pen, *npen;

    for (pen = (PEN *)PBase.mlh_Head; npen = (PEN *)pen->Node.mln_Succ; pen = npen) {
        if (WildCmp(av[1], pen->path)) {
            Remove((NODE *)pen);
            free(pen);
        }
    }
}

#endif

#ifndef NO_DO_CTAGS

/*
 *  Implement ctags
 */

void
do_ctags()
{
    char str[64];
    char path[128];
    char buf[128];
    char sbuf[128];
    short xlen;
    short slen;
    short dbaselen;
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);
    ED *ed;

    {
        short i, j;

        for (i = Ep->Column; Current[i] == ' '; ++i);
        for (j = i; ; ++j) {
            short c = Current[j];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= '0' && c <= '9'))
                continue;
            break;
        }
        j -= i;
        if (j > 63)
            j = 63;
        movmem(Current+i, str, j);
        str[j] = 0;
        xlen = j;
    }
    if (!Ep->iconmode)
        title("search tags");
    {
        FILE *fi;
        PEN *pen, *npen;
        long i;
        short j, len;

        dbaselen = dirpart(Ep->Name);
        movmem(Ep->Name, path, dbaselen);
        strcpy(path+dbaselen, "tags");

        /*
         *  Note: pen not used first pass and set to list head, so next
         *  pass it will be the first element.
         *
         *  Note2:  The file path depends on several factors.  (1) tags in
         *          'current' directory, use path to name of current window.
         *          (2) tags in directory in DME special path, use special
         *          path.  (3) tag entry is a full path name, override
         *          previous directories.
         */

        for (pen = (PEN *)&PBase; npen = (PEN *)pen->Node.mln_Succ; pen = npen) {
            mountrequest(0);
            if (fi = fopen(path, "r")) {
                mountrequest(1);
                while ((len = xefgets(fi, buf, 128)) >= 0) {
                    for (j = 0; buf[j] && buf[j] != ' '; ++j);
                    if (j == 0 || buf[0] == '#')
                        continue;
                    if (j == xlen && strncmp(str, buf, j) == 0) {
                        while (buf[j] == ' ')
                            ++j;
                        /*
                         *  Extract the file name into str.  If the
                         *  filename does not contain an absolute path,
                         *  prepend it with such.
                         */
                        {
                            char prep = 1;
                            for (i = 0; buf[j] && buf[j] != ' '; ++i, ++j) {
                                str[i] = buf[j];
                                if (str[i] == ':')
                                    prep = 0;
                            }
                            if (prep) {
                                movmem(str, str + dbaselen, i);
                                movmem(path, str, dbaselen);
                                i += dbaselen;
                            }
                        }
                        str[i] = 0;

                        while (buf[j] && buf[j] != '^')     /*  SEARCH ARG */
                            ++j;
                        fclose(fi);
                        if (buf[j] != '^') {
                            title("tags error");
                            goto done;
                        }
                        ++j;    /* Skip ^ */

                        /* UN*X FIX: Dme works for Aztec ctags, which has the format:   */
                        /*      <tag>   <file>  /^<pattern>                             */
                        /* However, ctags under UN*X has the following format:          */
                        /*      <tag>   <file>  /^<pattern>[$]/                         */
                        /* We just ignore the '$' and '/' so that both foramts work     */
                        /* Thomas Rolfs 27/3/91                                         */

                        {
                            short i = 0;

                            while (buf[j] && buf[j] != '$' && buf[j] != '/') {
                                if (buf[j] == '\\') {
                                    sbuf[i++] = buf[++j];
                                    ++j;
                                } else {
                                    sbuf[i++] = buf[j++];
                                }
                            }
                            sbuf[i] = 0;
                        }

                        /* End of fix.  */

                        slen = strlen(sbuf);
                        if ((ed = finded(str, 0)) == NULL) {
                            strcpy(buf, "newwindow newfile ");
                            strcat(buf, str);
                            do_command(buf);
                            ed = finded(str, 0);
                        } else {
                            WindowToFront(ed->Win);
                            ActivateWindow(ed->Win);
                        }
                        if (ed == NULL) {
                            title("unable to load file");
                            goto done;
                        }
                        text_switch(ed->Win);
                        if (Ep->iconmode)
                            uniconify();
                        else
                            text_cursor(0);
                        for (i = 0; i < ed->Lines; ++i) {
                            if (strncmp(ed->List[i], sbuf, slen) == 0)
                                break;
                        }
                        sprintf(buf, "first goto %ld", i+1);
                        do_command(buf);
                        goto done;
                    }
                }
                fclose(fi);
            } else {
                mountrequest(1);
            }
            if (npen->Node.mln_Succ) {
                strcpy(path, npen->path);
                strcat(path, "tags");
                dbaselen = strlen(npen->path);
            }
        }
        title("tag not found");
    }
done:
    CurrentDir(oldlock);
}

#endif

#ifndef NO_DO_REFS

/*
 *  Implement references
 */

void
do_refs()
{
    static char str[256];
    static char tmprfile[128];
    static char path[128];
    char *srch;
    char *file;
    char *estr;
    long len;
    int bcnt = 10;
    short i, j;
    short slen, elen;
    FILE *fi, *fj;
    short tmph, tmpw;
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    for (i = Ep->Column; Current[i] == ' '; ++i);     /*  skip spaces     */

    {
        char c;

        for (j = 0; c = Current[i]; ++i, ++j) {
            if (j == sizeof(str) - 1)
                break;
            str[j] = c;
            if (c >= 'a' && c <= 'z')
                continue;
            if (c >= '0' && c <= '9')
                continue;
            if (c >= 'A' && c <= 'Z')
                continue;
            if (c == '_')
                continue;
            break;
        }
        str[j] = 0;
        strcpy (tmprfile, "t:ref_");
        strncat (tmprfile, str, sizeof(tmprfile) - 32);
    }

    title("search .refs");

    {
        PEN *pen;
        PEN *npen;

        strcpy(path, "dme.refs");       /*  warning, am assuming 8 char name */
        mountrequest(0);
        for (pen = (PEN *)&PBase; npen = (PEN *)pen->Node.mln_Succ; pen = npen) {
            if (searchref(path, str, &srch, &file, &len, &estr)) {
                mountrequest(1);
                goto found;
            }
            if (npen->Node.mln_Succ) {
                strcpy(path, npen->path);
                strcat(path, "dme.refs");
            }
        }
        title("Reference not found");
        mountrequest(1);
        goto done;
    }
found:
    title("search file");
    slen = strlen(srch);
    if (estr)
        elen = strlen(estr);

    fi = fopen(file, "r");
    if (fi == NULL) {       /*  try using path prefix   */
        strcpy(str, path);
        strcpy(str + strlen(str) - 8, file);
        fi = fopen(str, "r");
    }
    if (fi) {
        short lenstr;
        if (srch[0] == '@' && srch[1] == '@') {
            fseek(fi, atoi(srch+2), 0);
            if ((lenstr = xefgets(fi, str, 256)) >= 0)
                goto autoseek;
        }
        while ((lenstr = xefgets(fi, str, 256)) >= 0) {
            if (strncmp(str, srch, slen) == 0) {
autoseek:
                title("load..");
                if (fj = fopen(tmprfile, "w")) {
                    tmph = 0;
                    tmpw = 0;
                    do {
                        if (lenstr > tmpw)
                            tmpw = strlen(str);
                        ++tmph;
                        fputs(str, fj);
                        fputc('\n', fj);
                        if (estr && strncmp(str,estr,elen) == 0)
                            break;
                        --len;
                    } while ((lenstr=xefgets(fi, str, 256)) >= 0 && len);
                    fclose(fj);
                    if (tmph > 10)
                        tmph = 10;
                    if (tmpw > 80)
                        tmpw = 80;
                    sprintf(str, "openwindow +0+0+%d+%d newfile %s", (tmpw<<3)+24, (tmph<<3)+24, tmprfile);
                    do_command(str);
                    unlink(tmprfile);
                } else {
                    sprintf (str, "Unable to open %s for write", tmprfile);
                    title (str);
                }
                fclose(fi);
                free(srch);
                free(file);
                if (estr)
                    free(estr);
                goto done;
            }
            if (--bcnt == 0) {      /* check break every so so  */
                bcnt = 50;
                if (breakcheck())
                    break;
            }
        }
        fclose(fi);
        title("Search failed");
    } else {
        title("Unable to open sub document");
    }
    free(srch);
    free(file);
    if (estr)
        free(estr);
done:
    CurrentDir(oldlock);
}

/*
 *  Reference file format:
 *
 *  `key' `lines' `file' `searchstring'
 *
 *  where `lines' can be a string instead ... like a read-until, otherwise
 *  the number of lines to read from the reference.
 */

searchref(file, find, psstr, pfile, plines, pestr)
char *file, *find;
char **psstr, **pfile, **pestr;
long *plines;
{
    FILE *fi;
    char buf[256];
    char *ptr, *base;
    char *b1, *b2, *b3, *b4;
    char quoted;
    short findlen = strlen(find);

    if (fi = fopen(file, "r")) {
        while (xefgets(fi, (base=buf), 256) >= 0) {
            if (buf[0]=='#')
                continue;
            ptr = breakout(&base, &quoted, &b1);
/*            if (ptr && *ptr && strncmp(ptr, find, findlen) == 0) { */
            if (ptr && *ptr && strcmp(ptr, find) == 0) {
                if (ptr = breakout(&base, &quoted, &b2)) {
                    *pestr = NULL;
                    *plines = atoi(ptr);
                    if (*plines == 0) {
                        *pestr = (char *)malloc(strlen(ptr)+1);
                        strcpy(*pestr, ptr);
                    }
                    if (ptr = breakout(&base, &quoted, &b3)) {
                        *pfile = (char *)malloc(strlen(ptr)+1);
                        strcpy(*pfile, ptr);
                        if (ptr = breakout(&base, &quoted, &b4)) {
                            *psstr = (char *)malloc(strlen(ptr)+1);
                            strcpy(*psstr, ptr);
                            fclose(fi);
                            if (b1) free(b1);
                            if (b2) free(b2);
                            if (b3) free(b3);
                            if (b4) free(b4);
                            return(1);
                        }
                        free(*pfile);
                        if (b4)
                            free(b4);
                    }
                    if (pestr)
                        free (*pestr);
                    if (b3)
                        free (b3);
                }
                if (b2)
                    free(b2);
            }
            if (b1)
                free(b1);
        }
        fclose(fi);
    }
    return(0);
}

#endif

#ifndef NO_DO_CTAGS

dirpart(str)
char *str;
{
    short i;

    for (i = strlen(str) - 1; i >= 0; --i) {
        if (str[i] == '/' || str[i] == ':')
            break;
    }
    return(i+1);
}

#endif


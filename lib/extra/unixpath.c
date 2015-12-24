
/*
 *  EXTRA/UNIXPATH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>
#include <lib/unix.h>

static char Buf[4][512];
static short Cnt;

char *
UnixToAmigaPath(name)
char *name;
{
    short i;
    char *buf = Buf[Cnt];

    Cnt = (Cnt + 1) & 3;

    i = 0;

    if (name[0] == '/') {
	short j;
	for (j = 1; name[j] && name[j] != '/'; ++j)
	    buf[i++] = name[j];
	buf[i++] = ':';
	name += j;
	if (*name == '/')
	    ++name;
    }

    while (*name && i < sizeof(Buf[0]) - 4) {
	if (name[0] == '.') {
	    if (name[1] == '/') {
		name += 2;
		continue;
	    }
	    if (name[1] == '\0') {
		name += 1;
		continue;
	    }
	    if (name[1] == '.') {
		if (name[2] == '/') {
		    name += 3;
		    buf[i++] = '/';
		    continue;
		}
		if (name[2] == '\0') {
		    name += 2;
		    buf[i++] = '/';
		    continue;
		}
	    }
	}
	buf[i++] = *name;
	++name;
    }
    buf[i] = 0;
    return(buf);
}

char *
AmigaToUnixPath(name)
char *name;
{
    short i = 0;
    char *buf = Buf[Cnt];

    Cnt = (Cnt + 1) & 3;

    while (*name == '/') {
	i += sprintf(buf + i, "../");
	++name;
    }
    while (*name && i < sizeof(Buf[0]) - 4) {
	buf[i++] = *name;
	++name;
    }
    buf[i] = 0;
    return(buf);
}


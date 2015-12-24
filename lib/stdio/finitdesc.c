
/*
 *  Initialize a zerod file pointer
 *
 *  if interactive set line buffering
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <fcntl.h>
#include <stdio.h>

void
_finitdesc(fi, fd, flags)
FILE *fi;
int fd;
short flags;
{
    if (fi == stdin || fi == stdout || fi == stderr)
	flags |= __SIF_NOFREE;

    fi->sd_UC = -1;
    fi->sd_Fd = fd;
    fi->sd_Flags = flags | __SIF_OPEN;
    fi->sd_BufSiz = _bufsiz;
    fi->sd_WLeft = -1;
    fi->sd_RLeft = -1;

    if (isatty(fi->sd_Fd) > 0)
	fi->sd_Flags |= __SIF_IOLBF;
    else
	fi->sd_Flags |= __SIF_FILE;
}


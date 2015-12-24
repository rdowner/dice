
/*
 *  MAKEFD.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Used by routines that want to get into our low level io system.  If exec
 *  is NULL then handle is an AmigaDOS file handle.  If exec is not NULL then
 *  it is a function capable of executing ioctls.
 */

#include <stdio.h>
#include <fcntl.h>

int
MakeFd(handle, flags, exec)
long handle;
long flags;
long (*exec)();
{
    int fd;
    _IOFDS *d = _MakeFD(&fd);

    d->fd_Fh = handle;
    d->fd_Flags = flags | O_ISOPEN;
    d->fd_Exec = exec;

    return(fd);
}




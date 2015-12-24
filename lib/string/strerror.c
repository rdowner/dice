
/*
 *  STRERROR.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <string.h>
#include <errno.h>

int sys_nerr = 18;      /* entries in table */

char *sys_errlist[] = {
    "no error",
    "domain",                   /* EDOM */
    "range",                    /* ERANGE */
    "bad file descriptor",      /* EBADF */
    "no permission",            /* ENOPERM */
    "no memory",                /* ENOMEM */
    "file not found",           /* ENOFILE, ENOENT */
    "no access",                /* EACCES */
    "invalid argument",         /* EINVAL */
    "ran out of descriptors",   /* EMFILE */
    "try again",                /* EAGAIN */
    "connection reset by peer", /* EPEER */
    "broken pipe",              /* EPIPE */
    "file not found",           /* ENOTFND */
    "unknown error",            /* ESTACK */
    "unknown error",            /* ENOTTY */
    "unknown error",            /* ENXIO */
    "file/directory exists",    /* EEXIST */
};


char *
strerror(error)
int error;
{
    if( error < sys_nerr )
        return(sys_errlist[error]);

    return("unknown error");
}

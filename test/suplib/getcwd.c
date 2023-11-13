
#include <stdlib.h>
#include <libc.h>
#include <sys/param.h>

char *
getcwd(char *buf, int maxlen)
{
    char xbuf[MAXPATHLEN];

    if (getwd(xbuf)) {
        strncpy(buf, xbuf, maxlen);
        return(buf);
    } else {
        return(NULL);
    }
}



#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#ifdef next
#include <libc.h>

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

#endif

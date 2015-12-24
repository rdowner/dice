/*
 *  unix/mktemp.c  - Contributed by Ben Jackson
 */

#include <string.h>
#include <sys/stat.h>
#include <clib/exec_protos.h>

static const char base36[]= "01234567890abcdefghijklmnopqrstuvwxyz";

static char *itob36(unsigned long num)
{
    static char result[7];
    int i;
    char *ptr;

    result[6] = '\0';

    for(i = 5, ptr = result + i; i && num; i--, ptr--) {
        *ptr = base36[num % 36];
        num /= 36;
    }
    return ptr+1;
}

char *mktemp(char *template)
{
    static unsigned long next = 0;
    int n = 0, len = strlen(template);
    unsigned long pid = (long)FindTask(NULL) * 36 * 36;
    struct stat buf;
    char *ptr;

    for(ptr = template + len - 1;'X' == *ptr; --ptr) ++n;
    ++ptr;

    if(!n)
        return NULL;  /* no trailing Xs */

    if(!strncmp(template, "/tmp/", 5)) {
        /* amiga hack */
        memcpy(template, "t:bjj", 5);
    }

    /* this loop could be very bad if someone did something stupid,
     * but UNIX will try 26^n files too.
     */
    do {
        char *repl = itob36(pid + next++);
        int l;

        l = strlen(repl);
        if(l > n) repl += l - n;
        strcpy(ptr, repl);
    } while(!stat(template, &buf));

    return template;
}


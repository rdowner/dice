
#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
strdup(const char *str)
{
    char *ptr;
    
    if ((ptr = malloc(strlen(str) + 1)))
        strcpy(ptr, str);
    return(ptr);
}

#endif


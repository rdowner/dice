
#include <stdlib.h>
#include <string.h>

void
strins(char *d, const char *s)
{
    int len = strlen(s);    /*	# bytes to insert   */
    char *ptr;

    /*
     *	make room
     */

    ptr = d + strlen(d);
    while (ptr >= d) {
	ptr[len] = ptr[0];
	--ptr;
    }

    /*
     *	insert string
     */

    while (*s)
	*++ptr = *s++;
}


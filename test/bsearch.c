
/*
 *  TEST BSEARCH
 */

#include <stdlib.h>
#include <string.h>

#define ELEMENTS    100

#if TEST1
typedef struct Fubar {
    short   fu_Key;
    short   fu_Magic;
} Fubar;
#elif TEST2
typedef struct Fubar {	    /*	not a power of 2    */
    short   fu_Key;
    short   fu_Magic;
    char    fu[36];
} Fubar;
#endif


Fubar	Ary[ELEMENTS];

int
mycmp(key, elm)
Fubar *key;
Fubar *elm;
{
    if (key->fu_Key < elm->fu_Key)
	return(-1);
    if (key->fu_Key == elm->fu_Key)
	return(0);
    return(1);
}

main(ac, av)
char *av[];
{
    short i;
    Fubar key;
    Fubar *res;

    /*
     *	fill array
     */

    for (i = 0; i < ELEMENTS; ++i) {
	Ary[i].fu_Key = i;
	Ary[i].fu_Magic = 0x5555;
    }
    for (i = -10; i < ELEMENTS + 10; ++i) {
	key.fu_Key = i;

	res = bsearch(&key, Ary, ELEMENTS, sizeof(Fubar), mycmp);
	if (res)
	    printf("%d : %d %04x\n", i, res->fu_Key, res->fu_Magic);
	else
	    printf("%d : not found\n", i);
    }
    return(0);
}

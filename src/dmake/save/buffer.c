
/*
 *
 */

#include "defs.h"

Prototype char *AllocPathBuffer(void);
Prototype void FreePathBuffer(char *);


List PathBufList = { (Node *)&PathBufList.lh_Tail, NULL, (Node *)&PathBufList.lh_Head };

char *
AllocPathBuffer()
{
    Node *node;

    if ((node = RemHead(&PathBufList)) == NULL)
	node = malloc(PBUFSIZE);
    return((char *)node);
}

void
FreePathBuffer(buf)
char *buf;
{
    AddTail(&PathBufList, (Node *)buf);
}


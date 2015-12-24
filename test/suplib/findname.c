
#include <stdio.h>
#include <suplib/lists.h>

void *
FindName(struct List *list, const char *name)
{
    struct Node *node;

    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ) {
	if (node->ln_Name && strcmp(node->ln_Name, name) == 0)
	    return((void *)node);
    }
    return(NULL);
}


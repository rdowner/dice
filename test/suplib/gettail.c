
#include <stdio.h>
#include <suplib/lists.h>

void *
GetTail(struct List *list)
{
    struct Node *node = list->lh_TailPred;

    if (node->ln_Pred == NULL)
        node = NULL;
    return((void *)node);
}


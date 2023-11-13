

#include <stdio.h>
#include "lists.h"

void *
RemHead(struct List *list)
{
    struct Node *node = list->lh_Head;
    if (node->ln_Succ) {
        node->ln_Succ->ln_Pred = node->ln_Pred;
        node->ln_Pred->ln_Succ = node->ln_Succ;
        node->ln_Succ = NULL;
        node->ln_Pred = NULL;
    } else {
        node = NULL;
    }
    return((void *)node);
}



#include <stdio.h>
#include "lists.h"

void *
Remove(struct Node *node)
{
    node->ln_Succ->ln_Pred = node->ln_Pred;
    node->ln_Pred->ln_Succ = node->ln_Succ;
    node->ln_Succ = NULL;
    node->ln_Pred = NULL;
    return((void *)node);
}


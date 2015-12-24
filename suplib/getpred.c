
#include <stdio.h>
#include "lists.h"

void *
GetPred(struct Node *node)
{
    struct Node *pred = node->ln_Pred;

    if (pred->ln_Pred == NULL)
	pred = NULL;
    return((void *)pred);
}


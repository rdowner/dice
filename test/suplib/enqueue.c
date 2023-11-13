
#include <stdio.h>
#include <suplib/lists.h>

void
Enqueue(struct List *list, struct Node *node)
{
    struct Node *scan;

    for (scan = list->lh_Head; scan->ln_Succ; scan = scan->ln_Succ) {
        if (node->ln_Pri > scan->ln_Pri)
            break;
    }
    /*
     *  Insert before scan.  scan might point to &lh_Tail
     */

    node->ln_Succ = scan;
    node->ln_Pred = scan->ln_Pred;
    scan->ln_Pred = node;
    node->ln_Pred->ln_Succ = node;
}


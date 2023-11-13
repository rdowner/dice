
#include <stdio.h>
#include "lists.h"

void
Insert(struct List *list, struct Node *node, struct Node *lnode)
{
    /*
     *  Insert node after lnode.  If lnode == NULL then insert 
     *  at head of list.
     */

    if (lnode == NULL)
        lnode = (struct Node *)&list->lh_Head;
    node->ln_Pred = lnode;
    node->ln_Succ = lnode->ln_Succ;
    lnode->ln_Succ = node;
    node->ln_Succ->ln_Pred = node;
}


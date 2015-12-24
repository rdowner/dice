
/*
 *  Default workbench entry handler does nothing.  Prevents CLI programs
 *  from incorrect operation if started as Workbench programs.
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */


int wbmain(void *);
int main(int, char *);

int
wbmain(void *wbmsg)
{
    return(-1);
}

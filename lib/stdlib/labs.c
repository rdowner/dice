
/*
 *  LABS()
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

long
labs(long i)
{
    if (i < 0)
	i = -i;
    return(i);
}


/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#if FOO || BAR
#error "FOO || BAR"
#else
#error "!(FOO || BAR)"
#endif

#if FOO
#error "FOO"
#else
#error "!FOO"
#endif

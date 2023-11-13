
/*
 *  MKTIME.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <time.h>
#include <lib/misc.h>

time_t
mktime(tm)
struct tm *tm;
{
    static int mon[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    struct tm tm_tmp;
    time_t  t;
    short notLeap;
    short i;

    /*
     *  1976 was a leap year, take quad years from 1976, each
     *  366+365+365+365 days each.  Then adjust for the year
     */

    t = ((tm->tm_year - 76) / 4) * ((366 + 365 * 3) * 86400);

    /*
     *  compensate to make it work the same as unix time (unix time
     *  started 8 years earlier)
     */

    t += _TimeCompensation;

    /*
     *  take care of the year within a four year set, add a day for
     *  the leap year if we are based at a year beyond it.
     */

    t += ((notLeap = (tm->tm_year - 76) % 4)) * (365 * 86400);

    if (notLeap)
        t += 86400;

    /*
     *  calculate days over months then days offset in the month
     */

    for (i = 0; i < tm->tm_mon; ++i) {
        t += mon[i] * 86400;
        if (i == 1 && notLeap == 0)
            t += 86400;
    }
    t += (tm->tm_mday - 1) * 86400;

    /*
     *  our time is from 1978, not 1976
     */

    t -= (365 + 366) * 86400;

    /*
     *  calculate hours, minutes, seconds
     */

    t += tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;

    localtime_tm(&t, &tm_tmp);
    tm->tm_wday = tm_tmp.tm_wday;
    tm->tm_yday = tm_tmp.tm_yday;

    return(t);
}

#ifdef TEST

#include <stdio.h>

main(ac, av)
char *av[];
{
    struct tm tm;
    time_t t;

    if (ac < 4) {
        puts("test day mo yr");
        exit(0);
    }
    clrmem(&tm, sizeof(tm));
    tm.tm_mday = atoi(av[1]);
    tm.tm_mon  = atoi(av[2]) - 1;
    tm.tm_year = atoi(av[3]) - 1900;

    t = mktime(&tm);

    printf("%08lx %s", t, ctime(&t));
    return(0);
}

#endif

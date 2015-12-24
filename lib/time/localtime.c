
/*
 *  time/localtime.c
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <time.h>
#include <lib/misc.h>

static char dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct tm *
localtime(tp)
const time_t *tp;
{
    static struct tm Tm;
    return(localtime_tm(tp, &Tm));
}

struct tm *
localtime_tm(tp, tm)
const time_t *tp;
struct tm *tm;
{
    time_t t = (tp) ? *tp : time(NULL);     /*	seconds since 1-jan-78	*/
    long days, years;
    short leap, month;

    t = t - _TimeCompensation;

    days = t / 86400 + 731;		    /*	days since 1976 (lyp yr)*/

    tm->tm_wday = (days + 4) % 7;
    tm->tm_sec = t % 60;
    tm->tm_min = t / 60 % 60;
    tm->tm_hour= t / (60*60) % 24;

    years = days / (366+365*3); 	    /*	#quad yrs	*/
    days -= years * (366+365*3);	    /*	days remaining	*/
					    /*	0 = jan 1	*/
    leap = (days <= 365);		    /*	0-365, is a leap yr */
    years = 1976 + 4 * years;		    /*	base yr 	*/
    if (leap == 0) {			    /*	days >= 366	*/
	days -= 366;			    /*	add a year	*/
	++years;
	years += days / 365;		    /*	non-lyrs left	*/
	days  %= 365;			    /*	0-364		*/
    }
    tm->tm_yday = days;
    tm->tm_year = years - 1900;
    tm->tm_isdst= -1; /* ???? XXX     */

    for (month = 0; (month==1) ? (days >= 28 + leap) : (days >= dim[month]); ++month)
	days -= (month==1) ? (28 + leap) : dim[month];

    tm->tm_mon	= month;
    tm->tm_mday = days + 1;

    return(tm);
}



/*
 *  STRFTIME.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

static char *AbMonth[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul",
			     "Aug","Sep","Oct","Nov","Dec"
			    };

static char *FuMonth[12] = { "January", "February", "March", "April", "May",
			     "June", "July", "August", "September", "October",
			     "November", "December"
			    };

static char *AbDow[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static char *FuDow[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
			   "Friday", "Saturday"
			};


size_t
strftime(buf, max, fmt, tm)
char *buf;
size_t max;
const char *fmt;
const struct tm *tm;
{
    short i = 0;
    char tmp[64];

    while (*fmt && max >= 0) {
	if (max <= 0) {         /*  buffer overflow */
	    buf[i] = 0;
	    return(0);
	}

	if (*fmt != '%') {
	    buf[i++] = *fmt++;
	    --max;
	    continue;
	}

	fmt += 2;

	switch(fmt[-1]) {
	case '%':
	    strcpy(tmp, "%");
	    break;
	case 'a':   /*  abbreviated name for dow    */
	    strcpy(tmp, AbDow[tm->tm_wday]);
	    break;
	case 'A':   /*  full name for dow           */
	    strcpy(tmp, FuDow[tm->tm_wday]);
	    break;
	case 'b':   /*  abbreviated name for month  */
	    strcpy(tmp, AbMonth[tm->tm_mon]);
	    break;
	case 'B':   /*  full name for month         */
	    strcpy(tmp, FuMonth[tm->tm_mon]);
	    break;
	case 'c':   /*  default rep for date & time */
	    sprintf(tmp, "%s %s %02d %02d:%02d:%02d %d",
		AbDow[tm->tm_wday], AbMonth[tm->tm_mon], tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year + 1900
	    );
	    break;
	case 'd':   /*  day of month as integer 01-31   */
	    sprintf(tmp, "%02d", tm->tm_mday);
	    break;
	case 'H':   /*  hour as integer 00-23       */
	    sprintf(tmp, "%02d", tm->tm_hour);
	    break;
	case 'I':   /*  hour as integer 01-12       */
	    {
		short hr = (tm->tm_hour % 12);
		sprintf(tmp, "%02d", (hr) ? hr : 12);
	    }
	    break;
	case 'j':   /*  day of year as int 001-366  */
	    sprintf(tmp, "%03d", tm->tm_yday + 1);
	    break;
	case 'm':   /*  month as integer 01-12      */
	    sprintf(tmp, "%02d", tm->tm_mon + 1);
	    break;
	case 'M':   /*  minute as integer 00-59     */
	    sprintf(tmp, "%02d", tm->tm_min);
	    break;
	case 'p':   /*  'AM' or 'PM'                */
	    if (tm->tm_hour >= 12) {
		strcpy(tmp, "PM");
	    } else {
		strcpy(tmp, "AM");
	    }
	    break;
	case 'S':   /*  the second as an int 00-59  */
	    sprintf(tmp, "%02d", tm->tm_sec);
	    break;
	case 'U':   /*  week of year as int 00-53, regard sunday as first day in week   */
	case 'W':   /*  week of year as int 00-53, regard monday as first day in week   */
	    {
		int fdiy = tm->tm_yday % 7 - tm->tm_wday;	/*  first sunday in year, can be negative */
		if (fmt[-1] == 'W')
		    ++fdiy;					/*  first monday in year, can be negative */

		while (fdiy > 0)                                /*  handle boundry cases                  */
		    fdiy -= 7;
		while (fdiy <= -7)
		    fdiy += 7;

		sprintf(tmp, "%02d", (tm->tm_yday - fdiy) / 7);
	    }
	    break;
	case 'w':   /*  day of week as int 0-6, sunday == 0 */
	    sprintf(tmp, "%d", tm->tm_wday);
	    break;
	case 'x':   /*  the locale's default rep for date   */
	    sprintf(tmp, "%s %s %02d",
		AbDow[tm->tm_wday], AbMonth[tm->tm_mon], tm->tm_mday
	    );
	    break;
	case 'X':   /*  the locale's default rep for time   */
	    sprintf(tmp, "%02d:%02d:%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec
	    );
	    break;
	case 'y':   /*  year within the century 00-99       */
	    sprintf(tmp, "%02d", tm->tm_year % 100);
	    break;
	case 'Y':   /*  the full year, including century    */
	    sprintf(tmp, "%04d", tm->tm_year + 1900);
	    break;
	case 'Z':   /*  the name of the time zone or ""     */
	    strcpy(tmp, "");
	    break;
	default:
	    strcpy(tmp, "%?");
	    break;
	}
	{
	    short len = strlen(tmp);

	    max -= len;
	    if (max < 0) {
		buf[i] = 0;
		return(0);
	    }
	    strcpy(buf + i, tmp);
	    i += len;
	}
    }
    buf[i] = 0;
    return(i);
}


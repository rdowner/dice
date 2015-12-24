
/*
 *  FPUTC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <stdio.h>
#include <fcntl.h>

int
fputc(c, fi)
unsigned char c;
FILE *fi;
{
    unsigned char cc = c;

    if (fi == NULL)
	return(-1);
    if (fi->sd_Flags & __SIF_WRITE) {
	if (fi->sd_WLeft <= 0) {
	    if (fi->sd_BufSiz == 0) {               /*  unbuffered  */
		if (write(fi->sd_Fd, &cc, 1) != 1)
		    fi->sd_Error = EOF;
		++fi->sd_Offset;
		if (fi->sd_Error < 0)
		    return(fi->sd_Error);
		return(c);
	    }
	    if (fflush(fi))                         /*  not unbuffered */
		return(EOF);
	}
	*fi->sd_WPtr++ = c;
	--fi->sd_WLeft;
	if (c == '\n' && (fi->sd_Flags & __SIF_IOLBF))
	    fflush(fi);
    } else {
	fi->sd_Error = EOF;
    }
    if (fi->sd_Error < 0)
	return(fi->sd_Error);
    return(c);
}


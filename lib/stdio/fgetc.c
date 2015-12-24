
/*
 *  FGETC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  no error checking for speed...
 */

#include <fcntl.h>
#include <stdio.h>
#include <lib/misc.h>

int
fgetc(fi)
FILE *fi;
{
    int error;
    unsigned char c;

    if (fi && (fi->sd_Flags & __SIF_READ)) {
	if (fi->sd_UC >= 0) {
	    c = fi->sd_UC;
	    fi->sd_UC = -1;
	    return(c);
	}
	if (fi->sd_RLeft <= 0) {
	    short flags = fi->sd_Flags;
	    if (error = _filbuf(fi)) {
		if (flags & __SIF_EOF)
		    fi->sd_Error = EOF;
		return(error);
	    }
	    if (fi->sd_RLeft == 0) {	/*  unbuffered! */
		error = read(fi->sd_Fd, &c, 1);
		if (error <= 0) {
		    if (error == 0 && (fi->sd_Flags & __SIF_EOF) == 0)
			fi->sd_Flags |= __SIF_EOF;
		    else
			fi->sd_Error = EOF;
		    return(EOF);
		}
		++fi->sd_Offset;
		return(c);
	    }
	}
	--fi->sd_RLeft;
	return(*fi->sd_RPtr++);
    }
    return(EOF);
}


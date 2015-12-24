
/*
 *  FSEEK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <fcntl.h>
#include <stdio.h>

int
fseek(fi, offset, how)
FILE *fi;
long offset;
int how;
{
    if (fi->sd_Flags & __SIF_APPEND) {  /*  fseek() illegal */
	fi->sd_Error = EOF;
	return(EOF);
    }

    /*
     *	Adjust the seek position relative to the actual fd seek pos and
     *	clear out RLeft.
     */

    if (how == SEEK_CUR) {
	if (fi->sd_UC >= 0) {
	    --offset;
	    fi->sd_UC = -1;
	}
	if (fi->sd_RLeft > 0)
	    offset -= fi->sd_RLeft;
    }
    if (fi->sd_RLeft > 0) {     /*  prevent fflush from doing extra seek */
	fi->sd_RPtr += fi->sd_RLeft;	/* ??? */
	fi->sd_RLeft = 0;
    }

    fflush(fi);
    fi->sd_Offset = lseek(fi->sd_Fd, offset, how);
    if (fi->sd_Offset < 0)
	fi->sd_Error = EOF;
    else
	fi->sd_Error = 0;
    fi->sd_Flags &= ~__SIF_EOF;
    return(fi->sd_Error);
}


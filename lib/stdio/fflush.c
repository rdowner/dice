
/*
 *  FFLUSH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  fflush is also called by other stdio routines to put everything into
 *  a known state.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int
fflush(fi)
FILE *fi;
{
    int n;

    /*
     *	note, if we need to allocate a buffer we are guarenteed not to be in
     *	read or write mode.
     */

    chkabort();
    if (fi && (fi->sd_Flags & __SIF_OPEN)) {
	fi->sd_UC = -1;
	if (fi->sd_BufSiz && fi->sd_WBuf == NULL) {
	    fi->sd_WBuf = malloc(fi->sd_BufSiz);
	    fi->sd_Flags |= __SIF_MYBUF;
	    if (fi->sd_WBuf == NULL)     /*  if couldn't malloc, make unbuf'd */
		fi->sd_BufSiz = 0;
	    fi->sd_WLeft = fi->sd_BufSiz;
	}

	/*
	 *  if a normal file reset the read buffer
	 */

	if (fi->sd_RLeft >= 0 && (fi->sd_Flags & __SIF_FILE)) {
	    if (fi->sd_RLeft > 0)       /*  seek to the right place */
		fi->sd_Offset = lseek(fi->sd_Fd, fi->sd_Offset - fi->sd_RLeft, 0);
	    fi->sd_RLeft = -1;
	    /*fi->sd_WLeft = fi->sd_BufSiz;*/
	}
	if (fi->sd_WLeft >= 0 && (n = fi->sd_BufSiz - fi->sd_WLeft)) {
	    if (write(fi->sd_Fd, fi->sd_WBuf, n) != n)
		fi->sd_Error = EOF;
	    fi->sd_Offset += n;
	    fi->sd_WLeft = fi->sd_BufSiz;
	}
	if (fi->sd_WLeft < 0 && fi->sd_BufSiz)
	    fi->sd_WLeft = fi->sd_BufSiz;
	fi->sd_WPtr = fi->sd_WBuf;
    }
    if (fi->sd_Error)
	return(-1);
    return(0);
}



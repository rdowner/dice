
/*
 *  SETVBUF
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  SendPacket call written by Phil Lindsay, Carolyn Scheppner, and Andy
 *  Finkel and is freely redistributable.
 *
 *  Hacked because my stdio utilizes two buffers to support full
 *  duplex streams.
 */

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

typedef struct MsgPort	MsgPort;

#ifdef NOTDEF
extern __stkargs MsgPort *CreatePort(char *, char *);
extern __stkargs void DeletePort(MsgPort *);
#endif

int set_console_raw(int, short);
long SendPacket(MsgPort *, long, long *, long);


int
setvbuf(fi, buf, mode, bytes)
FILE *fi;
char *buf;
int mode;
size_t bytes;
{
    fflush(fi);

    switch(mode) {
    case _IOFBF:
	fi->sd_Flags &= ~__SIF_IOLBF;
    case _IOLBF:
	if (fi->sd_BufSiz == 0) 	    /*	was raw before	*/
	    set_console_raw(fi->sd_Fd, 0);

	if (mode == _IOLBF)
	    fi->sd_Flags |= __SIF_IOLBF;
	if (bytes) {
	    if (fi->sd_Flags & __SIF_MYBUF) {
		if (fi->sd_RBuf) {
		    free(fi->sd_RBuf);
		}

		if (fi->sd_WBuf) {
		    free(fi->sd_WBuf);
		}
	    }
	    /*
	    fi->sd_Flags &= ~__SIF_MYBUF;
	    */
	    fi->sd_BufSiz = bytes;
	    fi->sd_WLeft = -1;
	    fi->sd_RLeft = -1;

	    /*	What should I do?  I'm using two buffers... which one should
	     *	I put the user's into?  So, silently do not use the user's
	     *	buffer.
	    fi->sd_WBuf = buf;
	    fi->sd_WPtr = buf;
	    */
	    fi->sd_WBuf = NULL;
	    fi->sd_WPtr = NULL;
	    fi->sd_RBuf = NULL;
	    fi->sd_RPtr = NULL;
	}
	break;
    case _IONBF:
	if (fi->sd_BufSiz)		    /*	was cooked before   */
	    set_console_raw(fi->sd_Fd, 1);

	fi->sd_Flags &= ~__SIF_IOLBF;
	fi->sd_BufSiz = 0;
	if (fi->sd_Flags & __SIF_MYBUF) {
	    if (fi->sd_RBuf) {
		free(fi->sd_RBuf);
		fi->sd_RBuf = NULL;
		fi->sd_RPtr = NULL;
	    }
	    if (fi->sd_WBuf) {
		free(fi->sd_WBuf);
		fi->sd_WBuf = NULL;
	    }
	}
	fi->sd_Flags &= ~__SIF_MYBUF;
	fi->sd_WBuf = NULL;
	fi->sd_WPtr = NULL;
	fi->sd_RBuf = NULL;
	fi->sd_RPtr = NULL;
	fi->sd_WLeft = -1;
	fi->sd_RLeft = -1;


	break;
    default:
	return(EOF);
    }
    return(0);
}

int
set_console_raw(fd, rawmode)
int fd;
short rawmode;
{
    MsgPort *mp;
    BPTR fh;
    long Arg[1], res;

    fh = (BPTR)fdtofh(fd);
    if (fh == NULL)
	return(-1);
    if (IsInteractive(fh) == 0) {
	errno = ENOTTY;
	return(-1);
    }
    mp = ((struct FileHandle *)(BADDR(fh)))->fh_Type;
    if (rawmode)
	Arg[0] = -1L;
    else
	Arg[0] = 0L;
    res = SendPacket(mp, ACTION_SCREEN_MODE, Arg, 1);	/* Put it in RAW: mode */
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

/*
 * Function - SendPacket written by Phil Lindsay, Carolyn Scheppner, and Andy
 * Finkel. This function will send a packet of the given type to the Message
 * Port supplied.
 */

typedef struct StandardPacket StandardPacket;

long
SendPacket(pid, action, args, nargs)
MsgPort *pid;	/* process indentifier ... (handler's message port )    */
long action;	/* packet type ... (what you want handler to do )	*/
long *args;	/* a pointer to an argument list    */
long nargs;	/* number of arguments in list	    */
{
    MsgPort *replyport;
    StandardPacket *packet;
    long count, *pargs, res1;

    replyport = CreatePort(NULL, 0L);
    if (!replyport)
	return(0);

    /* Allocate space for a packet, make it public and clear it */

    packet = AllocMem(sizeof(StandardPacket), MEMF_PUBLIC | MEMF_CLEAR);
    if (!packet) {
	DeletePort(replyport);
	return(0);
    }
    packet->sp_Msg.mn_Node.ln_Name = (char *) &(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port = replyport;
    packet->sp_Pkt.dp_Type = action;

    /* copy the args into the packet */

    pargs = &(packet->sp_Pkt.dp_Arg1);	    /* address of first argument */
    for (count = 0; count < nargs; count++)
	pargs[count] = args[count];

    PutMsg(pid, &packet->sp_Msg);   /* send packet */

    WaitPort(replyport);
    GetMsg(replyport);

    res1 = packet->sp_Pkt.dp_Res1;

    FreeMem(packet, sizeof(StandardPacket));
    DeletePort(replyport);

    return (res1);
}


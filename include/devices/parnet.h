
/*
 *  DEVICES/PARNET.H
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef DEVICES_PARNET_H
#define DEVICES_PARNET_H

#ifndef PARNET_SRC
#define UNIT   APTR
#define DEVICE APTR
#else
#define UNIT struct Unit
#define DEVICE Device
#endif

typedef struct {
    struct  Message io_Message;
    DEVICE  *io_Device;             /* device node pointer      */
    UNIT    *io_Unit;               /* unit (protocol private)  */
    UWORD   io_Command;             /* device command           */
    UBYTE   io_Flags;               /* IOF_QUICK                */
    BYTE    io_Error;               /* error or warning num     */
    ULONG   io_Actual;              /* actual number of bytes transferred   */
    ULONG   io_Length;              /* requested number bytes transferred   */
    APTR    io_Data;                /* points to data area                  */
    ULONG   io_Offset;              /* offset for block structured devices  */
    UWORD   io_Port;
    UWORD   io_Addr;
    APTR    io_Data2;               /* more data                            */
    ULONG   io_Length2;             /* must be 0 if you do not use these fields */
} IOParReq;

/*
 *  OpenDevice(), protocols.
 *
 *  DEFAULT
 */

#define PRO_MASK        0x001F
#define PRO_CONTROL     0x0000
#define PRO_DGRAM       0x0001
#define PRO_STREAM      0x0002

#define PRO_LAST        PRO_STREAM

/*
 *  Extended Command Set
 */

#define PPD_SETADDR     (CMD_NONSTD+0)      /*  set my network address  */
#define PPD_SETTO       (CMD_NONSTD+1)      /*  set network timeout     */
#define PPD_SHUTDOWN    (CMD_NONSTD+2)      /*  shutdown a stream conn. */
#define PPD_CONNECT     (CMD_NONSTD+3)      /*  connect to a port       */
#define PPD_LISTEN      (CMD_NONSTD+5)      /*  listen for a connection */

/*
 *  Extended Error codes    (placed in io_Actual for OpenDevice() call)
 */

#define PPERR_NOPORTRES     -32         /*  can't get MR_PARALLELPORT misc res. */
#define PPERR_NOBITSRES     -33         /*  can't get MR_PARALLELBITS misc res. */
#define PPERR_NOTASK        -34         /*  can't create sub task               */
#define PPERR_NOINT         -35         /*  can't allocate CIAA-FLAG interrupt  */
#define PPERR_BADPROTOCOL   -36         /*  illegal protocol requested          */
#define PPERR_BADFLAGS      -37         /*  illegal flags for protocol          */
#define PPERR_ILLPORT       -38         /*  illegal port for protocol                   */
#define PPERR_ILLPORTLSN    -39         /*  somebody already listening on that port!    */
#define PPERR_NOTIMPL       -40         /*  not implemented yet.                */

#define PPERR_WARN_OVFLOW   -41         /*  dgram, data overflow                */
#define PPERR_PORT_IN_USE   -42         /*  port is in use by another protocol  */
#define PPERR_NO_PORTS      -43         /*  no more ports available !!          */
#define PPERR_HOSTDOWN1     -44         /*  remote machine does not respond     */
#define PPERR_HOSTDOWN2     -45         /*  remote machine does not respond     */

#endif



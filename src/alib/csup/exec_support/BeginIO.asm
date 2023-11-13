
******* amiga.lib/BeginIO ****************************************************
*
*   NAME
*       BeginIO -- initiate asynchronous device I/O
*
*   SYNOPSIS
*       BeginIO(ioReq)
*
*       VOID BeginIO(struct IORequest *);
*
*   FUNCTION
*       This function takes an IORequest, and passes it directly to the
*       "BeginIO" vector of the proper device.  This is equivalent to
*       SendIO(), except that io_Flags is not cleared. A good understanding
*       of Exec device I/O is required to properly use this function.
*
*       This function does not wait for the I/O to complete.
*
*   INPUTS
*       ioReq - an initialized and opened IORequest structure with the
*               io_Flags field set to a reasonable value (set to 0 if you do
*               not require io_Flags).
*
*   SEE ALSO
*       exec.library/DoIO(), exec.library/SendIO(), exec.library/WaitIO()
*
******************************************************************************

        INCLUDE "exec/types.i"
        INCLUDE "exec/io.i"


;Call the BeginIO vector of a device directly.  Much like exec/SendIO, but
;does not touch IO_FLAGS.

                SECTION _BeginIO
                XDEF    _BeginIO
                XDEF    @BeginIO

_BeginIO:       move.l  4(sp),a1         ;Get IORequest pointer

@BeginIO:               move.l  a6,-(a7)
                move.l  IO_DEVICE(a1),a6 ;Pointer to device
                jsr     DEV_BEGINIO(a6)  ;Jump to device's BEGINIO vector
                move.l  (a7)+,a6
                rts

                END

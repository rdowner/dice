*************************************************************************
*                                                                       *
*       Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.  *
*                                                                       *
*************************************************************************


*************************************************************************
*
* TimeDelay.asm
*
* Source Control
* ------ -------
* 
* $Id: TimeDelay.asm,v 30.0 1994/06/10 18:10:51 dice Exp $
*
* $Locker:  $
*
*************************************************************************

        SECTION timer

*------ Included Files -----------------------------------------------

        INCLUDE 'exec/types.i'
;       INCLUDE 'exec/interrupts.i'

        INCLUDE 'devices/timer.i'

*------ Imported Names -----------------------------------------------

*------ Tables -------------------------------------------------------

*------ Defines ------------------------------------------------------

CALLSYS MACRO
        JSR     _LVO\1(A6)
        ENDM

XSYS    MACRO
        XREF    _LVO\1
        ENDM


*------ Functions ----------------------------------------------------


*------ System Library Functions -------------------------------------

        XSYS    AllocSignal
        XSYS    FreeSignal
        XSYS    DoIO
        XSYS    FindTask
        XSYS    OpenDevice
        XSYS    CloseDevice

*------ Exported Names -----------------------------------------------

*------ Functions ----------------------------------------------------

        XDEF    TimeDelay
        XDEF    _TimeDelay

*------ Data ---------------------------------------------------------

*------ Local Definitions --------------------------------------------

timerName:      TIMERNAME




******* amiga.lib/TimeDelay ******************************************
*
*   NAME
*       TimeDelay -- Return after a period of time has elapsed.
*
*   SYNOPSIS
*       Error = TimeDelay( Unit, Seconds, MicroSeconds )
*       D0                 D0    D1       D2
*
*       LONG TimeDelay( LONG, ULONG, ULONG );
*
*   FUNCTION
*       Waits for the period of time specified before returning to the
*       the caller.
*
*   INPUTS
*       Unit -- timer.device unit to open for this command.
*       Seconds -- The seconds field of a timerequest is filled with
*           this value. Check the documentation for what a particular
*           timer.device unit expects there.
*       MicroSeconds -- The microseconds field of a timerequest is
*           filled with this value. Check the documentation for what
*           a particular timer.device units expects there.
*
*   RESULTS
*       Error -- will be zero if all went well; otherwise, non-zero.
*
*   NOTES
*       Two likely reasons for failures are invalid unit numbers or
*       no more free signal bits for this task.
*
*       While this function first appears in V37 amiga.lib, it works
*       on Kickstart V33 and higher.
*
*   SEE ALSO
*       timer.device/TR_ADDREQUEST,
*       timer.device/TR_WAITUNTIL,
*       timer.device/WaitUnitl()
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

_TimeDelay:
                move.l  d2,-(sp)
                move.l  8(sp),d0                ; timer unit
                move.l  12(sp),d1               ; seconds
                move.l  16(sp),d2               ; useconds
                bsr.s   TimeDelay
                move.l  (sp)+,d2
                rts

TimeDelay:
                subq.l  #TV_SIZE,sp             ; make a timeval
                move.l  sp,a0
                movem.l d1/d2,(a0)              ; set timeval
                move.l  #TR_ADDREQUEST,d1       ; set the device command
                bsr.s   DoTimer                 ; gotoit
                addq.l  #TV_SIZE,sp
                rts


*****i* timer.device/internal/DoTimer ********************************
*
*   NAME
*       DoTimer -- "DoIO" a timer.device command. (V36)
*
*   SYNOPSIS
*       Error = DoTimer( TimeVal, Unit, Command )
*       D0               A0       D0    D1
*
*       LONG DoTimer( struct timeval *, LONG, WORD );
*
*   FUNCTION
*       Create a timerequest and reply port then perform the
*       device command via DoIO.
*
*   INPUTS
*       TimeVal -- pointer to a timeval structure. Contents copied
*           into IORequest before DoIO.
*       Unit -- timer.device unit to open for this command.
*       Command -- command to perform.
*
*   RESULTS
*       TimeVal -- filled from timeval structure of IORequest after
*           command completes.
*       Error -- will be zero if all went well; otherwise, non-zero.
*
*   NOTES
*
*   SEE ALSO
*
*   BUGS
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*

DoTimer:
                movem.l d2-d3/a2-a4/a6,-(sp)
                move.l  4,a6                    ; find execbase
                move.l  a0,a4                   ; timeval
                move.l  d0,d2                   ; unit
                move.l  d1,d3                   ; command

                lea     -MP_SIZE-IOTV_SIZE(sp),sp
                movea.l sp,a3                   ; IORequest
                lea     IOTV_SIZE(a3),a2        ; reply port

                move.b  #NT_MSGPORT,LN_TYPE(a2) ; set up the node type
                clr.b   LN_PRI(a2)
                clr.l   LN_NAME(a2)             ; no name
                clr.b   MP_FLAGS(a2)

                moveq.l #-1,d0
                CALLSYS AllocSignal             ; get port signal bit
                cmp.b   #-1,d0
                beq     DTError1
                move.b  d0,MP_SIGBIT(a2)

                suba.l  a1,a1                   ; find this task
                CALLSYS FindTask
                move.l  d0,MP_SIGTASK(a2)       ; and fill in this task
                lea     MP_MSGLIST(a2),a0
                NEWLIST a0                      ; initialise message port

                lea     timerName,A0
                movea.l a3,a1                   ; this IORequest
                move.l  d2,d0                   ; this unit
                moveq.l #0,d1                   ; no special flags
                CALLSYS OpenDevice
                tst.l   d0                      ; any errors ?
                bne.s   DTError2                ; yep

                move.l  a2,MN_REPLYPORT(a3)     ; replies go to this port
                
                movea.l a3,a1
                move.w  d3,IO_COMMAND(a1)       ; this command

                move.l  TV_SECS(a4),IOTV_TIME+TV_SECS(a1)
                move.l  TV_MICRO(a4),IOTV_TIME+TV_MICRO(a1)

                CALLSYS DoIO                    ; doit

                move.l  IOTV_TIME+TV_SECS(a3),TV_SECS(a4)
                move.l  IOTV_TIME+TV_MICRO(a3),TV_MICRO(a4)

                movea.l a3,a1
                CALLSYS CloseDevice             ; close the device

                moveq.l #0,d0
                move.b  MP_SIGBIT(a2),d0
                CALLSYS FreeSignal              ; free port signal

DTCleanup:
                lea.l   IOTV_SIZE+MP_SIZE(sp),sp        ; clean up stack
                movem.l (sp)+,d2-d3/a2-a4/a6
                rts

DTError2:
                moveq.l #0,d0
                move.b  MP_SIGBIT(a2),d0
                CALLSYS FreeSignal              ; free port signal

DTError1:
                moveq.l #-1,d0
                bra.s   DTCleanup


        END


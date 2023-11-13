        include 'exec/types.i'
        include 'exec/memory.i'
        include 'exec/nodes.i'
        include 'exec/lists.i'
        include 'exec/macros.i'
        include 'exec/alerts.i'
        include 'exec/libraries.i'

        section Pools

*******************************************************************************
*  ****************************              *******************************  *
* ****************************** EXEC Pools ********************************* *
*  ****************************              *******************************  *
*******************************************************************************

 STRUCTURE PrivatePool,0
        STRUCT  PP_List,MLH_SIZE        ; The pool's free list
        ULONG   PP_Flags                ; The pool's memory type
        ULONG   PP_Size                 ; The pool's puddle size
        ULONG   PP_Thresh               ; The pool's threshhold size
        LABEL   PP_SIZEOF

 STRUCTURE LargePuddle,0
        STRUCT  LP_Node,MLN_SIZE        ; This puddle's node
        ULONG   LP_Flag                 ; NULL if node is large puddle
        LABEL   LP_SIZEOF

*
* Macro used to check if EXEC is the right place to do this...
*
CHECK           MACRO                           ; Check if EXEC is available...
_Asm\1:         xdef    _Asm\1
                cmpi.w  #39,LIB_VERSION(a6)     ; Do we have V39 EXEC?
                bcs.s   Old_\1                  ; If too old, exit...
                JMPLIB  \1                      ; If new enough, call ROM
Old_\1          ; Fall into Function...
                ENDM

*
******* pools.lib/LibCreatePool ***********************************************
*
*    NAME
*       LibCreatePool -- Generate a private memory pool header (V33)
*
*    SYNOPSIS
*       newPool=LibCreatePool(memFlags,puddleSize,threshSize)
*       a0                    d0       d1         d2
*
*       void *LibCreatePool(ULONG,ULONG,ULONG);
*
*    FUNCTION
*       This function is a copy of the pool functions in V39 and up of
*       EXEC.  In fact, if you are running in V39, this function will
*       notice and call the EXEC function.  This function works in
*       V33 and up (1.2) Amiga system.
*
*       The C code interface is _LibCreatePool() and takes its arguments
*       from the stack just like the C code interface for CreatePool()
*       in amiga.lib.  The assembly code interface is with the symbol
*       _AsmCreatePool: and takes the parameters in registers with the
*       additional parameter of ExecBase being in a6 which can be used
*       from SAS/C 6 by a prototype of:
*
*       void * __asm AsmCreatePool(register __d0 ULONG,
*                                  register __d1 ULONG,
*                                  register __d2 ULONG,
*                                  register __a6 struct ExecBase *);
*
*       Allocate and prepare a new memory pool header.  Each pool is a
*       separate tracking system for memory of a specific type.  Any number
*       of pools may exist in the system.
*
*       Pools automatically expand and shrink based on demand.  Fixed sized
*       "puddles" are allocated by the pool manager when more total memory
*       is needed.  Many small allocations can fit in a single puddle.
*       Allocations larger than the threshSize are allocation in their own
*       puddles.
*
*       At any time individual allocations may be freed.  Or, the entire
*       pool may be removed in a single step.
*
*    INPUTS
*       memFlags - a memory flags specifier, as taken by AllocMem.
*       puddleSize - the size of Puddles...
*       threshSize - the largest allocation that goes into normal puddles
*                    This *MUST* be less than or equal to puddleSize
*                    (LibCreatePool() will fail if it is not)
*
*    RESULT
*       The address of a new pool header, or NULL for error.
*
*    SEE ALSO
*       DeletePool(), AllocPooled(), FreePooled(), exec/memory.i,
*       LibDeletePool(), LibAllocPooled(), LibFreePooled()
*
*******************************************************************************
                CHECK   CreatePool
                sub.l   a0,a0           ; Clear a0...
                cmp.l   d2,d1           ; Check threshold for too large...
                bcs.s   cp_BadThresh    ; Threshold is larger than puddle?!!
                move.l  d0,-(sp)        ; Save the flags...
                moveq.l #MEM_BLOCKMASK,d0       ; Get size mask...
                add.l   d0,d1           ; Round to the
                not.b   d0              ; ... memory block size
                and.b   d0,d1           ; ... before we save it
                move.l  d1,-(sp)        ; Save the puddle size...
                moveq.l #0,d1           ; Get *ANY* memory for the header
                moveq.l #PP_SIZEOF,d0   ; Size of the pool header...
                JSRLIB  AllocMem        ; Get the memory...
                move.l  (sp)+,d1        ; Get back from the stack...
                move.l  (sp)+,a0        ; the data needed...
                tst.l   d0              ; Check it out
                beq.s   cp_NoMem        ; If no memory, exit...
                exg     d0,a0           ; Swap A0/D0
                NEWLIST a0              ; (a0 - Preserved)
                lea     PP_Flags(a0),a1 ; Get a pointer to the flags...
                move.l  d0,(a1)+        ; Set up the flags
                move.l  d1,(a1)+        ; Set up the size
                move.l  d2,(a1)+        ; Set up the thresh
cp_BadThresh:   move.l  a0,d0           ; Get the return result...
cp_NoMem:       rts

******* pools.lib/LibDeletePool ***********************************************
*
*    NAME
*       LibDeletePool --  Drain an entire memory pool (V33)
*
*    SYNOPSIS
*       LibDeletePool(poolHeader)
*                     a0
*
*       void LibDeletePool(void *);
*
*    FUNCTION
*       This function is a copy of the pool functions in V39 and up of
*       EXEC.  In fact, if you are running in V39, this function will
*       notice and call the EXEC function.  This function works in
*       V33 and up (1.2) Amiga system.
*
*       The C code interface is _LibDeletePool() and takes its arguments
*       from the stack just like the C code interface for DeletePool()
*       in amiga.lib.  The assembly code interface is with the symbol
*       _AsmDeletePool: and takes the parameters in registers with the
*       additional parameter of ExecBase being in a6 which can be used
*       from SAS/C 6 by a prototype of:
*
*       void __asm AsmDeletePool(register __a0 void *,
*                                register __a6 struct ExecBase *);
*
*       Frees all memory in all puddles of the specified pool header, then
*       deletes the pool header.  Individual free calls are not needed.
*
*    INPUTS
*       poolHeader - as returned by LibCreatePool().
*
*    SEE ALSO
*       CreatePool(), AllocPooled(), FreePooled(),
*       LibCreatePool(), LibAllocPooled(), LibFreePooled()
*
*******************************************************************************
                CHECK   DeletePool
                move.l  a0,d0           ; Check for death
                beq.s   dp_NoPool       ; If no pool...
                movem.l a2/d2,-(sp)     ; Save on the stack...
                move.l  (a0),a2         ; Point at first node
dp_puddles:     move.l  (a2),d2         ; Check if next puddle is NULL
                beq.s   dp_header       ; If NULL, we are at the header...
                move.l  a2,a1           ; Get ready for FreeVec
                move.l  d2,a2           ; Get my next pointer
*
                move.l  -(a1),d0        ; Do the FreeVec() myself...
                JSRLIB  FreeMem
*
                bra.s   dp_puddles      ; And loop for more...
dp_NoPool:      rts
*
* Ok, we went through the whole list and a2 is now pointing at MLH_TAIL
* so subtract 4 and then free it too.
*
dp_header:      move.l  a2,a1           ; Get into register...
                subq.l  #4,a1           ; Back to top of header...
                movem.l (sp)+,a2/d2     ; Get registers back from the stack
                moveq.l #PP_SIZEOF,d0   ; Get size of pool header...
                JMPLIB  FreeMem         ; Release the header

******* pools.lib/LibAllocPooled **********************************************
*
*    NAME
*       LibAllocPooled -- Allocate memory with the pool manager (V33)
*
*    SYNOPSIS
*       memory=LibAllocPooled(poolHeader,memSize)
*       d0                    a0         d0
*
*       void *LibAllocPooled(void *,ULONG);
*
*    FUNCTION
*       This function is a copy of the pool functions in V39 and up of
*       EXEC.  In fact, if you are running in V39, this function will
*       notice and call the EXEC function.  This function works in
*       V33 and up (1.2) Amiga system.
*
*       The C code interface is _LibAllocPooled() and takes its arguments
*       from the stack just like the C code interface for AllocPooled()
*       in amiga.lib.  The assembly code interface is with the symbol
*       _AsmAllocPooled: and takes the parameters in registers with the
*       additional parameter of ExecBase being in a6 which can be used
*       from SAS/C 6 by a prototype of:
*
*       void * __asm AsmAllocPooled(register __a0 void *,
*                                   register __d0 ULONG,
*                                   register __a6 struct ExecBase *);
*
*       Allocate memSize bytes of memory, and return a pointer. NULL is
*       returned if the allocation fails.
*
*       Doing a LibDeletePool() on the pool will free all of the puddles
*       and thus all of the allocations done with LibAllocPooled() in that
*       pool.  (No need to LibFreePooled() each allocation)
*
*    INPUTS
*       memSize - the number of bytes to allocate
*       poolHeader - a specific private pool header.
*
*    RESULT
*       A pointer to the memory, or NULL.
*       The memory block returned is long word aligned.
*
*    NOTES
*       The pool function do not protect an individual pool from
*       multiple accesses.  The reason is that in most cases the pools
*       will be used by a single task.  If your pool is going to
*       be used by more than one task you must Semaphore protect
*       the pool from having more than one task trying to allocate
*       within the same pool at the same time.  Warning:  Forbid()
*       protection *will not work* in the future.  *Do NOT* assume
*       that we will be able to make it work in the future.  LibAllocPooled()
*       may well break a Forbid() and as such can only be protected
*       by a semaphore.
*
*       To track sizes yourself, the following code can be used:
*       *Assumes a6=ExecBase*
*
*       ;
*       ; Function to do AllocVecPooled(Pool,memSize)
*       ;
*       AllocVecPooled: addq.l  #4,d0           ; Get space for tracking
*                       move.l  d0,-(sp)        ; Save the size
*                       jsr     LibAllocPooled  ; Call pool...
*                       move.l  (sp)+,d1        ; Get size back...
*                       tst.l   d0              ; Check for error
*                       beq.s   avp_fail        ; If NULL, failed!
*                       move.l  d0,a0           ; Get pointer...
*                       move.l  d1,(a0)+        ; Store size
*                       move.l  a0,d0           ; Get result
*       avp_fail:       rts                     ; return
*
*       ;
*       ; Function to do LibFreeVecPooled(pool,memory)
*       ;
*       FreeVecPooled:  move.l  -(a1),d0        ; Get size / ajust pointer
*                       jmp     LibFreePooled
*
*    SEE ALSO
*       FreePooled(), CreatePool(), DeletePool(),
*       LibFreePooled(), LibCreatePool(), LibDeletePool()
*
*******************************************************************************
                CHECK   AllocPooled
                move.l  d0,d1           ; Save for a moment (and check)
                beq.s   ap_NoPool       ; If NULL size, silly call ;^)
                move.l  a0,d0           ; Get pool header
                beq.s   ap_NoPool       ; If NULL, we exit
*
* Ok, so we look like it will actually fly.  So set up our registers as needed
*
                movem.l a2/a3/d2/d3,-(sp)       ; Save for later...
                move.l  d1,d2                   ; Save of the needed allocation
                move.l  a0,a2                   ; Pool for the allocation
                cmp.l   PP_Thresh(a2),d2        ; Check against threshold
                bcc.s   ap_largeAlloc           ; If larger, we do large alloc
*
* Ok, now we need to do a search for space...
*
ap_LookAgain:   move.l  (a2),d3                 ; Get first free list...
ap_Looking:     move.l  d3,a3                   ; Next node...
                move.l  (a3),d3                 ; Get next node...
                beq.s   ap_newPuddle            ; If end, we need a new puddle
                move.l  a3,a0                   ; Get it ready...
                tst.l   LP_Flag(a0)             ; If large puddle, end of list
                beq.s   ap_newPuddle            ; (large puddle are at tail...)
                move.l  d2,d0                   ; Get the allocation size
                JSRLIB  Allocate                ; Try to allocate
                tst.l   d0                      ; We need to test...
                beq.s   ap_Looking              ; We are still looking...
*
* Now, we need to do the clearing if we have MEMF_CLEAR set...
*
                move.l  d0,a0                   ; Get into a0
                move.l  PP_Flags(a2),d0         ; Get flags
                btst.l  #MEMB_CLEAR,d0          ; Are we MEMF_CLEAR?
                beq.s   ap_Exit                 ; If not, exit
                move.l  a0,a1                   ; Get data block...
                move.l  d2,d1                   ; Get size...
                addq.l  #7,d1                   ; To block size rounding
                lsr.l   #3,d1                   ; Number of 8-byte chunks
                subq.l  #1,d1                   ; minus 1...
                move.w  d1,d0                   ; Get low word of size
                swap    d1                      ; Get high word of size
                moveq.l #0,d3                   ; Get a NULL...
ap_ClearLoop:   move.l  d3,(a1)+                ; Clear 4 bytes
                move.l  d3,(a1)+                ; and the next 4...
                dbra.s  d0,ap_ClearLoop         ; Clear loop...
                dbra.s  d1,ap_ClearLoop         ; outer (large) loop
*
* Now return pointer to data...
*
ap_Exit:        move.l  a0,d0                   ; Point at memory available
ap_Restore:     movem.l (sp)+,a2/a3/d2/d3       ; Restore
ap_NoPool:      rts
*
* For allocations larger than the threshold...
*
ap_largeAlloc:  move.l  d2,d0                   ; Size of allocation
                addq.l  #8,d0                   ; My list node (minnode)
                addq.l  #4,d0                   ; My flag (That this is large)
                move.l  PP_Flags(a2),d1         ; Get the flags
                bsr     AllocVec                ; Allocate it
                tst.l   d0                      ; Check for error
                beq.s   ap_Restore              ; If no pool...
                move.l  d0,a1                   ; Get the node
                move.l  a2,a0                   ; Get the list
                ADDTAIL                         ; Place it onto the list
                addq.l  #8,a1                   ; Point past the node
                clr.l   (a1)+                   ; Clear my flag...
                move.l  a1,d0                   ; Put it into d0...
                bra.s   ap_Restore              ; Go and exit with memory
*
* Ok, so we did not find space in the current puddles, so try to make a new
* new puddle...
*
ap_newPuddle:   move.l  PP_Flags(a2),d1         ; Memory flags
                move.l  PP_Size(a2),d0          ; Get size of puddle to make
                add.l   #MH_SIZE+4,d0           ; Size of puddle header
                bsr.s   AllocVec                ; Allocate the memory
                tst.l   d0                      ; Did we get it?
                beq.s   ap_Restore              ; If no memory...
                move.l  d0,a3                   ; Set up puddle pointer...
                move.l  d0,a1                   ; Get into a1
                move.l  a2,a0                   ; Pool header
                ADDHEAD                         ; Add it to the pool
                moveq.l #NT_MEMORY,d0           ; Get memory type...
                move.b  d0,LN_TYPE(a3)          ; Set up the type
                move.b  d0,LN_PRI(a3)           ; (Cheat: Priority field ;^)
                lea     PoolName(pc),a0         ; Get string "Pool"
                move.l  a0,LN_NAME(a3)          ; Name the pool...
                move.l  PP_Flags(a2),d1         ; Get flags
                move.w  d1,MH_ATTRIBUTES(a3)    ; Set the attributes
                lea     MH_SIZE+4(a3),a0        ; Point at first free...
***
*** Now need to check to make sure address is double-long alligned
***
                moveq.l #7,d1                   ; Get d1 to be 7...
                not.l   d1                      ; Invert... (bottom 3 bits 0)
                move.l  a0,d0                   ; Get address into register...
                and.l   d0,d1                   ; Mask off the bottom 3
                move.l  d1,a0                   ; Address now double-long
***
                move.l  PP_Size(a2),d0          ; Chunk byte size
                move.l  a0,MH_FIRST(a3)         ; Point at the first free
                move.l  a0,MH_LOWER(a3)         ; Point at the lower bound
                move.l  d0,MH_FREE(a3)          ; Set the FREE space
                clr.l   MC_NEXT(a0)             ; Clear next pointer
                move.l  d0,MC_BYTES(a0)         ; Set the chunk size
                add.l   d0,a0                   ; Add in the size
                move.l  a0,MH_UPPER(a3)         ; Point at end of puddle+1
                bra     ap_LookAgain            ; Look again (new puddle ;^)
*
* To support 1.3, we have a local AllocVec routine
*
AllocVec:       addq.l  #4,d0                   ; Make size 4 bytes larger
                move.l  d0,-(sp)                ; Save size
                JSRLIB  AllocMem                ; Allocate it...
                tst.l   d0                      ; Did we get the memory?
                beq.s   ac_fail                 ; If not, fail...
                move.l  d0,a0                   ; Get the memory pointer...
                move.l  (sp),(a0)+              ; Put size in first longword
                move.l  a0,d0                   ; get new memory pointer
ac_fail:        addq.l  #4,sp                   ; Clean up stack
ac_exit:        rts                             ; return...
*
******* pools.lib/LibFreePooled ***********************************************
*
*    NAME
*       LibFreePooled -- Free pooled memory  (V33)
*
*    SYNOPSIS
*       LibFreePooled(poolHeader,memory,memSize)
*                     a0         a1     d0
*
*       void LibFreePooled(void *,void *,ULONG);
*
*    FUNCTION
*       This function is a copy of the pool functions in V39 and up of
*       EXEC.  In fact, if you are running in V39, this function will
*       notice and call the EXEC function.  This function works in
*       V33 and up (1.2) Amiga system.
*
*       The C code interface is _LibFreePooled() and takes its arguments
*       from the stack just like the C code interface for FreePooled()
*       in amiga.lib.  The assembly code interface is with the symbol
*       _AsmFreePooled: and takes the parameters in registers with the
*       additional parameter of ExecBase being in a6 which can be used
*       from SAS/C 6 by a prototype of:
*
*       void __asm AsmFreePooled(register __a0 void *,
*                                register __a1 void *,
*                                register __d0 ULONG,
*                                register __a6 struct ExecBase *);
*
*       Deallocates memory allocated by LibAllocPooled().  The size of the
*       allocation *MUST* match the size given to LibAllocPooled().
*       The reason the pool functions do not track individual allocation
*       sizes is because many of the uses of pools have small allocation
*       sizes and the tracking of the size would be a large overhead.
*
*       Only memory allocated by LibAllocPooled() may be freed with this
*       function!
*
*       Doing a LibDeletePool() on the pool will free all of the puddles
*       and thus all of the allocations done with LibAllocPooled() in that
*       pool.  (No need to LibFreePooled() each allocation)
*
*    INPUTS
*       memory - pointer to memory allocated by AllocPooled.
*       poolHeader - a specific private pool header.
*
*    NOTES
*       The pool function do not protect an individual pool from
*       multiple accesses.  The reason is that in most cases the pools
*       will be used by a single task.  If your pool is going to
*       be used by more than one task you must Semaphore protect
*       the pool from having more than one task trying to allocate
*       within the same pool at the same time.  Warning:  Forbid()
*       protection *will not work* in the future.  *Do NOT* assume
*       that we will be able to make it work in the future.  LibFreePooled()
*       may well break a Forbid() and as such can only be protected
*       by a semaphore.
*
*       To track sizes yourself, the following code can be used:
*       *Assumes a6=ExecBase*
*
*       ;
*       ; Function to do AllocVecPooled(Pool,memSize)
*       ;
*       AllocVecPooled: addq.l  #4,d0           ; Get space for tracking
*                       move.l  d0,-(sp)        ; Save the size
*                       jsr     LibAllocPooled  ; Call pool...
*                       move.l  (sp)+,d1        ; Get size back...
*                       tst.l   d0              ; Check for error
*                       beq.s   avp_fail        ; If NULL, failed!
*                       move.l  d0,a0           ; Get pointer...
*                       move.l  d1,(a0)+        ; Store size
*                       move.l  a0,d0           ; Get result
*       avp_fail:       rts                     ; return
*
*       ;
*       ; Function to do LibFreeVecPooled(pool,memory)
*       ;
*       FreeVecPooled:  move.l  -(a1),d0        ; Get size / ajust pointer
*                       jmp     LibFreePooled
*
*    SEE ALSO
*       AllocPooled(), CreatePool(), DeletePool(),
*       LibAllocPooled(), LibCreatePool(), LibDeletePool()
*
*******************************************************************************
                CHECK   FreePooled
                move.l  a0,d1                   ; Check if NULL pool
                beq.s   fp_NoMem                ; If NULL, we exit...
                move.l  a1,d1                   ; Check if NULL memory
                beq.s   fp_NoMem                ; If NULL, we exit...
                cmp.l   PP_Thresh(a0),d0        ; Now check threshold
                bcc.s   fp_largeFree            ; If large one, do that...
                move.l  a3,-(sp)                ; Save some registers...
                move.l  (a0),d1                 ; Get ready for loop
fp_Looking:     move.l  d1,a3                   ; Get node
                move.l  (a3),d1                 ; Get next node
                beq.s   fp_Error                ; If not found we be in trouble
                tst.l   LP_Flag(a3)             ; Is it a large puddle
                beq.s   fp_Error                ; (They are at tail)
                cmp.l   MH_LOWER(a3),a1         ; Are we in this one
                bcs.s   fp_Looking              ; If not...
                cmp.l   MH_UPPER(a3),a1         ; ...just keep
                bcc.s   fp_Looking              ; ...on looking
                move.l  a3,a0                   ; Get memory header
                JSRLIB  Deallocate              ; Free this memory...
*******
* It may be usefull to bubble up pools that seem to be
* getting used more...
*******
*
* Now, bubble the node up one on the list (a performance tuning issue)
* We buble the node in a3
* We trash a0, a1, and d1
*
bubble:         move.l  LN_PRED(a3),a1          ; Get previous node
                move.l  LN_PRED(a1),d1          ; Get node before that
                beq.s   nobubble                ; If NULL, no bubble...
*
* This code here moves the node in a3 above the node in a1
* The node a1 is the node just above the node a3 before this.
*
                move.l  d1,a0                   ; Get node-2
                move.l  a3,LN_SUCC(a0)          ; Point down...
                move.l  a0,LN_PRED(a3)          ; Point up...
                move.l  a3,LN_PRED(a1)          ; Swap it...
                move.l  LN_SUCC(a3),a0          ; Get next node
                move.l  a0,LN_SUCC(a1)          ; Point down
                move.l  a1,LN_PRED(a0)          ; Point up...
                move.l  a1,LN_SUCC(a3)          ; Into both nodes
*
* Ok, a3 is now one higher
*
nobubble:
*
                move.l  a3,a1                   ; Get node into a1
                move.l  (sp)+,a3                ; Restore a3

* BUG!          move.l  MH_SIZE(a1),d0          ; Get size... (BUG!)

                move.l  MH_FREE(a1),d0          ; Get size...
                add.l   MH_LOWER(a1),d0         ; Add in lower bound
                sub.l   MH_UPPER(a1),d0         ; Subtract upper bound

                beq.s   fp_FreeNode             ; If NULL, it is empty
fp_NoMem:       rts
fp_largeFree:   subq.l  #4,a1           ; The special flag      :speed reasons
                subq.l  #8,a1           ; The "node" structure
*
* So free the node in a1 (after removing it first!
*
fp_FreeNode:    move.l  a1,d0           ; Save the value of a1
                REMOVE                  ; Remove (a1) from list (a0 trashed)
                move.l  d0,a1           ; Get pointer back
*
                move.l  -(a1),d0        ; Do the FreeVec() myself...
                JMPLIB  FreeMem         ; (tail recursion...  rts)
*
***************
fp_Error:       move.l  (sp)+,a3        ; Restore a3
                move.l  d7,-(sp)        ; Save
                move.l  #AN_BadFreeAddr,d7
                JSRLIB  Alert           ; Tell about bad memory free...
                move.l  (sp)+,d7
                rts

                dc.b    0               ; Make the name ODD
PoolName:       dc.b    'Pool',0        ; Now word aligned again...
*
*******************************************************************************
*
                END

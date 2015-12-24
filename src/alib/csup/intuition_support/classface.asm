*  classface.asm - Intuition object/class method invocation
*  converts "standard" C calling conventions to appropriate
*  hook conventions.
*
* Code in here "freezes" these facts (and no others):
*       - pointer to an object's class immediately precedes the object pointer
*       - pointer to a class's superclass is (h_SIZEOF+4) into the class

        INCLUDE 'exec/types.i'
        INCLUDE 'exec/nodes.i'

        INCLUDE 'utility/hooks.i'

* varargs interfaces for invoking method functions
        xdef    _DoMethod
        xdef    _DoSuperMethod
        xdef    _CoerceMethod
        xdef    _SetSuperAttrs

* corresponding method invocations for a pre-packaged parameter
* "message" packet
        xdef    _DoMethodA
        xdef    _DoSuperMethodA
        xdef    _CoerceMethodA

* DoMethod( o, method_id, param1, param2, ... )
* Invoke upon an object the method function defined by an object's class.
* This function (with its "short form" DoMethodA() ) is the only one that
* you should use unless you are implementing a class.
*
_DoMethod:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        move.l  8(sp),a2        ; object
        move.l  a2,d0           ; be safe
        beq.s   cmnullreturn
        lea     12(sp),a1       ; message
        move.l  -4(a2),a0       ; object class ptr precedes object

        bra.s   cminvoke        ; will cleanup a2
        ; ----- don't return here

* DoSuperMethod( cl, o, method_id, param1, param2, ... )
* Invoke upon an object the method defined for the superclass
* of the class specified.  In a class implementation, you
* are passed a pointer to the class you are implementing, which
* you pass to this function to send a message to the object
* considered as a member of your superclass.
*
_DoSuperMethod:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        movem.l 8(sp),a0/a2     ; class, object
        move.l  a2,d0           ; be safe (object)
        beq.s   cmnullreturn
        move.l  a0,d0           ; be safe (class)
        beq.s   cmnullreturn
        lea     16(sp),a1       ; message
        move.l  h_SIZEOF+4(a0),a0       ; substitute superclass

        bra.s   cminvoke        ; will cleanup a2
        ; ----- don't return here

* CoerceMethod( cl, o, method_id, param1, param2, ... );
* Invoke upon the given object a method function for whatever
* specified class.  This is sort of the primitive basis behind
* DoMethod and DoSuperMethod.
*
_CoerceMethod:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        movem.l 8(sp),a0/a2     ; get hook and object
        move.l  a2,d0           ; be safe (object)
        beq.s   cmnullreturn
        move.l  a0,d0           ; be safe (class)
        beq.s   cmnullreturn
        lea     16(sp),a1       ; varargs version
        ; --- registers ready, now call hook
        bra.s   cminvoke        ; ----- don't return here


* CoerceMethodA( a0: cl, a2: o, a1: msg )
* This is CoerceMethod for prepackaged "message" packets
*
_CoerceMethodA:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        movem.l 8(sp),a0/a2     ; get class and object
        move.l  a2,d0
        beq.s   cmnullreturn
        move.l  a0,d0
        beq.s   cmnullreturn
        move.l  16(sp),a1       ; get msg
        ; --- registers ready, now call hook

        ; --- performs call to hook in A0 and restores a2
cminvoke:
        pea.l   cmreturn
        move.l  h_Entry(a0),-(sp)
        rts
cmnullreturn:
        moveq.l #0,d0
cmreturn:
        move.l  (sp)+,a2
        rts

        
* DoMethodA( o, msg )
* This is DoMethod for prepackaged "message" packets
_DoMethodA:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        move.l  8(sp),a2        ; object
        move.l  a2,d0
        beq.s   cmnullreturn
        move.l  12(sp),a1       ; message
        move.l  -4(a2),a0       ; object class precedes object

        bra.s   cminvoke        ; will cleanup a2
        ; ----- don't return here


* DoSuperMethodA( cl, o, msg )
* This is DoSuperMethod for prepackaged "message" packets
_DoSuperMethodA:
        move.l  a2,-(a7)        ; rely on a6 being preserved
        movem.l 8(sp),a0/a2     ; class, object
        move.l  a2,d0
        beq.s   cmnullreturn
        move.l  a0,d0
        beq.s   cmnullreturn
        move.l  16(sp),a1       ; message
        move.l  h_SIZEOF+4(a0),a0       ; substitute superclass

        bra.s   cminvoke        ; will cleanup a2
        ; ----- don't return here


* SetSuperAttrs( cl, o, tag1, data1, ..., TAG_END );
* A useful varargs conversion to the proper OM_SET method.
_SetSuperAttrs:
        move.l  a2,-(a7)        ; save

        movem.l 8(sp),a0/a2     ; class, object
        move.l  a2,d0           ; be safe (object)
        beq.s   cmnullreturn
        move.l  a0,d0           ; be safe (class)
        beq.s   cmnullreturn

        move.l  h_SIZEOF+4(a0),a0       ; substitute superclass

        ; ----- build msg packet on the stack
        move.l  #0,-(sp)        ; NULL GadgetInfo
        pea.l   4+16(sp)        ; address of tags on deeper stack
        move.l  #$103,-(sp)     ; MethodID OM_SET
        lea.l   (sp),a1         ; put the address of the whole thing in a1

        pea.l   ssaret(pc)
        move.l  h_Entry(a0),-(sp)
        rts

ssaret:
        ; ----- return here to clean up stack
        lea.l   12(sp),sp       ; pop off three long parameters
        move.l  (sp)+,a2        ; pop/restore original a2
        rts


        end

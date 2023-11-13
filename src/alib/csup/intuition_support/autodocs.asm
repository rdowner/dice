******* amiga.lib/CallHook ***************************************************
*
*   NAME
*       CallHook -- Invoke a hook given a message on the stack.
*
*   SYNOPSIS
*       result = CallHook( hookPtr, obj, ... )
*
*       ULONG CallHook( struct Hook *, Object *, ... );
*
*   FUNCTION
*       Like CallHookA(), CallHook() invoke a hook on the supplied
*       hook-specific data (an "object") and a parameter packet ("message").
*       However, CallHook() allows you to build the message on your stack.
*
*   INPUTS
*       hookPtr - A system-standard hook
*       obj - hook-specific data object
*       ... - The hook-specific message you wish to send.  The hook is
*           expecting a pointer to the message, so a pointer into your
*           stack will be sent.
*
*   RESULT
*       result - a hook-specific result.
*
*   NOTES
*       This function first appeared in the V37 release of amiga.lib.
*       However, it does not depend on any particular version of the OS,
*       and works fine even in V34.
*
*   EXAMPLE
*       If your hook's message was
*
*           struct myMessage
*           {
*               ULONG mm_FirstGuy;
*               ULONG mm_SecondGuy;
*               ULONG mm_ThirdGuy;
*           };
*
*       You could write:
*
*           result = CallHook( hook, obj, firstguy, secondguy, thirdguy );
*
*       as a shorthand for:
*
*           struct myMessage msg;
*
*           msg.mm_FirstGuy = firstguy;
*           msg.mm_SecondGuy = secondguy;
*           msg.mm_ThirdGuy = thirdguy;
*
*           result = CallHookA( hook, obj, &msg );
*
*   SEE ALSO
*       CallHookA(), utility.library/CallHookPkt(), <utility/hooks.h>
*
******************************************************************************

******* amiga.lib/CallHookA **************************************************
*
*   NAME
*       CallHookA -- Invoke a hook given a pointer to a message.
*
*   SYNOPSIS
*       result = CallHookA( hookPtr, obj, message )
*
*       ULONG CallHook( struct Hook *, Object *, APTR );
*
*   FUNCTION
*       Invoke a hook on the supplied hook-specific data (an "object")
*       and a parameter packet ("message").  This function is equivalent
*       to utility.library/CallHookPkt().
*
*   INPUTS
*       hookPtr - A system-standard hook
*       obj - hook-specific data object
*       message - The hook-specific message you wish to send
*
*   RESULT
*       result - a hook-specific result.
*
*   NOTES
*       This function first appeared in the V37 release of amiga.lib.
*       However, it does not depend on any particular version of the OS,
*       and works fine even in V34.
*
*   SEE ALSO
*       CallHook(), utility.library/CallHookPkt(), <utility/hooks.h>
*
******************************************************************************

******* amiga.lib/HookEntry **************************************************
*
*   NAME
*       HookEntry -- Assembler to HLL conversion stub for hook entry.
*
*   SYNOPSIS
*       result = HookEntry( struct Hook *, Object *, APTR )
*       D0                  A0             A2        A1
*
*   FUNCTION
*       By definition, a standard hook entry-point must receive the
*       hook in A0, the object in A2, and the message in A1.  If your
*       hook entry-point is written in a high-level language and is
*       expecting its parameters on the stack, then HookEntry() will
*       put the three parameters on the stack and invoke the function
*       stored in the hook h_SubEntry field.
*
*       This function is only useful to hook implementers, and is
*       never called from C.
*
*   INPUTS
*       hook - pointer to hook being invoked
*       object - pointer to hook-specific data
*       msg - pointer to hook-specific message
*
*   RESULT
*       result - a hook-specific result.
*
*   NOTES
*       This function first appeared in the V37 release of amiga.lib.
*       However, it does not depend on any particular version of the OS,
*       and works fine even in V34.
*
*   EXAMPLE
*       If your hook dispatcher is this:
*
*       dispatch( struct Hook *hookPtr, Object *obj, APTR msg )
*       {
*           ...
*       }
*
*       Then when you initialize your hook, you would say:
*
*       myhook.h_Entry = HookEntry;     /* amiga.lib stub */
*       myhook.h_SubEntry = dispatch;   /* HLL entry */
*
*   SEE ALSO
*       CallHook(), CallHookA(), <utility/hooks.h>
*       
******************************************************************************

******* amiga.lib/DoMethodA **************************************************
*
*   NAME
*       DoMethodA -- Perform method on object.
*
*   SYNOPSIS
*       result = DoMethodA( obj, msg )
*
*       ULONG DoMethodA( Object *, Msg );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object.  The message is invoked on the
*       object's true class.
*
*   INPUTS
*       obj - pointer to boopsi object
*       msg - pointer to method-specific message to send
*
*   RESULT
*       result - specific to the message and the object's class.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*       Some early example code may refer to this function as DM().
*
*   SEE ALSO
*       DoMethod(), CoerceMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/DoMethod ***************************************************
*
*   NAME
*       DoMethod -- Perform method on object.
*
*   SYNOPSIS
*       result = DoMethod( obj, MethodID, ... )
*
*       ULONG DoMethod( Object *, ULONG, ... );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object.  The message is invoked on the
*       object's true class.  Equivalent to DoMethodA(), but allows
*       you to build the message on the stack.
*
*   INPUTS
*       obj - pointer to boopsi object
*       MethodID - which method to send (see <intuition/classusr.h>)
*       ... - method-specific message built on the stack
*
*   RESULT
*       result - specific to the message and the object's class.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*
*   SEE ALSO
*       DoMethodA(), CoerceMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/CoerceMethodA *********************************************************
*
*   NAME
*       CoerceMethodA -- Perform method on coerced object.
*
*   SYNOPSIS
*       result = CoerceMethodA( cl, obj, msg )
*
*       ULONG CoerceMethodA( struct IClass *, Object *, Msg );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object, as though it were the specified
*       class.
*
*   INPUTS
*       cl - pointer to boopsi class to receive the message
*       obj - pointer to boopsi object
*       msg - pointer to method-specific message to send
*
*   RESULT
*       result - class and message-specific result.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*       Some early example code may refer to this function as CM().
*
*   SEE ALSO
*       CoerceMethod(), DoMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/CoerceMethod ***********************************************
*
*   NAME
*       CoerceMethod -- Perform method on coerced object.
*
*   SYNOPSIS
*       result = CoerceMethod( cl, obj, MethodID, ... )
*
*       ULONG CoerceMethod( struct IClass *, Object *, ULONG, ... );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object, as though it were the specified
*       class.  Equivalent to CoerceMethodA(), but allows you to
*       build the message on the stack.
*
*   INPUTS
*       cl - pointer to boopsi class to receive the message
*       obj - pointer to boopsi object
*       ... - method-specific message built on the stack
*
*   RESULT
*       result - class and message-specific result.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*
*   SEE ALSO
*       CoerceMethodA(), DoMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/DoSuperMethodA *********************************************
*
*   NAME
*       DoSuperMethodA -- Perform method on object coerced to superclass.
*
*   SYNOPSIS
*       result = DoSuperMethodA( cl, obj, msg )
*
*       ULONG DoSuperMethodA( struct IClass *, Object *, Msg );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object, as though it were the superclass
*       of the specified class.
*
*   INPUTS
*       cl - pointer to boopsi class whose superclass is to
*           receive the message
*       obj - pointer to boopsi object
*       msg - pointer to method-specific message to send
*
*   RESULT
*       result - class and message-specific result.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*       Some early example code may refer to this function as DSM().
*
*   SEE ALSO
*       CoerceMethodA(), DoMethodA(), DoSuperMethod(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/DoSuperMethod **********************************************
*
*   NAME
*       DoSuperMethod -- Perform method on object coerced to superclass.
*
*   SYNOPSIS
*       result = DoSuperMethod( cl, obj, MethodID, ... )
*
*       ULONG DoSuperMethod( struct IClass *, Object *, ULONG, ... );
*
*   FUNCTION
*       Boopsi support function that invokes the supplied message
*       on the specified object, as though it were the superclass
*       of the specified class.  Equivalent to DoSuperMethodA(),
*       but allows you to build the message on the stack.
*
*   INPUTS
*       cl - pointer to boopsi class whose superclass is to
*           receive the message
*       obj - pointer to boopsi object
*       ... - method-specific message built on the stack
*
*   RESULT
*       result - class and message-specific result.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*
*   SEE ALSO
*       CoerceMethodA(), DoMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

******* amiga.lib/SetSuperAttrs **********************************************
*
*   NAME
*       SetSuperAttrs -- Invoke OM_SET method on superclass with varargs.
*
*   SYNOPSIS
*       result = SetSuperAttrs( cl, obj, tag, ... )
*
*       ULONG SetSuperAttrs( struct IClass *, Object *, ULONG, ... );
*
*   FUNCTION
*       Boopsi support function which invokes the OM_SET method on the
*       superclass of the supplied class for the supplied object.  Allows
*       the ops_AttrList to be supplied on the stack (i.e. in a varargs
*       way).  The equivalent non-varargs function would simply be
*
*           DoSuperMethod( cl, obj, OM_SET, taglist, NULL );
*
*   INPUTS
*       cl - pointer to boopsi class whose superclass is to
*           receive the OM_SET message
*       obj - pointer to boopsi object
*       tag - list of tag-attribute pairs, ending in TAG_DONE
*
*   RESULT
*       result - class and message-specific result.
*
*   NOTES
*       This function first appears in the V37 release of amiga.lib.
*       While it intrinsically does not require any particular release
*       of the system software to operate, it is designed to work with
*       the boopsi subsystem of Intuition, which was only introduced
*       in V36.
*
*   SEE ALSO
*       CoerceMethodA(), DoMethodA(), DoSuperMethodA(), <intuition/classusr.h>
*       ROM Kernel Manual boopsi section
*
******************************************************************************

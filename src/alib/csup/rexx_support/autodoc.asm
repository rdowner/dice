******* amiga.lib/CheckRexxMsg ************************************************
*
*   NAME
*       CheckRexxMsg - Check if a RexxMsg is from ARexx
*
*   SYNOPSIS
*       result = CheckRexxMsg(message)
*       D0                    A0
*
*       BOOL CheckRexxMsg(struct RexxMsg *);
*
*   FUNCTION
*       This function checks to make sure that the message is from ARexx
*       directly.  It is required when using the Rexx Variable Interface
*       routines (RVI) that the message be from ARexx.
*
*       While this function is new in the V37 amiga.lib, it is safe to
*       call it in all versions of the operating system.  It is also
*       PURE code, thus usable in resident/pure executables.
*
*   NOTE
*       This is a stub in amiga.lib.  It is only available via amiga.lib.
*       The stub has two labels.  One, _CheckRexxMsg, takes the arguments
*       from the stack.  The other, CheckRexxMsg, takes the arguments in
*       registers.
*
*   EXAMPLE
*       if (CheckRexxMsg(rxmsg))
*       {
*               /* Message is one from ARexx */
*       }
*
*   INPUTS
*       message         A pointer to the RexxMsg in question
*
*   RESULTS
*       result          A boolean - TRUE if message is from ARexx.
*
*   SEE ALSO
*       GetRexxVar(), SetRexxVar()
*
*******************************************************************************

******* amiga.lib/GetRexxVar **************************************************
*
*   NAME
*       GetRexxVar - Gets the value of a variable from a running ARexx program
*
*   SYNOPSIS
*       error = GetRexxVar(message,varname,bufpointer)
*       D0,A1              A0      A1      (C-only)
*
*       LONG GetRexxVar(struct RexxMsg *,char *,char **);
*
*   FUNCTION
*       This function will attempt to extract the value of the symbol
*       varname from the ARexx script that sent the message.  When called
*       from C, a pointer to the extracted value will be placed in the
*       pointer pointed to by bufpointer.  (*bufpointer will be the pointer
*       to the value)
*
*       When called from assembly, the pointer will be returned in A1.
*
*       The value string returned *MUST* *NOT* be modified.
*
*       While this function is new in the V37 amiga.lib, it is safe to
*       call it in all versions of the operating system.  It is also
*       PURE code, thus usable in resident/pure executables.
*
*   NOTE
*       This is a stub in amiga.lib.  It is only available via amiga.lib.
*       The stub has two labels.  One, _GetRexxVar, takes the arguments
*       from the stack.  The other, GetRexxVar, takes the arguments in
*       registers.
*
*       This routine does a CheckRexxMsg() on the message.
*
*   EXAMPLE
*
*       char    *value;
*
*       /* Message is one from ARexx */
*       if (!GetRexxVar(rxmsg,"TheVar",&value))
*       {
*               /* The value was gotten and now is pointed to by value */
*               printf("Value of TheVar is %s\n",value);
*       }
*
*   INPUTS
*       message         A message gotten from an ARexx script
*       varname         The name of the variable to extract
*       bufpointer      (For C only) A pointer to a string pointer.
*
*   RESULTS
*       error           0 for success, otherwise an error code.
*                       (Other codes may exists, these are documented)
*                       3  == Insufficient Storage
*                       9  == String too long
*                       10 == invalid message
*
*       A1              (Assembly only)  Pointer to the string.
*
*   SEE ALSO
*       SetRexxVar(), CheckRexxMsg()
*
*******************************************************************************

******* amiga.lib/SetRexxVar **************************************************
*
*   NAME
*       SetRexxVar - Sets the value of a variable of a running ARexx program
*
*   SYNOPSIS
*       error = SetRexxVar(message,varname,value,length)
*       D0                 A0      A1      D0    D1
*
*       LONG SetRexxVar(struct RexxMsg *,char *,char *,ULONG);
*
*   FUNCTION
*       This function will attempt to the the value of the symbol
*       varname in the ARexx script that sent the message.
*
*       While this function is new in the V37 amiga.lib, it is safe to
*       call it in all versions of the operating system.  It is also
*       PURE code, thus usable in resident/pure executables.
*
*   NOTE
*       This is a stub in amiga.lib.  It is only available via amiga.lib.
*       The stub has two labels.  One, _SetRexxVar, takes the arguments
*       from the stack.  The other, SetRexxVar, takes the arguments in
*       registers.
*
*       This routine does a CheckRexxMsg() on the message.
*
*   EXAMPLE
*
*       char    *value;
*
*       /* Message is one from ARexx */
*       if (!SetRexxVar(rxmsg,"TheVar","25 Dollars",10))
*       {
*               /* The value of TheVar will now be "25 Dollars" */
*       }
*
*   INPUTS
*       message         A message gotten from an ARexx script
*       varname         The name of the variable to set
*       value           A string that will be the new value of the variable
*       length          The length of the value string
*
*
*   RESULTS
*       error           0 for success, otherwise an error code.
*                       (Other codes may exists, these are documented)
*                       3  == Insufficient Storage
*                       9  == String too long
*                       10 == invalid message
*
*   SEE ALSO
*       SetRexxVar(), CheckRexxMsg()
*
*******************************************************************************

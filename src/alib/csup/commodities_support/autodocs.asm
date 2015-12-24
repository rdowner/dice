
******* amiga.lib/ArgArrayDone ***********************************************
*
*   NAME
*	ArgArrayDone -- release the memory allocated by a previous call
*			to ArgArrayInit(). (V36)
*
*   SYNOPSIS
*	ArgArrayDone();
*
*	VOID ArgArrayDone(VOID);
*
*   FUNCTION
*	This function frees memory and does cleanup required after a
*	call to ArgArrayInit(). Don't call this until you are done using
*	the ToolTypes argument strings.
*
*   SEE ALSO
*	ArgArrayInit()
*
******************************************************************************


******* amiga.lib/ArgArrayInit ***********************************************
*
*   NAME
*	ArgArrayInit -- allocate and initialize a tooltype array. (V36)
*
*   SYNOPSIS
*	ttypes = ArgArrayInit(argc,argv);
*
*	UBYTE **ArgArrayInit(LONG,UBYTE **);
*
*   FUNCTION
*	This function returns a null-terminated array of strings
*	suitable for sending to icon.library/FindToolType(). This array will
*	be the ToolTypes array of the program's icon, if it was started from
*	Workbench. It will just be 'argv' if the program was started from
*	a shell.
*
*	Pass ArgArrayInit() your startup arguments received by main().
*
*	ArgArrayInit() requires that icon.library be open (even if the caller
*	was started from a shell, so that the function FindToolType() can be
*	used) and may call GetDiskObject(), so clean up is necessary when
*	the strings are no longer needed. The function ArgArrayDone() does
*	just that.
*
*   INPUTS
*	argc - the number of arguments in argv, 0 when started from Workbench
*	argv - an array of pointers to the program's arguments, or the
*	       Workbench startup message when started from WB.
*
*   RESULTS
*	ttypes - the initialized argument array or NULL if it could not be
*	         allocated
*
*   EXAMPLE
*	Use of these routines facilitates the use of ToolTypes or command-
*	line arguments to control end-user parameters in Commodities
*	applications. For example, a filter used to trap a keystroke for
*	popping up a window might be created by something like this:
*
*    		char   *ttypes  = ArgArrayInit(argc, argv);
*    		CxObj   *filter = UserFilter(ttypes, "POPWINDOW", "alt f1");
*
*               ... with ...
*
*               CxObj *UserFilter(char **tt, char *action_name,
*				  char *default_descr)
*		{
*		char *desc;
*
*		    desc = FindToolType(tt,action_name);
*
*		    return(CxFilter((ULONG)(desc? desc: default_descr)));
*		}
*
*	In this way the user can assign "alt f2" to the action by
*	entering a tooltype in the program's icon of the form:
*
*		POPWINDOW=alt f2
*
*	or by starting the program from the CLI like so:
*
*		myprogram "POPWINDOW=alt f2"
*
*   NOTE
*	Your program must open icon.library and set up IconBase before calling
*	this routine. In addition IconBase must remain valid until after
*	ArgArrayDone() has been called!
*
*   SEE ALSO
*	ArgArrayDone(), ArgString(), ArgInt(), icon.library/FindToolType()
*
******************************************************************************


******* amiga.lib/ArgInt *****************************************************
*
*   NAME
*	ArgInt -- return an integer value from a ToolTypes array. (V36)
*
*   SYNOPSIS
*	value = ArgInt(tt,entry,defaultval)
*
*	LONG ArgInt(UBYTE **,STRPTR,LONG);
*
*   FUNCTION
*	This function looks in the ToolTypes array 'tt' returned
*	by ArgArrayInit() for 'entry' and returns the value associated
*	with it. 'tt' is in standard ToolTypes format such as:
*
*		ENTRY=Value
*
*	The Value string is passed to atoi() and the result is returned by
*	this function.
*
*	If 'entry' is not found, the integer 'defaultval' is returned.
*
*   INPUTS
*	tt - a ToolTypes array as returned by ArgArrayInit()
*	entry - the entry in the ToolTypes array to search for
*	defaultval - the value to return in case 'entry' is not found within
*		     the ToolTypes array
*
*   RESULTS
*	value - the value associated with 'entry', or defaultval if 'entry'
*		is not in the ToolTypes array
*
*   NOTES
*	This function requires that dos.library V36 or higher be opened.
*
*   SEE ALSO
*	ArgArrayInit()
*
******************************************************************************


******* amiga.lib/ArgString **************************************************
*
*   NAME
*	ArgString -- return a string pointer from a ToolTypes array. (V36)
*
*   SYNOPSIS
*	string = ArgString(tt,entry,defaultstring)
*
*	STRPTR ArgString(UBYTE **,STRPTR,STRPTR);
*
*   FUNCTION
*	This function looks in the ToolTypes array 'tt' returned
*	by ArgArrayInit() for 'entry' and returns the value associated
*	with it. 'tt' is in standard ToolTypes format such as:
*
*		ENTRY=Value
*
*	This function returns a pointer to the Value string.
*
*	If 'entry' is not found, 'defaultstring' is returned.
*
*   INPUTS
*	tt - a ToolTypes array as returned by ArgArrayInit()
*	entry - the entry in the ToolTypes array to search for
*	defaultstring - the value to return in case 'entry' is not found within
*		        the ToolTypes array
*
*   RESULTS
*	value - the value associated with 'entry', or defaultstring if 'entry'
*		is not in the ToolTypes array
*
*   SEE ALSO
*	ArgArrayInit()
*
******************************************************************************


******* amiga.lib/CxCustom ***************************************************
*
*   NAME
*	CxCustom -- create a custom commodity object. (V36)
*
*   SYNOPSIS
*	customObj = CxCustom(action,id);
*
*	CxObj *CxCustom(LONG(*)(),LONG);
*
*   FUNCTION
*	This function creates a custom commodity object. The action
*	of this object on receiving a commodity message is to call a
*	function of the application programmer's choice.
*
*	The function provided ('action') will be passed a pointer to
*	the actual commodities message (in commodities private data
*	space), and will actually execute as part of the input handler
*	system task. Among other things, the value of 'id' can be
*	recovered from the message by using the function CxMsgID().
*
*	The purpose of this function is two-fold. First, it allows
*	programmers to create Commodities Exchange objects with
*	functionality that was not imagined or chosen for inclusion
*	by the designers. Secondly, this is the only way to act
*	synchronously with Commodities.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	action - a function to call whenever a message reaches the object
*	id - a message id to assign to the object
*
*   RESULTS
*	customObj - a pointer to the new custom object, or NULL if it could
*		    not be created.
*
*  SEE ALSO
*	commodities.library/CreateCxObj(), commodities.library/CxMsgID()
*
******************************************************************************


******* amiga.lib/CxDebug ****************************************************
*
*   NAME
*	CxDebug -- create a commodity debug object. (V36)
*
*   SYNOPSIS
*	debugObj = CxDebug(id);
*
*	CxObj *CxDebug(LONG);
*
*   FUNCTION
*	This function creates a Commodities debug object. The action of this
*	object on receiving a Commodities message is to print out information
*	about the Commodities message through the serial port (using the
*	kprintf() routine). The value of 'id' will also be displayed.
*
*	Note that this is a synchronous occurrence (the printing is done by
*	the input device task). If screen or file output is desired, using a
*	sender object instead of a debug object is necessary, since such
*	output is best done by your application process.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	id - the id to assign to the debug object, this value is output
*	     whenever the debug object sends data to the serial port.
*
*   RESULTS
*	debugObj - a pointer to the debug object, or NULL if it could
*		   not be created.
*
*   SEE ALSO
*	commodities.library/CreateCxObj(), CxSender(), debug.lib/kprintf()
*
******************************************************************************


******* amiga.lib/CxFilter ***************************************************
*
*   NAME
*	CxFilter -- create a commodity filter object. (V36)
*
*   SYNOPSIS
*	filterObj = CxFilter(description);
*
*	CxObj *CxFilter(STRPTR)
*
*   FUNCTION
*	Creates an input event filter object that matches the
*	'description' string. If 'description' is NULL, the filter will not
*	match any messages.
*
*	A filter may be modified by the functions SetFilter(), using
*	a description string, and SetFilterIX(), which takes a
*	binary Input Expression as a parameter.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	description - the description string in the same format as strings
*		      expected by commodities.library/SetFilter()
*
*   RESULTS
*	filterObj - a pointer to the filter object, or NULL if there
*		    was not enough memory. If there is a problem in the
*		    description string, the internal error code of the filter
*		    object will be set to so indicate. This error code may be
*		    interrogated using the function CxObjError().
*
*   SEE ALSO
*	commodities.library/CreateCxObj(), commodities.library/SetFilter(),
*	commodities.library/SetFilterIX(), commodities.library/CxObjError()
*
******************************************************************************


******* amiga.lib/CxSender ***************************************************
*
*   NAME
*	CxSender -- create a commodity sender object. (V36)
*
*   SYNOPSIS
*	senderObj = CxSender(port,id)
*
*	CxObj *CxSender(struct MsgPort *,LONG);
*
*   FUNCTION
*	This function creates a Commodities sender object. The action
*	of this object on receiving a Commodities message is to copy the
*	Commodities message into a standard Exec Message, to put the value
*	'id' in the message as well, and to send the message off to the
*	message port 'port'.
*
*	The value 'id' is used so that an application can monitor
*	messages from several senders at a single port. It can be retrieved
*	from the Exec message by using the function CxMsgID(). The value can
*	be a simple integer ID, or a pointer to some application data
*	structure.
*
*	Note that Exec messages sent by sender objects arrive
*	asynchronously at the destination port. Do not assume anything about
*	the status of the Commodities message which was copied into the Exec
*	message you received.
*
*	All Exec messages sent to your ports must be replied. Messages may be
*	replied after the sender object has been deleted.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	port - the port for the sender to send messages to
*	id - the id of the messages sent by the sender
*
*   RESULTS
*	senderObj - a pointer to the sender object, or NULL if it could
*		    not be created.
*
*   SEE ALSO
*	commodities.library/CreateCxObj(), commodities.library/CxMsgID(),
*	exec.library/PutMsg(), exec.library/ReplyMsg()
*
******************************************************************************


******* amiga.lib/CxSignal ***************************************************
*
*   NAME
*	CxSignal -- create a commodity signaller object. (V36)
*
*   SYNOPSIS
*	signalerObj = CxSignal(task,signal);
*
*	CxObj *CxSignal(struct Task *,LONG);
*
*   FUNCTION
*	This function creates a Commodities signal object. The action
*	of this object on receiving a Commodities message is to
*	send the 'signal' to the 'task'. The caller is responsible
*	for allocating the signal and determining the proper task ID.
*
*	Note that 'signal' is the signal value as returned by AllocSignal(),
*	not the mask made from that value.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	task - the task for the signaller to signal
*	signal - the signal bit number for the signaller to send
*
*   RESULTS
*	signallerObj - a pointer to the signaller object, or NULL if it could
*		       not be created.
*
*   SEE ALSO
*	commodities.library/CreateCxObj(), exec.library/FindTask()
*	exec.library/Signal(), exec.library/AllocSignal(),
*
******************************************************************************


******* amiga.lib/CxTranslate ************************************************
*
*   NAME
*	CxTranslate -- create a commodity translator object. (V36)
*
*   SYNOPSIS
*	translatorObj = CxTranslate(ie);
*
*	CxObj *CxTranslate(struct InputEvent *);
*
*   FUNCTION
*	This function creates a Commodities 'translator' object.
*	The action of this object on receiving a Commodities message is to
*	replace that message in the commodities network with a chain of
*	Commodities input messages.
*
*	There is one new Commodities input message generated for each input
*	event in the linked list starting at 'ie' (and NULL terminated). The
*	routing information of the new input messages is copied from the input
*	message they replace.
*
*	The linked list of input events associated with a translator object
*	can be changed using the SetTranslate() function.
*
*	If 'ie' is NULL, the null translation occurs: that is, the original
*	commodities input message is disposed, and no others are created to
*	take its place.
*
*	This function is a C-language macro for CreateCxObj(), defined
*	in <libraries/commodities.h>.
*
*   INPUTS
*	ie - the input event list used as replacement by the translator
*
*   RESULTS
*	translatorObj - a pointer to the translator object, or NULL if it could
*		        not be created.
*
*   SEE ALSO
*	commodities.library/CreateCxObj(), commodities.library/SetTranslate()
*
******************************************************************************


******* amiga.lib/FreeIEvents ************************************************
*
*   NAME
*	FreeIEvents -- free a chain of input events allocated by
*		       InvertString(). (V36)
*
*   SYNOPSIS
*	FreeIEvents(events)
*
*	VOID FreeIEvents(struct InputEvent *);
*
*   FUNCTION
*	This function frees a linked list of input events as obtained from
*	InvertString().
*
*   INPUTS
*       events - the list of input events to free, may be NULL.
*
*   SEE ALSO
*	InvertString()
*
******************************************************************************


******* amiga.lib/HotKey *****************************************************
*
*   NAME
*	HotKey -- create a commodity triad. (V36)
*
*   SYNOPSIS
*	filterObj = Hotkey(description,port,id);
*
*	CxObj *HotKey(STRPTR,struct MsgPort *,LONG);
*
*   FUNCTION
*	This function creates a triad of commodity objects to accomplish a
*	high-level function.
*
*	The three objects are a filter, which is created to match by the call
*	CxFilter(description), a sender created by the call CxSender(port,id),
*	and a translator which is created by CxTranslate(NULL), so that it
*	swallows any commodity input event messages that are passed down by
*	the filter.
*
*	This is the simple way to get a message sent to your program when the
*	user performs a particular input action.
*
*	It is strongly recommended that the ToolTypes environment be used to
*	allow the user to specify the input descriptions for your application's
*	hotkeys.
*
*   INPUTS
*	description - the description string to use for the filter in the same
*		      format as accepted by commodities.library/SetFilter()
*	port - port for the sender to send messages to.
*	id - id of the messages sent by the sender
*
*   RESULTS
*	filterObj - a pointer to a filter object, or NULL if it could
*		    not be created.
*
*   SEE ALSO
*	CxFilter(), CxSender(), CxTranslate(),
*	commodities.library/CxObjError(), commodities.library/SetFilter()
*
******************************************************************************


******* amiga.lib/InvertString ***********************************************
*
*   NAME
*	InvertString -- produce input events that would generate the
*			given string. (V36)
*
*   SYNOPSIS
*	events = InvertString(str,km)
*
*	struct InputEvent *InvertString(STRPTR,struct KeyMap *);
*
*   FUNCTION
*	This function returns a linked list of input events which would
*	translate into the string using the supplied keymap (or the system
*	default keymap if 'km' is NULL).
*
*	'str' is null-terminated and may contain:
*	   - ANSI character codes
*	   - backslash escaped characters:
*		\n   -   CR
*		\r   -   CR
*		\t   -   TAB
*		\0   -   illegal, do not use!
*		\\   -   backslash
*	   - a text description of an input event as used by ParseIX(),
*	     enclosed in angle brackets.
*
*	An example is:
*          abc<alt f1>\nhi there.
*
*   INPUTS
*	str - null-terminated string to convert to input events
*	km - keymap to use for the conversion, or NULL to use the default
*	     keymap.
*
*   RESULTS
*	events - a chain of input events, or NULL if there was a problem. The
*		 most likely cause of failure is an illegal description
*		 enclosed in angled brackets.
*
*		 This chain should eventually be freed using FreeIEvents().
*
*   SEE ALSO
*	commodities.library/ParseIX(), FreeIEvents()
*
******************************************************************************

########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	BeginIO.obj\
	CreateExtIO.obj\
	CreatePort.obj\
	CreateStdIO.obj\
	CreateTask.obj\
	DeleteExtIO.obj\
	DeletePort.obj\
	DeleteStdIO.obj\
	DeleteTask.obj\
	NewList.obj\
	LibAllocPooled.obj\
	LibCreatePool.obj\
	LibDeletePool.obj\
	LibFreePooled.obj\
	FastRand.obj\
	RangeRand.obj\
	AddTOF.obj\
	RemTOF.obj\
	waitbeam.obj\
	afp.obj\
	arnd.obj\
	dbf.obj\
	fpa.obj\
	fpbcd.obj\
	TimeDelay.obj\
	DoTimer.obj\
	ArgArrayDone.obj\
	ArgArrayInit.obj\
	ArgInt.obj\
	ArgString.obj\
	HotKey.obj\
	InvertString.obj\
	FreeIEvents.obj\
	CheckRexxMsg.obj\
	GetRexxVar.obj\
	SetRexxVar.obj\
	CallHookA.obj\
	CallHook.obj\
	DoMethodA.obj\
	DoMethod.obj\
	DoSuperMethodA.obj\
	DoSuperMethod.obj\
	CoerceMethodA.obj\
	CoerceMethod.obj\
	SetSuperAttrs.obj\
	ACrypt.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../alib.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}

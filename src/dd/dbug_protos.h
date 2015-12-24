
/* MACHINE GENERATED */


/* umain.c              */


/* lists.c              */

Prototype void	FreeDLIST(LIST *list);
Prototype BOOL	AddListNODE(LIST *list, UBYTE type, char *data);
Prototype BOOL	DownList(void);
Prototype BOOL	UpList(void);
Prototype BOOL	PageDownList(void);
Prototype BOOL	PageUpList(void);
Prototype WORD	RefreshList(WORD maxLines, BOOL fullRefresh, LONG top);
Prototype BOOL	SprintfDLIST(LIST *list, UBYTE type, char *fmt, ...);
Prototype void	ListDosBase(DBugDisp *dp);
Prototype void	ListProcess(DBugDisp *dp);
Prototype void	ListInfo(DBugDisp *dp);
Prototype void	ListExecBase(DBugDisp *dp);
Prototype void	BuildDLIST(LIST *dst, LIST *src, ULONG type);
Prototype void	ListResources(DBugDisp *dp);
Prototype void	ListIntrs(DBugDisp *dp);
Prototype void	ListPorts(DBugDisp *dp);
Prototype void	ListTasks(DBugDisp *dp);
Prototype void	ListLibs(DBugDisp *dp);
Prototype void	ListDevices(DBugDisp *dp);
Prototype void	ListMemList(DBugDisp *dp);
Prototype int 	SizeDLIST(LIST *list);
Prototype BOOL	ListSymbols(DBugDisp *dp);

/* dbug.c               */

Prototype void		Newline(void);
Prototype void		PrintAddress(ULONG addr);
Prototype void		OffsetAddressBuf(ULONG addr, char *buf);
Prototype void		InitCommand(void);
Prototype void		InitModes(void);
Prototype void		RefreshCommand(int fullRefresh);
Prototype void		RefreshPrompt(BOOL fullRefresh);
Prototype void		RefreshWindow(int fullRefresh);
Prototype void		RefreshAllWindows(int fullRefresh);
Prototype void		SetDisplayMode(WORD, BOOL);
Prototype void		ReadPrefs(void);
Prototype void		WritePrefs(void);
Prototype void		abort(void);
Prototype BOOL		ParseArgToken(char *buf);
Prototype ULONG 	OnOffToggle(char *arg, ULONG val, ULONG mask);
Prototype int		main(int ac, char *av[]);
Prototype LONG		CalcDisplayLines(void);
Prototype void		SetModeSave(WORD mode);
Prototype ULONG		ValidMemCheck(ULONG address);
Prototype __stkargs void kprintf(unsigned char *arg, ...);
Prototype UBYTE 	LineBuf[128];
Prototype UBYTE 	DirBuf[128];
Prototype WORD		ForceFullRefresh;
Prototype UBYTE 	RexxReplyString[MAX_REXX_REPLY];
Prototype UBYTE 	DefaultPubName[128];
Prototype char		lastCommand[128];
Prototype int		RStepFlag;
Prototype char		rexxhostname[16];

/* cexp.c               */

Prototype long		ParseExp(char *, short *, long);
Prototype void		PushOp(short, short, short);
Prototype int		TopOfOpStack(void);
Prototype int		SecondOffOpStack(void);
Prototype void		PushAtom(long, short);

/* commands.c           */

Prototype void		RefreshFKeys(BOOL fullRefresh);
Prototype BOOL		FunctionKey(UWORD num);
Prototype BOOL		HelpKey(void);
Prototype BOOL 		CmdFKey(char *args);
Prototype BOOL		CmdRexx(char *args);
Prototype BOOL		CmdSavePrefs(char *args);
Prototype BOOL		CmdInfo(char *args);
Prototype BOOL		CmdExecBase(char *args);
Prototype BOOL		CmdResources(char *args);
Prototype BOOL		CmdIntrs(char *args);
Prototype BOOL		CmdPorts(char *args);
Prototype BOOL		CmdSymList(char *args);
Prototype BOOL		CmdTasks(char *args);
Prototype BOOL		CmdLibs(char *args);
Prototype BOOL		CmdDevices(char *args);
Prototype BOOL		CmdMemList(char *args);
Prototype BOOL		CmdDoBsBase(char *args);
Prototype BOOL		CmdProcess(char *args);
Prototype BOOL		CmdOffsets(char *args);
Prototype BOOL		CmdDown(char *args);
Prototype BOOL		CmdUp(char *args);
Prototype BOOL		CmdPageDown(char *args);
Prototype BOOL		CmdPageUp(char *args);
Prototype BOOL		CmdSource(char *args);
Prototype BOOL		CmdDism(char *args);
Prototype BOOL		CmdMixed(char *args);
Prototype BOOL		CmdBytes(char *args);
Prototype BOOL		CmdWords(char *args);
Prototype BOOL		CmdLongs(char *args);
Prototype BOOL		CmdBreakpoint(char *args);
Prototype BOOL		CmdBP(char *args);
Prototype BOOL		CmdClear(char *args);
Prototype BOOL		CmdHunks(char *args);
Prototype BOOL		CmdSymbol(char *args);
Prototype BOOL		CmdSet(char *args);
Prototype BOOL		RStepTarget(void);
Prototype BOOL		StepTarget(void);
Prototype BOOL		StepTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		RStepTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		CmdStep(char *args);
Prototype BOOL		CmdRStep(char *args);
Prototype BOOL		OverTarget(void);
Prototype BOOL		OverTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		CmdOver(char *args);
Prototype BOOL		CmdReset(char *args);
Prototype BOOL		GoTarget(void);
Prototype BOOL		CmdGo(char *args);
Prototype BOOL		CmdEval(char *args);
Prototype BOOL		CmdOpen(char *args);
Prototype BOOL		CmdClose(char *args);
Prototype BOOL		CmdQuit(char *args);
Prototype BOOL		CmdHelp(char *args) ;
Prototype BOOL		CmdWatchBytes(char *args);
Prototype BOOL		CmdWatchWords(char *args);
Prototype BOOL		CmdWatchLongs(char *args);
Prototype BOOL		CmdWatchClear(char *args);
Prototype BOOL		CmdRefresh(char *args);
Prototype BOOL		CmdDosBase(char *args);
Prototype WORD		RefreshHelp(WORD maxLines, BOOL fullRefresh, LONG);
Prototype BOOL		DownHelp(void);
Prototype BOOL		UpHelp(void);
Prototype int		HelpSize(void);
Prototype char		*SkipBlanks(char *s);
Prototype BOOL		DoCommand(char *command);
Prototype BOOL		CmdAlias(char *args);
Prototype BOOL		CmdUnAlias(char *args);
Prototype BOOL		CmdRegs(char *args);
Prototype BOOL		CmdJump(char *args);
Prototype BOOL 		CmdBar(char *args);
Prototype BOOL 		CmdEnd(char *args);
Prototype BOOL 		CmdItem(char *args);
Prototype BOOL 		CmdTitle(char *args);
Prototype BOOL 		CmdMenus(char *args);
Prototype BOOL 		CmdSubItem(char *args);
Prototype BOOL 		CmdChangeWindow(char *args);
Prototype BOOL 		CmdAgain(char *args);
Prototype void 		init_function_keys(void);
Prototype char	fkeys[10][16];
Prototype void kprintline(char *string);
Prototype void kprintlen(char *string, int len);

/* loadseg.c            */

Prototype WORD		RefreshHunks(WORD maxLines, BOOL fullRefresh, LONG index);
Prototype WORD		RefreshSymbols(WORD maxLines, BOOL fullRefresh, LONG index);
Prototype LONG		SymbolIndexOfAddr(ULONG val);
Prototype BOOL		DownHunks(void);
Prototype BOOL		UpHunks(void);
Prototype BOOL		DownSymbol(void);
Prototype BOOL		PageDownSymbol(void);
Prototype BOOL		UpSymbol(void);
Prototype BOOL		PageUpSymbol(void);
Prototype BOOL		DBugLoadSeg(char *filename);
Prototype void		AllocateStack(void);
Prototype void		ResetTarget(void);
Prototype DEBUG 	*FindDebug(ULONG address);
Prototype DEBUG 	*FindNearestDebug(ULONG address);
Prototype SOURCE	*FindSource(DEBUG *debug, ULONG address);
Prototype char		*FindSourceLine(DEBUG *debug, SOURCE *source);
Prototype __stkargs char *LookupValue(ULONG value);
Prototype ULONG 	*NearestSymbol(ULONG value);
Prototype char		*NearestValue(ULONG value);
Prototype BOOL		LookupSymbol(char *symbol, ULONG *value);
Prototype BOOL		LookupSymLen(char *symbol, UWORD len, ULONG *value);
Prototype int		CountSymbols(void);
Prototype void		CopySymbols(SYMLIST *symlist);
Prototype char		*addscore(char *string);
Prototype char		*addat(char *string);

/* breakpoints.c        */

Prototype WORD		RefreshBreakpoints(WORD maxLines, BOOL fullRefresh);
Prototype BOOL		UpBreak(void);
Prototype BOOL		DownBreak(void);
Prototype void		InitBreakpoints(void);
Prototype void		InstallBreakpoints(void);
Prototype void		CheckBreakpoints(void);
Prototype BOOL		IsBreakpoint(ULONG address);
Prototype BOOL		SetBreakpoint(ULONG address, UWORD count, UWORD type);
Prototype BOOL		ClearBreakpoint(ULONG address);
Prototype void		SetTempBreakpoint(ULONG address);
Prototype void		SetAllBreakpoints(void);
Prototype WORD topBP;

/* hex.c                */

Prototype WORD		RefreshBytes(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype WORD		RefreshWords(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype WORD		RefreshLongs(WORD maxLines, BOOL fullRefresh, ULONG addr);
Prototype BOOL		UpBytes(void);
Prototype BOOL		UpWords(void);
Prototype BOOL		UpLongs(void);
Prototype BOOL		PageUpHex(void);
Prototype BOOL		DownBytes(void);
Prototype BOOL		DownWords(void);
Prototype BOOL		DownLongs(void);
Prototype BOOL		PageDownHex(void);

/* mixed.c              */

Prototype BOOL		DownMixed(void);
Prototype BOOL		UpMixed(void);
Prototype BOOL		PageDownMixed(void);
Prototype BOOL		PageUpMixed(void);
Prototype BOOL		DownSource(void);
Prototype BOOL		UpSource(void);
Prototype BOOL		PageDownSource(void);
Prototype BOOL		PageUpSource(void);
Prototype WORD		RefreshMixed(WORD maxLines, BOOL fullRefresh, ULONG, ULONG, WORD);
Prototype WORD		PrevMixedLine(ULONG *addr, ULONG *line, LONG *info);
Prototype WORD		NextMixedLine(ULONG *addr, ULONG *line, LONG *info);
Prototype WORD		CurrentMixedLine(ULONG *addr, ULONG *line, LONG *info);

/* dism.c               */

Prototype BOOL		DownDism(void);
Prototype BOOL		UpDism(void);
Prototype BOOL		PageDownDism(void);
Prototype BOOL		PageUpDism(void);
Prototype WORD		RefreshDism(short maxLines, BOOL fullRefresh);
Prototype ULONG 	PreviousInstruction(ULONG addr);

/* registers.c          */

Prototype char		*StateText(ULONG staten);
Prototype WORD		RefreshRegisters(WORD maxLines, BOOL fullRefresh);
Prototype long		*RegisterAddress(char *name);

/* watchpoints.c        */

Prototype WORD		RefreshWatchpoints(WORD maxLines, BOOL fullRefresh);
Prototype void		InitWatchpoints(void);
Prototype BOOL		IsWatchpoint(char *expression);
Prototype BOOL		SetWatchpoint(char *expression, ULONG type);
Prototype BOOL		ClearWatchpoint(char *expression);
Prototype BOOL 		ShowWatchTitle(BOOL fullRefresh, WORD *count, WORD *maxLines);

/* menu.c               */

Prototype BOOL 	enable_menus(void);
Prototype void 	init_default_menus(void);
Prototype void 	free_menus(void);
Prototype void 	set_menu_item(int num, int type, unsigned char *str, unsigned char *cmd, unsigned char *comkey);
Prototype void 	do_scroller(void);
Prototype UWORD FindScrollerTop(UWORD total, UWORD displayable, UWORD pot);
Prototype int 	FindScrollerValues(UWORD total, UWORD displayable, UWORD top, WORD overlap, UWORD *body, UWORD *pot);
Prototype void 	setscrollbar(int flag);
Prototype void 	ActivateArrows(struct Window *win);
Prototype void 	InActivateArrows(struct Window *win);
Prototype BOOL 	ProcessMenuItem(char *args,int type);
Prototype int	FindSlot(char *args);
Prototype struct Menu *v_create_menus(struct NewMenu *menulist);
Prototype int v_layout_menus(struct Menu *menus);
Prototype void v_free_menus(struct Menu *menus);
Prototype int text_width(char *str);

/* rexxfunc.c           */

Prototype BOOL		RXGetPC(char *args);
Prototype BOOL		RXGetLine(char *args);
Prototype BOOL		RXGetInfo(char *args);
Prototype BOOL		RXGetDismLine(char *args);
Prototype BOOL 		RXGetEval(char *args);
Prototype BOOL 		RXGetBytes(char *args);
Prototype BOOL		RXGetWords(char *args);
Prototype BOOL		RXGetLongs(char *args);
Prototype BOOL		RXGetCommand(char *args);
Prototype BOOL		RXStartList(char *args);
Prototype BOOL		RXEndList(char *args);
Prototype BOOL		RXPutList(char *args);

/* console.c            */

Prototype void		InitConsole(void);
Prototype LONG		ScrMainBodyRange(void *ptop, void *pbot);
Prototype LONG		ScrColumns(WORD overhead, WORD itemWidth, WORD limit);
Prototype void		ScrFlush(void);
Prototype void		ScrPutNewline(void);
Prototype void		ScrPutChar(UBYTE c);
Prototype void		ScrWrite(char *s, ULONG len);
Prototype void		ScrPuts(char *s);
Prototype void		ScrPutsCtl(char *s);
Prototype WORD		ScrPrintf(const char *fmt, ...);
Prototype void		ScrStatus(const char *fmt, ...);
Prototype void		ScrHome(void);
Prototype void		ScrCursoff(void);
Prototype void		ScrCurson(void);
Prototype void		ScrClr(void);
Prototype void		ScrRowCol(UWORD row, UWORD col);
Prototype void		ScrEOL(void);
Prototype void		ScrHighlight(void);
Prototype void		ScrDull(void);
Prototype void		ScrPlain(void);
Prototype void		ScrInverse(void);
Prototype void		ScrItalics(void);
Prototype void		ScrUnderline(void);
Prototype void		ScrScrollup(void);
Prototype void		ScrScrolldown(void);
Prototype DBugDisp 	*ScrOpen(BOOL first, BOOL refresh, char *pubname);
Prototype void		ScrScrollClr(void);
Prototype void		GetWindowSize(void);
Prototype void		EnterDebugger(void);
Prototype void		CloseDisplay(struct DBugDisp *);
Prototype void		RequestCloseDisplay(struct DBugDisp *, int );
Prototype BOOL		CheckCloseDisplay(void);
Prototype void		ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y);
Prototype void 		SetTitle(char *title, DBugDisp *disp);
Prototype void 		draw_fkey_boxes(void);
Prototype void		drawdoublebox(void);
Prototype struct DBugDisp  *CurDisplay;
Prototype LIST	DisplayList;

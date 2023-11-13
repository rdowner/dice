
/* MACHINE GENERATED */


/* asm1.c               */

Prototype long AsmState;
Prototype long LabelReturn;
Prototype long LabelProfBeg;
Prototype long LabelProfEnd;
Prototype long LabelRegsUsed;
Prototype long RegReserved;
Prototype char LastSectBuf[256];
Prototype char *LastSectName;
Prototype char *SegNames[];
Prototype char *SegTypes[];
Prototype short OutAlign;
Prototype void asm_extern(Var *);
Prototype void asm_export(Symbol *);
Prototype void asm_procbegin(Var *);
Prototype void asm_regentry(Var *);
Prototype void asm_proclink(Var *);
Prototype void asm_procend(Var *, short);
Prototype void asm_returnstorage(Exp *);
Prototype void asm_dynamictag(Var *);   /* XXX */
Prototype void asm_segment(Var *);
Prototype void asm_align(long);
Prototype void asm_ds(Symbol *, long);
Prototype void asm_string(long, ubyte *, long, long, long);
Prototype void asm_label(long);
Prototype void asm_branch(long);
Prototype void asm_condbra(short, long);
Prototype long asm_push(Exp *, Type *, Stor *);
Prototype void asm_push_mask(long);
Prototype long asm_stackbytes(Type *);
Prototype void asm_call(Exp *, Stor *, Type *, Stor *, long, short);
Prototype void asm_pop(long);
Prototype void asm_pop_mask(long);
Prototype void asm_ext(Exp *, Stor *, Stor *, long);
Prototype void asm_test(Exp *, Stor *);
Prototype void asm_test_scc(Exp *, long, Stor *, short, Stor *);
Prototype void asm_cond_scc(Exp *, long, Stor *, Stor *, short *, Stor *);
Prototype void asm_sccb(Exp *, Stor *, short, short);
Prototype void asm_clr(Exp *, Stor *);
Prototype void asm_movei(Exp *, long, Stor *);
Prototype void asm_neg(Exp *, Stor *, Stor *);
Prototype void ThreeOp(Exp *, Stor *, Stor *, Stor *, void (*)(Exp *, Stor *, Stor *, Stor *));
Prototype void asm_add(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_sub(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_xor(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_and(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_or(Exp *, Stor *, Stor *, Stor *);
Prototype void asm_test_and(Exp *, Stor *, Stor *);
Prototype void asm_switch(Exp *, long, long *, long *, long);
Prototype long SizeFit(long);
Prototype long SizeFitSU(long, Stor *);
Prototype void asm_end(void);
Prototype long asm_rcomove(Exp *, char *, short, Stor *);
Prototype void asm_exg(Stor *, Stor *);
Prototype long RegCallOrder(Type *, char *, char *);
Prototype void AutoAssignRegisteredArgs(Var **, short);

/* asm2.c               */

Prototype void asm_move(Exp *, Stor *, Stor *);
Prototype void asm_move_cast(Exp *, Stor *, Stor *);
Prototype long ReverseOrder(short);
Prototype void asm_cmp(Exp *, Stor *, Stor *, short *);
Prototype long asm_div(Exp *, Stor *, Stor *, Stor *, short);
Prototype long asm_mul(Exp *, Stor *, Stor *, Stor *);
Prototype long asm_mul_requires_call(long);
Prototype void asm_shift(Exp *, long, Stor *, Stor *, Stor *);
Prototype void CallAsmSupport(Exp *, char *, Stor *, Stor *, Stor *, short);
Prototype void asm_bfext(Exp *, Stor *, Stor *);
Prototype void asm_bfsto(Exp *, Stor *, Stor *);
Prototype void asm_bftst(Exp *, Stor *);
Prototype void asm_blockfill(Exp *, Stor *, long, char);
Prototype void asm_illegal(void);
Prototype void asm_moveqAndSwap(long, char, short);

/* asmaddr.c            */

Prototype void asm_getindex(Exp *, Type *, Stor *, Stor *, long, Stor *, short, short);
Prototype void asm_getind(Exp *, Type *, Stor *, Stor *, short, short, short);
Prototype void asm_getlea(Exp *, Stor *, Stor *);
Prototype void asm_lea(Exp *, Stor *, long, Stor *);

/* asmflt.c             */

Prototype void asm_fpadd(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpsub(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpmul(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpdiv(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpneg(Exp *, Type *, Stor *, Stor *);
Prototype void asm_fpcmp(Exp *, Stor *, Stor *, short *);
Prototype void asm_fptest(Exp *, Stor *);
Prototype void asm_fptoint(Exp *, Stor *, Stor *);
Prototype void asm_inttofp(Exp *, Stor *, Stor *);
Prototype void asm_fptofp(Exp *, Stor *, Stor *);
Prototype void asm_fltconst(Exp *, Stor *, long *);
Prototype void CallFPSupport(Exp *, long, Stor *, Stor *, Stor *, char *, short);
Prototype void asm_layoutfpconst(Exp *, Stor *, Stor *);
Prototype void ConstFpNeg(Exp *, Stor *, Stor *);
Prototype void ConstFpAdd(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpSub(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpMul(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpDiv(Exp *, Stor *, Stor *, Stor *);
Prototype void CloseLibsExit();

/* asubs.c              */

Prototype char SizC[];
Prototype char StorBuf[2][1024];
Prototype char *RegMaskToString(long, short *);
Prototype char *StorToString(Stor *, short *);
Prototype char *StorToStringBuf(Stor *, char *);
Prototype long SameStorage(Stor *, Stor *);
Prototype long SameRegister(Stor *, Stor *);
Prototype long RegisterMaskConflict(Stor *, ulong);
Prototype long ImmStorage(Stor *);
Prototype void outop(char *, short, Stor *, Stor *);
Prototype void GenStaticData(Var *);
Prototype void GenDataElm(Exp *, Type *);
Prototype char *itohex(char *, ulong);
Prototype char *itodec(char *, ulong);
Prototype void AutoAggregateBeg(Stor *, Type *);
Prototype void AutoAggregate(void *, long);
Prototype void AutoAggregateEnd(void);
Prototype void AutoAggregateSync(void);

/* block.c              */

Prototype BlockStmt *CurGen;
Prototype short BlockCost;
Prototype BlockStmt *BlockDown(short);
Prototype BlockStmt *BlockUp(void);
Prototype Var *BlockAddVar(Var *);
Prototype void BlockAddStmt(Stmt *);
Prototype void BlockFlagCallMade(void);
Prototype long FindContinueLabel(void);
Prototype long FindBreakLabel(void);
Prototype void GenPush(BlockStmt *);
Prototype void GenFlagCallMade(void);
Prototype void GenPop(BlockStmt *);
Prototype void BlockAddTop(Var *);
Prototype Var *BlockRemoveDuplicateVar(long, Symbol *, Type *);
Prototype int CompareTypes(Type *, Type *, short);

/* cexp.c               */

Prototype short CompExp(short, Exp **, long);
Prototype short CompBracedAssign(short, Type *, Exp **, short, short);

/* exp.c                */

Prototype long ExpToConstant(Exp *);
Prototype Type *ExpToType(Exp *);
Prototype void ExpToLValue(Exp *, Stor *, Type *);
Prototype void InsertCast(Exp **, Type *);
Prototype void InsertNot(Exp **);
Prototype void InsertBranch(Exp **, long, long);
Prototype void InsertAssign(Exp **, Var *);
Prototype long AutoIncDecSize(Exp *);

/* gen.c                */

Prototype Var *ProcVar;
Prototype short GenPass;
Prototype short ForceLinkFlag;
Prototype short GenGlobal;  /*  global data gen for genass.c    */
Prototype long AllocLabel(void);
Prototype void InitGen(long);
Prototype void GenerateVar(Var *);
Prototype void GenBlock(BlockStmt *);
Prototype void GenFor(ForStmt *);
Prototype void GenWhile(WhileStmt *);
Prototype void GenDo(DoStmt *);
Prototype void GenIf(IfStmt *);
Prototype void GenSwitch(SwitchStmt *);
Prototype void GenBreak(BreakStmt *);
Prototype void GenContinue(ContinueStmt *);
Prototype void GenGoto(GotoStmt *);
Prototype void GenLabel(LabelStmt *);
Prototype void GenReturn(ReturnStmt *);
Prototype void GenBreakPoint(BreakPointStmt *);
Prototype void GenExp(ExpStmt *);
Prototype void GenExpResult(ExpStmt *);
Prototype int IsRegCall(long);
Prototype void DebugLine(long);

/* genarith.c           */

Prototype void GenDiv(Exp **);
Prototype void GenPercent(Exp **);
Prototype void GenStar(Exp **);
Prototype void GenMi(Exp **);
Prototype void GenPl(Exp **);
Prototype void GenNeg(Exp **);
Prototype void GenParen(Exp **);
Prototype void GenComma(Exp **);

/* genass.c             */

Prototype void GenEq(Exp **);
Prototype void GenBracEq(Exp **);
Prototype void GenBracedAssign(Exp **);
Prototype void GenPercentEq(Exp **);
Prototype void GenAndEq(Exp **);
Prototype void GenStarEq(Exp **);
Prototype void GenMiEq(Exp **);
Prototype void GenDivEq(Exp **);
Prototype void GenLtLtEq(Exp **);
Prototype void GenGtGtEq(Exp **);
Prototype void GenPlEq(Exp **);
Prototype void GenOrEq(Exp **);
Prototype void GenCaratEq(Exp **);
Prototype void GenAssEq(Exp **);
Prototype void GenSpecialAssignment(Exp **, void (*)(Exp **));

/* genbool.c            */

Prototype void BoolLabels(Exp *, long, long);
Prototype void GenCondBranch(Exp **);
Prototype void GenAndAnd(Exp **);
Prototype void GenOrOr(Exp **);
Prototype void GenBoolCompareSame(Exp **);
Prototype void GenBoolCompare(Exp **);
Prototype void GenNot(Exp **);
Prototype void GenColon(Exp **);
Prototype void GenQuestion(Exp **);

/* genlogic.c           */

Prototype void GenAnd(Exp **);
Prototype void GenOr(Exp **);
Prototype void GenXor(Exp **);
Prototype void GenLShf(Exp **);
Prototype void GenRShf(Exp **);
Prototype void GenCompl(Exp **);

/* genmisc.c            */

Prototype void GenCast(Exp **);
Prototype void GenSizeof(Exp **);
Prototype void GenAddr(Exp **);
Prototype void GenInd(Exp **);
Prototype void GenPreInc(Exp **);
Prototype void GenPreDec(Exp **);
Prototype void GenIntConst(Exp **);
Prototype void GenStrConst(Exp **);
Prototype void GenFltConst(Exp **);
Prototype void GenVarRef(Exp **);
Prototype void GenCall(Exp **);
Prototype void GenCastArgs(Type *, Exp *, Exp **);
Prototype void GenPosInc(Exp **);
Prototype void GenPosDec(Exp **);
Prototype void GenStructInd(Exp **);
Prototype void GenStructElm(Exp **);
Prototype void GenArray(Exp **);
Prototype void GenBFExt(Exp **);
Prototype void BitFieldResultExp(Exp *);
Prototype void BitFieldResultType(Exp **, int);

/* lex.c                */

Prototype void InitLex(void);
Prototype short LexSimpleToken(void);
Prototype short LexLineFeed(void);
Prototype short LexWhiteSpace(void);
Prototype short LexTLex(void);
Prototype short LexSymbol(void);
Prototype short LexInteger(void);
Prototype short LexDecimal(void);
Prototype short LexCharConst(void);
Prototype short LexString(void);
Prototype short LexToken(void);
Prototype short LexOctal(ubyte, long);
Prototype short LexHex(ubyte, long);
Prototype short LexFloating(long, long);
Prototype void PushLexFile(char *, short, long, long);
Prototype short PopLexFile(void);
Prototype int SpecialChar(long *);
Prototype long CharToNibble(short);
Prototype short SkipToken(short, short);
Prototype char *TokenToStr(short);
Prototype long  FindLexFileLine(long, char **, long *, long *);
Prototype short FindLexCharAt(long);
Prototype long    LexIntConst;
Prototype char    *LexStrConst;   /*  also flt constant   */
Prototype long    LexStrLen;
Prototype Symbol  *LexSym;
Prototype void    *LexData;
Prototype char    LexHackColon;
Prototype char    LexUnsigned;
Prototype char FileName[128];
Prototype char SymbolSpace[256];
Prototype long Depth;
Prototype short ErrorInFileValid;
Prototype   long    LexCacheHits;
Prototype   long    LexCacheMisses;
Prototype LexFileNode *LFBase;
Prototype short (*LexDispatch[256])(void);

/* main.c               */

Prototype short MC68020Opt;
Prototype short MC68881Opt;
Prototype short State;
Prototype short DDebug;
Prototype short SmallCode;
Prototype short SmallData;
Prototype short ConstCode;
Prototype short AbsData;
Prototype short ResOpt;
Prototype short PIOpt;
Prototype short VerboseOpt;
Prototype short ProtoOnlyOpt;
Prototype short XOpt;
Prototype short FFPOpt;
Prototype short GenStackOpt;
Prototype short GenLinkOpt;
Prototype short RegCallOpt;
Prototype short RegSpecOutputOpt;
Prototype short ProfOpt;
Prototype short ForkOpt;
Prototype short UnixCommonOpt;
Prototype short DebugOpt;
Prototype short AsmOnlyOpt;
Prototype short ErrorOpt;
Prototype long  ZAllocs;
Prototype long  TAllocs;
Prototype long  ZChunks, ZAloneChunks;
Prototype long  TChunks, TAloneChunks;
Prototype long  SymAllocs;
Prototype long  ExitCode;
Prototype long  RetryCount;
Prototype char  *OutFileName;
Prototype char  *InFileName;
Prototype Var   DummyDataVar;
Prototype Var   DummyCodeVar;
Prototype char  *ErrorFile;
Prototype int main(int, char **);
Prototype void help(void);
Prototype void _Assert(const char *, long);
Prototype void DumpStats(void);
Prototype void ParseOpts(short, char **, short);

/* reg.c                */

Prototype   void    ResetRegAlloc(void);
Prototype   void    AllocExternalStorage(Symbol *, Stor *, Type *, long);
Prototype   void    AllocStaticStorage(Symbol *, Stor *, Type *, long);
Prototype   void    AllocConstStor(Stor *, long, Type *);
Prototype   void    AllocFltConstStor(Stor *, char *, long, Type *);
Prototype   void    AllocStackStorage(Stor *, Type *, long);
Prototype   void    FreeStackStorage(Stor *);
Prototype   void    ReUseStackStorage(Stor *);
Prototype   void    AllocArgsStorage(Stor *, Type *, int, long);
Prototype   int     AllocRegVarStorageReq(Var *, short, long);
Prototype   int     AllocRegVarStorage(Var *);
Prototype   void    AllocTmpStorage(Stor *, Type *, Stor *);
Prototype   void    ReuseStorage(Stor *, Stor *);
Prototype   void    FreeStorage(Stor *);
Prototype   int     AllocDataRegister(Stor *, long);
Prototype   int     AllocDataRegisterAbs(Stor *, long, int);
Prototype   int     AllocAddrRegister(Stor *);
Prototype   int     AllocAddrRegisterAbs(Stor *, int);
Prototype   void    FreeRegister(Stor *);       /*  single registers only */
Prototype   void    LockStorage(Stor *);
Prototype   void    UnlockStorage(Stor *);
Prototype   void    RegDisableRegs(ulong);
Prototype   void    RegEnableRegs(void);
Prototype   void    UnscratchStorage(Exp *);
Prototype   int     RegInUse(short);
Prototype   int     AllocRegisterAbs(Stor *, short, short);
Prototype   int     AttemptAllocRegisterAbs(Stor *, short, short);
Prototype   ulong   GetAllocatedScratch(void);
Prototype   ulong   GetLockedScratch(void);
Prototype   ulong   GetUsedRegisters(void);
Prototype   ulong   RegCallUseRegister(short);
Prototype   int     TooManyRegs(void);
Prototype   int     CountDRegOver(void);
Prototype   int     CountARegOver(void);
Prototype   void    asm_save_regs(ulong);
Prototype   int     asm_restore_regs(ulong);
Prototype   void    RegFlagTryAgain(void);
Prototype   void    PushStackStorage(void);
Prototype   void    PopStackStorage(void);
Prototype ulong   RegAlloc;     /*  protos for debugging only   */
Prototype ulong   RegLocked;
Prototype ulong   RegUsed;

/* rules.c              */

Prototype   void BinaryRules(Exp *);
Prototype   void BinaryArithRules(Exp *);
Prototype   void BinaryLogicRules(Exp *);
Prototype   void UnaryLogicRules(Exp *);
Prototype   void UnaryRules(Exp *);
Prototype   void UnaryArithRules(Exp *);
Prototype   void ShiftRules(Exp *);
Prototype   void AddRules(Exp *);
Prototype   void SubRules(Exp *);
Prototype   void FloatingRules(Exp *);
Prototype   void AssignRules(Exp *);
Prototype   void CompareRules(Exp *, int);
Prototype   void MatchRules(Exp *);
Prototype   void OptBinaryArithRules(Exp *);
Prototype   void OptBinaryRules(Exp *);
Prototype   int  CheckConversion(Exp *, Type *, Type *);
Prototype   int  CreateBinaryResultStorage(Exp *, short);
Prototype   int  CreateUnaryResultStorage(Exp *, short);
Prototype   short   AutoResultStorage(Exp *);

/* sem.c                */

Prototype void SemanticLevelDown(void);
Prototype void SemanticLevelUp(void);
Prototype void SemanticAddTop(Symbol *, short, void *);
Prototype void SemanticAddTopBlock(Symbol *, short, void *);
Prototype void SemanticAdd(Symbol *, short, void *);

/* stmt.c               */

Prototype short CompProcedureArgDeclarators(short, Var ***, long *, long *);
Prototype short CompProcedure(short, Var *);
Prototype short CompStmtDeclExp(short, Stmt **, long);
Prototype short CompBlock(short, Stmt **);
Prototype short CompFor(short, Stmt **);
Prototype short CompWhile(short, Stmt **);
Prototype short CompDo(short, Stmt **);
Prototype short CompIf(short, Stmt **);
Prototype short CompSwitch(short, Stmt **);
Prototype short CompBreak(short, Stmt **);
Prototype short CompContinue(short, Stmt **);
Prototype short CompGoto(short, Stmt **);
Prototype short CompLabel(short, Stmt **);
Prototype short CompReturn(short, Stmt **);
Prototype short CompBreakPoint(short, Stmt **);

/* subs.c               */

Prototype long Align(long, long);
Prototype long PowerOfTwo(ulong);
Prototype void *zalloc(long);
Prototype void *talloc(long);
Prototype void tclear(void);
Prototype void *zrealloc(void *, long, long, long);
Prototype char *SymToString(Symbol *);
Prototype char *TypeToString(Type *);
Prototype char *TypeToProtoStr(Type *, short);
Prototype void eprintf(short, const char *, ...);
Prototype void veprintf(short, const char *, va_list);
Prototype void AddAuxSub(char *);
Prototype void DumpAuxSubs(void);
Prototype void MarkAreaMunged(long, long);
Prototype int  OffsetMunged(long);
Prototype long FPStrToInt(Exp *, char *, int);
Prototype char *IntToFPStr(long, long, long *);
Prototype int FltIsZero(Exp *, char *, int);
Prototype int FltIsNegative(char *, int);
Prototype long FPrefix(Exp *, char *, int, char *);
Prototype void StorToTmpFlt(Exp *, Stor *, TmpFlt *);
Prototype void TmpFltToStor(Exp *, TmpFlt *, Stor *);
Prototype void BalanceTmpFlt(TmpFlt *, TmpFlt *);
Prototype void NormalizeTmpFlt(TmpFlt *);
Prototype int  TmpFltMantDiv(uword *, short, uword);
Prototype int  TmpFltMantMul(uword *, short, uword);
Prototype long Internationalize(char *str, long size);
Prototype int LoadLocaleDefs(char *file);
Prototype void NoMem(void);

/* sym.c                */

Prototype void InitSym(int);
Prototype Symbol *MakeSymbol(const char *, short, short, void *);
Prototype Symbol *PragmaSymbol(char *, short);
Prototype SemInfo *FindSymbolId(Symbol *, short);
Prototype long hash(const ubyte *, short);

/* toplevel.c           */

Prototype short TopLevel(short);
Prototype short CompDecl(short, Var **, long);
Prototype short CompType(short, Type **, long *, long *);
Prototype short CompVar(short, Type *, long, long, Var **);
Prototype short CompTypeDeclarators(short, Type **, Symbol **, long);
Prototype short CompStructType(short, Type **, long);
Prototype short CompEnumType(short, Type **);
Prototype short CompTypeofType(short, Type **);
Prototype short ResolveStructUnionType(Type *);
Prototype void AddExternList(Var *);
Prototype void DumpExternList(long);
Prototype void AddStrList(ubyte *, long, long, long);
Prototype void MakeStringConst(long);
Prototype void DelStrList(long);
Prototype void DumpStrList(void);

/* types.c              */

Prototype long NumTypesAlloc;
Prototype Type VoidType;
Prototype Type CharType;
Prototype Type ShortType;
Prototype Type LongType;
Prototype Type CharPtrType;
Prototype Type CharAryType;
Prototype Type VoidPtrType;
Prototype Type LongPtrType;
Prototype Type UCharType;
Prototype Type UShortType;
Prototype Type ULongType;
Prototype Type FloatType;
Prototype Type DoubleType;
Prototype Type LongDoubleType;
Prototype Type DefaultProcType;
Prototype void InitTypes(int);
Prototype void LooseTypeLink(Type *, Type *);
Prototype void TypeLink(Type *, Type *);
Prototype void TypeLinkEnd(Type *, Type *);
Prototype Type *TypeToPtrType(Type *);
Prototype Type *TypeToAryType(Type *, Exp *, long);
Prototype Type *TypeToProcType(Type *, Var **, short, long);
Prototype Type *TypeToQualdType(Type *, long);
Prototype Type *FindStructUnionType(Symbol *, long);
Prototype Symbol *FindStructUnionTag(Type *);
Prototype long FindStructUnionElm(Type *, Exp *, int *);
Prototype Type *MakeStructUnionType(Symbol *, long);
Prototype void SetStructUnionType(Type *, Var **, long, long);
Prototype Type *MakeBitfieldType(long, int);
Prototype Type *FindEnumType(Symbol *);
Prototype Type *MakeEnumType(Symbol *);
Prototype void AddEnumIdent(Type *, Symbol *, long);
Prototype Type *ActualReturnType(Stmt *, Type *, Type *);
Prototype Type *ActualPassType(Type *, Type *, int);
Prototype Type *ActualArgType(Type *);
Prototype int SamePointerType(Type *, Type *);
Prototype void GenerateRegSpecOutput(Var *);

/* error.c              */

Prototype void cerror(short, const char *, ...);
Prototype void vcerror(long, short, const char *, va_list va, short);
Prototype void zerror(short, ...);
Prototype void yerror(long, short, ...);
Prototype char *ObtainErrorString(short);
Prototype void ExitError(short);

/* pragma.c             */

Prototype void ProcessLibCallPragma(char *, short);
Prototype char *plcsym(char *, char **, short *);
Prototype PragNode *TestPragmaCall(Var *, char *);

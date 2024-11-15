/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DLINK files options
 *
 *  The first object module is considered to be what is run.  Remaining object
 *  modules are included.  Any files not ending in .o are considered to be
 *  libraries and are resolved at exactly the point they occur on the command line.
 *
 *  For Any option requiring a file argument, that argument may
 *  be placed right next to the option or with an intervening space.  Options
 *  must each have their own '-'.
 *
 *  PC relative accesses beyond 32K cause a jump table to be installed for
 *  said accesses.   DLink cannot current handle jump-table creation
 *  when a given module is larger than 32K Bytes.
 *
 *  DLink currently loads every single file and object module into memory
 *
 *  SECTION ORDERING IS PRESERVED.  Sections of like names are merged
 *  together in the same order they appear on the command line.  Sections
 *  of like name within a given library are merged in no particular order
 *  but are guarenteed to occur after previously specified object files
 *  and libraries and before subsequently specified files.  PC-RELATIVE
 *  jump tables are placed in the same sections that reference them.
 *
 *  Note especially that the above guarentees allow DLink to provide
 *  an autoinit/autoexit section capability (where referencing one
 *  section in a module brings in a differently named section in that
 *  same module which is then coagulated to other sections from other
 *  modules named the same into, finally, a single contiguous section
 *  in the executable.
 *
 *  RESIDENT capabilities.  If -r is specified, DLink allows no
 *  absolute data/bss references except to __DATA_BAS.  Presumably
 *  all such references will use A4-relative addressing.  Additionaly,
 *  DLink assumes a copy of the DATA+BSS space will be allocated by
 *  the startup code so NO BSS SPACE IS ALLOCATED.  If the startup
 *  code attempts to use the (nonexistant) BSS space beyond the static
 *  data it will be referencing unallocated ram.  __RESIDENT is set to 1
 *
 *  If -r is not specified then __RESIDENT is set to 0 and BSS space
 *  will exist beyond the end of data, but will NOT be initialized to 0.
 *
 *  FRAGMENTATION is generally not specified, but will occur anyway for
 *  any segment name beginning with 'far' or with special hunk flags set.
 *
 *  file.o[bj]      an object file
 *  otherfile       a library
 *  @file           a data file containing FILES and LIBRARIES, *no* options
 *                  allowed.
 *
 *  -<option>       see switch below
 */

/*
**      $Filename: main.c $
**      $Author: dillon $
**      $Revision: 1.1.1.1 $
**      $Date: 1998/10/10 06:23:15 $
**      $Log: main.c,v $
**      Revision 1.1.1.1  1998/10/10 06:23:15  dillon
**
**
**          Initial import of DICE into CVS repository.  Logs for the thousands of
**          changes prior to this import have essentially been lost   -Matt.
**
 * Revision 30.326  1995/12/24  06:10:39  dice
 * .
 *
 * Revision 30.157  1995/01/11  13:20:42  dice
 * added VDISTRIBUTION .. prints registered, commercial, or minidice in help
 *
 * Revision 30.5  1994/06/13  18:38:36  dice
 * removed assignment prefix to include for unix portability
 *
 * Revision 30.0  1994/06/10  18:05:39  dice
 * .
 *
**/


#include "defs.h"
#include "DLink_rev.h"

static char *DCopyright =
"Copyright (c) 1992,1993,1994 Obvious Implementations Corp., Redistribution & Use under DICE-LICENSE.TXT." VERSTAG;


Prototype char *OutName;
Prototype short FragOpt;
Prototype short SymOpt;
Prototype short ResOpt;
Prototype short PIOpt;
Prototype short DDebug;
Prototype short ExitCode;
Prototype short AbsWordOpt;
Prototype short VerboseOpt;
Prototype short ChipOpt;
Prototype short DebugOpt;
Prototype short ErrorOpt;
Prototype FILE  *ErrorFi;
Prototype int32_t       WordBaseAddr;
Prototype short NumExtHunks;
Prototype char  PostFix[64];

Prototype char Tmp[256];

Prototype List FList;
Prototype List MList;
Prototype List LList;
Prototype List HList;
Prototype List LibDirList;
Prototype List FileList;

Prototype Sym *BssLenSym;
Prototype Sym *DataBasSym;
Prototype Sym *AbsoluteBasSym;
Prototype Sym *DataLenSym;
Prototype Sym *IsResSym;

Prototype int main(int, char **);
Prototype void help(void);
Prototype void AddFile(List *, char *);
Prototype void xexit(int);

char *OutName = "a.out";
short FragOpt;
short SymOpt;
short ResOpt;
short PIOpt;
short DDebug;
short ExitCode;
short AbsWordOpt;
short VerboseOpt;
short ChipOpt;
short NoLibDirsOpt;
short DebugOpt;
short ErrorOpt;
FILE  *ErrorFi;
int32_t  WordBaseAddr;
short NumExtHunks;

static FILE *Fo;

char Tmp[256];
char PostFix[64];

List FList;
List MList;
List LList;
List HList;
List LibDirList;
List FileList;

Sym *BssLenSym;
Sym *DataBasSym;
Sym *AbsoluteBasSym;
Sym *DataLenSym;
Sym *IsResSym;

char DLib[128];

int _DiceCacheEnable = 1;

int
main(int ac, char **av)
{
    short i;

    NewList(&FList);    /*  files/libraries in link                */
    NewList(&MList);    /*  modules in final executable            */
    NewList(&LList);    /*  temporary list of modules in a library */
    NewList(&HList);
    NewList(&LibDirList);
    NewList(&FileList);

    AddTail(&LibDirList, MakeNode(""));

    /*
     * Find prefix
     */

    {
        char *ptr;
        char *p2;

        if ((ptr = strrchr(av[0], '/')) || (ptr = strrchr(av[0], ':')))
            ++ptr;
        else
            ptr = av[0];

        if ((p2 = strchr(ptr, '_')) == NULL)
            p2 = ptr;
        else
            ++p2;

#ifdef AMIGA
        sprintf(DLib, "%.*sdlib:", p2 - ptr, ptr);
#else
        char *envVal = getenv("DLIB");
        if (envVal != NULL) {
            strcpy(DLib, envVal);
        } else {
            sprintf(DLib, INSTDIR "%.*sdlib/", (int)(p2 - ptr), ptr);
        }
#endif
    }

    SanityCheck(-1);

    if (ac == 1)
        help();

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];
        if (*ptr == '-') {
            ptr += 2;
            switch(ptr[-1]) {
            case 'F':
                ErrorOpt = 1;
                if (*ptr == 'F')
                    ErrorOpt = 2;
                ptr = av[++i];
                ErrorFi = fopen(ptr, "a");
                break;
            case 'v':
                VerboseOpt = 1;
                break;
            case 'P':
                strcpy(PostFix, ptr);
                break;
            case 'p':
                switch(*ptr) {
                case 'i':
                    if (ResOpt)
                        puts("Warning, -pi -r = -pr");
                    PIOpt = 1;
                    break;
                case 'r':
                    PIOpt = 1;
                    ResOpt = 1;
                    break;
                }
                break;
            case 'c':           /*  -chip               */
                if (strncmp(ptr, "hip", 3) == 0)
                    ChipOpt = 1;
                else
                    help();
                break;
            case 'o':           /*  -o[ ]outputname     */
                OutName = (*ptr) ? ptr : av[++i];
                break;
            case 'f':           /*  -f      (allow fragmentation)   */
                if (ResOpt)
                    cerror(EERROR_CANT_FRAG_RES);
                else
                    FragOpt = 1;
                break;
            case 's':           /*  -s      (include symbols)       */
                SymOpt = 1;
                break;
            case 'r':           /*  -r      (see comment at top)    */
                if (FragOpt)
                    cerror(EERROR_CANT_FRAG_RES);
#ifdef NOTDEF
                if (PIOpt && ResOpt == 0)
                    ;
                    /*puts("Warning, -pi -r = -pr");*/
#endif

                ResOpt = 1;
                FragOpt = 0;
                break;
            case 'm':
                if (*ptr == 'w' || *ptr == 'a') {
                    char *dummy;

                    AbsWordOpt = 1;
                    ++ptr;
                    if (*ptr == 0)
                        ptr = av[++i];
                    WordBaseAddr = strtol(ptr, &dummy, 0);
                }
                break;
            case 'd':           /*  -d[#]   (debug hunks)           */
                if (*ptr)
                    DebugOpt = strtol(ptr, NULL, 0);
                else
                    DebugOpt = 1;
                break;
            case 'Z':
                if (*ptr)
                    DDebug = atoi(ptr);
                else
                    DDebug = 1;
                break;
            case 'L':           /*  -Ldir   search directory        */
                if (strcmp(ptr, "0") == 0) {
                    NoLibDirsOpt = 1;
                    NewList(&LibDirList);
                    AddTail(&LibDirList, MakeNode(""));
                } else {
                    Node *node;
                    short len;

                    if (*ptr == 0)
                        ptr = av[++i];
                    len = strlen(ptr) - 1;
                    if (ptr[len] == '/' || ptr[len] == ':')
                        node = MakeNode(ptr);
                    else
                        node = MakeNode2(ptr, "/");
                    AddTail(&LibDirList, node);
                }
                break;
            default:
                cerror(EERROR_BAD_OPTION, ptr - 2);
                help();
            }
            continue;
        }
        if (*ptr == '@') {
            FILE *fi = fopen(ptr + 1, "r");
            short j;
            short c;

            if (fi == NULL) {
                cerror(EERROR_CANT_OPEN_FILE, ptr + 1);
                help();
            }
            c = getc(fi);
            while (c != EOF) {
                while (c == ' ' || c == 9 || c == '\n')
                    c = getc(fi);
                j = 0;
                while (c != ' ' && c != 9 && c != '\n' && c != EOF) {
                    Tmp[j++] = c;
                    c = getc(fi);
                }
                Tmp[j] = 0;
#ifdef LATTICE              /*  workaround bug in Lattice V5.04    */
                ftell(fi);
#endif
                if (j)
                    AddTail(&FileList, MakeNode(Tmp));
            }
            fclose(fi);
            continue;
        }
        AddTail(&FileList, MakeNode(ptr));
    }
    if (i > ac) {
        puts("expected file argument");
        exit(20);
    }
    if (NoLibDirsOpt == 0)
        AddTail(&LibDirList, MakeNode(DLib));
    {
        Node *node;
        while ((node = RemHead(&FileList)) != NULL) {
            AddFile(&FList, node->ln_Name);
            free(node);
        }
    }
    if (PIOpt) {
        if (AbsWordOpt) {
            puts("Cannot have both -pi and -mw");
            exit(20);
        }
        if (FragOpt) {
            puts("Warning: -frag does not work with -pi");
            FragOpt = 0;
        }
    }

    /*
     *  Create default symbols
     */

    BssLenSym  = CreateSymbol("__BSS_LEN\0\0\0" , 3, NULL, 0, 2);
    DataBasSym = CreateSymbol("__DATA_BAS\0\0\0", 3, NULL, 0, 2);
    DataLenSym = CreateSymbol("__DATA_LEN\0\0\0", 3, NULL, 0, 2);
    IsResSym   = CreateSymbol("__RESIDENT\0\0\0", 3, NULL, 0, 2);

    if (ResOpt == 0 || AbsWordOpt || PIOpt)
        AbsoluteBasSym = CreateSymbol("__ABSOLUTE_BAS\0\0", 4, NULL, WordBaseAddr, 2);

    SanityCheck(0);

    /*
     *  Create module list from file nodes.  Counts the number of hunks in
     *  each module and generally fills out Module(s) and Hunk(s) structures.
     *
     *  When a library is found, all modules in the library are added to their
     *  own separate list then the library is scanned with appropriate modules
     *  transfered to the master list and symbol ref table created for said.
     *
     *  Generates symbol reference table as a side effect.  Duplicate
     *  definitions from different file nodes are reported.
     */

    {
        FileNode *fn;

        for (fn = GetHead(&FList); fn; fn = GetSucc(&fn->Node)) {
            List *list = (fn->Node.ln_Type == NT_FTOBJ) ? &MList : &LList;

            dbprintf(0, ("File %s\n", fn->Node.ln_Name));

            while ((char *)fn->DPtr < (char *)fn->Data + fn->Bytes) {
                Module *module;

                if ((module = CreateModule(fn)) != NULL) {
#ifdef DEBUG
                    if ((DDebug && list == &MList) || DDebug > 4)
                        printf(" %s %s, %d hunks %s\n", ((list == &MList) ? "Mod" : "Lib"), module->FNode->Node.ln_Name, module->NumHunks, module->Node.ln_Name);
#endif
                    if (list == &MList)
                        CreateSymbolTable(module);
                    AddTail(list, &module->Node);
                } else {
                    printf("Bad hunk in %-15s $%08x at offset $%08x\n",
                           fn->Node.ln_Name,
                           *fn->DPtr,
                           (uint32_t)((char *)fn->DPtr - (char *)fn->Data));
                    break;
                }
            }

            /*
             *  If a library scan library modules for inclusion
             */

            if (list == &LList) {
                int32_t numUndef;               /*  non-zero dummy value */

                do {
                    Module *mod;
                    Module *nextMod;

                    numUndef = 0;
                    dbprintf(0, ("Scan Lib\n"));
                    for (mod = GetHead(&LList); mod; mod = nextMod) {
                        nextMod = GetSucc(&mod->Node);
                        if (ScanLibForInclusion(mod)) {
                            dbprintf(0, (" Include %s : %s\n", mod->FNode->Node.ln_Name, mod->Node.ln_Name));
                            numUndef += CreateSymbolTable(mod);
                            dbprintf(0, (" -- End Include --\n"));
                            Remove(&mod->Node);
                            AddTail(&MList, &mod->Node);
                        }
                    }
                } while (numUndef);

                /*
                 *  clear out unused modules by putting them on the free list
                 */
                {
                    Module *mod;
                    short i;

                    while ((mod = (Module *)RemHead(&LList)) != NULL) {
                        for (i = 0; i < mod->NumHunks; ++i)
                            zfree(mod->Hunks[i], sizeof(Hunk));
                        if (mod->NumHunks)
                            zfree(mod->Hunks, sizeof(Hunk *) * mod->NumHunks);
                        while (mod->ModEnd - mod->ModBeg >= ALIGN(sizeof(Hunk))) {
                            zfree(mod->ModBeg, sizeof(Hunk));
                            ++MemNumHunksMalReclaim;
                            mod->ModBeg += ALIGN(sizeof(Hunk));
                        }
                        while (mod->ModEnd - mod->ModBeg >= ALIGN(sizeof(Sym))) {
                            zfree(mod->ModBeg, sizeof(Sym));
                            ++MemNumSymsMalReclaim;
                            mod->ModBeg += ALIGN(sizeof(Sym));
                        }
                        zfree(mod, sizeof(Module));
                    }
                }
            }
        }
    }

    SanityCheck(1);

    /*
     *  Scan modules that will be part of executable and create HunkListNodes
     *  on the HunkList.    (combine hunks together)
     *
     *  This also makes the linker variables more usable by assigning
     *  them to appropriate hunklistnodes.
     */

    {
        Module *mod;
        for (mod = GetHead(&MList); mod; mod = GetSucc(&mod->Node))
            CreateHunkListNodes(mod, &HList);
    }

    SanityCheck(2);

    FinalCombineHunkListNodes(&HList);

    /*
     *  Scan hunks for unresolved COMMON symbols, resolve them into their
     *  BSS hunk as appropriate
     */

    {
        HunkListNode *hn;
        HunkListNode *nextHn;

        for (hn = GetHead(&HList); hn; hn = nextHn) {
            Hunk *hunk;

            nextHn = GetSucc(&hn->Node);
            for (hunk = GetHead(&hn->HunkList); hunk; hunk = GetSucc(&hunk->Node))
                ScanHunkExt(hunk, SCAN_COMMON_RESOLVE);
        }
    }

    /*
     *  Delete empty hunk list nodes and number non-empty ones.
     *
     *  Generate a base Offset for each hunk.  This is repeated until
     *  HandleJumpTable() tells us we are ok.  HandleJumpTable() checks
     *  all external-label PC-rel-16 relocations.
     *
     *  hn->AddSize specifies bytes allocated beyond any real data, usually
     *  for BSS space that is tagged onto a DATA hunk.  The HF_DATABSS flag
     *  indicates this.
     */

    SanityCheck(3);

    do {
        HunkListNode *hn;
        HunkListNode *nextHn;
        short hunkNo = 0;

        for (hn = GetHead(&HList); hn; hn = nextHn) {
            nextHn = GetSucc(&hn->Node);

            hn->FinalSize = 0;
            hn->AddSize = 0;
            {
                Hunk *hunk;
                for (hunk = GetHead(&hn->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                    hunk->Offset = hn->FinalSize + hn->AddSize;
                    if (hunk->Flags & HF_DATABSS) {
                        hn->AddSize += (hunk->TotalBytes + 3) & ~3;
                    } else {
                        if (hn->AddSize)
                            cerror(EERROR_253, hn->AddSize);
                        hn->FinalSize += (hunk->TotalBytes + 3) & ~3;
                    }
                }
            }
            if (hn->FinalSize + hn->AddSize == 0 && hn->FinalExtDefs == 0) {
                Remove(&hn->Node);
            } else {
                hn->FinalHunkNo = hunkNo++;
            }
        }
        NumExtHunks = hunkNo;
    } while (HandleJumpTable(&HList));

    SanityCheck(4);

    /*
     *  Resident option.  Put static data into the code segment and
     *  create additional BSS space.  Redirect references to static
     *  data to references to BSS space.
     */

    FixInternalSymbols(&HList);

    SanityCheck(5);

    /*
     *  allocate memory for ExtReloc for each combined hunk, count the amount
     *  of relocation information for each reference index (include any
     *  incidental Reloc32's in the hunks), then copy the appropriate
     *  information taking into account the newly assigned hunk offsets.
     *
     *  There is a ExtReloc32 for each 'final' hunk.  This ExtReloc32 holds
     *  relocation information from this 'final' hunk to other 'final' hunks
     *  in the system and is indexed by final hunk number.
     */

    {
        HunkListNode *hn;
        Hunk *hunk;

        for (hn = GetHead(&HList); hn; hn = GetSucc(&hn->Node)) {
            hn->ExtReloc32 = zalloc(sizeof(uint32_t *) * NumExtHunks);
            hn->CntReloc32 = zalloc(sizeof(uint32_t) * NumExtHunks);
            hn->CpyReloc32 = zalloc(sizeof(uint32_t) * NumExtHunks);

            for (hunk = GetHead(&hn->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                /*
                 *  scan hunk->Reloc32 and add # of relocations to hn->Cnt*
                 */

                ScanHunkReloc32(hunk, 0);

                /*
                 *  relocate 8 and 16 bit references.
                 */

                ScanHunkReloc8(hunk);
                ScanHunkReloc16(hunk);
                ScanHunkRelocD8(hunk);
                ScanHunkRelocD16(hunk);
                ScanHunkRelocD32(hunk);

                /*
                 *  scan hunk->Ext symbols and take into account imported
                 *  references.  Only 32 bit references are counted for the
                 *  final relocation.
                 */

                ScanHunkExt(hunk, SCAN_RELOC_CNT);
            }

            /*
             *  allocate final relocation arrays according to accumulated
             *  statistics above.  Note that CntReloc32[] entries can be 0
             *  indicating no relocation info to that hunk.
             */

            {
                short i;
                for (i = 0; i < NumExtHunks; ++i)
                    hn->ExtReloc32[i] = zalloc(sizeof(uint32_t) * hn->CntReloc32[i]);
            }
        }
    }

    SanityCheck(6);

    /*
     *  Final relocation pass.  Copy appropriate relocation information and
     *  update Data or Code destinations (for 32 bit relocations referencing
     *  hunks with non-zero offsets)
     */

    {
        HunkListNode *hn;
        Hunk *hunk;

        for (hn = GetHead(&HList); hn; hn = GetSucc(&hn->Node)) {
            dbprintf(0, ("Final file %s\n", hn->Node.ln_Name));

            for (hunk = GetHead(&hn->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                /*
                 *  copy relocation info to the appropriate place in the
                 *  final relocation array and make appropriate modifications
                 *  for references to hunks with non-zero offsets.
                 */

                dbprintf(0, (" HUNK %s\n", HunkToStr(hunk)));

                SanityCheck(16);
                ScanHunkReloc32(hunk, 1);
                SanityCheck(17);

                /*
                 *  Copy relocation info for external 32 bit references and
                 *  make appropriate modifications for references to hunks
                 *  with non-zero offsets.
                 *
                 *  Also performs 8 and 16 bit relocations.
                 */

                SanityCheck(18);
                ScanHunkExt(hunk, SCAN_RELOC_RUN);
                SanityCheck(19);
            }
        }
    }

    SanityCheck(7);

    /*
     *  PIOpt option, combine DATA & BSS into single CODE hunk
     */

    if (PIOpt)
        PIOptCombineIntoCode(&HList);

    /*
     *  Generate final executable
     */

    {
        FILE *fo = fopen(OutName, "w");

        if (fo) {
            Fo = fo;
            GenerateFinalExecutable(fo, &HList);
            fclose(fo);
#ifdef AMIGA
            if (ResOpt)
                SetProtection(OutName, 0x00000020);     /*  set Pure and Exec*/
            else
                SetProtection(OutName, 0x00000000);     /*  ensure E bit set */
#endif
        } else {
            cerror(EERROR_CANT_CREATE_FILE, OutName);
        }
    }

    SanityCheck(8);

    if (VerboseOpt) {
        printf("Memory: %d+%d allocated (%d req %d recl)\n",
            MemMalloced, MemAllocated, MemRequested, MemReclaimed
        );
        printf("\t%d symbols %d hunks %d modules (%zd,%zd,%zd)\n",
            MemNumSyms, MemNumHunks, MemNumModules,
            MemNumSyms * sizeof(Sym),
            MemNumHunks* sizeof(Hunk),
            MemNumModules * sizeof(Module)
        );
        printf("\t%d hunks %d syms reclaimed from body\n",
            MemNumHunksMalReclaim,
            MemNumSymsMalReclaim
        );
    }
    xexit(0);
    return(0); /* not reached */
}

void
help()
{
    printf("%s\n%s\n", VSTRING VDISTRIBUTION, DCopyright);
    puts("dlink [files/libs/@files] -o outname -r -s -v <other-options>");
    exit(5);
}

void
AddFile(list, name)
List *list;
char *name;
{
    FileNode *fn = zalloc(sizeof(FileNode) + strlen(name) + 1);
    int fd;

    fn->Node.ln_Name = (char *)(fn + 1);
    strcpy(fn->Node.ln_Name, name);

    fd = open_lpath(name, O_RDONLY | O_BINARY);
    if (fd < 0)
        return;

    if ((fn->Bytes = lseek(fd, 0L, 2)) > 0) {
        MemMalloced += fn->Bytes;
        fn->Data = malloc(fn->Bytes);
        fn->DPtr = fn->Data;
        if (fn->Data == NULL)
            NoMemory();
        lseek(fd, 0L, 0);
        if (read(fd, fn->Data, fn->Bytes) != fn->Bytes)
            cerror(EFATAL_ERROR_READING_FILE, name);
        {
            char *str;
            for (str = name + strlen(name); str >= name && *str != '.'; --str);
            if (str >= name && *str == '.' && (str[1] == 'o' || str[1] == 'O')) {
                fn->Node.ln_Type = NT_FTOBJ;
                fn->Node.ln_Pri = 32;
            } else {
                fn->Node.ln_Type = NT_FTLIB;
                fn->Node.ln_Pri = 32;
            }
        }
        dbprintf(0, ("load %-15s %d %d\n", fn->Node.ln_Name, fn->Node.ln_Type, fn->Bytes));
        Enqueue(list, &fn->Node);
    }
    close(fd);
}

void
xexit(code)
int code;
{
    if (ExitCode < code)
        ExitCode = code;
    if (ExitCode > 5) {
        if (Fo)
            fclose(Fo);
        remove(OutName);
    }
    exit(ExitCode);
}


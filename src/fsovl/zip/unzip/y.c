

#include "unzip.h"               /* includes, defines, and macros */
#ifdef MSWIN
#  include "wizunzip.h"          /* see History.500 for version history */
#endif

#define VERSION  "v5.0 of 21 August 1992"
/* #define VERSION  "v5.0p BETA of 8-21-92" */   /* internal beta level */
#define PAKFIX   /* temporary(?) solution to PAK-created zipfiles */

char *SeekMsg = "AA";
char *ReportMsg = "BB";


/**********************/
/*  Global Variables  */
/**********************/

int aflag=0;          /* -a: do ASCII to EBCDIC translation, or CR-LF  */
                      /*     to CR or LF conversion of extracted files */
/* int bflag=0; RESERVED for -b: extract as binary */
int cflag=0;          /* -c: output to stdout */
int fflag=0;          /* -f: "freshen" (extract only newer files) */
int jflag=0;          /* -j: junk pathnames */
int overwrite_none=0; /* -n: never overwrite files (no prompting) */
int overwrite_all=0;  /* -o: OK to overwrite files without prompting */
int force_flag=0;     /* (shares -o for now): force to override errors, etc. */
int quietflg=0;       /* -q: produce a lot less output */
#ifdef DOS_OS2
   int sflag=1;       /* -s: allow spaces (blanks) in filenames */
#endif /* DOS_OS2 */
int tflag=0;          /* -t: test */
int uflag=0;          /* -u: "update" (extract only newer & brand-new files) */
static int U_flag=0;  /* -U: leave filenames in upper or mixed case */
static int vflag=0;   /* -v: view directory (only used in unzip.c) */
int V_flag=0;         /* -V: don't strip VMS version numbers */
#ifdef VMS
   int secinf=0;      /* -X: keep owner/protection */
#endif /* VMS */
int zflag=0;          /* -z: display only the archive comment */
int process_all_files=0;

longint csize;        /* used by list_files(), ReadByte(): must be signed */
longint ucsize;       /* used by list_files(), unReduce(), explode() */

char *fnames[2] = {"*", NULL};   /* default filenames vector */
char **fnv = fnames;
char sig[5];
char answerbuf[10];

min_info info[DIR_BLKSIZ], *pInfo=info;

#ifdef OS2
   int longname;              /* used in extract.c, mapname.c and file_io.c */
   char longfilename[FILNAMSIZ];
#endif /* OS2 */

#ifdef CRYPT
   char *key = (char *)NULL;  /* password with which to decrypt data, or NULL */
#endif /* CRYPT */

/*---------------------------------------------------------------------------
    unShrink/unReduce/explode/inflate working storage and globals:
  ---------------------------------------------------------------------------*/

#ifdef NOTDEF
__far union work area;              /* see unzip.h for the definition of work */
#endif
ULONG crc32val;

UWORD mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/*---------------------------------------------------------------------------
    Input file variables:
  ---------------------------------------------------------------------------*/

byte *inbuf, *inptr;     /* input buffer (any size is legal) and pointer */
int incnt;

ULONG bitbuf;
int bits_left;
boolean zipeof;

int zipfd;               /* zipfile file handle */
#ifdef MSWIN
   char *zipfn;
#else
   char zipfn[FILNAMSIZ];
#endif

char local_hdr_sig[5] = "\120";    /* remaining signature bytes come later   */
char central_hdr_sig[5] = "\120";  /*  (must initialize at runtime so unzip  */
char end_central_sig[5] = "\120";  /*  executable won't look like a zipfile) */
/* char extd_local_sig[5] = "\120";  NOT USED YET */

cdir_file_hdr crec;      /* used in unzip.c, extract.c, misc.c */
local_file_hdr lrec;     /* used in unzip.c, extract.c */
ecdir_rec ecrec;         /* used in unzip.c, extract.c */
struct stat statbuf;     /* used by main(), mapped_name(), check_for_newer() */

longint extra_bytes = 0;        /* used in unzip.c, misc.c */
longint cur_zipfile_bufstart;   /* extract_or_test_files, readbuf, ReadByte */
  
#ifdef MACOS
   short  gnVRefNum;
   long  glDirID;
   OSType  gostCreator;
   OSType  gostType;
   boolean  fMacZipped;
   boolean  macflag;
   CursHandle  rghCursor[4];    /* status cursors */
   short  giCursor = 0;
#endif

/*---------------------------------------------------------------------------
    Output stream variables:
  ---------------------------------------------------------------------------*/

byte *outbuf;                   /* buffer for rle look-back */
byte *outptr;
#ifdef MSWIN
   byte __far *outout;
   char *filename;
#else /* !MSWIN */
   byte *outout;                /* scratch pad for ASCII-native trans */
   char filename[FILNAMSIZ];
#endif /* ?MSWIN */
byte *extra_field = (byte *)NULL;  /* used by VMS, Mac and OS/2 versions */
longint outpos;                 /* absolute position in outfile */
int outcnt;                     /* current position in outbuf */
int outfd;
int mem_mode = 0;
int disk_full;


/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  KEYBOARD.C
 *
 *  Handle keyboard related stuff such as keyboard mappings.  Every time
 *  a key is pressed, KEYCTL() is called with the code.  KEYCTL() remembers
 *  which qualifier keys are currently held down, and when a non-qualifier
 *  key is pressed finds the hash entry for the key.  If no hash entry
 *  exists (e.g. you type a normal 'a') the default keymap is used.
 */

#include "defs.h"
#include <devices/keymap.h>
#include <devices/console.h>

Prototype void keyctl (struct IntuiMessage *, int, unsigned short);
Prototype void dealloc_hash (void);
Prototype void resethash (void);
Prototype int returnoveride (int);
Prototype void addhash (ubyte, ubyte, ubyte, ubyte, ubyte *);
Prototype int remhash (ubyte, ubyte, ubyte);
Prototype char *keyspectomacro (char *);
Prototype void do_map (void);
Prototype void do_unmap (void);
Prototype void do_clearmap (void);
Prototype void do_savemap (void);
Prototype void keyboard_init (void);
Prototype unsigned char *cqtoa (int, int);
Prototype int get_codequal (ubyte *, ubyte *, ubyte *);

typedef struct IOStdReq CIO;

#define QUAL_SHIFT   0x01
#define QUAL_CTRL    0x02
#define QUAL_AMIGA   0x04
#define QUAL_ALT     0x08
#define QUAL_LMB     0x10
#define QUAL_MMB     0x20
#define QUAL_RMB     0x40

#define HASHSIZE  64                /*  power of 2  */
#define HASHMASK  (HASHSIZE-1)

typedef struct _HASH {
    struct _HASH *next;     /* next hash   */
    ubyte code;             /* keycode     */
    ubyte mask;             /* qual. mask  */
    ubyte qual;             /* qual. comp  */
    ubyte stat;             /* string static? */
    char *str;              /* command string */
} HASH;

HASH *Hash[HASHSIZE];

struct Library *ConsoleDevice;

ubyte   ctoa[128];
ubyte   cstoa[128];

void
keyctl(im, code, qual)
IMESS *im;
USHORT qual;
{
    ubyte buf[256];
    ubyte c2;
    short blen = 0;

    code &= 0xFF;
    if (im) {
        uword oldQual = im->Qualifier;

        im->Qualifier &= ~IEQUALIFIER_REPEAT;
        blen = DeadKeyConvert(im, buf+1, 254, NULL);
        im->Qualifier = oldQual;
        if (blen < 0)
            return;
    }
    c2 = 0;
    if (qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))
        c2 |= QUAL_SHIFT;
    if (qual & (IEQUALIFIER_CONTROL))
        c2 |= QUAL_CTRL;
    if (qual & (IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND))
        c2 |= QUAL_AMIGA;
    if (qual & (IEQUALIFIER_LALT|IEQUALIFIER_RALT))
        c2 |= QUAL_ALT;
    if ((qual & IEQUALIFIER_CAPSLOCK) && blen == 1 && buf[1] >= 'a' && buf[1] <= 'z')
        c2 |= QUAL_SHIFT;
    if (qual & IEQUALIFIER_LEFTBUTTON)
        c2 |= QUAL_LMB;
    if (qual & IEQUALIFIER_MIDBUTTON)
        c2 |= QUAL_MMB;
    if (qual & (IEQUALIFIER_RBUTTON))
        c2 |= QUAL_RMB;

    {
        HASH *hash;
        for (hash = Hash[code&HASHMASK]; hash; hash = hash->next) {
            if (hash->code == code && (c2 & hash->mask) == hash->qual)
                break;
        }

        /*
         *  Use hash entry only if not in command line mode, or if the
         *  entry does not correspond to an alpha key.
         */

        if (hash) {
            if (c2 || !Comlinemode || blen > 1 || !ctoa[code]) {
                strcpy(buf, hash->str);
                do_command(buf);
                return;
            }
        }
    }

    /*
     *  No hash entry
     */

    if (blen == 1) {
        buf[0] = '\'';
        buf[2] = 0;
    } else {
        buf[0] = '\`';
        buf[blen+1] = '\'';
    }
    if (blen)
        do_command(buf);
}

void
dealloc_hash()
{
    HASH *hash, *hnext = NULL;
    short i;

    for (i = 0; i < HASHSIZE; ++i) {
        for (hash = Hash[i]; hash; hash = hnext) {
            hnext = hash->next;
            if (!hash->stat)
                FreeMem(hash->str, strlen(hash->str)+1);
            FreeMem(hash, sizeof(HASH));
        }
        Hash[i] = NULL;
    }
}

void
resethash()
{
    short i;
    CIO cio;
    static const struct {
        const char *from, *to;
    } defmap[] = {
        "esc",      "esc",
        "c-esc",    "recall",
        "return",   "ifelse b (return) (return insline) up firstnb down",
        "enter",    "return",
        "up",       "up",
        "down",     "down",
        "right",    "right",
        "left",     "left",
        "bs",       "bs",
        "del",      "del",
        "tab",      "tab",
        "a-up",     "scrollup",
        "a-down",   "scrolldown",
        "a-r",      "nextr",
        "a-u",      "while cl (tlate -32 right)",
        "a-l",      "while cu (tlate +32 right)",
        "s-up",     "top",
        "s-down",   "bottom",
        "s-right",  "last",
        "s-left",   "first",
        "s-tab",    "backtab",
        "s-del",    "deline",
        "s- ",      "( )",              /* shift space to space */
        "c-1",      "goto block",
        "c-c",      "",                 /* break.. map to a nop */
        "c-l",      "wleft",
        "c-r",      "wright",
        "c-i",      "insertmode on",
        "c-o",      "insertmode off",
        "c-j",      "join",
        "c-s",      "split first down",
        "c-del",    "remeol",
        "c-n",      "next",
        "c-p",      "prev",
        "c-/",      "escimm (find )",
        "c-]",      "ref",
        "c-[",      "ctags",
        "c-g",      "escimm (goto )",
        "c-up",     "pageup",
        "c-down",   "pagedown",
        "c-q",      "quit",
        "c-f",      "reformat",
        "c-w",      "wordwrap toggle",
        "f1",       "escimm (insfile )",
        "f2",       "escimm (newfile )",
        "f3",       "escimm (newwindow newfile )",
        "f6",       "saveold iconify",
        "f7",       "escimm (bsave )",
        "f8",       "saveold escimm (newfile )",
        "f9",       "saveold",
        "f10",      "saveold quit",
        "c-b",      "block",
        "c-u",      "unblock",
        "a-d",      "bdelete",
        "a-c",      "bcopy",
        "a-m",      "bmove",
        "a-s",      "bsource",
        "a-S",      "unblock block block bsource",
        "L-lmb",    "tomouse",      /*  left button                 */
        "L-mmo",    "tomouse",      /*  mouse move w/left held down */
        "R-rmb",    "iconify",      /*  right button                */
        NULL, NULL
    };

    dealloc_hash();
    OpenDevice("console.device", -1, (struct IORequest *)&cio, 0);
    ConsoleDevice = (struct Library *)cio.io_Device;
    keyboard_init();
    for (i = 0; defmap[i].from; ++i) {
        ubyte code, qual;
        if (get_codequal(defmap[i].from, &code, &qual))
            addhash(code, 1, 0xFF, qual, defmap[i].to);
    }
}

returnoveride(n)
{
    HASH *hash;
    static ubyte *str;
    static int stat;

    for (hash = Hash[0x44&HASHMASK]; hash; hash = hash->next) {
        if (hash->code == 0x44 && hash->qual == 0) {
            if (n) {
                str = (ubyte *)hash->str;
                stat= hash->stat;
                hash->str = "return";
                hash->stat = 1;
            } else {
                if (str == NULL) {
                    remhash(0x44, (ubyte)-1, 0);
                } else {
                    hash->str = (char *)str;
                    hash->stat= stat;
                }
            }
            return(0);
        }
    }
    if (n) {
        addhash(0x44, 1, 0xFF, 0, "return");
        str = NULL;
    }
}

void
addhash(code, stat, mask, qual, str)
ubyte code, stat, mask, qual;
ubyte *str;
{
    HASH **p, *hash;

    hash = *(p = &Hash[code&HASHMASK]);
    while (hash) {
        if (hash->code == code && hash->qual == qual && hash->mask == mask) {
            if (!hash->stat)
                FreeMem(hash->str, strlen(hash->str)+1);
            goto newstr;
        }
        hash = *(p = &hash->next);
    }
    *p = hash = (HASH *)AllocMem(sizeof(HASH), 0);
    hash->next = NULL;
newstr:
    hash->code = code;
    hash->stat = stat;
    hash->mask = mask;
    hash->qual = qual;
    hash->str = (char *)str;
    if (!stat)                  /* if not static */
        hash->str = (char *)strcpy((char *)AllocMem(strlen(str)+1, MEMF_PUBLIC), str);
}


remhash(code, mask, qual)
ubyte code, mask, qual;
{
    HASH *hash, **p;

    hash = *(p = &Hash[code&HASHMASK]);
    while (hash) {
        if (hash->code == code && hash->qual == qual && hash->mask == mask) {
            if (!hash->stat)
                FreeMem(hash->str, strlen(hash->str)+1);
            *p = hash->next;
            FreeMem(hash, sizeof(HASH));
            return(1);
        }
        hash = *(p = &hash->next);
    }
    return(0);
}

char *
keyspectomacro(str)
char *str;
{
    HASH *hash;
    ubyte code, qual;

    if (get_codequal(str, &code, &qual)) {
        for (hash = Hash[code&HASHMASK]; hash; hash = hash->next) {
            if (hash->code == code) {
                if (hash->qual == (qual & hash->mask)) {
                    return(hash->str);
                }
            }
        }
    }
    return(NULL);
}

void
do_map()
{
    ubyte code, qual;

    if (get_codequal(av[1], &code, &qual)) {
        addhash(code, 0, 0xFF, qual, av[2]);
    } else {
        title("Unknown Key");
    }
}

void
do_unmap()        /* key   */
{
    ubyte code, qual;

    if (get_codequal(av[1], &code, &qual)) {
        remhash(code, (ubyte)-1, qual);
    } else {
        title("Unknown Command");
    }
}

void
do_clearmap()
{
    resethash();
}

/*
 * SAVEMAP  file
 * SAVESMAP file
 */

void
do_savemap()
{
    char sysalso;
    char err = 0;
    char buf[256];
    FILE *fi;
    int i;
    HASH *hash;
    ubyte *ptr;

    fi = fopen(av[1], "w");
    if (fi) {
        sysalso = av[0][4] == 's';
        for (i = 0; i < HASHSIZE; ++i) {
            for (hash = Hash[i]; hash; hash = hash->next) {
                if (hash->stat == 0 || sysalso) {
                    char soc = '(';
                    char eoc = ')';
                    char ksoc = '(';
                    char keoc = ')';
                    short len;

                    for (ptr = (ubyte *)hash->str; *ptr; ++ptr) {
                        if (*ptr == '(')
                            break;
                        if (*ptr == '\`') {
                            soc = '\`';
                            eoc = '\'';
                            break;
                        }
                    }
                    len = strlen(ptr = cqtoa(hash->code, hash->qual)) - 1;
                    if (ptr[len] == '(' || ptr[len] == ')') {
                        ksoc = '\`';
                        keoc = '\'';
                    }
                    sprintf(buf, "map %c%s%c %c%s%c\n", ksoc, cqtoa(hash->code, hash->qual), keoc, soc, hash->str, eoc);
                    fputs(buf, fi);
                }
            }
        }
        fclose(fi);
        if (err)
            title ("Unable to Write");
        else
            title ("OK");
    } else {
        title("Unable to open file");
    }
}

/*
 *  Nitty Gritty.
 *
 *  keyboard_init:  initialize for get_codequal() and cqtoa()
 *  get_codequal:   convert a qualifier-string combo to a keycode and qual.
 *  cqtoa:          convert a keycode and qual to a qual & string
 */

#define LN(a,b,c,d)  ((a<<24)|(b<<16)|(c<<8)|d)

static long lname[] = {
    LN('e','s','c', 0  ), LN('f','1', 0 , 0  ), LN('f','2', 0 , 0  ),
    LN('f','3', 0 , 0  ), LN('f','4', 0 , 0  ), LN('f','5', 0 , 0  ),
    LN('f','6', 0 , 0  ), LN('f','7', 0 , 0  ), LN('f','8', 0 , 0  ),
    LN('f','9', 0 , 0  ), LN('f','1','0', 0  ), LN('d','e','l', 0  ),
    LN('b','a','c', 0  ), LN('b','s', 0 , 0  ), LN('t','a','b', 0  ),
    LN('h','e','l', 0  ), LN('r','e','t', 0  ), LN('u','p', 0 , 0  ),
    LN('d','o','w', 0  ), LN('r','i','g', 0  ), LN('l','e','f', 0  ),
    LN('e','n','t', 0  ), LN('n','k','-', 0  ), LN('n','k','.', 0  ),
    LN('n','k','0', 0  ),   /* 24 */
    LN('n','k','1', 0  ), LN('n','k','2', 0  ), LN('n','k','3', 0  ),
    LN('n','k','4', 0  ), LN('n','k','5', 0  ), LN('n','k','6', 0  ),
    LN('n','k','7', 0  ), LN('n','k','8', 0  ), LN('n','k','9', 0  ),
    LN('n','k','(', 0  ), LN('n','k',')', 0  ), LN('n','k','/', 0  ), /*34-36*/
    LN('n','k','*', 0  ), LN('n','k','+', 0  ),
    LN('l','m','b',0xE8), LN('m','m','b',0xEA), LN('r','m','b',0xE9),
    LN('m','m','o',QMOVE),
    0
};


/*
 *  ESC:        x1B
 *  FUNCKEYS:   x9B 30 7E to x9B 39 7E
 *  DEL:        x7E
 *  BS:         x08
 *  TAB:        x09
 *  RETURN:     x0D
 *  HELP        x9B 3F 7E
 *  UP/D/L/R    x9B 41/42/44/43
 *  NK0-9,-,.,ENTER
 *
 *  Mouse buttons
 */

void
keyboard_init()
{
    static struct InputEvent ievent = { NULL, IECLASS_RAWKEY };
    ubyte buf[32];
    short i, len;

    lname[16] |= 0x44;
    lname[21] |= 0x43;

    for (i = 0; i < 128; ++i) {
        ievent.ie_Code = i;
        ievent.ie_Qualifier = 0;
        ievent.ie_position.ie_addr = NULL;
        len = RawKeyConvert(&ievent,buf,32,NULL);
        switch(len) {
        case 1:     /*  ESC/DEL/BS/TAB/NKx  */
            if ((buf[0] & 0x7F) >= 32 && (buf[0] & 0x7F) < 127)
                ctoa[i] = buf[0];
            switch(buf[0]) {
            case 0x1B:  lname[ 0] |= i; break;
            case 0x7F:  lname[11] |= i; break;
            case 0x09:  lname[14] |= i; break;
            case 0x08:  lname[12] |= i; lname[13] |= i; break;
            case '(': if (i > 0x3A) lname[34] |= i; break;
            case ')': if (i > 0x3A) lname[35] |= i; break;
            case '/': if (i > 0x3A) lname[36] |= i; break;
            case '*': if (i > 0x3A) lname[37] |= i; break;
            case '-': if (i > 0x3A) lname[22] |= i; break;
            case '+': if (i > 0x3A) lname[38] |= i; break;
            case '.': if (i > 0x3A) lname[23] |= i; break;
            default:
                if (i >= 0x0F && buf[0] >= '0' && buf[0] <= '9')
                    lname[24+buf[0]-'0'] |= i;
            }
            break;
        case 2:     /*  cursor              */
            if (buf[0] == 0x9B) {
                switch(buf[1]) {
                case 0x41:  lname[17] |= i;  break;
                case 0x42:  lname[18] |= i;  break;
                case 0x43:  lname[19] |= i;  break;
                case 0x44:  lname[20] |= i;  break;
                }
            }
            break;
        case 3:     /*  function/help       */
            if (buf[0] == 0x9B && buf[2] == 0x7E) {
                if (buf[1] == 0x3F)
                    lname[15] |= i;
                if (buf[1] >= 0x30 && buf[1] <= 0x39)
                    lname[buf[1]-0x30+1] |= i;
            }
            break;
        }
    }
    for (i = 0; i < 128; ++i) {
        ievent.ie_Code = i;
        ievent.ie_Qualifier = IEQUALIFIER_LSHIFT;
        ievent.ie_position.ie_addr = NULL;
        len = RawKeyConvert(&ievent,buf,32,NULL);
        if (len == 1)
            cstoa[i] = buf[0];
    }
    {
        ubyte code, qual;
        get_codequal("c", &code, &qual);
        CtlC = code;
    }
}


ubyte *
cqtoa(code, qual)
int qual;
{
    static ubyte buf[32];
    ubyte *ptr = buf;
    int i;

    if (qual & QUAL_SHIFT)
        *ptr++ = 's';
    if (qual & QUAL_CTRL)
        *ptr++ = 'c';
    if (qual & QUAL_ALT)
        *ptr++ = 'a';
    if (qual & QUAL_AMIGA)
        *ptr++ = 'A';
    if (qual & QUAL_LMB)
        *ptr++ = 'L';
    if (qual & QUAL_MMB)
        *ptr++ = 'M';
    if (qual & QUAL_RMB)
        *ptr++ = 'R';
    if (qual)
        *ptr++ = '-';
    for (i = 0; i < sizeof(lname)/sizeof(lname[0]); ++i) {
        if ((lname[i]&0xFF) == code) {
            *ptr++ = (lname[i]>>24);
            *ptr++ = (lname[i]>>16);
            *ptr++ = (lname[i]>>8);
            break;
        }
    }
    if (i == sizeof(lname)/sizeof(lname[0]))
        *ptr++ = ctoa[code];
    *ptr++ = 0;
    return(buf);
}


get_codequal(str, pcode, pqual)
ubyte *pcode, *pqual;
ubyte *str;
{
    char *base = str;
    ubyte qual;
    short i;

    qual = 0;
    if (strlen(str) > 1) {
        for (; *str && *str != '-'; ++str) {
            if (*str == 's')
                qual |= QUAL_SHIFT;
            if (*str == 'c')
                qual |= QUAL_CTRL;
            if (*str == 'a')
                qual |= QUAL_ALT;
            if (*str == 'A')
                qual |= QUAL_AMIGA;
            if (*str == 'L')
                qual |= QUAL_LMB;
            if (*str == 'M')
                qual |= QUAL_MMB;
            if (*str == 'R')
                qual |= QUAL_RMB;
            if (!qual)
                goto notqual;
        }
        if (*str == 0) {
            qual = 0;
            str = base;
        } else {
            ++str;
        }
    }
notqual:
    if (strlen(str) != 1) {           /* long name   */
        short shift = 24;
        long mult = 0;
        ubyte c;

        *pqual = qual;
        while ((c = *str) && shift >= 8) {
            if (c >= 'A' && c <= 'Z')
                c = c - 'A' + 'a';
            mult |= c << shift;
            shift -= 8;
            ++str;
        }
        for (i = 0; lname[i]; ++i) {
            if (mult == (lname[i] & 0xFFFFFF00)) {
                *pcode = lname[i] & 0xFF;
                return(1);
            }
        }
    } else {                /*  single character keycap */
        for (i = 0; i < sizeof(ctoa); ++i) {
            if (*str == ctoa[i]) {
                *pcode = i;
                *pqual = qual;
                return(1);
            }
        }
        for (i = 0; i < sizeof(cstoa); ++i) {
            if (*str == cstoa[i]) {
                *pcode = i;
                *pqual = qual|QUAL_SHIFT;
                return(1);
            }
        }
    }
    return(0);
}


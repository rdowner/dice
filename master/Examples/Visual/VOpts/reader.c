#include "vopts.h"

Prototype int get_simple_token(char *buf);
Prototype int get_token(char *buf1, char *buf2, char *buf3);
Prototype struct G_OBJECT **newobj(struct G_OBJECT **objlist, int class, char *buf);
Prototype int parse_config(char * cfname);
Prototype int init_config(char *cfname);
Prototype void close_config(void);

/*********************************************************************
 * TOKENS:
 *      TOK_EOF     End of file
 *                  '/''*' '*''/'
 *      TOK_STRING  "string"
 *      TOK_STRING  'string'
 *      TOK_BAR     BAR
 *      TOK_TEXT    TEXT   <string>
 *      TOK_BUTTON  BUTTON <string>
 *      TOK_CHECK   CHECK  <string> <string> <string>
 *      TOK_CYCLE   CYCLE  <string>
 *      TOK_GROUP   GROUP  <string>
 *      TOK_LGROUP  LGROUP <string>
 *      TOK_ITEM    ITEM   <string> <string>
 *      TOK_LIST    LIST   <string> <string> <string>
 *      TOK_MENU    MENU   <string>
 *      TOK_STRING  STRING <string> <string>
 *      TOK_TITLE   TITLE  <string>
 *      TOK_VALUE   VALUE  <string> <string>
 */
#define TOK_ERROR  -1
#define TOK_EOF     0 /* Note ordering of ERROR/EOF is assumed for easy tests below */
#define TOK_STRTOK  1
#define TOK_TOKEN   2
#define TOK_BAR     3
#define TOK_BUTTON  4
#define TOK_CHECK   5
#define TOK_CYCLE   6
#define TOK_GROUP   7
#define TOK_LGROUP  8
#define TOK_ITEM    9
#define TOK_LIST   10
#define TOK_MENU   11
#define TOK_STRING 12
#define TOK_TITLE  13
#define TOK_VALUE  14
#define TOK_TEXT   15

struct TOK_LOOK {
   char toktype;   /* Value to return for this token                        */
   char token[6];  /* Note that we only allow 6 characters in a token       */
   char strings;   /* Number of strings that the token takes as an argument */
};

struct TOK_LOOK tokens[] = {
   { TOK_BAR,     "BAR   ", 0},
   { TOK_BUTTON,  "BUTTON", 1},
   { TOK_CHECK,   "CHECK ", 3},
   { TOK_CYCLE,   "CYCLE ", 1},
   { TOK_GROUP,   "GROUP ", 1},
   { TOK_LGROUP,  "LGROUP", 1},
   { TOK_ITEM,    "ITEM  ", 3},
   { TOK_LIST,    "LIST  ", 2},
   { TOK_MENU,    "MENU  ", 1},
   { TOK_STRING,  "STRING", 2},
   { TOK_TITLE,   "TITLE ", 1},
   { TOK_VALUE,   "VALUE ", 2},
   { TOK_TEXT,    "TEXT  ", 1},
};
#define MAX_LOOK (sizeof(tokens)/sizeof(struct TOK_LOOK))

/*********************************************************************
 * Character classes:
 *  0  CL_EOF    - EOF
 *  1  CL_SLASH  - /
 *  2  CL_STAR   - *
 *  3  CL_DQUOTE - "
 *  4  CL_SQUOTE - '
 *  5  CL_ALPHA  - A-Z a-z
 *  6  CL_BLANK  - ' ' \t \n
 *  7  CL_OTHER  - Anything else
 */
#define CL_EOF     0
#define CL_SLASH   1
#define CL_STAR    2
#define CL_DQUOTE  3
#define CL_SQUOTE  4
#define CL_ALPHA   5
#define CL_BLANK   6
#define CL_OTHER   7
#define MAX_CL     8

/*********************************************************************
 * States:
 *  0  ST_SCN - SCAN   - Scanning - Looking for any character - skipping white space
 *  1  ST_GSL - GOTSL  - Found /  - Checking for a matching * to start a comment
 *  2  ST_CMT - CMT    - Comment  - Looking for a * to close a comment
 *  3  ST_GST - GOTST  - Found *  - Checking for a / to close a comment
 *  4  ST_DQT - DQUOTE - String " - Looking for a matching "
 *  5  ST_SQT - SQUOTE - String ' - Looking for a matching '
 *  6  ST_TOK - TOKEN  - Token    - Gathering a keyword token
 */
#define ST_SCN  0
#define ST_GSL  1
#define ST_CMT  2
#define ST_GST  3
#define ST_DQT  4
#define ST_SQT  5
#define ST_TOK  6
#define MAX_ST 7
#define MASK_ST 7

/*********************************************************************
 * State Table Transitions:
 *
 *            Character Class
 *            0-EOF   1-/      2-*      3-"      4-'      5-A-Z      6-Blank  7-Other
 *   State
 *   0 SCAN    END    GOTSL   ERROR    DQUOTE   SQUOTE    >TOKEN     SCAN     ERROR
 *   1 GOTSL  ERROR   ERROR   CMT      ERROR    ERROR     ERROR      ERROR    ERROR
 *   2 CMT    ERROR   CMT     GOTST    CMT      CMT       CMT        CMT      CMT
 *   3 GOTST  ERROR   SCAN    CMT      CMT      CMT       CMT        CMT      CMT
 *   4 DQUOTE ERROR   >DQUOTE >DQUOTE  !SCAN    >DQUOTE   >DQUOTE    >DQUOTE  >DQUOTE
 *   5 SQUOTE ERROR   >SQUOTE >SQUOTE  >SQUOTE  !SCAN     >SQUOTE    >SQUOTE  >SQUOTE
 *   6 TOKEN  +END    +GOTSL  ERROR    +DQUOTE  +SQUOTE   >TOKEN     +SCAN    ERROR
 * Note: >     - AC_SAV - Means to append the current character and continue scan
 *       !     - AC_STR - Means to return the current token as a string.
 *       +     - AC_TOK - Means to return the current token as a keyword
 *       ERROR - AC_ERR - Indicates issuing an error
 *       END   - AC_END - Indicates returing the END token.
 */
#define AC_SKP (0<<5) /* Must be zero - the default to do nothing */
#define AC_SAV (1<<5)
#define AC_STR (2<<5)
#define AC_TOK (3<<5)
#define AC_ERR (4<<5)
#define AC_END (5<<5)
#define MASK_AC (7<<5)

char statetab[MAX_ST][MAX_CL] =
{
/* 0 ST_SCN  */
   { AC_END,        ST_GSL,         AC_ERR,         ST_DQT,
     ST_SQT,        AC_SAV|ST_TOK,  ST_SCN,         AC_ERR         },
/* 1 ST_GSL */
   { AC_ERR,        AC_ERR,         ST_CMT,         AC_ERR,
     AC_ERR,        AC_ERR,         AC_ERR,         AC_ERR         },
/* 2 ST_CMT   */
   { AC_ERR,        ST_CMT,         ST_GST,         ST_CMT,
     ST_CMT,        ST_CMT,         ST_CMT,         ST_CMT         },
/* 3 ST_GST */
   { AC_ERR,        ST_SCN,         ST_CMT,         ST_CMT,
     ST_CMT,        ST_CMT,         ST_CMT,         ST_CMT         },
/* 4 ST_DQT*/
   { AC_ERR,        AC_SAV|ST_DQT,  AC_SAV|ST_DQT,  AC_STR|ST_SCN,
     AC_SAV|ST_DQT, AC_SAV|ST_DQT,  AC_SAV|ST_DQT,  AC_SAV|ST_DQT  },
/* 5 ST_SQT*/
   { AC_ERR,        AC_SAV|ST_SQT,  AC_SAV|ST_SQT,  AC_SAV|ST_SQT,
     AC_STR|ST_SCN, AC_SAV|ST_SQT,  AC_SAV|ST_SQT,  AC_SAV|ST_SQT  },
/* 6 ST_TOK */
   { AC_TOK|ST_SCN, AC_TOK|ST_GSL,  AC_ERR,         AC_TOK|ST_DQT,
     AC_TOK|ST_SQT, AC_SAV|ST_TOK,  AC_TOK|ST_SCN,  AC_ERR         },
};

/*
 * Get a simple token from the file.
 * No parsing of semantics is done at this level.
 */
int get_simple_token(char *buf)
{
   int c;
   int pos;
   int class;
   int action;

   pos = 0;

   for(;;)
   {
      /* Get the next character from the input file and assign a character class */
      c = getc(global.fp);
      class = CL_OTHER;
      switch(c)
      {
         case EOF:  class = CL_EOF;    break;
         case '/':  class = CL_SLASH;  break;
         case '*':  class = CL_STAR;   break;
         case '"':  class = CL_DQUOTE; break;
         case '\'': class = CL_SQUOTE; break;
         case '\n': global.line++;
         case ' ':
         case '\t':  class = CL_BLANK;  break;
         default:
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
               class = CL_ALPHA;
            break;
      }
      /* Run us through the state table to get an action and a new state */
      action = statetab[global.state][class];
      global.state = (action & MASK_ST);

      /* Perform the work for the action */
      switch(action & MASK_AC)
      {
         case AC_SAV: if (pos < 64) /* make sure we don't overflow the buffer */
                      {
                         buf[pos++] = c;  /* Save the character */
                         break;
                      }
                      /* Else fall through to the error case */
         case AC_ERR: return(TOK_ERROR);
         case AC_STR: buf[pos] = 0;   /* Null terminate what we gathered */
                      return(TOK_STRTOK);
         case AC_TOK: buf[pos] = 0;
                      return(TOK_TOKEN);
         case AC_END: return(TOK_EOF);
      }
   }
}

/*
 * Get the next token and any associated strings.
 */
int get_token(char *buf1,
              char *buf2,
              char *buf3
             )
{
   int i;
   int j;
   int toktype;
   int len;
   char *strs[3];

   strs[0] = buf1;
   strs[1] = buf2;
   strs[2] = buf3;

   toktype = get_simple_token(buf1);
   if (toktype == TOK_EOF)   return(toktype);
   if (toktype != TOK_TOKEN) return(TOK_ERROR);

   /* Go through and uppercase the string */
   /* we KNOW that it consists of only upper/lower case letters */
   len = strlen(buf1);
   if (len > 6) return(TOK_ERROR);
   strcpy(buf1+len, "     "); /* Make sure we blank pad for the memcmp */
   for(i = 0; i < 6; i++)
      if (buf1[i] >= 'a') buf1[i] -= ('a'-'A');

   /* Now go through the table and look for a token */
   for (i = 0; i < MAX_LOOK; i++)
      if (!memcmp(buf1, tokens[i].token, 6)) break;
   if (i == MAX_LOOK) return(TOK_ERROR);

   /* We have a matching token, get any strings that it wants */
   for(j = 0; j < tokens[i].strings; j++)
   {
      toktype = get_simple_token(strs[j]);
      if (toktype != TOK_STRTOK) return(TOK_ERROR);
   }

   /* Everything checks out, let them know what type of token they got */
   return((int)tokens[i].toktype);
}

void init_group(struct G_OBJECT *object)
{
   while(object != NULL)
   {
      if (object->next) object->next->prev = object;
      if (object->class == CLASS_CYCLE)
      {
         struct G_VALUE *val;
         struct G_CYCLE *cyc;

         cyc = (struct G_CYCLE *)object;

         cyc->curval = cyc->values;
         for(val = cyc->curval; val; val = val->next)
         {
            if (cyc->base.next) cyc->base.next->prev = (struct G_OBJECT *)cyc;
         }
      }
      object = object->next;
   }
}
/*
 * Allocate an object structure to hold an entry
 */
static char sizetab[] = { 0,
                          sizeof(struct G_STRING),
                          sizeof(struct G_CYCLE),
                          sizeof(struct G_CHECK),
                          sizeof(struct G_GROUP),
                          sizeof(struct G_LIST)
                         };

struct G_OBJECT **newobj(struct G_OBJECT **objlist,
                         int class,
                         char *buf
                        )
{
   struct G_OBJECT *obj;

   obj = get_mem(sizetab[class]);
   if (obj == NULL) return(NULL);

   if (objlist) *objlist = obj;
   obj->class = class;
   obj->title = savestr(buf);
   return(&obj->next);
}

/*
 * Parse the configuration file
 */
int parse_config(char *cfname)
{
   static char buf1[66], buf2[66], buf3[66];
   int toktype;
   int menupos;
   int buttonpos;
   int groupcnt;
   int listflag;
   int textpos;
   struct G_OBJECT **objlist;
   struct G_GROUP  **grplist;
   struct G_VALUE  **valent;
   struct G_GROUP  *group;

   grplist = &global.groups;
   objlist = &global.objects;
   valent = NULL;

   textpos = menupos = buttonpos = 0;
   groupcnt = 0;
/*
   listflag = 0;
*/
   listflag = 1;
   if (init_config(cfname)) return(2);

   while((toktype = get_token(buf1, buf2, buf3)) > TOK_EOF)
   {
      switch(toktype)
      {
         case TOK_TEXT:   global.text[textpos] = savestr(buf1);
                          if (textpos++ > MAX_TEXT) goto error;
                          break;
         case TOK_BAR:    global.menuitem[menupos].nm_Type   = MENU_ITEM;
                          global.menuitem[menupos].nm_Label  = NM_BARLABEL;
                          if (menupos++ > MAX_MENU) goto error;
                          break;
         case TOK_MENU:   global.menuitem[menupos].nm_Type   = MENU_MENU;
                          global.menuitem[menupos].nm_Label  = savestr(buf1);
                          if (menupos++ > MAX_MENU) goto error;
                          break;
         case TOK_ITEM:   global.menuitem[menupos].nm_Type   = MENU_ITEM;
                          global.menuitem[menupos].nm_Label  = savestr(buf1);
                          if (buf2[0])
                             global.menuitem[menupos].nm_CommKey = savestr(buf2);
                          global.menuitem[menupos].nm_UserData = savestr(buf3);
                          if (menupos++ > MAX_MENU) goto error;
                          break;
         case TOK_BUTTON: if (buttonpos > MAX_BUTTON) goto error;
                          global.button[buttonpos++].title = savestr(buf1);
                          break;
         case TOK_TITLE:  strcpy(global.wtitle, buf1);
                          break;
         case TOK_LGROUP:
         case TOK_GROUP:  grplist = (struct G_GROUP **)
                                    newobj(
                                           (struct G_OBJECT **)grplist,
                                           CLASS_GROUP, buf1);
                          if (!grplist) return(1);
                          objlist = &(((struct G_GROUP *)grplist)->objects);
                          valent = NULL;
/*
                          (*grplist)->local = toktype == TOK_LGROUP ? 1 : 0;
*/
                          ((struct G_GROUP *)grplist)->local
                                            = (toktype == TOK_LGROUP ? 1 : 0);
                          if (groupcnt > global.maxsize) global.maxsize = groupcnt;
                          groupcnt = 2;
                          break;
         case TOK_STRING: objlist = newobj(objlist, CLASS_STRING, buf1);
                          if (!objlist) return(1);
                          groupcnt++;
                          ((struct G_STRING *)objlist)->option = savestr(buf2);
                          valent = NULL;
                          break;
         case TOK_CHECK:  objlist = newobj(objlist, CLASS_CHECK, buf1);
                          if (!objlist) return(1);
                          groupcnt++;
                          ((struct G_CHECK *)objlist)->option0 = savestr(buf2);
                          ((struct G_CHECK *)objlist)->option1 = savestr(buf3);
                          valent = NULL;
                          break;
         case TOK_LIST:   objlist = newobj(objlist, CLASS_LIST, buf1);
                          if (!objlist) return(1);
                          groupcnt += 3;
                          /* lists are just a bit bigger than three lines,   */
                          /* so fudge on the first one, a few more will fit  */
/*
                          listflag = !listflag;
*/
                          groupcnt += listflag;
/* Next line replaces trick above to add half ine per list */
                          listflag = 0; 
                          ((struct G_LIST *)objlist)->option = savestr(buf2);
                          valent = NULL;
                          break;
         case TOK_CYCLE:  objlist = newobj(objlist, CLASS_CYCLE, buf1);
                          if (!objlist) return(1);
                          groupcnt++;
                          valent = &((struct G_CYCLE *)objlist)->values;
                          break;
         case TOK_VALUE:  if (valent == NULL) goto error;
                          *valent = get_mem(sizeof(struct G_VALUE));
                          if (!*valent) return(1);
                          (*valent)->next = NULL;
                          (*valent)->title = savestr(buf1);
                          (*valent)->option = savestr(buf2);
                          (*valent)->string = NULL;
                          if (strchr(buf2, '%'))
                          {
                             struct G_STRING *gstr;

                             gstr = get_mem(sizeof(struct G_STRING));
                             if (!gstr) return(1);
                             gstr->base.class = CLASS_STRING;
                             (*valent)->string = gstr;
                          }
                          valent = &((*valent)->next);
                          break;
      }
   }
   if (groupcnt > global.maxsize) global.maxsize = groupcnt;
   global.curgroup = global.groups;
   global.menuitem[menupos].nm_Type = MENU_END;

   if (toktype == TOK_ERROR)
   {
error:
      sprintf(global.title, "ERROR- %s line %d", cfname, global.line);
      return(1);
   }
   close_config();

   /* Now we need to go through the groups and sanitize the pointers as well */
   /* as initialize all the default states                                   */
   init_group(global.objects);
   for(group = global.groups; group != NULL;
       group = (struct G_GROUP *)group->base.next)
   {
      if (group->base.next) group->base.next->prev = (struct G_OBJECT *)group;
      init_group(group->objects);
   }
   return(0);
}

/*
 * Open the configuration file and initialize any global data
 */
int init_config(char *cfname)
{
   struct Process *mytask;
   APTR saveptr;

   global.state = ST_SCN;
   global.line  = 1;
   mytask = (struct Process *)FindTask(NULL);
   saveptr = mytask->pr_WindowPtr;
   mytask->pr_WindowPtr = (APTR)-1;
   global.fp = fopen(cfname, "r");
   mytask->pr_WindowPtr = saveptr;
   if (global.fp == NULL)
   {
      sprintf(global.title, "ERROR- No %s", cfname);
      return(1);
   }
   return(0);
}

/*
 * Close the configuration file and clean up anything else necessary
 */
void close_config()
{
   if (global.fp)
      fclose(global.fp);
   global.fp = NULL;
}

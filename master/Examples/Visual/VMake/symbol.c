#ifdef MAIN
#include <stdio.h>
#include <string.h>
#define free_mem(a,b) free(a)
#define get_mem(a)    malloc(a)
#define Prototype
#else
#include "vmake.h"
#endif
struct SYMBOL {
  struct SYMBOL *next;
  char *val;
  short curlen;
  short allotlen;
  char name[1];
};
Prototype void Sym_Clear(void);
Prototype void *Sym_Next(void *cursym, char **name, char **val);
Prototype char *Sym_Lookup(char *name);
Prototype int Sym_Set(char *name, char *val1, char *val2);

struct SYMBOL *base, *last; /* Root and end of all symbol entries   */
struct SYMBOL *symcache;    /* Cached entry of last symbol found    */
/***********************************************************************************
 * Procedure: Sym_Clear
 * Synopsis:  rc = set_option(object, optstr)
 * Purpose:   Clear out all of the symbols.  This leaves private ones untouched
 ***********************************************************************************/
void Sym_Clear()
{
   struct SYMBOL *sym, *symnext, *savesym;

   savesym = NULL;

   sym = base;
   base = last = NULL;

   while(sym != NULL)
   {
      symnext = sym->next;
      if (sym->name[0] == '_')
      {
         if (base == NULL)
         {
            base = last = sym;
         }
         else
         {
            last = last->next = sym;
         }
         sym->next = NULL;
      }
      else
      {
         free_mem(sym->val, sym->allotlen);
         free_mem(sym, sizeof(struct SYMBOL) + strlen(sym->name));
      }
      sym = symnext;
   }
   symcache = NULL;
}

/***********************************************************************************
 * Procedure: Sym_Next
 * Synopsis:  place = Sym_Next(place, &name, &val);
 * Purpose:   Return the next public symbol in a string
 ***********************************************************************************/
void *Sym_Next(void *cursym,
               char **name,
               char **val)
{
   struct SYMBOL *sym;
   sym = (struct SYMBOL *)cursym;

   if (sym == NULL) sym = base;
   else sym = sym->next;

   while (sym && (sym->name[0] == '_'))
   {
      sym = sym->next;
   }

   *name = "ILLEGAL";
   *val  = "ILLEGAL";
   if (sym != NULL)
   {
      *name = sym->name;
      *val  = sym->val;
   }
   return(sym);
}

/***********************************************************************************
 * Procedure: findsymbol
 * Synopsis:  SYMBOL = findsymbol(name);
 * Purpose:   Find the symbol entry for a given name
 ***********************************************************************************/
static struct SYMBOL *findsymbol(char *name)
{
   struct SYMBOL *sym;
   /* See if they are asking for the one we hit last time */
   if (symcache && !strcmp(symcache->name, name)) return(symcache);
   /* Not the same one, do a sequential search on the list.  We can do this     */
   /* Because we know that the number of entries that are generally stored      */
   /* is quite small.  If we change this, this routine would have to be updated */
   for(sym = base; sym != NULL; sym = sym->next)
   {
      if (!strcmp(sym->name, name))
      {
         symcache = sym;      /* Remember it for the cache */
         break;
      }
   }
   return(sym);
}

/***********************************************************************************
 * Procedure: Sym_Lookup
 * Synopsis:  val = Sym_Lookup(name)
 * Purpose:   Find the substitution value for a string
 *            If the symbol is not found, a null string is returned.
 *            Originally this returned NULL, but everyone that wants to use it was
 *            checking for this case and then setting the string to "" anyway.
 *            We can create a new routine if this were desired.
 ***********************************************************************************/
char *Sym_Lookup(char *name)
{
   struct SYMBOL *sym;

   sym = findsymbol(name);
   if (sym) return(sym->val);
   return("");
}

/***********************************************************************************
 * Procedure: Sym_Set
 * Synopsis:  rc = Sym_Set(name, val1, val2)
 * Purpose:   Set a symbol to the concatenation of two values
 ***********************************************************************************/
int Sym_Set(char *name, char *val1, char *val2)
{
   struct SYMBOL *sym;
   int len2, len, len1;
   char *newval;

   sym = findsymbol(name);
   if (sym == NULL)
   {
      /* There is a first time for everything, create an empty structure */
      sym = get_mem(sizeof(struct SYMBOL)+strlen(name));
      if (sym == NULL) return(1);
      strcpy(sym->name, name);
      sym->curlen = sym->allotlen = 0;
      sym->val = "";
      sym->next = NULL;
      if (last)
         last->next = sym;
      else
         base = sym;
      last = sym;
   }

   /* If they pass NULL as the first parameter, we are doing a concat */
   if (val1 == NULL) val1 = sym->val;

   /* We have a valid symbol, see if we can tack on to the end of it */
   /* This one special case will occur when we do concenation to the */
   /* end of a string                                                */
   if (val2 == NULL) val2 = "";
   len2 = strlen(val2);
   if ((val1 == sym->val) && ((sym->curlen + len2) < sym->allotlen))
   {
      /* It is just a tackon case  */
#ifdef MAIN
      printf("Tackon: %d to %d allot=%d\n", len2, sym->curlen, sym->allotlen);
#endif
      strcpy(sym->val+sym->curlen, val2);
      sym->curlen += len2;
      return(0);
   }

   /* No, we need to allocate a new area and copy from the two strings into  */
   /* That location.  Note that we can not free the original area because it */
   /* is possible that one of the strings point to it.                       */
   len1 = strlen(val1);
   len = (len1 + len2 + 64) & ~63;
#ifdef MAIN
   printf("Allocate %d for %d + %d\n", len, len1, len2);
#endif
   newval = get_mem(len);
   if (newval == NULL) return(1);
   strcpy(newval, val1);
   strcpy(newval+len1, val2);
   if (sym->allotlen) free_mem(sym->val, sym->allotlen);
   sym->val = newval;
   sym->allotlen = len;
   sym->curlen = len1+len2;
   return(0);
}
/***********************************************************************************
 * Procedure: set_option
 * Synopsis:  rc = set_option(object, optstr)
 * Purpose:   Set an option based on a option string
 ***********************************************************************************/
#ifdef MAIN
void main()
{
   char buf[100];
   char *p;
   void *place;
   char *name, *val;
   int rc;

   while((printf(">"), fflush(stdout), gets(buf)) != NULL)
   {
      name = buf+1;
      switch(buf[0])
      {
         case 'd':
            place = NULL;
            while(place = Sym_Next(place, &name, &val))
               printf("%s= %s\n", name, val);
            break;
         case 'p':
            val = Sym_Lookup(name);
            if (val)
               printf("%s= %s\n", name, val);
            else
               printf("%s not found\n", name);
            break;
         case 'c':
            Sym_Clear();
            printf("Cleared\n");
            break;
         case 'a':
         case 's':
            p = name+1;
            while(*p && *p != ' ') p++;
            *p = 0;
            val = p+1;
            if (buf[0] == 'a')
            {
               p = Sym_Lookup(name);
               printf("Setting '%s' to '%s%s'\n", name, p, val);
               rc = Sym_Set(name, NULL, val);
            }
            else
            {
               printf("Setting '%s' to '%s'\n", name, val);
               rc = Sym_Set(name, val, NULL);
            }
            if (rc) printf("Sym_Set failed\n");
            break;
         case 'q':
            Sym_Clear();
            return;
         default:
            printf("Unknown command: %s\n", buf);
            printf("d         - Dump all symbols\n");
            printf("p sym     - Print value of sym\n");
            printf("c         - Clear all symbold\n");
            printf("a sym val - Append val to sym\n");
            printf("s sym val - Set sym to val\n");
            break;
      }
   }
}
#endif

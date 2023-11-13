/*
**      $Id: CED_ErrorParse.rexx,v 30.0 1994/06/10 18:06:05 dice Exp $
**
**      DICE Error Parsing Script.  Script for Oxxi TurboText (tm).
**
**  Notes: This assumes that your DCC:Config/DCC.Config file contains the
**         following line:
**  cmd= rx DCC:Rexx/TTX_ErrorParse.rexx %e "%c" "%f" "%0"
*/

OPTIONS RESULTS

PARSE ARG EFile '"' Fn '" "' CurDir '" "' CFile '" "' VPort '"'
IF VPort = '?' THEN VPort = ''

portname = 'DICE_ERROR_PARSER'  /* DICEHelp's port name */

if ~show('p',portname) then
   do
      address COMMAND 'RUN >NIL: <NIL: DError REXXSTARTUP'

      do i = 1 to 6
         if ~show('p',portname) then
            address COMMAND 'wait 1'
      end

      if ~show('p',portname) then
         do
            say "Dice Error Parser (DERROR) program not found!"
            address COMMAND 'type' EFile
            exit
         end
   end

/*

/**
 ** Get the error messages loaded in.
 ** This will return a list of lines within the file that have
 ** errors associated with them (if any)
 **/
ADDRESS DICE_ERROR_PARSER LOAD EFile '"'CurDir'" "'Fn'" "'VPort'"'
LINES = RESULT

/**
 ** go to the first error message
 **/

ADDRESS DICE_ERROR_PARSER Current E
IF rc ~= 0 THEN
   DO
      'DM No More Errors'
      exit 0
   END

IF E.LINE = 0 THEN
   DO
   IF LEFT(E.TEXT, 5) = 'DLINK' THEN
      DO
         TT = TRANSLATE(E.STRING, '-', '"')
         /* This is a DLINK error, we need to handle it special */
         SAY 'There were DLINK Errors'
         ADDRESS COMMAND TYPE EFILE
         exit 0
      END
   END

ADDRESS COMMAND 'ed' E.FPATH '-i'
IF RC ~= 0 THEN
   DO
      Say 'Unable to open' E.FPATH
      exit 0
   END

do i = 1 to 6
   if ~show('p','rexx_ced') then
      address COMMAND 'wait 1'
end

ADDRESS 'rexx_ced'
'jump to line' E.line   /* Jump straight to the line number. */
'DM' E.STRING

/*
**      $Id: DME_ErrorParse.rexx,v 30.0 1994/06/10 18:06:11 dice Exp $
**
**      DICEHelp help system.  Script for DME
**
**      Rexx script modeled after version by David Joiner
**
**  Notes: This assumes that your DCC:Config/DCC.Config file contains the
**         following line:
**  cmd= rx DCC:Rexx/DME_ErrorParse.rexx %e "%c" "%f" "%0"
*/

OPTIONS RESULTS

PARSE ARG EFile '"' Fn '" "' CurDir '" "' CFile '" "' VPort '"'

say "TEST"

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

/**
 ** Get the error messages loaded in.
 ** This will return a list of lines within the file that have
 ** errors associated with them (if any)
 **/
ADDRESS DICE_ERROR_PARSER LOAD EFile '"'CurDir'" "'Fn'" "'VPort'"'
LINES = RESULT
ADDRESS COMMAND 'Delete' EFile

/*
 * Start DME if it is not already running
 */

IF ~show('p','DME.01') THEN
    DO
        address COMMAND 'RUN >NIL: <NIL: DME'
        address COMMAND 'Wait 5'
    END

ADDRESS DME.01 title "( Hit F7 for first error, F8 for next )"


/* My UUTRAF script */

rc = Open('dbase', 'errors', 'R');
if ~rc
   then
   do
      say 'Unable to open errors'
      exit 20
   end

file = ""

do forever
   str = readln('dbase')
   if eof('dbase') then exit

   parse var str with base '*** ' extra

   if extra ~= '' then
   do
      file = extra
   end
   else
   do
      if str ~= '' then
         say left(file,10) str
   end
end

/* $VER: BuildCTags 37.1 (21.2.93) */ 
/* Program to build ctags file */

parse arg file .

if open('infile',file,'R') = 0 then do;
   say "can't open" file
   exit
   end

if pos(':',file) = 0 then do;
    direct = pragma('D')
    if right(direct,1) ~= ':' then direct = direct||'/'
    end;
else do;
    direct = ' '
end

do until eof('infile')
    instring = readln('infile')
    if (compare(instring,'Prototype') > 9) then iterate
    if (datatype(Left(instring,1),'m') = 1) then do;
        j = pos( '(', instring);
        if j ~= 0 then do;
            i = lastpos(' ',instring,j)
            search = '/^' || instring || '^/'
            name = substr(instring,i+1,j-i-1)
            say name direct||file search
            end;
        end;
end;
close('infile')

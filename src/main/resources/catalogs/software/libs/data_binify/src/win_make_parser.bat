@echo off
bin\release\win_flex -d lexer.l
bin\release\win_bison -d parser.y
echo -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
echo "You will need to change #include <FlexLeyer.h> to #include "FlexLexer.h" in scanner.cxx before compiling"
echo -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

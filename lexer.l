%option noyywrap

%{
#include "formula.h"
#include "parser.hpp"
%}

%%

[A-Za-z][A-Za-z_0-9]*        { yylval.str = yytext; return VAR; }
\(                           { return *yytext; }
\)                           { return *yytext; }
&                            { return AND; }
\|                           { return OR; }
=\>                          { return IMP; }
\<=\>                        { return IFF; }
!                            { return NOT; }
[ \t\n]

%%
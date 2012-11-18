%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timedAutomataParser.tab.h"
#include "stringGarbage.h" 
%}

%option noyywrap
%option yylineno

%%

[ \t\n\r] {}

"network" { initStringGarbageCollector(); 
            return NETWORK; }
"automaton" { return AUTOMATON; }
"locations" { return LOCATIONS; }
"location" { return LOCATION; }
"invariant" { return INVARIANT; }
"transitions" { return TRANSITIONS; }
"action" { return ACTION; }
"guard" { return GUARD; }
"reset" { return RESET; }
"initial" { return INITLOC; }
"labelling" { return LABELLING; }
"property" { return PROPERTY; }

"true"|"false" { yylval.str = strdupgar(yytext); return LOGVAL; }
"and"|"or" { yylval.str = strdupgar(yytext); return LOPERATOR; }
"not" { yylval.str = strdupgar(yytext); return LNEG; }
"==" { yylval.str = "="; return RELATION; }
"<"|">"|"<="|">=" { yylval.str = strdupgar(yytext); return RELATION; }
"=" { yylval.str = strdupgar(yytext); return SUBSTITUTION; }
"-"|"+" { yylval.str = strdupgar(yytext); return MINADD; }
"*" { yylval.str = strdupgar(yytext); return MUL; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LCURLY; }
"}" { return RCURLY; }
";" { return SEMICOLON; }
":" { return COLON; }
"," { return COMMA; }

"x"[0-9]* { yylval.str = strdupgar(yytext); return CLOCK; }

[+|-]?[0-9][0-9]* { yylval.str = strdupgar(yytext); return NUMBER; }

[a-zA-Z]+[a-zA-Z0-9_]* { yylval.str = strdupgar(yytext); return STRING; }

%%

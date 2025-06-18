
%option noyywrap yylineno
%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include <string.h>
#include "robot_lang.tab.h"
static long long parse_hex(const char *s) {
    return strtoll(s + 2, NULL, 16);
}
%}

DIGIT       [0-9]
HEX_DIGIT   [0-9A-Fa-f]
LETTER      [A-Za-z]
ID          {LETTER}({LETTER}|{DIGIT}|_)*

WS          [ \t\r]+
NEWLINE     \n

%%
"VAR"           { return VAR; }
"TRUE"          { yylval.i = 1; return TRUE_LIT; }
"FALSE"         { yylval.i = 0; return FALSE_LIT; }
"INF"           { return INF_LIT; }
"NAN"           { return NAN_LIT; }

"if"            { return IF; }
"do"            { return DO; }
"elif"          { return ELIF; }
"else"          { return ELSE; }
"elseif"        { return ELIF; }
"done"          { return DONE; }

"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }

"function"      { return FUNCTION; }
"return"        { return RETURN; }

"EMPTY"         { return EMPTY; }
"WALL"          { return WALL; }
"BOX"           { return BOX; }
"EXIT"          { return EXIT; }
"UNDEF"         { return UNDEF; }

"LOOK"          { return LOOK; }
"TEST"          { return TEST; }
"see"           { return LOOK; }
"see!"          { return TEST; }
"forward"       { return FORWARD; }
"backward"      { return BACKWARD; }
"left"          { return LEFT; }
"right"         { return RIGHT; }
"load"          { return LOAD; }
"drop"          { return DROP; }

"0x"{HEX_DIGIT}+ {
                    long long v = parse_hex(yytext);
                    yylval.i = v;
                    return HEX_LIT;
                }

{DIGIT}+        {
                    yylval.i = atoll(yytext);
                    return INT_LIT;
                }

"#"{ID}         {
                    yylval.s = strdup(yytext + 1);
                    return SUMARR;
                }

{ID}            {
                    yylval.s = strdup(yytext);
                    return IDENT;
                }

":="            { return ASSIGN; }
"<"             { return '<'; }
">"             { return '>'; }
"="             { return '='; }
"?"             { return '?'; }
"^"             { return '^'; }
"-"             { return '-'; }
"+"             { return '+'; }
"["             { return '['; }
"]"             { return ']'; }
"("             { return '('; }
")"             { return ')'; }
","             { return ','; }

{NEWLINE}       { return NEWLINE; }
{WS}            { /* пропускаем пробелы */ }

.               { return yytext[0]; }
%%


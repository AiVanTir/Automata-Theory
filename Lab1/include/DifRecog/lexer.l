%{
#include <iostream>
#include <string>
#include <vector>
#include "DifRecog/FlexRecog.hpp"

extern std::vector<Token> tokens;

extern "C" {
    int yylex();
}

yy_buffer_state* yy_scan_string(const char* str);
%}

%%

"int"       { tokens.push_back(TYPE_INT);}
"short"     { tokens.push_back(TYPE_SHORT);}
"long"      { tokens.push_back(TYPE_LONG);}

[a-zA-Z][a-zA-Z0-9]{0,15} { tokens.push_back(IDENTIFIER);}

-?[0-9]+      { tokens.push_back(NUMBER);}

"["         { tokens.push_back(OPEN_BRACKET);}
"]"         { tokens.push_back(CLOSE_BRACKET);}
"{"         { tokens.push_back(OPEN_BRACE);}
"}"         { tokens.push_back(CLOSE_BRACE);}
"="         { tokens.push_back(EQUALS);}
","         { tokens.push_back(COMMA);}

[ \t\n]     ;

.           { tokens.push_back(ERROR);}

%%

int yywrap() {
    return 1;
}
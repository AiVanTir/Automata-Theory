%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "interp.h"
#include "maze.h"

extern int yylex(void);
extern void yyerror(const char *s);
extern int yylineno;

AST *ast_root = NULL;
%}

%define parse.trace


%union {
    long long i;   /* для литералов INT_LIT, HEX_LIT, TRUE_LIT/FALSE_LIT */
    char    *s;    /* для IDENT и SUMARR («#имяМассива») */
    AST     *a;    /* для узлов AST */
    int      cell; /* для CELL-литералов */
    struct {
        char  **names;
        int     count;
    } varlist;
    char    *sumarr_name;
}

%token <i> INT_LIT
%token <i> HEX_LIT
%token <i> TRUE_LIT
%token <i> FALSE_LIT
%token     INF_LIT
%token     NAN_LIT

%token <s> IDENT
%token  EMPTY
%token  WALL
%token  BOX
%token  EXIT
%token  UNDEF
%token <sumarr_name> SUMARR

%token VAR
%token ASSIGN

%token IF DO ELIF ELSE DONE
%token WHILE
%token BREAK CONTINUE
%token FUNCTION RETURN

%token LOOK TEST FORWARD BACKWARD LEFT RIGHT LOAD DROP

%token NEWLINE

/* приоритеты и ассоциативность для выражений */
%right UMINUS          /* унарный минус (самый высокий приоритет) */
%left '<' '>' '=' '?'  /* сравнения (низший приоритет) */
%left '^'              /* XOR (средний приоритет) */
%left '+' '-'          /* арифметика (высший из бинарных) */

%type <varlist> var_list
%type <a> program stmt_list stmt expr primary

%%

program
  : VAR var_list NEWLINE stmt_list
      {
         AST *p = ast_decl($2.names, $2.count, $4);
         ast_root = p;
         $$ = p;
       }
   | stmt_list
       {
         ast_root = $1;
         $$ = $1;
       }
   ;

var_list
    : IDENT
      {
        $$ .names = malloc(sizeof(char*));
        $$ .names[0] = strdup($1);
        $$ .count    = 1;
      }
    | var_list ',' IDENT
      {
        $1.names = realloc($1.names, sizeof(char*) * ($1.count + 1));
        $1.names[$1.count] = strdup($3);
        $1.count++;
        $$ = $1;
      }
  ;

stmt_list
    :
        { $$ = NULL; }
    | stmt_list stmt
        { $$ = ast_append($1, $2); }
    ;

stmt
    /* пустая строка → игнорируется (возвращаем NULL) */
    : NEWLINE
        { $$ = NULL; }
        
    | IDENT ASSIGN expr NEWLINE
      { $$ = ast_assign($1, NULL, $3); }

    /* присваивание элементу массива: a[expr] := expr */
    | IDENT '[' expr ']' ASSIGN expr NEWLINE
        { $$ = ast_assign($1, $3, $6); }

    /* присваивание суммы массива (#arr := expr) */
    | SUMARR ASSIGN expr NEWLINE
        { $$ = ast_assign($1, NULL, $3); }

    | IDENT NEWLINE { $$ = ast_var_ref($1); }

    | IF expr DO stmt_list ELIF expr DO stmt_list ELSE stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, $6, $8, $10); }
    | IF expr DO stmt_list ELIF expr DO stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, $6, $8, NULL); }
    | IF expr DO stmt_list ELSE stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, NULL, NULL, $6); }
    | IF expr DO stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, NULL, NULL, NULL); }

    | WHILE expr DO stmt_list DONE NEWLINE
        { $$ = ast_while($2, $4); }
    | CONTINUE NEWLINE
        { $$ = ast_cmd(AST_CONTINUE, NULL); }
    | BREAK    NEWLINE
        { $$ = ast_cmd(AST_BREAK,    NULL); }

    | FUNCTION IDENT '(' IDENT ')' DO stmt_list DONE NEWLINE
        { $$ = ast_func_decl($2, $4, $7); }

    | RETURN expr NEWLINE
        { $$ = ast_return($2); }

    /* вызов функции как отдельный оператор: IDENT '(' expr ')' NEWLINE */
    | IDENT '(' expr ')' NEWLINE
        { $$ = ast_func_call($1, $3); }

    /* роботические команды */
    | LOOK NEWLINE
        { $$ = ast_cmd(AST_LOOK, NULL); }
    | TEST NEWLINE
        { $$ = ast_cmd(AST_TEST, NULL); }
    | FORWARD expr NEWLINE
        { $$ = ast_cmd(AST_FORWARD, $2); }
    | BACKWARD expr NEWLINE
        { $$ = ast_cmd(AST_BACKWARD, $2); }
    | LEFT NEWLINE
        { $$ = ast_cmd(AST_LEFT, NULL); }
    | RIGHT NEWLINE
        { $$ = ast_cmd(AST_RIGHT, NULL); }
    | LOAD expr NEWLINE
        { $$ = ast_cmd(AST_LOAD, $2); }
    | DROP expr NEWLINE
        { $$ = ast_cmd(AST_DROP, $2); }
    ;

expr
    : expr '<'  expr   { $$ = ast_binop('<',  $1, $3); }
    | expr '>'  expr   { $$ = ast_binop('>',  $1, $3); }
    | expr '='  expr   { $$ = ast_binop('=',  $1, $3); }
    | expr '?'  expr   { $$ = ast_binop('?',  $1, $3); }
    | expr '^'  expr   { $$ = ast_binop('^',  $1, $3); }
    | expr '+'  expr   { $$ = ast_binop('+',  $1, $3); }
    | expr '-'  expr   { $$ = ast_binop('-',  $1, $3); }

    | '-' expr %prec UMINUS
        { $$ = ast_unop('-', $2); }

    | primary
        { $$ = $1; }
    ;

primary
    :
     EMPTY
        { $$ = ast_cell_literal(CELL_EMPTY); }
    | WALL
        { $$ = ast_cell_literal(CELL_WALL); }
    | BOX
        { $$ = ast_cell_literal(CELL_BOX); }
    | EXIT
        { $$ = ast_cell_literal(CELL_EXIT); }
    | UNDEF
        { $$ = ast_cell_literal(CELL_UNDEF); }
    | INT_LIT
        { $$ = ast_int_literal($1); }
    | HEX_LIT
        { $$ = ast_int_literal($1); }
    | TRUE_LIT
        { $$ = ast_bool_literal(true); }
    | FALSE_LIT
        { $$ = ast_bool_literal(false); }
    | INF_LIT
        { $$ = ast_inf(); }
    | NAN_LIT
        { $$ = ast_nan(); }

    /* в выражениях можно использовать «test» как булеву константу */
    | TEST
        { $$ = ast_cmd(AST_TEST, NULL); }
    /* аналогично «look» – возвращает число */
    | LOOK
        { $$ = ast_cmd(AST_LOOK, NULL); }

    /* переменная без индекса */
    | SUMARR
        { $$ = ast_sumarr($1); }

    /* обращение к элементу массива: var[idx] */
    | IDENT '[' expr ']'
        { $$ = ast_arr_ref($1, $3); }

    /* скалярная переменная: var */
    | IDENT
        { $$ = ast_var_ref($1); }

    /* вызов функции: IDENT '(' expr ')' */
    | IDENT '(' expr ')'
        { $$ = ast_func_call($1, $3); }

    /* скобки */
    | '(' expr ')'
        { $$ = $2; }
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
    exit(1);
}

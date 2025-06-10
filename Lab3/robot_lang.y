/* robot_lang.y */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "interp.h"

extern int yylex(void);
extern void yyerror(const char *s);
extern int yylineno;

AST *ast_root = NULL;
%}

%define parse.trace

%union {
    long long i;      /* для литералов INT_LIT, HEX_LIT, TRUE_LIT/FALSE_LIT */
    char    *s;       /* для IDENT и SUMARR */
    AST     *a;       /* для узлов AST */
}

%token <i> INT_LIT      /* десятичный литерал */
%token <i> HEX_LIT      /* шестнадцатеричный (0x...) */
%token <i> TRUE_LIT     /* TRUE (1) */
%token <i> FALSE_LIT    /* FALSE (0) */
%token     INF_LIT      /* INF */
%token     NAN_LIT      /* NAN */

%token <s> IDENT        /* идентификатор */
%token <s> SUMARR       /* #имяМассива */

%token VAR
%token ASSIGN          /* ':=' */

%token IF DO ELIF ELSE DONE
%token WHILE
%token BREAK
%token FUNCTION RETURN

%token LOOK TEST FORWARD BACKWARD LEFT RIGHT LOAD DROP

%token NEWLINE

%right UMINUS          /* унарный минус (самый высокий приоритет) */
%left '<' '>' '=' '?'  /* сравнения (низший приоритет) */
%left '^'              /* XOR (средний) */
%left '+' '-'          /* арифметика (высший из бинарных) */

/* Нетерминалы, возвращающие AST* */
%type <a> program decl_block var_list stmt_list stmt expr primary

%%

/* 1) Программа: опционально блок VAR … + список stmt */
program
    : decl_block stmt_list
        { ast_root = ast_append($1, $2); }
    ;

/* 2) Блок объявлений переменных: «VAR a, b, c» (игнорируем их в AST) */
decl_block
    : VAR var_list NEWLINE
        { $$ = NULL; }
    | /* пусто */
        { $$ = NULL; }
    ;

/* 3) var_list → IDENT | var_list ',' IDENT */
var_list
    : IDENT
        { $$ = NULL; }
    | var_list ',' IDENT
        { $$ = $1; }
    ;

/* 4) Список операторов (stmt_list) может быть пустым или состоять из нескольких stmt */
stmt_list
    : /* пусто */
        { $$ = NULL; }
    | stmt_list stmt
        { $$ = ast_append($1, $2); }
    ;

/* 5) Отдельный оператор stmt (всегда заканчивается на NEWLINE, кроме «пустого» первого) */
stmt
    /* 5.1) Пустая строка → игнорируем */
    : NEWLINE
        { $$ = NULL; }

    /* 5.2) Присваивание скалярной переменной: a := expr */
    | IDENT ASSIGN expr NEWLINE
        { $$ = ast_assign($1, NULL, $3); }

    /* 5.3) Присваивание элементу массива: a[expr] := expr */
    | IDENT '[' expr ']' ASSIGN expr NEWLINE
        { $$ = ast_assign($1, $3, $6); }

    /* 5.4) Присваивание суммы массива (#arr := expr) */
    | SUMARR ASSIGN expr NEWLINE
        { $$ = ast_assign($1, NULL, $3); }

    /* 5.5) Просто «IDENT» + NEWLINE → игнорируем */
    | IDENT NEWLINE
        { $$ = NULL; }

    /* 5.6) IF … DO … [ELIF … DO …] [ELSE …] DONE NEWLINE */
    | IF expr DO stmt_list ELIF expr DO stmt_list ELSE stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, $6, $8, $10); }
    | IF expr DO stmt_list ELIF expr DO stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, $6, $8, NULL); }
    | IF expr DO stmt_list ELSE stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, NULL, NULL, $6); }
    | IF expr DO stmt_list DONE NEWLINE
        { $$ = ast_if($2, $4, NULL, NULL, NULL); }

    /* 5.7) WHILE expr DO stmt_list DONE NEWLINE */
    | WHILE expr DO stmt_list DONE NEWLINE
        { $$ = ast_while($2, $4); }

    /* 5.8) function IDENT '(' IDENT ')' DO stmt_list DONE NEWLINE */
    | FUNCTION IDENT '(' IDENT ')' DO stmt_list DONE NEWLINE
        { $$ = ast_func_decl($2, $4, $7); }

    /* 5.9) return expr NEWLINE */
    | RETURN expr NEWLINE
        { $$ = ast_return($2); }

    /* 5.10) Вызов функции как отдельный оператор: IDENT '(' expr ')' NEWLINE */
    | IDENT '(' expr ')' NEWLINE
        { $$ = ast_func_call($1, $3); }

    /* 5.11) Команды робота: look, test, forward, backward, left, right, load, drop */
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

/* 6) Выражения (expr): бинарные операции и унарный минус */
expr
    : expr '<'  expr   { $$ = ast_binop('<',  $1, $3); }
    | expr '>'  expr   { $$ = ast_binop('>',  $1, $3); }
    | expr '='  expr   { $$ = ast_binop('=',  $1, $3); }
    | expr '?'  expr   { $$ = ast_binop('?',  $1, $3); }
    | expr '^'  expr   { $$ = ast_binop('^',  $1, $3); }
    | expr '+'  expr   { $$ = ast_binop('+',  $1, $3); }
    | expr '-'  expr   { $$ = ast_binop('-',  $1, $3); }

    /* Унарный минус */
    | '-' expr %prec UMINUS
        { $$ = ast_unop('-', $2); }

    /* Пускай «expr» может быть «primary» */
    | primary
        { $$ = $1; }
    ;

/* 7) «Нижний уровень» (primary):
      • литералы INT_LIT, HEX_LIT, TRUE_LIT, FALSE_LIT, INF_LIT, NAN_LIT
      • ключевое слово TEST (возвращает bool) – теперь используется в выражениях
      • ключевое слово LOOK (возвращает int) – аналогично
      • переменные: IDENT или IDENT[expr]
      • сумма массива: SUMARR
      • вызов функции: IDENT '(' expr ')'
      • скобки: (expr) */
primary
    : INT_LIT
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

    /* 7.1) Эта строка добавляет возможность писать «x := test» */
    | TEST
        { $$ = ast_cmd(AST_TEST, NULL); }
    /* 7.2) Можно также писать «y := look» */
    | LOOK
        { $$ = ast_cmd(AST_LOOK, NULL); }

    /* 7.3) Переменная без индекса */
    | IDENT
        { $$ = ast_var_ref($1, NULL); }
    /* 7.4) Переменная с индексом: IDENT '[' expr ']' */
    | IDENT '[' expr ']'
        { $$ = ast_var_ref($1, $3); }

    /* 7.5) Сумма по массиву: #arr */
    | SUMARR
        { $$ = ast_sumarr($1); }

    /* 7.6) Вызов функции: IDENT '(' expr ')' */
    | IDENT '(' expr ')'
        { $$ = ast_func_call($1, $3); }

    /* 7.7) Скобки */
    | '(' expr ')'
        { $$ = $2; }
    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, s);
    exit(1);
}


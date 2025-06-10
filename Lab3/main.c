/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "robot_lang.tab.h"
#include "interp.h"

extern FILE *yyin;
extern AST  *ast_root;

extern int yydebug;

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <prog.dsl> [maze.txt]\n", argv[0]);
        return 1;
    }
    const char *prog = argv[1];
    const char *mz   = (argc == 3 ? argv[2] : "maze.txt");
    yyin = fopen(prog, "r");
    if (!yyin) {
        perror("fopen prog.dsl");
        return 1;
    }
    extern int yydebug;
    yydebug = 1;
    if (yyparse() != 0) {
        fprintf(stderr, "Parse error\n");
        return 1;
    }
    /* получаем готовое AST в ast_root */
    interp_execute(ast_root, mz);
    ast_free(ast_root);
    return 0;
}


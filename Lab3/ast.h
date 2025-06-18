#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    AST_INT,
    AST_BOOL,
    AST_INF,
    AST_NAN,
    AST_CELL,
    AST_UNDEF,
    AST_VAR_REF,
    AST_ARR_REF,
    AST_BINOP,
    AST_UNOP,
    AST_SUMARR,
    AST_ASSIGN,
    AST_IF,
    AST_DECL,
    AST_WHILE,
    AST_BREAK,
    AST_RETURN,
    AST_FUNC_DECL,
    AST_FUNC_CALL,
    AST_LOOK,
    AST_TEST,
    AST_CONTINUE,
    AST_FORWARD,
    AST_BACKWARD,
    AST_LEFT,
    AST_RIGHT,
    AST_LOAD,
    AST_DROP
} ASTKind;

typedef struct AST {
    ASTKind       kind;
    struct AST   *next;
    union {
        long long   int_val;
        bool        bool_val;
        struct {
            char *name;
            struct AST *idx;
        } var_ref;
        struct {
            char *name;
        } sumarr;
        struct {
            int cell;
        } cell_lit;
        struct {
            char   **names;
            int      count;
        } decl;
        struct {
            char        op;
            struct AST *left;
            struct AST *right;
        } binop;
        struct {
            char        op;
            struct AST *operand;
        } unop;
        char *sumarr_name;
        struct {
            char       *name;
            struct AST *idx;
            struct AST *expr;
        } assign;
        /* AST_IF */
        struct {
            struct AST *cond;
            struct AST *then_branch;
            struct AST *elif_cond;
            struct AST *elif_then;
            struct AST *else_branch;
        } if_stmt;
        struct {
            struct AST *cond;
            struct AST *body;
        } while_stmt;
        struct AST *ret_expr;
        struct {
            char       *fname;
            char       *param;
            struct AST *body;
        } func_decl;
        struct {
            char       *fname;
            struct AST *arg;
        } func_call;
        struct {
            struct AST *expr;
        } cmd_arg;
    } d;
} AST;

AST *ast_decl(char **names, int count, AST *body);

AST *ast_cell_literal(int cell);
AST *ast_int_literal(long long x);
AST *ast_bool_literal(bool b);
AST *ast_inf();
AST *ast_nan();
AST *ast_undef();

AST *ast_var_ref(const char *name);
AST *ast_arr_ref(const char *name, AST *idx);
AST *ast_sumarr(const char *name);
AST *ast_binop(char op, AST *L, AST *R);
AST *ast_unop(char op, AST *operand);
AST *ast_assign(char *name, AST *idx, AST *expr);
AST *ast_if(AST *cond, AST *then_b, AST *elif_cond, AST *elif_then, AST *else_b);
AST *ast_while(AST *cond, AST *body);
AST *ast_return(AST *expr);
AST *ast_func_decl(char *fname, char *param, AST *body);
AST *ast_func_call(char *fname, AST *arg);
AST *ast_cmd(ASTKind kind, AST *expr);
AST *ast_break(void);

AST *ast_append(AST *a, AST *b);

void   ast_free(AST *a);
void   ast_print(AST *a);
#endif

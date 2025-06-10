#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    AST_INT,         /* целочисленный литерал */
    AST_BOOL,        /* логический литерал */
    AST_INF,         /* INF */
    AST_NAN,         /* NAN */
    AST_UNDEF,       /* UNDEF */
    AST_VAR_REF,     /* ссылка на переменную */
    AST_BINOP,       /* бинарная операция (+, -, <, >, =, ?, ^)  '?" – это «не равно» */
    AST_UNOP,        /* унарная операция ( „-x“ ) */
    AST_SUMARR,      /* #var – сумма элементов массива */
    AST_ASSIGN,      /* присваивание */
    AST_IF,          /* if ... do ... elif ... do ... else ... done */
    AST_WHILE,       /* while ... do ... done */
    AST_RETURN,      /* return ... */
    AST_FUNC_DECL,   /* function name(param) do ... done */
    AST_FUNC_CALL,   /* имя(выражение) */
    /* команды робота: */
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
        /* AST_INT */     long long   int_val;
        /* AST_BOOL */    bool        bool_val;
        /* AST_VAR_REF */
        struct {
            char       *name;
            struct AST *idx;   /* индекс (или NULL, если без [i]) */
        } var_ref;
        /* AST_BINOP */
        struct {
            char        op;    /* '+', '-', '<','>','=', '?','^' (XOR) */
            struct AST *left;
            struct AST *right;
        } binop;
        /* AST_UNOP */
        struct {
            char        op;    /* '-' */
            struct AST *operand;
        } unop;
        /* AST_SUMARR */    char *sumarr_name; /* имя массива */
        /* AST_ASSIGN */
        struct {
            char       *name;
            struct AST *idx;   /* может быть NULL (тогда скаляр) или ненулевой указатель */
            struct AST *expr;
        } assign;
        /* AST_IF */
        struct {
            struct AST *cond;
            struct AST *then_branch;
            struct AST *elif_cond;     /* может быть NULL */
            struct AST *elif_then;     /* может быть NULL */
            struct AST *else_branch;   /* может быть NULL */
        } if_stmt;
        /* AST_WHILE */
        struct {
            struct AST *cond;
            struct AST *body;
        } while_stmt;
        /* AST_RETURN */    struct AST *ret_expr;
        /* AST_FUNC_DECL */
        struct {
            char       *fname;
            char       *param;
            struct AST *body;
        } func_decl;
        /* AST_FUNC_CALL */
        struct {
            char       *fname;
            struct AST *arg;
        } func_call;
        /* AST_FORWARD, AST_BACKWARD, AST_LOAD, AST_DROP */
        struct {
            struct AST *expr;  /* выражение, определяющее «количество» или «массу» */
        } cmd_arg;
        /* AST_LEFT, AST_RIGHT, AST_LOOK, AST_TEST – без полей */
    } d;
} AST;

/* Конструкторы узлов AST */
AST *ast_int_literal(long long x);
AST *ast_bool_literal(bool b);
AST *ast_inf();
AST *ast_nan();
AST *ast_undef();

AST *ast_var_ref(char *name, AST *idx);
AST *ast_binop(char op, AST *L, AST *R);
AST *ast_unop(char op, AST *operand);
AST *ast_sumarr(char *name);
AST *ast_assign(char *name, AST *idx, AST *expr);
AST *ast_if(AST *cond, AST *then_b, AST *elif_cond, AST *elif_then, AST *else_b);
AST *ast_while(AST *cond, AST *body);
AST *ast_return(AST *expr);
AST *ast_func_decl(char *fname, char *param, AST *body);
AST *ast_func_call(char *fname, AST *arg);
AST *ast_cmd(ASTKind kind, AST *expr);  /* kind = AST_FORWARD, AST_LOAD и т.п. */

AST *ast_append(AST *a, AST *b);  /* «соединить два оператора в цепочку» */

void   ast_free(AST *a);
void   ast_print(AST *a);  /* отладочный принт */
#endif


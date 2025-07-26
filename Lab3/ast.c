#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static AST *new_node(ASTKind kind) {
    AST *n = calloc(1, sizeof(AST));
    n->kind = kind;
    return n;
}

AST *ast_int_lit(long long x) {
    AST *n = new_node(AST_INT);
    n->d.int_val = x;
    return n;
}

AST *ast_bool_lit(bool b) {
    AST *n = new_node(AST_BOOL);
    n->d.bool_val = b;
    return n;
}

AST *ast_cell_literal(int cell) {
    AST *n = new_node(AST_CELL);
    n->d.cell_lit.cell = cell;
    return n;
}


AST *ast_int_literal(long long x) { return ast_int_lit(x); }
AST *ast_bool_literal(bool b)  { return ast_bool_lit(b); }
AST *ast_inf(void)             { return new_node(AST_INF); }
AST *ast_nan(void)             { return new_node(AST_NAN); }

AST *ast_var_ref(const char *name) {
    AST *n = new_node(AST_VAR_REF);
    n->d.var_ref.name = strdup(name);
    return n;
}

AST *ast_arr_ref(const char *name, AST *idx) {
    AST *n = new_node(AST_ARR_REF);
    n->d.arr_ref.name = strdup(name);
    n->d.arr_ref.idx  = idx;
    return n;
}

AST *ast_sumarr(const char *name) {
    AST *n = new_node(AST_SUMARR);
    n->d.sumarr.name = strdup(name);
    return n;
}

AST *ast_decl(char **names, int count, AST *init) {
    AST *n = new_node(AST_DECL);
    n->d.decl.names = names;
    n->d.decl.count = count;
    n->d.decl.init  = init;
    return n;
}

AST *ast_unop(char op, AST *operand) {
    AST *n = new_node(AST_UNOP);
    n->d.unop.op      = op;
    n->d.unop.operand = operand;
    return n;
}

AST *ast_binop(char op, AST *L, AST *R) {
    AST *n = new_node(AST_BINOP);
    n->d.binop.op    = op;
    n->d.binop.left  = L;
    n->d.binop.right = R;
    return n;
}

AST *ast_if(AST *cond, AST *then_b, AST *elif_cond, AST *elif_then, AST *else_b) {
    AST *n = new_node(AST_IF);
    n->d.if_stmt.cond        = cond;
    n->d.if_stmt.then_branch = then_b;
    n->d.if_stmt.elif_cond   = elif_cond;
    n->d.if_stmt.elif_then   = elif_then;
    n->d.if_stmt.else_branch = else_b;
    return n;
}

AST *ast_while(AST *cond, AST *body) {
    AST *n = new_node(AST_WHILE);
    n->d.while_stmt.cond = cond;
    n->d.while_stmt.body = body;
    return n;
}

AST *ast_break(void)    { return new_node(AST_BREAK); }
AST *ast_continue(void) { return new_node(AST_CONTINUE); }

AST *ast_return(AST *e) {
    AST *n = new_node(AST_RETURN);
    n->d.ret_expr = e;
    return n;
}

AST *ast_assign(const char *name, AST *idx, AST *expr) {
    AST *n = new_node(AST_ASSIGN);
    n->d.assign.name = strdup(name);
    n->d.assign.expr = expr;
    return n;
}

AST *ast_func_decl(const char *fname, const char *param, AST *body) {
    AST *n = new_node(AST_FUNC_DECL);
    n->d.func_decl.fname = strdup(fname);
    n->d.func_decl.param = strdup(param);
    n->d.func_decl.body  = body;
    return n;
}

AST *ast_func_call(const char *fname, AST *arg) {
    AST *n = new_node(AST_FUNC_CALL);
    n->d.func_call.fname = strdup(fname);
    n->d.func_call.arg   = arg;
    return n;
}

AST *ast_cmd(ASTKind kind, AST *expr) {
    AST *n = new_node(kind);
    n->d.cmd_arg.expr = expr;
    return n;
}

AST *ast_append(AST *a, AST *b) {
    if (!a) return b;
    AST *p = a;
    while (p->next) p = p->next;
    p->next = b;
    return a;
}

void ast_free(AST *a) {
    if (!a) return;
    ast_free(a->next);
    switch (a->kind) {
        case AST_VAR_REF:
            free(a->d.var_ref.name);
            break;
        case AST_ARR_REF:
            free(a->d.arr_ref.name);
            ast_free(a->d.arr_ref.idx);
            break;
        case AST_SUMARR:
            free(a->d.sumarr.name);
            break;
        case AST_DECL:
            for (int i = 0; i < a->d.decl.count; i++)
                free(a->d.decl.names[i]);
            free(a->d.decl.names);
            ast_free(a->d.decl.init);
            break;
        case AST_IF:
            ast_free(a->d.if_stmt.cond);
            ast_free(a->d.if_stmt.then_branch);
            ast_free(a->d.if_stmt.elif_cond);
            ast_free(a->d.if_stmt.elif_then);
            ast_free(a->d.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(a->d.while_stmt.cond);
            ast_free(a->d.while_stmt.body);
            break;
        case AST_RETURN:
            ast_free(a->d.ret_expr);
            break;
        case AST_ASSIGN:
            free(a->d.assign.name);
            ast_free(a->d.assign.expr);
            break;
        case AST_FUNC_DECL:
            free(a->d.func_decl.fname);
            free(a->d.func_decl.param);
            ast_free(a->d.func_decl.body);
            break;
        case AST_FUNC_CALL:
            free(a->d.func_call.fname);
            ast_free(a->d.func_call.arg);
            break;
        case AST_UNOP:
            ast_free(a->d.unop.operand);
            break;
        case AST_BINOP:
            ast_free(a->d.binop.left);
            ast_free(a->d.binop.right);
            break;
        case AST_LOOK:
        case AST_TEST:
        case AST_FORWARD:
        case AST_BACKWARD:
        case AST_LEFT:
        case AST_RIGHT:
        case AST_LOAD:
        case AST_DROP:
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_INF:
        case AST_NAN:
        case AST_INT:
        case AST_BOOL:
        case AST_CELL:
        case AST_UNDEF:
            break;
        default:
            break;
    }
    free(a);
}

void ast_print(AST *a) {
    if (!a) return;
    ast_print(a->next);
    switch (a->kind) {
        case AST_VAR_REF:
            printf("VAR_REF %s\n", a->d.var_ref.name);
            break;
        case AST_ARR_REF:
            printf("ARR_REF %s[]\n", a->d.arr_ref.name);
            break;
        case AST_SUMARR:
            printf("SUMARR %s\n", a->d.sumarr.name);
            break;
        case AST_RETURN:
            printf("RETURN\n");
            ast_print(a->d.ret_expr);
            break;
        default:
            printf("NODE %d\n", a->kind);
            break;
    }
}

#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static AST *new_node(ASTKind kind) {
    AST *n = (AST *) calloc(1, sizeof(AST));
    n->kind = kind;
    n->next = NULL;
    return n;
}

AST *ast_cell_literal(int cell) {
    AST *n = new_node(AST_CELL);
    n->d.cell_lit.cell = cell;
    return n;
}

AST *ast_decl(char **names, int count, AST *body) {
    AST *n = new_node(AST_DECL);
    n->d.decl.names = names;
    n->d.decl.count = count;
    n->next         = body;
    return n;
}

AST *ast_int_literal(long long x) {
    AST *n = new_node(AST_INT);
    n->d.int_val = x;
    return n;
}

AST *ast_bool_literal(bool b) {
    AST *n = new_node(AST_BOOL);
    n->d.bool_val = b;
    return n;
}

AST *ast_inf() {
    AST *n = new_node(AST_INF);
    return n;
}

AST *ast_nan() {
    AST *n = new_node(AST_NAN);
    return n;
}

AST *ast_undef() {
    AST *n = new_node(AST_UNDEF);
    return n;
}

AST *ast_var_ref(const char *name) {
    AST *n = new_node(AST_VAR_REF);
    n->d.var_ref.name = strdup(name);
    n->d.var_ref.idx  = NULL;
    return n;
}

AST *ast_arr_ref(const char *name, AST *idx) {
    AST *n = new_node(AST_ARR_REF);
    n->d.var_ref.name = strdup(name);
    n->d.var_ref.idx  = idx;
    return n;
}

AST *ast_sumarr(const char *name) {
    AST *n = new_node(AST_SUMARR);
    n->d.sumarr.name = strdup(name);
    return n;
}

AST *ast_binop(char op, AST *L, AST *R) {
    AST *n = new_node(AST_BINOP);
    n->d.binop.op    = op;
    n->d.binop.left  = L;
    n->d.binop.right = R;
    return n;
}

AST *ast_unop(char op, AST *operand) {
    AST *n = new_node(AST_UNOP);
    n->d.unop.op      = op;
    n->d.unop.operand = operand;
    return n;
}

AST *ast_assign(char *name, AST *idx, AST *expr) {
    AST *n = new_node(AST_ASSIGN);
    n->d.assign.name = strdup(name);
    n->d.assign.idx  = idx;
    n->d.assign.expr = expr;
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

AST *ast_return(AST *expr) {
    AST *n = new_node(AST_RETURN);
    n->d.ret_expr = expr;
    return n;
}

AST *ast_func_decl(char *fname, char *param, AST *body) {
    AST *n = new_node(AST_FUNC_DECL);
    n->d.func_decl.fname = strdup(fname);
    n->d.func_decl.param = strdup(param);
    n->d.func_decl.body  = body;
    return n;
}

AST *ast_func_call(char *fname, AST *arg) {
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
    switch (a->kind) {
      case AST_VAR_REF:
        free(a->d.var_ref.name);
        ast_free(a->d.var_ref.idx);
        break;
      case AST_BINOP:
        ast_free(a->d.binop.left);
        ast_free(a->d.binop.right);
        break;
      case AST_UNOP:
        ast_free(a->d.unop.operand);
        break;
      case AST_SUMARR:
        free(a->d.sumarr_name);
        break;
      case AST_ASSIGN:
        free(a->d.assign.name);
        ast_free(a->d.assign.idx);
        ast_free(a->d.assign.expr);
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
      case AST_FUNC_DECL:
        free(a->d.func_decl.fname);
        free(a->d.func_decl.param);
        ast_free(a->d.func_decl.body);
        break;
      case AST_FUNC_CALL:
        free(a->d.func_call.fname);
        ast_free(a->d.func_call.arg);
        break;
      case AST_FORWARD:
      case AST_BACKWARD:
      case AST_LOAD:
      case AST_DROP:
        ast_free(a->d.cmd_arg.expr);
        break;
      case AST_LOOK:
      case AST_TEST:
        break;
      case AST_INT:
      case AST_BOOL:
      case AST_INF:
      case AST_NAN:
      case AST_UNDEF:
        break;
      default:
        break;
    }
    ast_free(a->next);
    free(a);
}

void ast_print(AST *a) {
    if (!a) return;
    printf("[Node kind=%d]  ", a->kind);
    if (a->kind == AST_INT) {
        printf("INT(%lld)", a->d.int_val);
    } else if (a->kind == AST_BOOL) {
        printf("BOOL(%s)", a->d.bool_val ? "true" : "false");
    } else if (a->kind == AST_INF) {
        printf("INF");
    } else if (a->kind == AST_NAN) {
        printf("NAN");
    } else if (a->kind == AST_UNDEF) {
        printf("UNDEF");
    } else if (a->kind == AST_VAR_REF) {
        if (a->d.var_ref.idx) {
            printf("VARREF %s[...]", a->d.var_ref.name);
        } else {
            printf("VARREF %s", a->d.var_ref.name);
        }
    } else if (a->kind == AST_BINOP) {
        printf("BINOP '%c'", a->d.binop.op);
    } else if (a->kind == AST_UNOP) {
        printf("UNOP '%c'", a->d.unop.op);
    } else if (a->kind == AST_SUMARR) {
        printf("SUMARR %s", a->d.sumarr_name);
    } else if (a->kind == AST_ASSIGN) {
        printf("ASSIGN %s", a->d.assign.name);
    } else if (a->kind == AST_IF) {
        printf("IF ...");
    } else if (a->kind == AST_WHILE) {
        printf("WHILE ...");
    } else if (a->kind == AST_RETURN) {
        printf("RETURN ...");
    } else if (a->kind == AST_FUNC_DECL) {
        printf("FUNC_DECL %s(%s)", a->d.func_decl.fname, a->d.func_decl.param);
    } else if (a->kind == AST_FUNC_CALL) {
        printf("FUNC_CALL %s(...)", a->d.func_call.fname);
    } else if (a->kind == AST_LOOK) {
        printf("LOOK");
    } else if (a->kind == AST_TEST) {
        printf("TEST");
    } else if (a->kind == AST_FORWARD) {
        printf("FORWARD ...");
    } else if (a->kind == AST_BACKWARD) {
        printf("BACKWARD ...");
    } else if (a->kind == AST_LEFT) {
        printf("LEFT");
    } else if (a->kind == AST_RIGHT) {
        printf("RIGHT");
    } else if (a->kind == AST_LOAD) {
        printf("LOAD ...");
    } else if (a->kind == AST_DROP) {
        printf("DROP ...");
    }
    printf("\n");
    ast_print(a->next);
}

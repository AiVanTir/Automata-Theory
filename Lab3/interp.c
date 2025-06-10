/* interp.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "context.h"
#include "ast.h"
#include "maze.h"     /* <— добавлено, чтобы видеть все прототипы maze_* */

/* ────────────────────────────────────────────────────────────────────────── */
/* Прототипы функций, чтобы не было implicit-declaration                      */
/* ────────────────────────────────────────────────────────────────────────── */

/* eval_expr может вызывать exec_list внутри (например, при вызове функции) */
static Value eval_expr(Context *ctx, AST *e);

/* exec_list может вызываться внутри eval_expr (при выполнении AST_FUNC_CALL) */
static void exec_list(Context *ctx, AST *p);


/* ────────────────────────────────────────────────────────────────────────── */
/* 1) Реализация «Value‐функций» ↔ Value constructors                           */
/* ────────────────────────────────────────────────────────────────────────── */
Value val_int(long long x) {
    Value v;
    v.kind = V_INT;
    v.i = x;
    return v;
}

Value val_bool(bool b) {
    Value v;
    v.kind = V_BOOL;
    v.b = b;
    return v;
}

Value val_inf(void) {
    Value v;
    v.kind = V_INF;
    return v;
}

Value val_nan(void) {
    Value v;
    v.kind = V_NAN;
    return v;
}

Value val_undef(void) {
    Value v;
    v.kind = V_UNDEF;
    return v;
}


/* ────────────────────────────────────────────────────────────────────────── */
/* 2) Работа с переменными (Var как динамический массив Value)                  */
/* ────────────────────────────────────────────────────────────────────────── */
static Var *find_var(Context *ctx, const char *name) {
    for (Var *vv = ctx->vars; vv != NULL; vv = vv->next) {
        if (strcmp(vv->name, name) == 0) {
            printf("[DEBUG] Variable '%s' found.\n", name);
            return vv;
        }
    }
    printf("[DEBUG] Variable '%s' not found.\n", name);
    return NULL;
}

static void ensure_capacity(Var *v, int newcap) {
    if (newcap <= v->cap) return;
    v->arr = (Value *) realloc(v->arr, sizeof(Value) * newcap);
    for (int i = v->cap; i < newcap; i++) {
        v->arr[i] = val_undef();
    }
    v->cap = newcap;
}

void set_var(Context *ctx, const char *name, AST *idx, Value val) {
    Var *v = find_var(ctx, name);
    if (!v) {
        v = (Var *) malloc(sizeof(Var));
        v->name = strdup(name);
        v->cap = 0;
        v->arr = NULL;
        v->next = ctx->vars;
        ctx->vars = v;
        printf("[DEBUG] Created variable '%s'.\n", name);
    }
    if (idx != NULL) {
        Value iv = eval_expr(ctx, idx);
        if (iv.kind != V_INT) {
            printf("[DEBUG] Attempt to index variable '%s' with non-integer.\n", name);
            return; /* Индекс не целое → ничего не делаем */
        }
        int i = (int) iv.i;
        if (i < 0) {
            printf("[DEBUG] Attempt to index variable '%s' with negative index %d.\n", name, i);
            return; /* Отрицательный индекс → игнор */
        }
        ensure_capacity(v, i + 1);
        v->arr[i] = val;
        if (val.kind == V_INT) {
            printf("[DEBUG] Set variable '%s'[%d] = %lld.\n", name, i, (long long)val.i);
        } else if (val.kind == V_BOOL) {
            printf("[DEBUG] Set variable '%s'[%d] = %s.\n", name, i, val.b ? "TRUE" : "FALSE");
        } else if (val.kind == V_INF) {
            printf("[DEBUG] Set variable '%s'[%d] = INF.\n", name, i);
        } else if (val.kind == V_NAN) {
            printf("[DEBUG] Set variable '%s'[%d] = NAN.\n", name, i);
        } else {
            printf("[DEBUG] Set variable '%s'[%d] = UNDEF.\n", name, i);
        }
    } else {
        if (v->cap == 0) {
            v->cap = 1;
            v->arr = (Value *) malloc(sizeof(Value) * 1);
            v->arr[0] = val;
            if (val.kind == V_INT) {
                printf("[DEBUG] Initialized variable '%s' = %lld.\n", name, (long long)val.i);
            } else if (val.kind == V_BOOL) {
                printf("[DEBUG] Initialized variable '%s' = %s.\n", name, val.b ? "TRUE" : "FALSE");
            } else if (val.kind == V_INF) {
                printf("[DEBUG] Initialized variable '%s' = INF.\n", name);
            } else if (val.kind == V_NAN) {
                printf("[DEBUG] Initialized variable '%s' = NAN.\n", name);
            } else {
                printf("[DEBUG] Initialized variable '%s' = UNDEF.\n", name);
            }
        } else {
            v->arr[0] = val;
            if (val.kind == V_INT) {
                printf("[DEBUG] Updated variable '%s' = %lld.\n", name, (long long)val.i);
            } else if (val.kind == V_BOOL) {
                printf("[DEBUG] Updated variable '%s' = %s.\n", name, val.b ? "TRUE" : "FALSE");
            } else if (val.kind == V_INF) {
                printf("[DEBUG] Updated variable '%s' = INF.\n", name);
            } else if (val.kind == V_NAN) {
                printf("[DEBUG] Updated variable '%s' = NAN.\n", name);
            } else {
                printf("[DEBUG] Updated variable '%s' = UNDEF.\n", name);
            }
        }
    }
}

Value get_var(Context *ctx, const char *name, AST *idx) {
    Var *v = find_var(ctx, name);
    if (!v) {
        printf("[DEBUG] Access to undefined variable '%s'.\n", name);
        return val_undef();
    }
    if (idx != NULL) {
        Value iv = eval_expr(ctx, idx);
        if (iv.kind != V_INT) {
            printf("[DEBUG] Attempt to index variable '%s' with non-integer.\n", name);
            return val_undef();
        }
        int i = (int) iv.i;
        if (i < 0 || i >= v->cap) {
            printf("[DEBUG] Index %d out of bounds for variable '%s'.\n", i, name);
            return val_undef();
        }
        Value result = v->arr[i];
        if (result.kind == V_INT) {
            printf("[DEBUG] Accessed variable '%s'[%d] = %lld.\n", name, i, (long long)result.i);
        } else if (result.kind == V_BOOL) {
            printf("[DEBUG] Accessed variable '%s'[%d] = %s.\n", name, i, result.b ? "TRUE" : "FALSE");
        } else if (result.kind == V_INF) {
            printf("[DEBUG] Accessed variable '%s'[%d] = INF.\n", name, i);
        } else if (result.kind == V_NAN) {
            printf("[DEBUG] Accessed variable '%s'[%d] = NAN.\n", name, i);
        } else {
            printf("[DEBUG] Accessed variable '%s'[%d] = UNDEF.\n", name, i);
        }
        return result;
    } else {
        if (v->cap == 0) {
            printf("[DEBUG] Accessed variable '%s', but it is UNDEF (no capacity).\n", name);
            return val_undef();
        }
        Value result = v->arr[0];
        if (result.kind == V_INT) {
            printf("[DEBUG] Accessed variable '%s' = %lld.\n", name, (long long)result.i);
        } else if (result.kind == V_BOOL) {
            printf("[DEBUG] Accessed variable '%s' = %s.\n", name, result.b ? "TRUE" : "FALSE");
        } else if (result.kind == V_INF) {
            printf("[DEBUG] Accessed variable '%s' = INF.\n", name);
        } else if (result.kind == V_NAN) {
            printf("[DEBUG] Accessed variable '%s' = NAN.\n", name);
        } else {
            printf("[DEBUG] Accessed variable '%s' = UNDEF.\n", name);
        }
        return result;
    }
}

static Value eval_sumarr(Context *ctx, const char *name) {
    Var *v = find_var(ctx, name);
    if (!v) {
        printf("[DEBUG] Attempt to sumarr undefined variable '%s'.\n", name);
        return val_undef();
    }
    long long acc = 0;
    bool sawInt = false;
    for (int i = 0; i < v->cap; i++) {
        Value cur = v->arr[i];
        if (cur.kind == V_INT) {
            acc += cur.i;
            sawInt = true;
        } else if (cur.kind == V_INF) {
            printf("[DEBUG] Summing array variable '%s' encountered INF at index %d.\n", name, i);
            return val_inf();
        } else if (cur.kind == V_NAN) {
            printf("[DEBUG] Summing array variable '%s' encountered NAN at index %d.\n", name, i);
            return val_nan();
        } else if (cur.kind == V_BOOL) {
            sawInt = true;
            acc += (cur.b ? 1LL : 0LL);
        }
    }
    if (!sawInt) {
        printf("[DEBUG] Summing array variable '%s' found no integer/bool values.\n", name);
        return val_undef();
    }
    printf("[DEBUG] Summed array variable '%s' = %lld.\n", name, acc);
    return val_int(acc);
}


/* ────────────────────────────────────────────────────────────────────────── */
/* 3) make_binop — арифметические/логические операции                          */
/* ────────────────────────────────────────────────────────────────────────── */
static Value make_binop(Value L, Value R, char op) {
    /* Любая NAN → NAN */
    if (L.kind == V_NAN || R.kind == V_NAN) {
        printf("[DEBUG] make_binop: one operand is NAN, returning NAN.\n");
        return val_nan();
    }
    /* Любая UNDEF → UNDEF */
    if (L.kind == V_UNDEF || R.kind == V_UNDEF) {
        printf("[DEBUG] make_binop: one operand is UNDEF, returning UNDEF.\n");
        return val_undef();
    }
    /* Оба типа BOOL */
    if (L.kind == V_BOOL && R.kind == V_BOOL) {
        long long a = L.b ? 1LL : 0LL;
        long long b = R.b ? 1LL : 0LL;
        switch (op) {
            case '+':  printf("[DEBUG] Bool binop: %lld + %lld = %lld\n", a, b, a + b); return val_int(a + b);
            case '-':  printf("[DEBUG] Bool binop: %lld - %lld = %lld\n", a, b, a - b); return val_int(a - b);
            case '*':  printf("[DEBUG] Bool binop: %lld * %lld = %lld\n", a, b, a * b); return val_int(a * b);
            case '/':  printf("[DEBUG] Bool binop: %lld / %lld\n", a, b);
                       return (b == 0 ? val_nan() : val_int(a / b));
            case '<':  printf("[DEBUG] Bool binop: %lld < %lld = %s\n", a, b, (a < b) ? "TRUE" : "FALSE");
                       return val_bool(a <  b);
            case '>':  printf("[DEBUG] Bool binop: %lld > %lld = %s\n", a, b, (a > b) ? "TRUE" : "FALSE");
                       return val_bool(a >  b);
            case '=':  printf("[DEBUG] Bool binop: %lld == %lld = %s\n", a, b, (a == b) ? "TRUE" : "FALSE");
                       return val_bool(a == b);
            case '?':  printf("[DEBUG] Bool binop: %lld != %lld = %s\n", a, b, (a != b) ? "TRUE" : "FALSE");
                       return val_bool(a != b);
            case '^':  printf("[DEBUG] Bool binop: %lld XOR %lld = %s\n", a, b, ((a != 0) ^ (b != 0)) ? "TRUE" : "FALSE");
                       return val_bool(((a != 0) ^ (b != 0)));
            default:   printf("[DEBUG] Bool binop: unknown op '%c'\n", op);
                       return val_undef();
        }
    }
    /* INF */
    if (L.kind == V_INF || R.kind == V_INF) {
        if (L.kind == V_INF && R.kind == V_INF) {
            printf("[DEBUG] INF vs INF binop with op '%c'\n", op);
            switch (op) {
                case '+': case '-': case '*': return val_inf();
                case '/': return val_nan();  /* INF/INF → NAN */
                case '<': return val_bool(false);
                case '>': return val_bool(false);
                case '=': return val_bool(true);
                case '?': return val_bool(false);
                case '^': return val_bool(false);
                default:  return val_nan();
            }
        }
        if (op == '+' || op == '-' || op == '*') {
            printf("[DEBUG] INF binop with op '%c', returning INF.\n", op);
            return val_inf();
        }
        if (op == '/') {
            if (L.kind == V_INF && R.kind != V_INF) {
                long long b = (R.kind == V_INT ? R.i : (R.kind == V_BOOL ? (R.b ? 1LL : 0LL) : 0LL));
                if (b == 0) {
                    printf("[DEBUG] INF / 0 → NAN\n");
                    return val_nan();
                }
                printf("[DEBUG] INF / %lld → INF\n", b);
                return val_inf();
            }
            if (R.kind == V_INF && L.kind != V_INF) {
                printf("[DEBUG] %lld / INF → 0\n", (L.kind == V_INT) ? L.i : (L.b ? 1LL : 0LL));
                return val_int(0);
            }
        }
        if (op == '<' || op == '>' || op == '=' || op == '?') {
            if (L.kind == V_INF && R.kind != V_INF) {
                long long b = (R.kind == V_INT ? R.i : (R.kind == V_BOOL ? (R.b ? 1LL : 0LL) : 0LL));
                bool res;
                switch (op) {
                    case '<': res = false; break;
                    case '>': res = true;  break;
                    case '=': res = false; break;
                    case '?': res = true;  break;
                    default:  res = false; break;
                }
                printf("[DEBUG] INF %c %lld = %s\n", op, b, res ? "TRUE" : "FALSE");
                return val_bool(res);
            }
            if (R.kind == V_INF && L.kind != V_INF) {
                long long a = (L.kind == V_INT ? L.i : (L.kind == V_BOOL ? (L.b ? 1LL : 0LL) : 0LL));
                bool res;
                switch (op) {
                    case '<': res = true;  break;
                    case '>': res = false; break;
                    case '=': res = false; break;
                    case '?': res = true;  break;
                    default:  res = false; break;
                }
                printf("[DEBUG] %lld %c INF = %s\n", a, op, res ? "TRUE" : "FALSE");
                return val_bool(res);
            }
        }
    }
    /* Оба V_INT или один BOOL и один INT */
    long long a_val = 0, b_val = 0;
    if (L.kind == V_INT)           a_val = L.i;
    else if (L.kind == V_BOOL)     a_val = L.b ? 1LL : 0LL;
    if (R.kind == V_INT)           b_val = R.i;
    else if (R.kind == V_BOOL)     b_val = R.b ? 1LL : 0LL;

    switch (op) {
        case '+':  printf("[DEBUG] Int binop: %lld + %lld = %lld\n", a_val, b_val, a_val + b_val);
                   return val_int(a_val + b_val);
        case '-':  printf("[DEBUG] Int binop: %lld - %lld = %lld\n", a_val, b_val, a_val - b_val);
                   return val_int(a_val - b_val);
        case '*':  printf("[DEBUG] Int binop: %lld * %lld = %lld\n", a_val, b_val, a_val * b_val);
                   return val_int(a_val * b_val);
        case '/':  if (b_val == 0) {
                       printf("[DEBUG] Int binop: %lld / 0 → NAN\n", a_val);
                       return val_nan();
                   } else {
                       printf("[DEBUG] Int binop: %lld / %lld = %lld\n", a_val, b_val, a_val / b_val);
                       return val_int(a_val / b_val);
                   }
        case '<':  printf("[DEBUG] Int binop: %lld < %lld = %s\n", a_val, b_val, (a_val < b_val) ? "TRUE" : "FALSE");
                   return val_bool(a_val <  b_val);
        case '>':  printf("[DEBUG] Int binop: %lld > %lld = %s\n", a_val, b_val, (a_val > b_val) ? "TRUE" : "FALSE");
                   return val_bool(a_val >  b_val);
        case '=':  printf("[DEBUG] Int binop: %lld == %lld = %s\n", a_val, b_val, (a_val == b_val) ? "TRUE" : "FALSE");
                   return val_bool(a_val == b_val);
        case '?':  printf("[DEBUG] Int binop: %lld != %lld = %s\n", a_val, b_val, (a_val != b_val) ? "TRUE" : "FALSE");
                   return val_bool(a_val != b_val);
        case '^':  printf("[DEBUG] Int binop: %lld XOR %lld = %s\n", a_val, b_val, ((a_val != 0) ^ (b_val != 0)) ? "TRUE" : "FALSE");
                   return val_bool(((a_val != 0) ^ (b_val != 0)));
        default:   printf("[DEBUG] Int binop: unknown op '%c'\n", op);
                   return val_undef();
    }
}


/* ────────────────────────────────────────────────────────────────────────── */
/* 4) eval_expr — вычисление выражений AST → Value                            */
/* ────────────────────────────────────────────────────────────────────────── */
static Value eval_expr(Context *ctx, AST *e) {
    if (!e) {
        printf("[DEBUG] eval_expr: encountered NULL AST node, returning UNDEF.\n");
        return val_undef();
    }
    switch (e->kind) {
        case AST_INT:
            printf("[DEBUG] eval_expr: integer literal %lld.\n", (long long)e->d.int_val);
            return val_int(e->d.int_val);

        case AST_BOOL:
            printf("[DEBUG] eval_expr: boolean literal %s.\n", e->d.bool_val ? "TRUE" : "FALSE");
            return val_bool(e->d.bool_val);

        case AST_INF:
            printf("[DEBUG] eval_expr: INF literal.\n");
            return val_inf();

        case AST_NAN:
            printf("[DEBUG] eval_expr: NAN literal.\n");
            return val_nan();

        case AST_UNDEF:
            printf("[DEBUG] eval_expr: UNDEF literal.\n");
            return val_undef();

        case AST_VAR_REF: {
            /* Встроенные константы (WALL=1, BOX=2, EXIT=3, TRUE/FALSE) */
            if (strcmp(e->d.var_ref.name, "WALL") == 0 && e->d.var_ref.idx == NULL) {
                printf("[DEBUG] eval_expr: literal WALL → 1.\n");
                return val_int(1);
            }
            if (strcmp(e->d.var_ref.name, "BOX") == 0 && e->d.var_ref.idx == NULL) {
                printf("[DEBUG] eval_expr: literal BOX → 2.\n");
                return val_int(2);
            }
            if (strcmp(e->d.var_ref.name, "EXIT") == 0 && e->d.var_ref.idx == NULL) {
                printf("[DEBUG] eval_expr: literal EXIT → 3.\n");
                return val_int(3);
            }
            if (strcmp(e->d.var_ref.name, "TRUE") == 0 && e->d.var_ref.idx == NULL) {
                printf("[DEBUG] eval_expr: literal TRUE.\n");
                return val_bool(true);
            }
            if (strcmp(e->d.var_ref.name, "FALSE") == 0 && e->d.var_ref.idx == NULL) {
                printf("[DEBUG] eval_expr: literal FALSE.\n");
                return val_bool(false);
            }
            /* Если индекс: var[idx], иначе — сумма массива (#var) */
            if (e->d.var_ref.idx) {
                printf("[DEBUG] eval_expr: variable reference '%s'[%s].\n",
                       e->d.var_ref.name,
                       (e->d.var_ref.idx->kind == AST_INT ?
                            ({ char buf[64]; sprintf(buf, "%lld", (long long)e->d.var_ref.idx->d.int_val); strdup(buf); })
                         : "[expr]")
                );
                return get_var(ctx, e->d.var_ref.name, e->d.var_ref.idx);
            } else {
                printf("[DEBUG] eval_expr: sum array variable '#%s'.\n", e->d.var_ref.name);
                return eval_sumarr(ctx, e->d.var_ref.name);
            }
        }

        case AST_SUMARR:
            printf("[DEBUG] eval_expr: SUMARR '%s'.\n", e->d.sumarr_name);
            return eval_sumarr(ctx, e->d.sumarr_name);

        case AST_BINOP: {
            printf("[DEBUG] eval_expr: BINOP '%c'.\n", e->d.binop.op);
            Value L = eval_expr(ctx, e->d.binop.left);
            Value R = eval_expr(ctx, e->d.binop.right);
            return make_binop(L, R, e->d.binop.op);
        }

        case AST_UNOP: {
            printf("[DEBUG] eval_expr: UNOP '-'.\n");
            Value X = eval_expr(ctx, e->d.unop.operand);
            if (X.kind == V_INT) {
                return val_int(-X.i);
            } else if (X.kind == V_INF) {
                return val_inf();
            } else if (X.kind == V_NAN) {
                return val_nan();
            } else {
                return val_undef();
            }
        }

        case AST_FUNC_CALL: {
            printf("[DEBUG] eval_expr: calling function '%s'.\n", e->d.func_call.fname);
            Func *f = ctx->funcs;
            while (f) {
                if (strcmp(f->name, e->d.func_call.fname) == 0) break;
                f = f->next;
            }
            if (!f) {
                printf("[DEBUG] eval_expr: function '%s' not found, returning UNDEF.\n", e->d.func_call.fname);
                return val_undef();
            }
            /* Сохраняем старое состояние returning и ret_value */
            bool old_ret = ctx->returning;
            Value old_val = ctx->ret_value;

            Value arg = eval_expr(ctx, e->d.func_call.arg);
            if (arg.kind == V_INT) {
                printf("[DEBUG] eval_expr: argument to '%s' = %lld.\n", e->d.func_call.fname, (long long)arg.i);
            } else if (arg.kind == V_BOOL) {
                printf("[DEBUG] eval_expr: argument to '%s' = %s.\n", e->d.func_call.fname, arg.b ? "TRUE" : "FALSE");
            } else if (arg.kind == V_INF) {
                printf("[DEBUG] eval_expr: argument to '%s' = INF.\n", e->d.func_call.fname);
            } else if (arg.kind == V_NAN) {
                printf("[DEBUG] eval_expr: argument to '%s' = NAN.\n", e->d.func_call.fname);
            } else {
                printf("[DEBUG] eval_expr: argument to '%s' = UNDEF.\n", e->d.func_call.fname);
            }

            ctx->returning = false;
            set_var(ctx, f->param, NULL, arg);
            exec_list(ctx, f->body);

            Value res = ctx->returning ? ctx->ret_value : val_undef();
            if (res.kind == V_INT) {
                printf("[DEBUG] eval_expr: function '%s' returned %lld.\n", e->d.func_call.fname, (long long)res.i);
            } else if (res.kind == V_BOOL) {
                printf("[DEBUG] eval_expr: function '%s' returned %s.\n", e->d.func_call.fname, res.b ? "TRUE" : "FALSE");
            } else if (res.kind == V_INF) {
                printf("[DEBUG] eval_expr: function '%s' returned INF.\n", e->d.func_call.fname);
            } else if (res.kind == V_NAN) {
                printf("[DEBUG] eval_expr: function '%s' returned NAN.\n", e->d.func_call.fname);
            } else {
                printf("[DEBUG] eval_expr: function '%s' returned UNDEF.\n", e->d.func_call.fname);
            }

            ctx->returning = old_ret;
            ctx->ret_value = old_val;
            return res;
        }

        case AST_TEST: {
            bool isWall = test_obstacle(ctx);
            printf("[DEBUG] eval_expr: TEST → %d.\n", isWall ? 1 : 0);
            return val_int(isWall ? 1 : 0);
        }

        case AST_LOOK: {
            int dist = look_distance(ctx);
            printf("[DEBUG] eval_expr: LOOK → %d.\n", dist);
            return val_int(dist);
        }

        default:
            printf("[DEBUG] eval_expr: unknown AST kind %d, returning UNDEF.\n", e->kind);
            return val_undef();
    }
}


/* ────────────────────────────────────────────────────────────────────────── */
/* 5) exec_list — выполнение списка AST-операторов                            */
/* ────────────────────────────────────────────────────────────────────────── */
static void exec_list(Context *ctx, AST *p) {
    while (p) {
        /* Если предыдущий RETURN уже поднял returning, прерываем */
        if (ctx->returning) {
            printf("[DEBUG] exec_list: returning is true, breaking.\n");
            return;
        }

        switch (p->kind) {
            /* ────────────────────────────────────────────────────────── */
            /* «continue» как bare-идентификатор → досрочно выйти       */
            case AST_VAR_REF:
                if (p->d.var_ref.idx == NULL && strcmp(p->d.var_ref.name, "continue") == 0) {
                    printf("[DEBUG] exec_list: encountered 'continue', returning from exec_list.\n");
                    return;
                }
                break;

            case AST_ASSIGN: {
                printf("[DEBUG] exec_list: ASSIGN to '%s'.\n", p->d.assign.name);
                Value v = eval_expr(ctx, p->d.assign.expr);
                set_var(ctx, p->d.assign.name, p->d.assign.idx, v);
                break;
            }

            case AST_IF: {
                printf("[DEBUG] exec_list: IF condition.\n");
                Value C = eval_expr(ctx, p->d.if_stmt.cond);
                bool cond = false;
                if (C.kind == V_BOOL) cond = C.b;
                else if (C.kind == V_INT) cond = (C.i != 0);
                printf("[DEBUG] exec_list: IF condition evaluated to %s.\n", cond ? "TRUE" : "FALSE");
                if (cond) {
                    exec_list(ctx, p->d.if_stmt.then_branch);
                } else if (p->d.if_stmt.elif_cond) {
                    printf("[DEBUG] exec_list: evaluating ELIF condition.\n");
                    Value C2 = eval_expr(ctx, p->d.if_stmt.elif_cond);
                    bool cond2 = false;
                    if (C2.kind == V_BOOL) cond2 = C2.b;
                    else if (C2.kind == V_INT) cond2 = (C2.i != 0);
                    printf("[DEBUG] exec_list: ELIF condition evaluated to %s.\n", cond2 ? "TRUE" : "FALSE");
                    if (cond2) {
                        exec_list(ctx, p->d.if_stmt.elif_then);
                    } else if (p->d.if_stmt.else_branch) {
                        exec_list(ctx, p->d.if_stmt.else_branch);
                    }
                } else if (p->d.if_stmt.else_branch) {
                    printf("[DEBUG] exec_list: executing ELSE branch.\n");
                    exec_list(ctx, p->d.if_stmt.else_branch);
                }
                break;
            }

            case AST_WHILE: {
                printf("[DEBUG] exec_list: entering WHILE loop.\n");
                while (1) {
                    if (ctx->returning) {
                        printf("[DEBUG] exec_list: returning is true inside WHILE, breaking.\n");
                        return;
                    }
                    Value C = eval_expr(ctx, p->d.while_stmt.cond);
                    bool cond = false;
                    if (C.kind == V_BOOL) cond = C.b;
                    else if (C.kind == V_INT) cond = (C.i != 0);
                    printf("[DEBUG] exec_list: WHILE condition evaluated to %s.\n", cond ? "TRUE" : "FALSE");
                    if (!cond) break;

                    printf("[DEBUG] exec_list: executing WHILE body.\n");
                    exec_list(ctx, p->d.while_stmt.body);
                    if (ctx->returning) {
                        printf("[DEBUG] exec_list: returning is true after WHILE body, breaking.\n");
                        return;
                    }
                    /* Если внутри тела был «continue», exec_list уже вернулась досрочно,
                       и мы придём сюда, проверим условие заново и продолжим. */
                }
                printf("[DEBUG] exec_list: exiting WHILE loop.\n");
                break;
            }

            case AST_RETURN: {
                printf("[DEBUG] exec_list: RETURN statement.\n");
                Value v = eval_expr(ctx, p->d.ret_expr);
                ctx->ret_value = v;
                ctx->returning = true;
                printf("[DEBUG] exec_list: set returning = true.\n");
                return;
            }

            case AST_FUNC_DECL: {
                printf("[DEBUG] exec_list: function declaration '%s'.\n", p->d.func_decl.fname);
                Func *f = (Func *) malloc(sizeof(Func));
                f->name  = strdup(p->d.func_decl.fname);
                f->param = strdup(p->d.func_decl.param);
                f->body  = p->d.func_decl.body;
                f->next  = ctx->funcs;
                ctx->funcs = f;
                break;
            }

            case AST_TEST: {
                printf("[DEBUG] exec_list: TEST operator.\n");
                bool isWall = test_obstacle(ctx);
                printf("TEST -> %d\n", isWall ? 1 : 0);
                break;
            }

            case AST_LOOK: {
                printf("[DEBUG] exec_list: LOOK operator.\n");
                int dist = look_distance(ctx);
                printf("LOOK -> %d\n", dist);
                break;
            }

            case AST_FORWARD: {
                printf("[DEBUG] exec_list: FORWARD operator.\n");
                Value v = eval_expr(ctx, p->d.cmd_arg.expr);
                if (v.kind != V_INT) {
                    printf("FORWARD: argument is not INT\n");
                    break;
                }
                bool ok = maze_move(ctx, (int) v.i);
                printf("FORWARD %lld -> %s\n", (long long) v.i, ok ? "true" : "false");
                break;
            }

            case AST_BACKWARD: {
                printf("[DEBUG] exec_list: BACKWARD operator.\n");
                Value v = eval_expr(ctx, p->d.cmd_arg.expr);
                if (v.kind != V_INT) {
                    printf("BACKWARD: argument is not INT\n");
                    break;
                }
                bool ok = maze_move(ctx, -(int) v.i);
                printf("BACKWARD %lld -> %s\n", (long long) v.i, ok ? "true" : "false");
                break;
            }

            case AST_LEFT: {
                printf("[DEBUG] exec_list: LEFT operator.\n");
                maze_turn(ctx, -1);
                printf("LEFT\n");
                break;
            }

            case AST_RIGHT: {
                printf("[DEBUG] exec_list: RIGHT operator.\n");
                maze_turn(ctx, +1);
                printf("RIGHT\n");
                break;
            }

            case AST_LOAD: {
                printf("[DEBUG] exec_list: LOAD operator.\n");
                Value v = eval_expr(ctx, p->d.cmd_arg.expr);
                if (v.kind != V_INT) {
                    printf("LOAD: argument is not INT\n");
                    break;
                }
                bool ok = maze_load_box(ctx, (int) v.i);
                printf("LOAD %lld -> %s\n", (long long) v.i, ok ? "true" : "false");
                break;
            }

            case AST_DROP: {
                printf("[DEBUG] exec_list: DROP operator.\n");
                Value v = eval_expr(ctx, p->d.cmd_arg.expr);
                if (v.kind != V_INT) {
                    printf("DROP: argument is not INT\n");
                    break;
                }
                bool ok = maze_drop_box(ctx, (int) v.i);
                printf("DROP %lld -> %s\n", (long long) v.i, ok ? "true" : "false");
                break;
            }

            default:
                /* Любой другой узел (например, AST_NOP или неизвестный) — пропускаем */
                printf("[DEBUG] exec_list: unknown AST kind %d, skipping.\n", p->kind);
                break;
        }

        /* Переходим к следующему оператору в списке */
        p = p->next;
    }
}


/* ────────────────────────────────────────────────────────────────────────── */
/* 6) Точка входа интерпретатора                                                */
/*    main.c должен вызвать parse() → AST* prog, затем interp_execute(prog, maze_file) */
/* ────────────────────────────────────────────────────────────────────────── */
void interp_execute(AST *prog, const char *maze_file) {
    Context ctx;
    ctx.vars = NULL;
    ctx.funcs = NULL;
    ctx.returning = false;
    ctx.ret_value = val_undef();
    ctx.carried_weight = 0;

    printf("[DEBUG] Loading maze from '%s'.\n", maze_file);
    ctx.mz = maze_load(maze_file, &ctx);
    if (!ctx.mz) {
        fprintf(stderr, "Cannot load maze from '%s'\n", maze_file);
        return;
    }
    printf("[DEBUG] Maze loaded successfully. Start at (q=%d, r=%d), dir=%d.\n", ctx.q, ctx.r, ctx.dir);

    printf("[DEBUG] Starting execution of AST.\n");
    exec_list(&ctx, prog);
    printf("[DEBUG] Finished execution of AST.\n");

    bool atExit = is_at_exit(&ctx);
    if (atExit) {
        printf("Robot reached EXIT at (q=%d, r=%d), dir=%d\n", ctx.q, ctx.r, ctx.dir);
    } else {
        printf("Robot did NOT reach EXIT, ended at (q=%d, r=%d), dir=%d\n", ctx.q, ctx.r, ctx.dir);
    }

    printf("[DEBUG] Freeing maze resources.\n");
    maze_free(ctx.mz);
    printf("[DEBUG] Maze resources freed.\n");
}


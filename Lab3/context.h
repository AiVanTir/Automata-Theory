#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>

typedef struct Maze Maze;

typedef enum {
    V_INT,
    V_BOOL,
    V_INF,
    V_NAN,
    V_UNDEF
} ValueKind;

typedef struct {
    ValueKind kind;
    union {
        long long i;
        bool      b;
    };
} Value;

typedef struct Var {
    char      *name;
    int        cap;
    Value     *arr;
    struct Var *next;
} Var;

typedef struct Func {
    char      *name;
    char      *param;
    struct AST *body;
    struct Func *next;
} Func;

typedef struct Context {
    Maze   *mz;             /* загруженный лабиринт (структура Maze определяется в maze.h) */
    int     q, r, dir;      /* axial-координаты для шестиугольной сетки и направление (0..5) */
    int     carried_weight; /* вес коробок, которые сейчас в руках */
    Var    *vars;           /* список переменных */
    Func   *funcs;          /* список объявленных функций */
    bool    returning;      /* флаг: внутри функции отработал return? */
    Value   ret_value;      /* значение, возвращённое через return */
} Context;

Value val_int(long long x);
Value val_bool(bool b);
Value val_inf(void);
Value val_nan(void);
Value val_undef(void);

#endif

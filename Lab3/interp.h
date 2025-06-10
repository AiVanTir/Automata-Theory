/* context.h */
#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include "ast.h"

struct Maze;
typedef struct Maze Maze;

typedef enum {
    V_INT,
    V_BOOL,
    V_INF,
    V_NAN,
    V_UNDEF
} ValueKind;

typedef struct Value {
    ValueKind kind;
    long long i;
    bool b;
} Value;

Value val_int(long long x);
Value val_bool(bool b);
Value val_inf(void);
Value val_nan(void);
Value val_undef(void);

typedef struct Var {
    char       *name;  /* имя переменной */
    int         cap;   /* текущая ёмкость массива (0 — если скаляр) */
    Value      *arr;   /* указатель на блок Value [0..cap-1] (NULL, если скаляр) */
    struct Var *next;  /* указатель на следующую переменную */
} Var;

typedef struct Func {
    char       *name;   /* имя функции */
    char       *param;  /* имя параметра (имя переменной внутри тела функции) */
    AST        *body;   /* указатель на AST с телом функции */
    struct Func *next;  /* следующая функция в списке */
} Func;

typedef struct Context {
    Maze   *mz;            /* указатель на загруженный лабиринт */
    int     x, y, dir;     /* текущее положение и направление (0–3) */
    int     carried_weight;/* текущий вес «на борту» (суммарный из загруженных коробок) */
    Var    *vars;          /* список всех переменных и массивов */
    Func   *funcs;         /* список объявленных функций */
    bool    returning;     /* флаг: выполнен ли return внутри функции */
    Value   ret_value;     /* значение, возвращённое функцией (если returning==true) */
} Context;

#endif


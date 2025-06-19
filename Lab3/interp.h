#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include "ast.h"

struct Maze;
typedef struct Maze Maze;

/* тип-обёртка Value для наших значений */
typedef enum {
    V_INT,    /* целое число */
    V_BOOL,   /* логическое */
    V_INF,    /* бесконечность */
    V_NAN,    /* не число */
    V_UNDEF   /* неопределённое значение */
} ValueKind;

typedef struct Value {
    ValueKind kind;    /* какой вид значения хранится */
    long long i;       /* если kind==V_INT, здесь число */
    bool b;            /* если kind==V_BOOL, здесь true/false */
    /* если V_INF, V_NAN или V_UNDEF, поля i и b игнорируются */
} Value;

Value val_int(long long x);
Value val_bool(bool b);
Value val_inf(void);
Value val_nan(void);
Value val_undef(void);

/* структура Var: список переменных (имя, массив Value, ёмкость) */
typedef struct Var {
    char       *name;  /* имя переменной */
    int         cap;   /* текущая ёмкость массива (0 — если скаляр) */
    Value      *arr;   /* указатель на блок Value [0..cap-1] (NULL, если скаляр) */
    struct Var *next;  /* указатель на следующую переменную */
} Var;

/* структура Func: список функций (имя, параметр, тело AST) */
typedef struct Func {
    char       *name;   /* имя функции */
    char       *param;  /* имя параметра (имя переменной внутри тела функции) */
    AST        *body;   /* указатель на AST с телом функции */
    struct Func *next;  /* следующая функция в списке */
} Func;

/* структура Context дополняется полями для интерпретатора */
typedef struct Context {
    Maze   *mz;            /* указатель на загруженный лабиринт */
    int     q, r, dir;     /* текущее положение и направление (0–3) */
    int     carried_weight;/* текущий вес «на борту» (суммарный из загруженных коробок) */
    Var    *vars;          /* список всех переменных и массивов */
    Func   *funcs;         /* список объявленных функций */
    bool    returning;     /* флаг: выполнен ли return внутри функции */
    Value   ret_value;     /* значение, возвращённое функцией (если returning==true) */
} Context;

void interp_execute(AST *ast, const char *mz);

#endif

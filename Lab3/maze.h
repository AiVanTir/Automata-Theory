#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>
#include "context.h"

typedef enum {
    CELL_WALL,
    CELL_EXIT,
    CELL_FLOOR,
    CELL_BOX
} CellType;

typedef struct Maze {
    int       width;
    int       height;
    CellType *cells;
} Maze;

// загружает лабиринт из файла filename в память и сразу инициализирует
// ctx->q, ctx->r, ctx->dir. Возвращает указатель на новую Maze* или NULL.
Maze *maze_load(const char *filename, Context *ctx);

// Освобождает память, занятую Maze в heap (cells + сам объект).
void  maze_free(Maze *mz);

// Проверяет, есть ли непосредственно «впереди» (в направлении ctx->dir) стена 
// или выход за границы. Возвращает true, если впереди WALL или за пределы.
bool  test_obstacle(const Context *ctx);

// Считает расстояние (количество пустых шестиугольников) от текущей позиции 
// ctx->q,ctx->r до ближайшего препятствия (WALL или граница) вдоль ctx->dir.
int   look_distance(const Context *ctx);

// Пытается сдвинуть робота на steps шестиугольников вперёд (если steps>0) 
// или назад (если steps<0). Возвращает true, если все шаги прошли без препятствий, 
// иначе false и останавливается при первой же стене.
bool  maze_move(Context *ctx, int steps);

// Поворачивает направление ctx->dir на step60 единиц (каждая «единица» = 60°).
// step60 = +1 означает «вправо» (прибавить +1), step60 = −1 означает «влево» (−1). 
void  maze_turn(Context *ctx, int step60);

// Если на текущей клетке (ctx->q,ctx->r) есть BOX, то поднимает коробку весом weight,
// увеличивая ctx->carried_weight. Возвращает true, если коробка (CELL_BOX) была и поднялась, 
// иначе false.
bool  maze_load_box(Context *ctx, int weight);

// Если у робота в руках (ctx->carried_weight) есть хотя бы weight, то сбрасывает 
// коробку весом weight в текущую клетку (ставит CELL_BOX в cells[r*width+q]). 
// Возвращает true, если вес был достаточен и коробка сброшена, иначе false.
bool  maze_drop_box(Context *ctx, int weight);

// Проверяет, находится ли робот на клетке EXIT (cells[r*width+q] == CELL_EXIT). 
// Возвращает true, если да, иначе false.
bool  is_at_exit(const Context *ctx);

#endif


#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>
#include "context.h"

typedef enum {
    CELL_EMPTY,
    CELL_WALL,
    CELL_BOX,
    CELL_EXIT,
    CELL_UNDEF
} CellType;

typedef struct Maze {
    int       width;     /* количество «столбцов» (q from 0..width−1) */
    int       height;    /* количество «строк»   (r from 0..height−1) */
    CellType *cells;     /* одномерный массив size = width*height, индекс: cells[r*width + q] */
} Maze;

Maze *maze_load(const char *filename, Context *ctx);
void  maze_free(Maze *mz);
bool  test_obstacle(const Context *ctx);
int   look_distance(const Context *ctx);
bool  maze_move(Context *ctx, int steps);
void  maze_turn(Context *ctx, int step60);
bool  maze_load_box(Context *ctx, int weight);
bool  maze_drop_box(Context *ctx, int weight);
bool  is_at_exit(const Context *ctx);

#endif


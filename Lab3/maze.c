#include "maze.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ══ Смещения для 6 направлений в осевых координатах (axial coordinates) ═════ */
/*  dir=0 → (dq=+1, dr= 0) — вправо                                      */
/*  dir=1 → (dq= 0, dr=+1) — «вниз-вправо»                                */
/*  dir=2 → (dq=−1, dr=+1) — «вниз-влево»                                 */
/*  dir=3 → (dq=−1, dr= 0) — влево                                       */
/*  dir=4 → (dq= 0, dr=−1) — «вверх-влево»                                */
/*  dir=5 → (dq=+1, dr=−1) — «вверх-вправо»                               */
const int hex_dq[6] = { +1,  0, -1, -1,  0, +1 };
const int hex_dr[6] = {  0, +1, +1,  0, -1, -1 };

#define IDX(mz, q, r)  ((r) * (mz)->width + (q))

static bool is_wall_or_oob_hex(const Maze *mz, int nq, int nr) {
    if (nq < 0 || nq >= mz->width || nr < 0 || nr >= mz->height) {
        return true;
    }
    return (mz->cells[ IDX(mz, nq, nr) ] == CELL_WALL);
}

bool test_obstacle(const Context *ctx) {
    const Maze *mz = ctx->mz;
    int q   = ctx->q;
    int r   = ctx->r;
    int dir = ctx->dir % 6;
    int nq = q + hex_dq[dir];
    int nr = r + hex_dr[dir];
    printf("Test: %d %d %d %d\n", q, r, nq, nr);
    return is_wall_or_oob_hex(mz, nq, nr);
}

int look_distance(const Context *ctx) {
    const Maze *mz = ctx->mz;
    int q   = ctx->q;
    int r   = ctx->r;
    int dir = ctx->dir % 6;
    int dist = 0;
    while (1) {
        int nq = q + hex_dq[dir];
        int nr = r + hex_dr[dir];
        if (is_wall_or_oob_hex(mz, nq, nr)) {
            break;
        }
        dist++;
        q = nq; 
        r = nr;
    }
    return dist;
}

bool maze_move(Context *ctx, int steps) {
    Maze *mz = ctx->mz;
    int dir = ctx->dir % 6;
    int dq = 0, dr = 0;

    if (steps > 0) {
        dq = hex_dq[dir];
        dr = hex_dr[dir];
    } else if (steps < 0) {
        int back_dir = (dir + 3) % 6;
        dq = hex_dq[back_dir];
        dr = hex_dr[back_dir];
        steps = -steps;
    }
    for (int i = 0; i < steps; i++) {
        int nq = ctx->q + dq;
        int nr = ctx->r + dr;
        if (is_wall_or_oob_hex(mz, nq, nr)) {
            return false;
        }
        ctx->q = nq;
        ctx->r = nr;
    }
    return true;
}

void maze_turn(Context *ctx, int step60) {
    ctx->dir = (ctx->dir + step60 + 6) % 6;
}

bool maze_load_box(Context *ctx, int weight) {
    Maze *mz = ctx->mz;
    int q = ctx->q;
    int r = ctx->r;
    CellType cell = mz->cells[ IDX(mz, q, r) ];
    if (cell == CELL_BOX) {
        ctx->carried_weight += weight;
        mz->cells[ IDX(mz, q, r) ] = CELL_EMPTY;
        return true;
    }
    return false;
}

bool maze_drop_box(Context *ctx, int weight) {
    Maze *mz = ctx->mz;
    int q = ctx->q;
    int r = ctx->r;
    if (ctx->carried_weight >= weight) {
        ctx->carried_weight -= weight;
        mz->cells[ IDX(mz, q, r) ] = CELL_BOX;
        return true;
    }
    return false;
}

bool is_at_exit(const Context *ctx) {
    const Maze *mz = ctx->mz;
    int q = ctx->q;
    int r = ctx->r;
    if (q < 0 || q >= mz->width || r < 0 || r >= mz->height) {
        return false;
    }
    return (mz->cells[ IDX(mz, q, r) ] == CELL_EXIT);
}

Maze *maze_load(const char *filename, Context *ctx) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open maze file '%s'\n", filename);
        return NULL;
    }

    int width, height, numBoxes, startCol, startRow, startDir;
    if (fscanf(f, "%d %d %d %d %d %d\n",
               &width, &height, &numBoxes, &startCol, &startRow, &startDir) != 6) {
        fprintf(stderr, "Invalid header in maze file\n");
        fclose(f);
        return NULL;
    }

    Maze *mz = (Maze *) malloc(sizeof(Maze));
    if (!mz) {
        fprintf(stderr, "Out of memory allocating Maze\n");
        fclose(f);
        return NULL;
    }
    mz->width  = width;
    mz->height = height;
    mz->cells  = (CellType *) malloc(sizeof(CellType) * width * height);
    if (!mz->cells) {
        fprintf(stderr, "Out of memory allocating Maze cells\n");
        free(mz);
        fclose(f);
        return NULL;
    }

    for (int r = 0; r < height; r++) {
        for (int q = 0; q < width; q++) {
            char token[64];
            if (fscanf(f, "%63s", token) != 1) {
                fprintf(stderr, "Unexpected EOF at row %d, col %d\n", r, q);
                free(mz->cells);
                free(mz);
                fclose(f);
                return NULL;
            }
            if (token[0] == '#') {
                mz->cells[ IDX(mz, q, r) ] = CELL_WALL;
            } else if (token[0] == 'E') {
                mz->cells[ IDX(mz, q, r) ] = CELL_EXIT;
            } else if (token[0] == '.') {
                if (strchr(token, ':') != NULL) {
                    mz->cells[ IDX(mz, q, r) ] = CELL_BOX;
                } else {
                    mz->cells[ IDX(mz, q, r) ] = CELL_EMPTY;
                }
            } else {
                /* Всё остальное считаем FLOOR */
                mz->cells[ IDX(mz, q, r) ] = CELL_EMPTY;
            }
        }
    }
    fclose(f);

    ctx->q   = startCol - 1;
    ctx->r   = startRow - 1;
    ctx->dir = startDir % 6;

    return mz;
}

void maze_free(Maze *mz) {
    if (!mz) return;
    free(mz->cells);
    free(mz);
}


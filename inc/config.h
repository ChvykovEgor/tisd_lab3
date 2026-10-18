#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

#define N 1000

typedef struct
{
    size_t rows;
    size_t cols;
    int matr[N][N];
} MatrFull;

typedef struct
{
    size_t rows;
    size_t cols;
    size_t nnz;
    int *a;
    size_t *ia;
    size_t *ja;
} MatrCSC;

#endif
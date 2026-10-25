#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

#define MAX_MATR 1000
#define MECHANIC_INPUT 1
#define AUTO_INPUT 2

typedef struct
{
    size_t rows;
    size_t cols;
    int matr[MAX_MATR][MAX_MATR];
} Matr;

typedef struct
{
    size_t rows;
    size_t cols;
    size_t nnz;
    int *a;
    size_t *ia;
    size_t *ja;
} MatrCSC;

typedef enum
{
    EXIT,
    INPUT_FIRST_MATR,
    INPUT_SECOND_MATR,
    VIEW_FIRST_MATR,
    VIEW_SECOND_MATR,
    COMPRESSED_MATR_SUM,
    FULL_MATR_SUM,
    BENCHMARK,
    NOT_CHOSEN
} menu;

#endif
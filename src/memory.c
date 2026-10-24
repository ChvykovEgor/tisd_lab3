#include "memory.h"
#include <stdlib.h>

error_code matr_csc_init(MatrCSC **matr_csc, size_t rows, size_t cols, size_t nnz)
{
    error_code rc = ERR_OK;
    if (*matr_csc != NULL)
        free_matr_csc(*matr_csc);
    *matr_csc = malloc(sizeof(MatrCSC));
    if (*matr_csc == NULL)
        rc = ERR_MEMORY_ALLOC;
    size_t *ia = NULL, *ja = NULL;
    int *a = NULL;
    if (rc == ERR_OK)
    {
        (*matr_csc)->rows = rows;
        (*matr_csc)->cols = cols;
        (*matr_csc)->nnz = nnz;
        a = malloc(nnz * sizeof(int));
        if (a == NULL)
            rc = ERR_MEMORY_ALLOC;
    }
    if (rc == ERR_OK)
    {
        ia = malloc(nnz * sizeof(size_t));
        if (ia == NULL)
            rc = ERR_MEMORY_ALLOC;
    }
    if (rc == ERR_OK)
    {

        ja = malloc((cols + 1) * sizeof(size_t));
        if (ja == NULL)
            rc = ERR_MEMORY_ALLOC;
    }
    if (rc == ERR_OK)
    {
        (*matr_csc)->a = a;
        (*matr_csc)->ia = ia;
        (*matr_csc)->ja = ja;
    }
    return rc;
}

void free_matrs(Matr *matr1, Matr *matr2, MatrCSC *matr_csc1, MatrCSC *matr_csc2)
{
    free(matr1);
    free(matr2);
    free_matr_csc(matr_csc1);
    free_matr_csc(matr_csc2);
}

void free_matr_csc(MatrCSC *matr)
{
    if (matr != NULL)
    {
        free(matr->a);
        free(matr->ia);
        free(matr->ja);
        free(matr);
    }
}
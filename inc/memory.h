#ifndef MEMORY_H
#define MEMORY_H

#include "config.h"
#include "errors.h"

error_code matr_csc_init(MatrCSC **matr_csc, size_t rows, size_t cols, size_t nnz);
void free_matrs(Matr *matr1, Matr *matr2, MatrCSC *matr_csc1, MatrCSC *matr_csc2);
void free_matr_csc(MatrCSC *matr);

#endif

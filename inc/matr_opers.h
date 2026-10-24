#ifndef MATR_OPERS_H
#define MATR_OPERS_H

#include "config.h"
#include "errors.h"

void standard_matr_addition(const Matr *matr1, const Matr *matr2, Matr *matr_res, size_t *nnz);
void compressed_matr_addition(MatrCSC *matr1, MatrCSC *matr2, MatrCSC *matr_res);
void add_compressed_row(MatrCSC *matr1, MatrCSC *matr2, MatrCSC *matr_res, size_t *nnz_res, size_t j);
void random_matr_fill(Matr *matr, size_t nnz);
void convert_matr_to_csc(Matr *matr, MatrCSC *matr_csc);
size_t rand_below(size_t n);
int rand_value(void);

#endif

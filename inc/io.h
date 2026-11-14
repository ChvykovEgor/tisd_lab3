#ifndef IO_H
#define IO_H

#include "errors.h"
#include "config.h"
#include <stdio.h>

#define INPUT_EXIT 100
void print_matr_standard(const Matr *m, const char *message);
void print_matr_csc_arrays(const MatrCSC *m, const char *message);
error_code auto_matr_input(Matr **matr, size_t rows, size_t cols, size_t *nnz);
error_code mechanic_matr_input(Matr **matr, size_t rows, size_t cols, size_t *nnz);
error_code general_matr_input(Matr **matr, size_t rows, size_t cols);
error_code input_matr_one_elem(Matr *matr, size_t num);
error_code input_matr_fill_percentage(size_t *percent);
error_code input_matr_size(size_t *rows, size_t *cols);
error_code enter_matr_input_method(int *chosen);
int number_width(long long value);
int matr_column_width(const Matr *m);
void clear_input_buffer(void);

#endif
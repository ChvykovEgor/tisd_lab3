#ifndef MENU_H
#define MENU_H

#include "errors.h"
#include "config.h"

error_code menu_process();
error_code menu_full_matr_sum(const Matr *matr1, const Matr *matr2);
error_code menu_compressed_matr_sum(MatrCSC *matr1, MatrCSC *matr2);
void menu_view_matr(Matr *matr, MatrCSC *matr_csc, char *message);
error_code menu_input_matrs(Matr **matr, MatrCSC **matr_csc, size_t rows, size_t cols);
error_code input_menu_var(int *chosen);

#endif
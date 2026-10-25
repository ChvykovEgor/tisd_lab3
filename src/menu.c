#include "menu.h"
#include "io.h"
#include "matr_opers.h"
#include "memory.h"
#include "benchmark.h"
#include <stdio.h>

error_code menu_process()
{
    error_code rc = ERR_OK;
    Matr *matr1 = NULL, *matr2 = NULL;
    MatrCSC *matr_csc1 = NULL, *matr_csc2 = NULL;
    int chosen = NOT_CHOSEN;
    while (chosen != EXIT && rc == ERR_OK)
    {
        if (input_menu_var(&chosen) != ERR_OK)
        {
            printf("Некорректный вариант. Чтобы выбрать, введите число от 0 до 7\n");
            chosen = NOT_CHOSEN;
        }
        switch (chosen)
        {
            case INPUT_FIRST_MATR:
                if (matr2 != NULL)
                    rc = menu_input_matrs(&matr1, &matr_csc1, matr2->rows, matr2->cols);
                else
                    rc = menu_input_matrs(&matr1, &matr_csc1, 0, 0);
                break;
            case INPUT_SECOND_MATR:
                if (matr1 != NULL)
                    rc = menu_input_matrs(&matr2, &matr_csc2, matr1->rows, matr1->cols);
                else
                    rc = menu_input_matrs(&matr2, &matr_csc2, 0, 0);
                break;
            case VIEW_FIRST_MATR:
                menu_view_matr(matr1, matr_csc1, "Матрица 1");
                break;
            case VIEW_SECOND_MATR:
                menu_view_matr(matr2, matr_csc2, "Матрица 2");
                break;
            case COMPRESSED_MATR_SUM:
                rc = menu_compressed_matr_sum(matr_csc1, matr_csc2);
                break;
            case FULL_MATR_SUM:
                rc = menu_full_matr_sum(matr1, matr2);
                break;
            case BENCHMARK:
                rc = run_benchmark();
                break;
            case NOT_CHOSEN:
                break;
        }
    }
    free_matrs(matr1, matr2, matr_csc1, matr_csc2);
    return rc;
}

error_code menu_full_matr_sum(const Matr *matr1, const Matr *matr2)
{
    if (matr1 == NULL || matr2 == NULL)
    {
        printf("Сначала введите обе матрицы (пункты 1 и 2)\n");
        return ERR_OK;
    }
    if (matr1->rows != matr2->rows || matr1->cols != matr2->cols)
    {
        printf("Размеры матриц не совпадают, сложение невозможно\n");
        return ERR_OK;
    }

    Matr *matr_res = malloc(sizeof(Matr));
    if (matr_res == NULL)
        return ERR_MEMORY_ALLOC;

    standard_matr_addition(matr1, matr2, matr_res);
    printf("Результат сложения: матрица %zu x %zu",
           matr_res->rows, matr_res->cols);
    print_matr_standard(matr_res, "Результат сложения стандартных матриц");
    free(matr_res);
    return ERR_OK;
}

error_code menu_compressed_matr_sum(MatrCSC *matr1, MatrCSC *matr2)
{
    error_code rc = ERR_OK;
    if (matr1 == NULL || matr2 == NULL)
        printf("Ошибка. Для сложения нужно ввести обе матрицы\n");
    else
    {
        MatrCSC *matr_res = NULL;
        rc = matr_csc_init(&matr_res, matr1->rows, matr1->cols, matr1->nnz + matr2->nnz);
        if (rc == ERR_OK)
        {
            compressed_matr_addition(matr1, matr2, matr_res);
            print_matr_csc_arrays(matr_res, "Результат сложения матриц");
            free_matr_csc(matr_res);
        }
    }
    return rc;
}

void menu_view_matr(Matr *matr, MatrCSC *matr_csc, char *message)
{
    if (matr != NULL)
    {
        print_matr_csc_arrays(matr_csc, message);
        if (matr->cols <= 30 && matr->rows <= 30)
            print_matr_standard(matr, message);
        else
            printf("Матрица в стандартном виде не печатается, так как ее размер слишком большой\n");
    }
    else
        printf("Матрица не введена\n");
}

error_code menu_input_matrs(Matr **matr, MatrCSC **matr_csc, size_t rows, size_t cols)
{
    error_code rc = ERR_OK;
    int chosen;
    printf("Выберите формат ввода матрицы:\n"
           "1 - Вручную по координатам\n"
           "2 - Случайно с указанием процента заполнения\n");
    while (enter_matr_input_method(&chosen) != ERR_OK)
    {
        printf("Некорректный выбор формата\n");
        printf("Выберите формат ввода матрицы:\n"
           "1 - Вручную по координатам\n"
           "2 - Случайно с указанием процента заполнения\n");
        clear_input_buffer();
    }
    clear_input_buffer();
    size_t nnz;
    switch (chosen)
    {
        case MECHANIC_INPUT:
            rc = mechanic_matr_input(matr, rows, cols, &nnz);
            break;
        case AUTO_INPUT:
            rc = auto_matr_input(matr, rows, cols, &nnz);
            break;
    }
    if (rc == ERR_OK)
        rc = matr_csc_init(matr_csc, (*matr)->rows, (*matr)->cols, nnz);
    if (rc == ERR_OK)
        convert_matr_to_csc(*matr, *matr_csc);
    return rc;
}

error_code input_menu_var(int *chosen)
{
    error_code rc = ERR_OK;
    printf("============== Главное меню =============\n"
        "0 - Выход\n"
        "1 - Ввести 1-ю матрицу\n"
        "2 - Ввести 2-ю матрицу\n"
        "3 - Просмотреть 1-ю матрицу\n"
        "4 - Просмотреть 2-ю матрицу\n"
        "5 - Сложить 2-е сжатые матрицы и вывести результат в виде сжатой матрицы\n"
        "6 - Сложить 2-е полные матрицы и вывести результат в виде полной матрицы\n"
        "7 - Сравнить эффективность 2-х алгоритмов сложения матриц\n");
    printf("Введите вариант, который хотите выбрать:\n");
    if (scanf("%d", chosen) != 1 || *chosen > 7 || *chosen < 0)
    {
        rc = ERR_CHOSE_MENU;
        clear_input_buffer();
    }
    clear_input_buffer();
    return rc;
}
#include "io.h"
#include "matr_opers.h"
#include <math.h>
#include <string.h>

void print_matr_standard(const Matr *m, const char *message)
{
    int w = matr_column_width(m);

    printf("=== %s в стандартном виде ===\n", message);
    printf("Размер матрицы: %zu x %zu\n", m->rows, m->cols);
    printf("     |");
    for (size_t j = 0; j < m->cols; ++j)
        printf("%*zu", w, j);
    printf("\n-----+");
    for (size_t j = 0; j < m->cols; ++j)
        for (int k = 0; k < w; ++k)
            putchar('-');
    printf("\n");

    for (size_t i = 0; i < m->rows; ++i)
    {
        printf("%4zu |", i);
        for (size_t j = 0; j < m->cols; ++j)
            printf("%*d", w, m->matr[i][j]);
        printf("\n");
    }
}

void print_matr_csc_arrays(const MatrCSC *m, const char *message)
{
    printf("=== %s в форме 3-х объектов ===\n", message);
    printf("Размер матрицы: %zu x %zu, ненулевых элементов: %zu\n",
           m->rows, m->cols, m->nnz);

    if (m->nnz == 0)
        printf("Матрица нулевая: массивы A и IA пусты.\n");
    else
    {
        printf("\nМассивы A и IA:\n");
        printf("+------------+----------------+-----------------+\n");
        printf("| Позиция k  | Строка IA[k]   | Значение A[k]   |\n");
        printf("+------------+----------------+-----------------+\n");
        for (size_t k = 0; k < m->nnz; ++k)
            printf("| %10zu | %14zu | %15d |\n", k, m->ia[k], m->a[k]);
        printf("+------------+----------------+-----------------+\n");
    }

    printf("\nМассив JA (начала столбцов):\n");
    printf("+------------+------------+---------------------+\n");
    printf("| Столбец j  | JA[j]      | Элементов в столбце |\n");
    printf("+------------+------------+---------------------+\n");
    for (size_t j = 0; j < m->cols; ++j)
        printf("| %10zu | %10zu | %19zu |\n", j, m->ja[j], m->ja[j + 1] - m->ja[j]);
    printf("| %10zu | %10zu | %19s |\n", m->cols, m->ja[m->cols], "-");
    printf("+------------+------------+---------------------+\n");
}

error_code auto_matr_input(Matr **matr, size_t rows, size_t cols, size_t *nnz)
{
    error_code rc = general_matr_input(matr, rows, cols);
    size_t fill_percent;
    if (rc == ERR_OK)
    {
        while (input_matr_fill_percentage(&fill_percent) != ERR_OK)
        {
            printf("Процент введен неверно. Повторите попытку\n");
            clear_input_buffer();
        }
        clear_input_buffer();
        *nnz = (size_t) round((double) ((*matr)->rows * (*matr)->cols * fill_percent) / 100.0);
        random_matr_fill(*matr, *nnz);
    }
    return rc;
}

error_code mechanic_matr_input(Matr **matr, size_t rows, size_t cols, size_t *nnz)
{
    error_code rc = general_matr_input(matr, rows, cols);
    if (rc == ERR_OK)
    {
        while (input_no_zero_count(nnz, (*matr)->rows * (*matr)->cols) != ERR_OK)
        {
            printf("Число ненулевых элементов введено неверно\n");
            clear_input_buffer();
        }
        clear_input_buffer();
        for (size_t i = 0; i < *nnz; i++)
        {
            error_code elem_rc = input_matr_one_elem(*matr, i + 1);
            while (elem_rc != ERR_OK)
            {
                if (elem_rc == ERR_MATR_INPUT)
                    printf("Числа введены неверно. Повторите попытку\n");
                else if (elem_rc == ERR_ZERO_INPUT)
                    printf("Введенный элемент должен быть отличен от 0. Повторите попытку\n");
                else if (elem_rc == ERR_ELEM_ALREADY_EXIST)
                    printf("Вы уже ввели элемент на данной позиции. Повторите попытку\n");
                clear_input_buffer();
                elem_rc = input_matr_one_elem(*matr, i + 1);
            }
        }
    }
    return rc;
}

error_code general_matr_input(Matr **matr, size_t rows, size_t cols)
{
    error_code rc = ERR_OK;
    if (*matr != NULL)
        free(*matr);
    *matr = malloc(sizeof(Matr));
    if (*matr == NULL)
        rc = ERR_MEMORY_ALLOC;
    if (rc == ERR_OK)
    {
        if (rows == 0)
        {
            while (input_matr_size(&rows, &cols) != ERR_OK)
            {
                printf("Размерность матрицы введена неверно\n");
                clear_input_buffer();
            }
            clear_input_buffer();
        }
        else
            printf("Размерность матрицы %zux%zu\n", rows, cols);
        (*matr)->rows = rows;
        (*matr)->cols = cols;
        memset((*matr)->matr, 0, sizeof((*matr)->matr));
    }
    return rc;
}

error_code input_matr_one_elem(Matr *matr, size_t num)
{
    error_code rc = ERR_OK;
    long long ll_i, ll_j;
    int elem;
    printf("%zu. Введите 3 целых числа - индекс строки, столбца и ненулевой элемент матрицы. "
           "Индекс строки и столбца не должны повторяться с теми, которые вы уже ввели. \n", num);
    if (scanf("%lld%lld%d", &ll_i, &ll_j, &elem) != 3 || ll_i < 0 || ll_j < 0 ||
        (size_t) ll_i >= matr->rows || (size_t) ll_j >= matr->cols)
        rc = ERR_MATR_INPUT;
    else if (elem == 0)
        rc = ERR_ZERO_INPUT;
    else if (matr->matr[ll_i][ll_j] != 0)
        rc = ERR_ELEM_ALREADY_EXIST;
    else
        matr->matr[ll_i][ll_j] = elem;
    return rc;
}

error_code input_matr_fill_percentage(size_t *percent)
{
    error_code rc = ERR_OK;
    long long ll_percent;
    printf("Введите целое число от 0 до 100 - процент заполнения матрицы "
           "ненулевыми элементами:\n");
    if (scanf("%lld", &ll_percent) != 1 || ll_percent < 0 || ll_percent > 100)
        rc = ERR_MATR_INPUT;
    else
        *percent = (size_t) ll_percent;
    return rc;
}

error_code input_no_zero_count(size_t *nnz, size_t max_nnz)
{
    error_code rc = ERR_OK;
    long long ll_nnz;
    printf("Введите целое число от 0 до %zu - количество ненулевых элементов:\n",
           max_nnz);
    if (scanf("%lld", &ll_nnz) != 1 || ll_nnz < 0 || (unsigned long long) ll_nnz > max_nnz)
        rc = ERR_MATR_INPUT;
    else
        *nnz = (size_t) ll_nnz;
    return rc;
}

error_code input_matr_size(size_t *rows, size_t *cols)
{
    error_code rc = ERR_OK;
    long long ll_rows, ll_cols;
    printf("Введите через пробел 2 положительных целое число не большее 1000 "
           "- число строк и столбцов матрицы:\n");
    if (scanf("%lld%lld", &ll_rows, &ll_cols) != 2 || ll_rows > MAX_MATR || ll_cols > MAX_MATR
        || ll_cols <= 0 || ll_rows <= 0)
        rc = ERR_MATR_INPUT;
    else
    {
        *rows = (size_t) ll_rows;
        *cols = (size_t) ll_cols;
    }
    return rc;
}

error_code enter_matr_input_method(int *chosen)
{
    error_code rc = ERR_OK;
    if (scanf("%d", chosen) != 1 || *chosen < 1 || *chosen > 2)
        rc = ERR_CHOSE_MENU;
    return rc;
}

int number_width(long long value)
{
    int width = value < 0 ? 2 : 1;
    unsigned long long v = value < 0 ? (unsigned long long) (-value) : (unsigned long long) value;
    while (v >= 10)
    {
        v /= 10;
        width++;
    }
    return width;
}

int matr_column_width(const Matr *m)
{
    int width = number_width((long long) m->cols - 1);
    for (size_t i = 0; i < m->rows; ++i)
    {
        for (size_t j = 0; j < m->cols; ++j)
        {
            int w = number_width(m->matr[i][j]);
            if (w > width)
                width = w;
        }
    }
    return width + 1;
}

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
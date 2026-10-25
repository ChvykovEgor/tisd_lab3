#include "matr_opers.h"

void standard_matr_addition(const Matr *matr1, const Matr *matr2, Matr *matr_res)
{
    matr_res->rows = matr1->rows;
    matr_res->cols = matr1->cols;
    for (size_t i = 0; i < matr1->rows; i++)
        for (size_t j = 0; j < matr1->cols; j++)
        {
            int sum = matr1->matr[i][j] + matr2->matr[i][j];
            matr_res->matr[i][j] = sum;
        }
}

void compressed_matr_addition(MatrCSC *matr1, MatrCSC *matr2, MatrCSC *matr_res)
{
    matr_res->ja[0] = 0;
    size_t nnz_res = 0;
    for (size_t j = 0; j < matr1->cols; j++)
        add_compressed_row(matr1, matr2, matr_res, &nnz_res, j);
    matr_res->nnz = nnz_res;
}

void add_compressed_row(MatrCSC *matr1, MatrCSC *matr2, MatrCSC *matr_res, size_t *nnz_res, size_t j)
{
    size_t left1 = matr1->ja[j], right1 = matr1->ja[j + 1];
    size_t left2 = matr2->ja[j], right2 = matr2->ja[j + 1];
    while (left1 < right1 && left2 < right2)
    {
        int sum;
        size_t ia_res;
        if (matr1->ia[left1] == matr2->ia[left2])
        {
            sum = matr1->a[left1] + matr2->a[left2];
            ia_res = matr1->ia[left1];
            left1++;
            left2++;
        }
        else if (matr1->ia[left1] < matr2->ia[left2])
        {
            sum = matr1->a[left1];
            ia_res = matr1->ia[left1];
            left1++;
        }
        else
        {
            sum = matr2->a[left2];
            ia_res = matr2->ia[left2];
            left2++;
        }
        if (sum != 0)
        {
            matr_res->a[*nnz_res] = sum;
            matr_res->ia[*nnz_res] = ia_res;
            (*nnz_res)++;
        }
    }
    while (left1 < right1)
    {
        matr_res->a[*nnz_res] = matr1->a[left1];
        matr_res->ia[*nnz_res] = matr1->ia[left1];
        left1++;
        (*nnz_res)++;
    }
    while (left2 < right2)
    {
        matr_res->a[*nnz_res] = matr2->a[left2];
        matr_res->ia[*nnz_res] = matr2->ia[left2];
        left2++;
        (*nnz_res)++;
    }
    matr_res->ja[j + 1] = *nnz_res;
}

void random_matr_fill(Matr *matr, size_t nnz)
{
    size_t total = matr->rows * matr->cols;
    size_t left = total;
    size_t need = nnz;
    for (size_t i = 0; i < matr->rows; ++i)
    {
        for (size_t j = 0; j < matr->cols; ++j)
        {
            if (need > 0 && rand_below(left) < need)
            {
                matr->matr[i][j] = rand_value();
                --need;
            }
            else
                matr->matr[i][j] = 0;
            --left;
        }
    }
}

void convert_matr_to_csc(Matr *matr, MatrCSC *matr_csc)
{
    size_t cur_nn = 0;
    for (size_t i = 0; i < matr->cols; i++)
    {
        matr_csc->ja[i] = cur_nn;
        for (size_t j = 0; j < matr->rows; j++)
        {
            int elem = matr->matr[j][i];
            if (elem != 0)
            {
                matr_csc->a[cur_nn] = elem;
                matr_csc->ia[cur_nn] = j;
                cur_nn++;
            }
        }
    }
    matr_csc->ja[matr->cols] = cur_nn;
}

size_t rand_below(size_t n)
{
    unsigned long long r = 0;
    for (int k = 0; k < 3; ++k)
        r = r * ((unsigned long long) RAND_MAX + 1ULL) + (unsigned long long) rand();
    return (size_t) (r % n);
}

int rand_value(void)
{
    int v = (int) rand_below(999) + 1;
    return rand_below(2) ? v : -v;
}

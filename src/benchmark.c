#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "benchmark.h"
#include "io.h"
#include "matr_opers.h"
#include "memory.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

error_code run_benchmark(void)
{
    error_code rc = ERR_OK;
    int mode;
    size_t rows = 0, cols = 0;
    const size_t sizes[BENCH_SIZES_COUNT] = BENCH_SIZES_INIT;

    printf("Выберите режим замеров:\n"
           "1 - Набор квадратных матриц: 10, 100, 500, 1000\n"
           "2 - Один размер, который вы зададите\n");
    while (input_bench_mode(&mode) != ERR_OK)
    {
        printf("Некорректный режим. Введите 1 или 2:\n");
        clear_input_buffer();
    }
    clear_input_buffer();

    if (mode == BENCH_CUSTOM)
    {
        while (input_matr_size(&rows, &cols) != ERR_OK)
        {
            printf("Размерность матрицы введена неверно\n");
            clear_input_buffer();
        }
        clear_input_buffer();
    }

    Matr *a = malloc(sizeof(Matr));
    Matr *b = malloc(sizeof(Matr));
    Matr *res = malloc(sizeof(Matr));
    if (a == NULL || b == NULL || res == NULL)
        rc = ERR_MEMORY_ALLOC;

    if (rc == ERR_OK)
    {
        printf("Идут замеры, это может занять несколько секунд...\n");
        if (mode == BENCH_CUSTOM)
            rc = bench_size(rows, cols, a, b, res);
        else
            for (size_t k = 0; k < BENCH_SIZES_COUNT && rc == ERR_OK; k++)
                rc = bench_size(sizes[k], sizes[k], a, b, res);
    }
    if (rc == ERR_OK)
        print_bench_notes();

    free(a);
    free(b);
    free(res);
    return rc;
}

error_code input_bench_mode(int *mode)
{
    error_code rc = ERR_OK;
    if (scanf("%d", mode) != 1 || *mode < BENCH_PRESET || *mode > BENCH_CUSTOM)
        rc = ERR_CHOSE_MENU;
    return rc;
}

error_code bench_size(size_t rows, size_t cols, Matr *a, Matr *b, Matr *res)
{
    error_code rc = ERR_OK;
    size_t total = rows * cols;
    size_t prev_nnz = 0;
    double prev_fill = -1.0;
    double time_lo = -1.0, time_hi = -1.0;
    double mem_lo = -1.0, mem_hi = -1.0;

    a->rows = rows;
    a->cols = cols;
    b->rows = rows;
    b->cols = cols;

    printf("\nРазмер матриц: %zu x %zu\n", rows, cols);
    print_bench_header();
    for (size_t step = 0; step <= BENCH_PERCENTS_COUNT && rc == ERR_OK; step++)
    {
        size_t nnz = bench_nnz(total, step);
        if (nnz == prev_nnz)
            continue;
        prev_nnz = nnz;

        BenchResult r;
        rc = bench_measure(a, b, res, nnz, 1, &r);
        if (rc == ERR_OK)
        {
            print_bench_row(&r);
            if (time_hi < 0.0 && !csc_is_better(&r, 0))
            {
                time_hi = r.fill;
                time_lo = prev_fill;
            }
            if (mem_hi < 0.0 && !csc_is_better(&r, 1))
            {
                mem_hi = r.fill;
                mem_lo = prev_fill;
            }
            prev_fill = r.fill;
        }
    }
    print_bench_sep();

    if (rc == ERR_OK)
        rc = bench_report(a, b, res, time_lo, time_hi, 0);
    if (rc == ERR_OK)
        rc = bench_report(a, b, res, mem_lo, mem_hi, 1);
    return rc;
}

error_code bench_report(Matr *a, Matr *b, Matr *res, double lo, double hi, int by_memory)
{
    error_code rc = ERR_OK;
    const char *name = by_memory ? "По памяти" : "По времени";

    if (hi < 0.0)
        printf("%s: сжатый вариант выгоднее стандартного при всех проверенных заполнениях\n", name);
    else if (lo < 0.0)
        printf("%s: сжатый вариант невыгоден уже при минимальном заполнении (1 ненулевой элемент)\n",
               name);
    else
    {
        size_t percent = 0;
        rc = bench_refine(a, b, res, lo, hi, by_memory, &percent);
        if (rc == ERR_OK)
            printf("%s: сжатый вариант перестает быть выгодным начиная с %zu %% заполнения "
                   "(с точностью до 1 %%)\n", name, percent);
    }
    return rc;
}

error_code bench_refine(Matr *a, Matr *b, Matr *res, double lo, double hi, int by_memory,
                        size_t *percent)
{
    error_code rc = ERR_OK;
    size_t total = a->rows * a->cols;
    size_t lo_i = (size_t) lo;
    size_t hi_i = (size_t) ceil(hi);

    while (rc == ERR_OK && hi_i - lo_i > 1)
    {
        size_t mid = lo_i + (hi_i - lo_i) / 2;
        BenchResult r;
        rc = bench_measure(a, b, res, percent_to_nnz(total, (double) mid), !by_memory, &r);
        if (rc == ERR_OK)
        {
            if (csc_is_better(&r, by_memory))
                lo_i = mid;
            else
                hi_i = mid;
        }
    }
    *percent = hi_i;
    return rc;
}

error_code bench_measure(Matr *a, Matr *b, Matr *res, size_t nnz, int with_time, BenchResult *out)
{
    size_t rows = a->rows;
    size_t cols = a->cols;
    size_t total = rows * cols;
    MatrCSC *csc_a = NULL, *csc_b = NULL, *csc_res = NULL;

    random_matr_fill(a, nnz);
    random_matr_fill(b, nnz);

    error_code rc = matr_csc_init(&csc_a, rows, cols, nnz);
    if (rc == ERR_OK)
        rc = matr_csc_init(&csc_b, rows, cols, nnz);
    if (rc == ERR_OK)
        rc = matr_csc_init(&csc_res, rows, cols, 2 * nnz);
    if (rc == ERR_OK)
    {
        convert_matr_to_csc(a, csc_a);
        convert_matr_to_csc(b, csc_b);
        compressed_matr_addition(csc_a, csc_b, csc_res);

        out->nnz = nnz;
        out->fill = 100.0 * (double) nnz / (double) total;
        out->mem_std = 3 * total * sizeof(int);
        out->mem_csc = csc_memory(csc_a) + csc_memory(csc_b) + csc_memory(csc_res);
        out->mem_gain = (double) out->mem_std / (double) out->mem_csc;
        out->t_std_ns = 0.0;
        out->t_csc_ns = 0.0;
        out->speedup = 0.0;

        if (with_time)
        {
            out->t_std_ns = time_standard_ns(a, b, res);
            out->t_csc_ns = time_compressed_ns(csc_a, csc_b, csc_res);
            out->speedup = out->t_std_ns / (out->t_csc_ns > 1e-3 ? out->t_csc_ns : 1e-3);
        }
    }
    free_matr_csc(csc_a);
    free_matr_csc(csc_b);
    free_matr_csc(csc_res);
    return rc;
}

double time_standard_ns(const Matr *a, const Matr *b, Matr *res)
{
    struct timespec t_beg, t_end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_beg);
    for (size_t r = 0; r < BENCH_REPS; r++)
        standard_matr_addition(a, b, res);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_end);
    double time = (double) calc_elapsed_ns(&t_beg, &t_end) / BENCH_REPS;
    return time;
}

double time_compressed_ns(MatrCSC *a, MatrCSC *b, MatrCSC *res)
{
    struct timespec t_beg, t_end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_beg);
    for (size_t r = 0; r < BENCH_REPS; r++)
        compressed_matr_addition(a, b, res);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_end);
    double time = (double) calc_elapsed_ns(&t_beg, &t_end) / BENCH_REPS;
    return time;
}

unsigned long long calc_elapsed_ns(const struct timespec *beg, const struct timespec *end)
{
    long long ns = (long long) (end->tv_sec - beg->tv_sec) * 1000000000LL +
                   (long long) (end->tv_nsec - beg->tv_nsec);
    return (unsigned long long) ns;
}

size_t csc_memory(const MatrCSC *m)
{
    return m->nnz * (sizeof(int) + sizeof(size_t)) + (m->cols + 1) * sizeof(size_t);
}

int csc_is_better(const BenchResult *r, int by_memory)
{
    return by_memory ? r->mem_gain >= 1.0 : r->speedup >= 1.0;
}

size_t bench_nnz(size_t total, size_t step)
{
    const double percents[BENCH_PERCENTS_COUNT] = BENCH_PERCENTS_INIT;
    size_t nnz = 1;
    if (step > 0)
        nnz = percent_to_nnz(total, percents[step - 1]);
    return nnz;
}

size_t percent_to_nnz(size_t total, double percent)
{
    size_t nnz = (size_t) round((double) total * percent / 100.0);
    if (nnz < 1)
        nnz = 1;
    if (nnz > total)
        nnz = total;
    return nnz;
}

void print_bench_header(void)
{
    const size_t widths[BENCH_COLS] = BENCH_WIDTHS_INIT;
    const char *names[BENCH_COLS] = { "Заполн., %", "nnz", "Станд., нс", "Сжатый, нс",
                                      "Ускорение", "Станд., байт", "Сжатый, байт", "Экономия" };
    print_bench_sep();
    for (size_t c = 0; c < BENCH_COLS; c++)
    {
        printf("| ");
        print_padded(names[c], widths[c]);
        putchar(' ');
    }
    printf("|\n");
    print_bench_sep();
}

void print_bench_row(const BenchResult *r)
{
    printf("| %10.4f | %8zu | %12.1f | %12.1f | %10.2f | %13zu | %13zu | %10.2f |\n",
           r->fill, r->nnz, r->t_std_ns, r->t_csc_ns, r->speedup, r->mem_std, r->mem_csc,
           r->mem_gain);
}

void print_bench_sep(void)
{
    const size_t widths[BENCH_COLS] = BENCH_WIDTHS_INIT;
    for (size_t c = 0; c < BENCH_COLS; c++)
    {
        putchar('+');
        for (size_t k = 0; k < widths[c] + 2; k++)
            putchar('-');
    }
    printf("+\n");
}

void print_padded(const char *s, size_t width)
{
    size_t len = 0;
    for (const char *p = s; *p; p++)
        if (((unsigned char) *p & 0xC0) != 0x80)
            len++;
    printf("%s", s);
    for (; len < width; len++)
        putchar(' ');
}

void print_bench_notes(void)
{
    printf("\nУсловные обозначения:\n"
           "  nnz       - число ненулевых элементов в каждой из складываемых матриц\n"
           "  нс        - время одного сложения в наносекундах (CLOCK_MONOTONIC_RAW):\n"
           "Берется среднее время из %d сложений"
           "  Ускорение - время стандартного алгоритма / время сжатого (больше 1 - сжатый быстрее)\n"
           "  Экономия  - память стандартного хранения / память сжатого (больше 1 - сжатый экономнее)\n"
           "Статический массив Matr физически занимает %zu байт независимо от размера матрицы.\n"
           "Порог уточняется половинным делением между соседними замерами до 1 %%\n"
           "и предполагает, что выгодность меняется один раз.\n", BENCH_REPS, sizeof(Matr));
}
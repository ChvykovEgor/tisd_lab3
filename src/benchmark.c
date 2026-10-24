#include "benchmark.h"
#include "io.h"
#include "matr_opers.h"
#include "memory.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

error_code run_benchmark(void)
{
    error_code rc = ERR_OK;
    int mode;
    size_t rows = 0, cols = 0;
    int all_match = 1;
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
            rc = bench_size(rows, cols, a, b, res, &all_match);
        else
            for (size_t k = 0; k < BENCH_SIZES_COUNT && rc == ERR_OK; k++)
                rc = bench_size(sizes[k], sizes[k], a, b, res, &all_match);
    }
    if (rc == ERR_OK)
        print_bench_notes(all_match);

    free(a);
    free(b);
    free(res);
    return rc;
}

error_code bench_size(size_t rows, size_t cols, Matr *a, Matr *b, Matr *res, int *all_match)
{
    error_code rc = ERR_OK;
    size_t total = rows * cols;
    size_t prev_nnz = 0;
    double slow_time = -1.0;
    double slow_mem = -1.0;

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
        rc = bench_row(a, b, res, nnz, &r);
        if (rc == ERR_OK)
        {
            if (!r.match)
                *all_match = 0;
            if (slow_time < 0 && r.speedup < 1.0)
                slow_time = r.fill;
            if (slow_mem < 0 && r.mem_gain < 1.0)
                slow_mem = r.fill;
        }
    }
    print_bench_sep();

    if (rc == ERR_OK)
    {
        if (slow_time < 0)
            printf("По времени: сжатое сложение быстрее стандартного при всех проверенных заполнениях\n");
        else
            printf("По времени: сжатое сложение перестает быть выгодным, начиная с заполнения %.4f %%\n",
                   slow_time);
        if (slow_mem < 0)
            printf("По памяти: сжатое хранение экономнее при всех проверенных заполнениях\n");
        else
            printf("По памяти: сжатое хранение перестает быть выгодным, начиная с заполнения %.4f %%\n",
                   slow_mem);
    }
    return rc;
}

size_t bench_nnz(size_t total, size_t step)
{
    const double percents[BENCH_PERCENTS_COUNT] = BENCH_PERCENTS_INIT;
    if (step == 0)
        return 1;
    size_t nnz = (size_t) round((double) total * percents[step - 1] / 100.0);
    if (nnz < 1)
        nnz = 1;
    if (nnz > total)
        nnz = total;
    return nnz;
}

error_code bench_row(Matr *a, Matr *b, Matr *res, size_t nnz, BenchResult *out)
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

        size_t reps = (size_t) (BENCH_TARGET_CELLS / total);
        if (reps == 0)
            reps = 1;

        size_t nnz_std = 0;
        clock_t start = clock();
        for (size_t r = 0; r < reps; r++)
            standard_matr_addition(a, b, res, &nnz_std);
        double t_std = elapsed_us(start, clock(), reps);

        start = clock();
        for (size_t r = 0; r < reps; r++)
            compressed_matr_addition(csc_a, csc_b, csc_res);
        double t_csc = elapsed_us(start, clock(), reps);

        size_t mem_std = 3 * total * sizeof(int);
        size_t mem_csc = csc_memory(csc_a) + csc_memory(csc_b) + csc_memory(csc_res);
        double t_safe = t_csc > 1e-9 ? t_csc : 1e-9;

        out->fill = 100.0 * (double) nnz / (double) total;
        out->speedup = t_std / t_safe;
        out->mem_gain = (double) mem_std / (double) mem_csc;
        out->match = results_match(res, nnz_std, csc_res);

        printf("| %10.4f | %8zu | %12.2f | %12.2f | %10.2f | %13zu | %13zu | %10.2f |\n",
               out->fill, nnz, t_std, t_csc, out->speedup, mem_std, mem_csc, out->mem_gain);
    }
    free_matr_csc(csc_a);
    free_matr_csc(csc_b);
    free_matr_csc(csc_res);
    return rc;
}

size_t csc_memory(const MatrCSC *m)
{
    return m->nnz * (sizeof(int) + sizeof(size_t)) + (m->cols + 1) * sizeof(size_t);
}

double elapsed_us(clock_t start, clock_t end, size_t reps)
{
    return (double) (end - start) * 1e6 / CLOCKS_PER_SEC / (double) reps;
}

int results_match(const Matr *dense, size_t dense_nnz, const MatrCSC *csc)
{
    if (dense_nnz != csc->nnz)
        return 0;
    for (size_t j = 0; j < csc->cols; j++)
        for (size_t p = csc->ja[j]; p < csc->ja[j + 1]; p++)
            if (dense->matr[csc->ia[p]][j] != csc->a[p])
                return 0;
    return 1;
}

error_code input_bench_mode(int *mode)
{
    if (scanf("%d", mode) != 1 || *mode < BENCH_PRESET || *mode > BENCH_CUSTOM)
        return ERR_CHOSE_MENU;
    return ERR_OK;
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

void print_bench_header(void)
{
    const size_t widths[BENCH_COLS] = BENCH_WIDTHS_INIT;
    const char *names[BENCH_COLS] = { "Заполн., %", "nnz", "Станд., мкс", "Сжатый, мкс",
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

void print_bench_notes(int all_match)
{
    printf("\nУсловные обозначения:\n"
           "  nnz       - число ненулевых элементов в каждой из складываемых матриц\n"
           "  мкс       - время одного сложения (среднее по нескольким повторам),\n"
           "              генерация и перевод в CSC во время не входят\n"
           "  Ускорение - время стандартного алгоритма / время сжатого (больше 1 - сжатый быстрее)\n"
           "  Экономия  - память стандартного хранения / память сжатого (больше 1 - сжатый экономнее)\n"
           "Статический массив Matr физически занимает %zu байт независимо от размера матрицы.\n",
           sizeof(Matr));
    if (all_match)
        printf("Проверка: результаты обоих алгоритмов совпали во всех замерах\n");
    else
        printf("ВНИМАНИЕ: результаты алгоритмов различаются, в алгоритме есть ошибка\n");
}
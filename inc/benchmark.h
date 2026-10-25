#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include "errors.h"
#include "config.h"

#define BENCH_COLS 8
#define BENCH_WIDTHS_INIT { 10, 8, 12, 12, 10, 13, 13, 10 }
#define BENCH_SIZES_COUNT 4
#define BENCH_SIZES_INIT { 10, 100, 500, 1000 }
#define BENCH_PERCENTS_COUNT 9
#define BENCH_PERCENTS_INIT { 0.1, 1, 5, 10, 20, 30, 50, 70, 100 }
#define BENCH_REPS 200
#define BENCH_PRESET 1
#define BENCH_CUSTOM 2

struct timespec;

typedef struct
{
    size_t nnz;
    double fill;
    double t_std_ns;
    double t_csc_ns;
    size_t mem_std;
    size_t mem_csc;
    double speedup;
    double mem_gain;
} BenchResult;

error_code run_benchmark(void);
error_code input_bench_mode(int *mode);
error_code bench_size(size_t rows, size_t cols, Matr *a, Matr *b, Matr *res);
error_code bench_report(Matr *a, Matr *b, Matr *res, double lo, double hi, int by_memory);
error_code bench_refine(Matr *a, Matr *b, Matr *res, double lo, double hi, int by_memory,
                        size_t *percent);
error_code bench_measure(Matr *a, Matr *b, Matr *res, size_t nnz, int with_time, BenchResult *out);
double time_standard_ns(const Matr *a, const Matr *b, Matr *res);
double time_compressed_ns(MatrCSC *a, MatrCSC *b, MatrCSC *res);
unsigned long long calc_elapsed_ns(const struct timespec *beg, const struct timespec *end);
size_t csc_memory(const MatrCSC *m);
int csc_is_better(const BenchResult *r, int by_memory);
size_t bench_nnz(size_t total, size_t step);
size_t percent_to_nnz(size_t total, double percent);
void print_bench_header(void);
void print_bench_row(const BenchResult *r);
void print_bench_sep(void);
void print_padded(const char *s, size_t width);
void print_bench_notes(void);

#endif
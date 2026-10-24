#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include <time.h>
#include "errors.h"
#include "config.h"

#define BENCH_COLS 8
#define BENCH_WIDTHS_INIT { 10, 8, 12, 12, 10, 13, 13, 10 }
#define BENCH_SIZES_COUNT 4
#define BENCH_SIZES_INIT { 10, 100, 500, 1000 }
#define BENCH_PERCENTS_COUNT 9
#define BENCH_PERCENTS_INIT { 0.1, 1, 5, 10, 20, 30, 50, 70, 100 }
#define BENCH_TARGET_CELLS 20000000ULL
#define BENCH_PRESET 1
#define BENCH_CUSTOM 2

void print_padded(const char *s, size_t width);
void print_bench_sep(void);
void print_bench_header(void);
size_t csc_memory(const MatrCSC *m);
double elapsed_us(clock_t start, clock_t end, size_t reps);
int results_match(const Matr *dense, size_t dense_nnz, const MatrCSC *csc);
size_t bench_nnz(size_t total, size_t step);
error_code bench_row(Matr *a, Matr *b, Matr *res, size_t nnz, BenchResult *out);
error_code bench_size(size_t rows, size_t cols, Matr *a, Matr *b, Matr *res, int *all_match);
error_code input_bench_mode(int *mode);
void print_bench_notes(int all_match);
error_code run_benchmark(void);

#endif
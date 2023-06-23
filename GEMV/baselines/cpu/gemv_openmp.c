#include <stdlib.h>
#include <stdio.h>
#include "../../support/timer.h"
#include "gemv_utils.h"

#include "./dram_ap.h"



int main(int argc, char *argv[])
{
  // const size_t rows = 20480;
  // const size_t cols = 8192;

  const size_t rows = 30;
  const size_t cols = 40;

  double **A, *b, *x;

  b = (double*) malloc(sizeof(double)*rows);
  x = (double*) malloc(sizeof(double)*cols);

  allocate_dense(rows, cols, &A);

  make_hilbert_mat(rows,cols, &A);

  // printf("input matrix A: \n");
  // print_mat(A, rows, cols);
  // printf(" \n");


#pragma omp parallel
    {
#pragma omp for
    for (size_t i = 0; i < cols; i++) {
      x[i] = (double) i+1 ;
    }

#pragma omp for
    for (size_t i = 0; i < rows; i++) {
      b[i] = (double) 0.0;
    }
    }

  Timer timer;
  start(&timer, 0, 0);

   gemv(A, x, rows, cols, &b);
   
   stop(&timer, 0);


    printf("Kernel ");
    print(&timer, 0, 1);
    printf("\n");

    /* START DRAMAP */

  // int n_ranks = 16;
  // int n_banks = 8;
  // int n_sa = 2;
  // int n_col = 262144;

  // int n_tasklets = n_ranks * n_banks * n_sa
  // int rows_per_tasklet = rows / (n_tasklets); // distribute matrix rows evenly to each RLU vector tasklet

  double *output_row; // host var
  output_row = (double*) malloc(sizeof(double)*rows);

  double *mat_row;
  double *v_row;
  double *res_row;

  dram_ap_valloc(&mat_row, cols);
  dram_ap_valloc(&res_row, cols);
  dram_ap_valloc(&v_row, cols);

  for (size_t r = 0; r < 30; r++) {

    dram_ap_fld(A, mat_row, r, cols);
    dram_ap_vld(x, v_row, cols);

    dram_ap_vmul(res_row, mat_row, v_row, cols);
    double acc = 0.0;
    dram_ap_vredsum(&acc, res_row, cols);
    output_row[r] = acc;

    // printf("acc: %f\n ", acc);
    
  }

  // sanity check
  // for (int i = 0; i < 30; i++) {
  //   printf("output_row[%d]: %f, b[%d]: %f \n", i, output_row[i], i, b[i]);
  // }
  free(mat_row);
  free(v_row);
  free(res_row);
  free(output_row);

  /* END DRAMAP */

#if 0
  print_vec(x, rows);
  print_mat(A, rows, cols);
  print_vec(b, rows);
#endif

  printf("sum(x) = %f, sum(Ax) = %f\n", sum_vec(x,cols), sum_vec(b,rows));
  return 0;
}

void gemv(double** A, double* x, size_t rows, size_t cols, double** b) {
#pragma omp parallel for
  for (size_t i = 0; i < rows; i ++ )
  for (size_t j = 0; j < cols; j ++ ) {
    (*b)[i] = (*b)[i] + A[i][j]*x[j];
  }
}

void make_hilbert_mat(size_t rows, size_t cols, double*** A) {
#pragma omp parallel for
  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      (*A)[i][j] = 1.0/( (double) i + (double) j + 1.0);
    }
  }
}

double sum_vec(double* vec, size_t rows) {
  double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
  for (int i = 0; i < rows; i++) sum = sum + vec[i];
  return sum;
}

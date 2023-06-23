#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void dram_ap_valloc(double **src_v, unsigned long long vl)
{	
	*src_v = malloc(vl * sizeof(double));
	return;
}

void dram_ap_fld(double **mat, double *src_v, int row_i, unsigned long long vl) 
{
	for (unsigned long long i = 0; i < vl; i++) {
		
		src_v[i] = mat[row_i][i];
		// printf("%f ", src_v[i]);
	}
	// printf("\n");
}

void dram_ap_vld(double *src_v, double *dst_v, unsigned long long vl)
{
	for (unsigned long long i = 0; i < vl; i++) {
		dst_v[i] = src_v[i];
	}
}

void dram_ap_vmul(double *dst_v, double *src1_v, double *src2_v, unsigned long long vl) 
{
	for (unsigned long long i = 0; i < vl; i++) {
		dst_v[i] = src1_v[i] * src2_v[i];
		// printf("dst_v[%lld]: %f ", i, dst_v[i]);
	}
	// printf("\n");
}

void dram_ap_vredsum(double *acc, double *src_row, unsigned long long vl) {
	for (unsigned long long i = 0; i < vl; i++) {
		*acc += (double) src_row[i];
	}
}


// void allocate_dense(size_t rows,size_t  cols, double*** dense) {

//   *dense = malloc(sizeof(double)*rows);
//   **dense = malloc(sizeof(double)*rows*cols);

//   for (size_t i=0; i < rows; i++ ) {
//     (*dense)[i] = (*dense)[0] + i*cols;
//   }

// }
/**
* @file app.c
* @brief Template for a Host Application Source File.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <stdint.h>
#include "../../support/timer.h"
#include "../../support/common.h"

#include "./dram_ap.h"

T** A;
T* B;
T* C;

T** _A;
T* _B;
T* _C;

// Create input arrays
static void init_data(T** A, T* B, T** _A, T* _B, unsigned int m_size, unsigned int n_size){
    for (unsigned int l = 0; l < NUM_LAYERS; l++)
		for (unsigned int i = 0; i < m_size * n_size; i++){
			if(i % 100 < 98){
				A[l][i] = 0;
			}else{
				A[l][i] = (l+i) % 2;
			}
      _A[l][i] = A[l][i];
      // printf("%d",A[l][i]);
		}
	for (unsigned int i = 0; i < n_size; i++){
		if(i % 50 < 48){
			B[i] = 0;
		}
		else{
			B[i] = i % 2;
		}
    _B[i] = B[i];
	}
}

// static void copy_data(T** A, T* B, T** _A, T* _B, unsigned int m_size, unsigned int n_size) {
//   for (unsigned int l = 0; l < NUM_LAYERS; l++) {
//     for (unsigned int i = 0; i < m_size * n_size; i++){
//       _A[l][i] = A[l][i];
//     }
//   }
//   for (unsigned int i = 0; i < n_size; i++){
//     _B[i] = B[i];
//   }
// }

// Compute output in the host
static void mlp_host(T* C, T** A, T* B, unsigned int m_size, unsigned int n_size) {
	for (unsigned int nl = 0; nl < NUM_LAYERS; nl++){
		for (unsigned int m = 0; m < m_size; m++){
			C[m] = 0;
		}
		#pragma omp parallel for
		for (unsigned int m = 0; m < m_size; m++){
			for (unsigned int n = 0; n < n_size; n++){
				C[m] += A[nl][m * n_size + n] * B[n];
			}
      // printf("%d ", C[m]);
			C[m] = max(0, C[m]);
      
		}
    // printf("\n");
		for (unsigned int n = 0; n < n_size; n++){
			B[n] = C[n];
		}
	}
}

static void mlp_dramap(T* _C, T** _A, T* _B, unsigned int m_size, unsigned int n_size) {

  for (unsigned int nl = 0; nl < NUM_LAYERS; nl++) {

    for (unsigned int m = 0; m < m_size; m++){
      _C[m] = 0;
    }
    
    T* w_v; // weigths for one output node
    T* in_v; // output of the last layer or input of the cur layer
    unsigned int vl = n_size;
    
    dram_ap_valloc(&w_v, vl);
    dram_ap_valloc(&in_v, vl);

    // #pragma DRAMAP parallel for
    for (unsigned int m = 0; m < m_size; m++) { // nodes in each output layer
      
      T* out_v; // dot-product for one output node
      dram_ap_valloc(&out_v, vl);

      dram_ap_vld(A[nl], w_v, m, vl); // load a weight vector for one output node
      dram_ap_vld(B, in_v, 0, vl); // load input vector from last layer

      dram_ap_vmul(out_v, w_v, in_v, vl);

      T output = 0;
      dram_ap_vredsum(&output, out_v, vl);
      // printf("%d ", output);
      _C[m] = max(0, output);
      

      free(out_v);
    }
    // printf("\n");
    for (unsigned int n = 0; n < n_size; n++){
      _B[n] = _C[n];
    }

    free(w_v);
    free(in_v);
  }

}

static uint64_t mlp_host_sum(uint64_t n_size, uint64_t m_size) {
  uint64_t sum = 0;
  for (uint64_t m = 0; m < n_size; m++){
    sum += B[m];
  }
  return sum;
}

static uint64_t mlp_dramap_sum(uint64_t n_size, uint64_t m_size) {
  uint64_t sum = 0;
  for (uint64_t m = 0; m < n_size; m++){
    sum += _B[m];
  }
  return sum;
}

// Params ---------------------------------------------------------------------
typedef struct Params {
  char* dpu_type;
  int   nr_of_ranks;
  int   input_size_n;
  int   input_size_m;
  int   n_warmup;
  int   n_reps;
}Params;

void usage() {
  fprintf(stderr,
    "\nUsage:  ./program [options]"
    "\n"
    "\nGeneral options:"
    "\n    -h        help"
    "\n    -d <D>    DPU type (default=fsim)"
    "\n    -r <R>    # of ranks (default=2)"
    "\n"
    "\nBenchmark-specific options:"
    "\n    -i <I>    input size (default=8M elements)"
    "\n");
  }

  struct Params input_params(int argc, char **argv) {
    struct Params p;
    p.dpu_type      = "fsim";
    p.nr_of_ranks   = 1;
    p.input_size_n  = 1 << 9;
    p.input_size_m  = 1 << 9;
    p.n_warmup      = 2;
    p.n_reps        = 3;

    int opt;
    while((opt = getopt(argc, argv, "hd:r:i:")) >= 0) {
      switch(opt) {
        case 'h':
        usage();
        exit(0);
        break;
        case 'd': p.dpu_type        = optarg; break;
        case 'r': p.nr_of_ranks     = atoi(optarg); break;
        case 'n': p.input_size_n    = atoi(optarg); break;
        case 'm': p.input_size_m    = atoi(optarg); break;
        default:
        fprintf(stderr, "\nUnrecognized option!\n");
        usage();
        exit(0);
      }
    }
    assert(p.nr_of_ranks > 0 && "Invalid # of ranks!");

    return p;
  }

  /**
  * @brief Main of the Host Application.
  */
  int main(int argc, char **argv) {

    struct Params p = input_params(argc, argv);
    uint64_t n_size = 8192;
    uint64_t m_size = 20480;
    // uint64_t n_size = 30;
    // uint64_t m_size = 30;

    Timer timer;
    A = malloc(NUM_LAYERS * sizeof(T*));
    for(int l = 0; l < NUM_LAYERS; l++)
        A[l] = malloc(n_size*m_size*sizeof(unsigned int));
    B = malloc(m_size*sizeof(unsigned int));
    C = malloc(m_size*sizeof(unsigned int));

    _A = malloc(NUM_LAYERS * sizeof(T*));
    for(int l = 0; l < NUM_LAYERS; l++)
        _A[l] = malloc(n_size*m_size*sizeof(unsigned int));
    _B = malloc(m_size*sizeof(unsigned int));
    _C = malloc(m_size*sizeof(unsigned int));

    // Create an input file with arbitrary data.
    init_data(A, B, _A, _B, m_size, n_size);

    // copy to DRAM_AP
    // copy_data(A, B, _A, _B, m_size, n_size);


    start(&timer, 0, 1);

    mlp_host(C, A, B, n_size, m_size);

    stop(&timer, 0);


    /* START DRAM_AP */
    mlp_dramap(_C, _A, _B, n_size, m_size);
    /* END DRAM_AP */

    
    uint32_t sum = mlp_host_sum(n_size, m_size);
    uint32_t _sum = mlp_dramap_sum(n_size, m_size);
   
    printf("Kernel ");
    print(&timer, 0, 1);
    printf("\n");

    printf("SUM = %d \n", sum);
    printf("_SUM = %d \n", _sum);

    for(int l = 0; l < NUM_LAYERS; l++)
        free(A[l]);
    free(A);
    free(B);
    free(C);

    return 0;
}

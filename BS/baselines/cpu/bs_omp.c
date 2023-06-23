
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include "timer.h"

#include "./dram_ap.h"

#define DTYPE uint64_t
/*
* @brief creates a "test file" by filling a bufferwith values
*/
void create_test_file(DTYPE * input, uint64_t  nr_elements, DTYPE * querys, uint64_t n_querys) {

  uint64_t max = UINT64_MAX;
  uint64_t min = 0;

  srand(time(NULL));

  // printf("inputs: ");
  input[0] = 1;
  // printf("%ld ",input[0]);
  for (uint64_t i = 1; i < nr_elements; i++) {
        input[i] = input[i - 1] + (rand() % 10) + 1;
        // printf("%ld ",input[i]);
  }
  // printf("\n");

  // printf("queries: ");
  for(uint64_t i = 0; i < n_querys; i++)
  {
	querys[i] = input[rand() % (nr_elements - 2)];
	// printf("%ld ",querys[i]);
  }
  // printf("\n");
}

/**
* @brief compute output in the host
*/
uint64_t binarySearch(DTYPE * input, uint64_t input_size, DTYPE* querys, unsigned n_querys)
{

	uint64_t found = -1;
	uint64_t q, r, l, m;
	
       // #pragma omp parallel for private(q,r,l,m)
     	for(q = 0; q < n_querys; q++)
      	{
					l = 0;
					r = input_size;
					while (l <= r) 
					{
				    		m = l + (r - l) / 2;

				    		// Check if x is present at mid
				     		if (input[m] == querys[q])
								{	
									// printf("found: %ld\n",found);
							    found += m;
							    // found += 1;
							    // printf("input[%ld]: %ld, querys[%ld]: %ld, found: %ld\n",m,input[m], q,querys[q], found);
									break;
								}
				    		// If x greater, ignore left half
				    		if (input[m] < querys[q])
						    	l = m + 1;

				    		// If x is smaller, ignore right half
								else
							    r = m - 1;
					
					}
       	}

      	return found;
}

uint64_t dramapSearch(DTYPE * input, uint64_t input_size, DTYPE* querys, unsigned n_querys)
{
	uint64_t res = -1;
	DTYPE *references;


	dram_ap_valloc(&references, input_size);
	dram_ap_vld(input, references, input_size);

	for (uint64_t i = 0; i < n_querys; i++) {

		DTYPE *keys;
		DTYPE *bitmap;
		DTYPE found_i = -1;
		dram_ap_valloc(&keys, input_size);
		dram_ap_valloc(&bitmap, input_size);
		dram_ap_brdcst(querys[i], keys, input_size); // use SEL for rapid input population
		dram_ap_brdcst(0, bitmap, input_size);

		dram_ap_match(bitmap, references, keys, input_size);
		dram_ap_uniIdx(&found_i, bitmap, input_size);
		res += found_i;
		// printf("dramap found at: %ld\n", found_i);

		free(keys);
	}
	free(references);

	return res;

}

  /**
  * @brief Main of the Host Application.
  */
  int main(int argc, char **argv) {

    Timer timer;
    // uint64_t input_size = atol(argv[1]);
    // uint64_t n_querys = atol(argv[2]);

    uint64_t input_size = 2048;
    uint64_t n_querys = 65536;



    printf("Vector size: %lu, num searches: %lu\n", input_size, n_querys);
	
    DTYPE * input = malloc((input_size) * sizeof(DTYPE));
    DTYPE * querys = malloc((n_querys) * sizeof(DTYPE));

    DTYPE result_host = -1;

    // Create an input file with arbitrary data.
    create_test_file(input, input_size, querys, n_querys);
	
    start(&timer, 0, 0);
    result_host = binarySearch(input, input_size - 1, querys, n_querys);   
    printf("result_host: %ld\n",result_host);
    stop(&timer, 0);

    /* start DRAMAP */
    DTYPE result_dramap = -1;
    result_dramap = dramapSearch(input, input_size - 1, querys, n_querys);
    printf("result_dramap: %ld\n",result_dramap);
    /* end DRAMAP */

    int status = (result_host);
    if (status) {
        printf("[OK] Execution time: ");
				print(&timer, 0, 1);
				printf("ms.\n");
    } else {
        printf("[ERROR]\n");
    }
    free(input);
    free(querys);

    return status ? 0 : 1;
}


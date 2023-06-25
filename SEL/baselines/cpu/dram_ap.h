#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
   uint64_t *matrix_A;
   uint64_t *matrix_out;
   int pos;
   uint64_t matrix_len;
} mm_data_t;

typedef struct {
   int *data;
   int size; // matrix_len
} matrix_file_handler;

bool pred(const uint64_t x)
{
  return (x % 2) == 0;
}

void dram_ap_valloc(uint64_t **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = (uint64_t *) malloc(vl * sizeof(uint64_t));
	return;
}

void dram_ap_vcpy(uint64_t *dst_v, uint64_t *src_v, unsigned int vl, unsigned int group_id)
{
	for(int i = vl; i < vl+group_id; i++){
	    dst_v[i] = src_v[i];
	}
	return;
}

void dram_ap_vcmp(uint64_t *dst_v, uint64_t *src1_v, int *cu_p, int vl, int bit_len)
{
	for (int i = vl; i < vl+bit_len; i++) {
		if (!pred(dst_v[i])) {
			src1_v[*cu_p] = dst_v[i];
			*cu_p += 1;
		}
	}
	return;
}
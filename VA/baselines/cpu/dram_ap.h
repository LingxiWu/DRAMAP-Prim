#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
   int32_t *matrix_A;
   int32_t *matrix_B;
   int32_t *matrix_out;
   int32_t matrix_len;
} mm_data_t;

typedef struct {
   int *data;
   int size; // matrix_len
} matrix_file_handler;

void dram_ap_valloc(int **src_v, int group_id, unsigned long long vl, int bit_len)
{	
	*src_v = (uint32_t*) malloc(vl * sizeof(uint32_t));
	return;
}

void dram_ap_vcpy(int32_t *dst_v, int32_t *src_v, unsigned int vl, unsigned int group_id)
{
    //memcpy(dst_v, src_v, sizeof(int32_t)*vl);
	for(int i = vl; i < vl+group_id; i++){
	    dst_v[i] = src_v[i];
	}
}

void dram_ap_vadd(int32_t *dst_v, int32_t *src1_v, int32_t *src2_v, unsigned long long vl, int bit_len)
{
	for (int i = vl; i < vl+bit_len; i++) {
		dst_v[i] = src1_v[i] + src2_v[i];
	}
}

void dram_ap_vredsum(int *scalar, int* src_v, unsigned long long vl, int bit_len)
{
	for (int i = 0; i < vl; i++) {
		*scalar += src_v[i];
	}
}
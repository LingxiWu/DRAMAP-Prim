#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define DTYPE uint64_t

void dram_ap_valloc(DTYPE **src_v, unsigned int vl)
{	
	*src_v = (DTYPE*) malloc(vl * sizeof(DTYPE));
	return;
}

void dram_ap_vld(DTYPE *src_v, DTYPE *dst_v, unsigned int vl)
{
	for (unsigned int i = 0; i < vl; i++) {
		dst_v[i] = src_v[i];
	}
}

void dram_ap_brdcst(DTYPE query, DTYPE *q_v, unsigned int vl) 
{
	for (unsigned int i = 0; i < vl; i++) {
		q_v[i] = query;
	}
}

void dram_ap_match(DTYPE *bitmap, DTYPE *references, DTYPE *keys, DTYPE input_size)
{
	for (unsigned int i = 0; i < input_size; i++) {
		if (references[i] == keys[i]) {
			bitmap[i] = 1;
		}
	}
	
}

void dram_ap_uniIdx(DTYPE *found, DTYPE *bitmap, DTYPE input_size) // return the first matched index (1st 1)
{
	for (unsigned int i = 0; i < input_size; i++) {
		if (bitmap[i] == 1) {
			*found = i;
			return;
		}
	}
}


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void dram_ap_valloc(T **src_v, unsigned int vl)
{	
	*src_v = (T*) malloc(vl * sizeof(T));
	return;
}

void dram_ap_vld(T* src_v, T *dst_v, unsigned int vl)
{
	for (unsigned int i = 0; i < vl; i++) {
		dst_v[i] = src_v[i];
	}
}

void dram_ap_vredsum(T *acc, T *src_row, unsigned int vl) {
	for (unsigned int i = 0; i < vl; i++) {
		*acc += (T) src_row[i];
	}
}
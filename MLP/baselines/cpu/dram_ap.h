#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void dram_ap_valloc(T **src_v, unsigned int vl)
{	
	*src_v = malloc(vl * sizeof(T));
	return;
}

void dram_ap_vld(T* src_v, T *dst_v, T m_index, unsigned int vl) 
{
	for (unsigned int i = 0; i < vl; i++) {
		dst_v[i] = src_v[m_index * vl + i];
	}
}

void dram_ap_vmul(T *dst_v, T *src1_v, T *src2_v, unsigned int vl) 
{
	for (unsigned int i = 0; i < vl; i++) {
		dst_v[i] = src1_v[i] * src2_v[i];
		// printf("dst_v[%d]: %d ", i, dst_v[i]);
	}
	// printf("\n");
}

void dram_ap_vredsum(T *acc, T *src_row, unsigned int vl) {
	for (unsigned int i = 0; i < vl; i++) {
		*acc += (T) src_row[i];
	}
}
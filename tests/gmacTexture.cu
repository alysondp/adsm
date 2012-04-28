#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <gmac.h>
#include <cuda.h>

#include "debug.h"


const int width = 16;
const int height = 16;

texture<unsigned short, 2, cudaReadModeElementType> tex;

__global__ void vecAdd(unsigned short *c, int width, int height)
{
	unsigned x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned y = blockIdx.y * blockDim.y + threadIdx.y;

	c[y * width + x] = tex2D(tex, x, y);
}


int main(int argc, char *argv[])
{
	unsigned short *data, *c;
	struct cudaArray *array;

	data = (unsigned short *)malloc(width * height * sizeof(unsigned short));
	for(int i = 0; i < width * height; i++)
		data[i] = i;
	assert(cudaMallocArray(&array, &tex.channelDesc, width, height) == cudaSuccess);
	cudaMemcpy2DToArray(array, 0, 0, data, width * sizeof(unsigned short),
			width * sizeof(unsigned short), height, cudaMemcpyHostToDevice);

	assert(cudaBindTextureToArray(tex, array) == cudaSuccess);

	// Alloc output data
	if(gmacMalloc((void **)&c, width * height * sizeof(unsigned short)) != gmacSuccess)
		CUFATAL();

	// Call the kernel
	dim3 Db(width, height);
	dim3 Dg(1);
	vecAdd<<<Dg, Db>>>(gmacPtr(c), width, height);
	if(gmacThreadSynchronize() != gmacSuccess) CUFATAL();

	for(int i = 0; i < width * height; i++) {
		if(c[i] == data[i]) continue;
		fprintf(stderr,"Error on %d (0x%x)\n", i, c[i]);
		abort();
	}
	fprintf(stderr,"Done!\n");

	gmacFree(c);

    return 0;
}
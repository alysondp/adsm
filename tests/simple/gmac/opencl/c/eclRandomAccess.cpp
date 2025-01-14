#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gmac/opencl.h>

#include "utils.h"
#include "debug.h"

const char *vecSizeStr = "GMAC_VECSIZE";
const unsigned vecSizeDefault = 16 * 1024 * 1024;
unsigned vecSize = vecSizeDefault;

const char *operationsStr = "GMAC_OPERATIONS";
const unsigned operationsDefault = 256 * 1024;
unsigned operations = operationsDefault;

const char *msg = "Done!";

const char *kernel = "\
					 __kernel void null(__global float *a)\
					 {\
					 }\
					 ";

int main(int argc, char *argv[])
{
	float *a;
	gmactime_t s, t;
	ecl_error ret;

	assert(eclCompileSource(kernel) == eclSuccess);

	setParam<unsigned>(&vecSize, vecSizeStr, vecSizeDefault);
	fprintf(stdout, "Vector: %f\n", 1.0 * vecSize / 1024 / 1024);

	getTime(&s);
	// Alloc data
	ret = eclMalloc((void **)&a, vecSize * sizeof(float));
	assert(ret == eclSuccess);

	getTime(&t);
	printTime(&s, &t, "Alloc: ", "\n");

	// Init input data
	getTime(&s);
	eclMemset(a, 0, vecSize * sizeof(float));

	for(unsigned i = 0; i < operations; i++) {
		double rnd = (double(rand()) / RAND_MAX);
		unsigned pos = unsigned(rnd * (vecSize - 1));
		a[pos]++;
	}

	getTime(&t);
	printTime(&s, &t, "Init: ", "\n");

	// Call the kernel
	getTime(&s);
	size_t globalSize = vecSize;
	ecl_kernel kernel;
	ret = eclGetKernel("null", &kernel);
	assert(ret == eclSuccess);
#ifndef __GXX_EXPERIMENTAL_CXX0X__
	ret = eclSetKernelArgPtr(kernel, 0, a);
	assert(ret == eclSuccess);
	ret = eclCallNDRange(kernel, 1, NULL, &globalSize, NULL);
	assert(ret == eclSuccess);
#else
	assert(kernel(globalSize)(a) == eclSuccess);
#endif

	getTime(&t);
	printTime(&s, &t, "Run: ", "\n");

	getTime(&s);
	float sum = 0.f;
	for(unsigned i = 0; i < vecSize; i++) {
		sum += a[i];
	}
	getTime(&t);
	printTime(&s, &t, "Check: ", "\n");
	fprintf(stderr, "Error: %f\n", sum - float(operations));

	eclFree(a);

	return sum != float(operations);
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>

#include "gmac/cl.h"

#include "utils.h"
#include "debug.h"


const char *vecSizeStr = "GMAC_VECSIZE";
const unsigned vecSizeDefault = 16 * 1024 * 1024;
unsigned vecSize = vecSizeDefault;

const char *msg = "Done!";

const char *kernel_source = "\
__kernel void vecAdd(__global float *c, __global const float *a, __global const float *b, unsigned size)\
{\
    unsigned i = get_global_id(0);\
    if(i >= size) return;\
\
    c[i] = a[i] + b[i];\
}\
";


int main(int argc, char *argv[])
{
	cl_helper helper;
	size_t platforms;
	cl_program program;
	cl_int error_code;
	cl_kernel kernel;
	const char *kernel_file ="d:\\kernel.txt";

	if (clInitHelpers(&platforms)!=CL_SUCCESS) return -1;
	if (platforms == 0) return -1;
	helper = clGetHelpers()[0];
	program = clHelperLoadProgramFromFile(helper, kernel_file, &error_code);
	if (error_code != CL_SUCCESS){
		clReleaseHelpers();
		return -1;
	}

	float *a, *b, *c;
	gmactime_t s, t;

	setParam<unsigned>(&vecSize, vecSizeStr, vecSizeDefault);
	fprintf(stdout, "Vector: %f\n", 1.0 * vecSize / 1024 / 1024);

	kernel = clCreateKernel(program, "vecAdd", &error_code);
    assert(error_code == CL_SUCCESS);

    getTime(&s);
    // Alloc & init input data
	assert(clMalloc(helper.command_queues[0], (void **)&a, vecSize * sizeof(float)) == CL_SUCCESS);
    assert(clMalloc(helper.command_queues[0], (void **)&b, vecSize * sizeof(float)) == CL_SUCCESS);
    // Alloc output data
    assert(clMalloc(helper.command_queues[0], (void **)&c, vecSize * sizeof(float)) == CL_SUCCESS);
    getTime(&t);
    printTime(&s, &t, "Alloc: ", "\n");


    float sum = 0.f;

    getTime(&s);
    valueInit(a, 1.f, vecSize);
    valueInit(b, 1.f, vecSize);
    getTime(&t);
    printTime(&s, &t, "Init: ", "\n");

    for(unsigned i = 0; i < vecSize; i++) {
        sum += a[i] + b[i];
    }
    

    // Call the kernel
    getTime(&s);
    size_t global_size = vecSize;
	cl_mem mem;

    mem = clGetBuffer(helper.contexts[0], c);
    error_code = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem);
	if (mem == NULL || error_code != CL_SUCCESS)
		return error_code;
    mem = clGetBuffer(helper.contexts[0], a);
    error_code = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem);
	if (mem == NULL || error_code != CL_SUCCESS)
		return error_code;
    mem = clGetBuffer(helper.contexts[0], b);
    error_code = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem);
	if (mem == NULL || error_code != CL_SUCCESS)
		return error_code;
    assert(clSetKernelArg(kernel, 3, sizeof(vecSize), &vecSize) == CL_SUCCESS);

    assert(clEnqueueNDRangeKernel(helper.command_queues[0], kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL) == CL_SUCCESS);
	assert(clFinish(helper.command_queues[0]) == CL_SUCCESS);

    getTime(&t);
    printTime(&s, &t, "Run: ", "\n");


    getTime(&s);
    float error = 0.f;
    float check = 0.f;
    for(unsigned i = 0; i < vecSize; i++) {
        error += c[i] - (a[i] + b[i]);
        check += c[i];
    }
    getTime(&t);
    printTime(&s, &t, "Check: ", "\n");
    fprintf(stderr, "Error: %f\n", error);

    if (sum != check) {
        printf("Sum: %f vs %f\n", sum, check);
        abort();
    }

    clFree(helper.command_queues[0], a);
    clFree(helper.command_queues[0], b);
    clFree(helper.command_queues[0], c);
	clReleaseHelpers();
    return 0;
}

#include <stdio.h>
#include <cstring>
#include <gmac/opencl.h>

const size_t size = 4 * 1024 * 1024;
const size_t blockSize = 32;

const char *kernel = "\
__kernel void reset(__global long *a, unsigned long size, long v)\
{\
    unsigned i = get_global_id(0);\
    if(i >= size) return;\
\
	a[i] += v;\
}\
";

int check(long *ptr, int s)
{
	int a = 0;
	for(unsigned i = 0; i < size; i++)
		a += ptr[i];
	return a - s;
}

int main(int argc, char *argv[])
{
	long *ptr;

    assert(oclCompileSource(kernel) == oclSuccess);

	assert(oclMalloc((void **)&ptr, size * sizeof(long)) == oclSuccess);

	// Call the kernel
    size_t localSize = blockSize;
    size_t globalSize = size / blockSize;
    if(size % blockSize) globalSize++;
    globalSize *= localSize;
    cl_mem tmp = cl_mem(oclPtr(ptr));
    long val = 1;

    oclMemset(ptr, 0, size * sizeof(long));

    ocl_kernel kernel;

    assert(oclGetKernel("reset", &kernel) == oclSuccess);

    assert(oclSetKernelArg(kernel, 0, sizeof(cl_mem), &tmp) == oclSuccess);
    assert(oclSetKernelArg(kernel, 1, sizeof(size), &size) == oclSuccess);
    assert(oclSetKernelArg(kernel, 2, sizeof(val), &val) == oclSuccess);
    assert(oclCallNDRange(kernel, 1, NULL, &globalSize, &localSize) == oclSuccess);

	fprintf(stderr,"%d\n", check(ptr, size));

	fprintf(stderr, "Test partial memset: ");
	oclMemset(&ptr[size / 8], 0, 3 * size / 4 * sizeof(long));
	fprintf(stderr,"%d\n", check(ptr, size / 4));

	fprintf(stderr,"Test full memset: ");
    memset(ptr, 0, size * sizeof(long));

    assert(oclCallNDRange(kernel, 1, NULL, &globalSize, &localSize) == oclSuccess);

	fprintf(stderr,"%d\n", check(ptr, size));

	fprintf(stderr, "Test partial memset: ");
	memset(&ptr[size / 8], 0, 3 * size / 4 * sizeof(long));
	fprintf(stderr,"%d\n", check(ptr, size / 4));

    oclReleaseKernel(kernel);

	oclFree(ptr);

    return 0;
}
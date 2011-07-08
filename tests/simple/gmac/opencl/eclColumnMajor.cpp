#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gmac/opencl>

#include "utils.h"
#include "debug.h"

const char *xSizeStr = "GMAC_XSIZE";
const unsigned xSizeDefault = 1024;
unsigned xSize = xSizeDefault;

const char *ySizeStr = "GMAC_YSIZE";
const unsigned ySizeDefault = 128;
unsigned ySize = ySizeDefault;

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

    assert(ecl::compileSource(kernel) == eclSuccess);

    setParam<unsigned>(&xSize, xSizeStr, xSizeDefault);
    setParam<unsigned>(&ySize, ySizeStr, ySizeDefault);

    fprintf(stdout, "Matrix: %u x %u\n", xSize , ySize);

    getTime(&s);
    // Alloc data
    a = new (ecl::allocator) float[xSize * ySize];

    assert(a != NULL);

    getTime(&t);
    printTime(&s, &t, "Alloc: ", "\n");

    // Init input data
    getTime(&s);
    ecl::memset(a, 0, xSize * ySize * sizeof(float));

    for (unsigned j = 0; j < xSize; j++) {
        for (unsigned i = 0; i < ySize; i++) {
            a[j + i * xSize] = 1.f;
        }
    }

    getTime(&t);
    printTime(&s, &t, "Init: ", "\n");

    // Call the kernel
    getTime(&s);
    ecl::config globalSize(xSize * ySize);

    ecl::error err;
    ecl::kernel kernel("null", err);
    assert(err == eclSuccess);
#ifndef __GXX_EXPERIMENTAL_CXX0X__
    assert(kernel.setArg(0, a) == eclSuccess);
    assert(kernel.callNDRange(globalSize) == eclSuccess);
#else
    assert(kernel(globalSize)(a) == eclSuccess);
#endif

    getTime(&t);
    printTime(&s, &t, "Run: ", "\n");

    getTime(&s);
    float sum = 0.f;
    for (unsigned i = 0; i < ySize; i++) {
        for (unsigned j = 0; j < xSize; j++) {
            sum += a[j + i * xSize];
        }
    }
    getTime(&t);
    printTime(&s, &t, "Check: ", "\n");
    fprintf(stderr, "Error: %f\n", sum - float(xSize * ySize));

    ecl::free(a);

    return sum != float(xSize * ySize);
}

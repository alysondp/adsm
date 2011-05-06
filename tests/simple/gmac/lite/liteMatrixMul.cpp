#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <gmac/lite.h>

#include "debug.h"
#include "utils.h"

#include "liteMatrixMulKernel.cl"

#define BLOCK_SIZE 8

const char * WAStr = "GMAC_WA";
const char * HAStr = "GMAC_HA";
const char * WBStr = "GMAC_WB";
const char * HBStr = "GMAC_HB";
const char * checkStr = "GMAC_CHECK";

const unsigned WADefault = (32 * BLOCK_SIZE); // Matrix A width
const unsigned HADefault = (32 * BLOCK_SIZE); // Matrix A height
const unsigned WBDefault = (32 * BLOCK_SIZE); // Matrix B width
const unsigned HBDefault = (32 * BLOCK_SIZE); // Matrix B height
const int checkDefault = false; // Matrix B height

static unsigned WA = 0; // Matrix A width
static unsigned HA = 0; // Matrix A height
static unsigned WB = 0; // Matrix B width
static unsigned HB = 0; // Matrix B height
static bool check = checkDefault; // Matrix B height

#define WC WB  // Matrix C width 
#define HC HA  // Matrix C height

static float * A, * B, * C;
struct param {
	int i;
	float * ptr;
};

unsigned elemsC;
unsigned sizeC;

void
computeGold(float* C, const float* A, const float* B, unsigned int hA, unsigned int wA, unsigned int wB)
{
    for (unsigned int i = 0; i < hA; ++i)
        for (unsigned int j = 0; j < wB; ++j) {
            double sum = 0;
            for (unsigned int k = 0; k < wA; ++k) {
                double a = A[i * wA + k];
                double b = B[k * wB + j];
                sum += a * b;
            }
            C[i * wB + j] = (float)sum;
        }
}

int
main(int argc, char** argv)
{
    cl_platform_id platform;
    cl_device_id device;
    cl_int error_code;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;

    error_code = clGetPlatformIDs(1, &platform, NULL);
    assert(error_code == CL_SUCCESS);
    error_code = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    assert(error_code == CL_SUCCESS);
    context = clCreateContext(0, 1, &device, NULL, NULL, &error_code);
    assert(error_code == CL_SUCCESS);
    command_queue = clCreateCommandQueue(context, device, 0, &error_code);
    assert(error_code == CL_SUCCESS);
    program = clCreateProgramWithSource(context, 1, &code, NULL, &error_code);
    assert(error_code == CL_SUCCESS);
    error_code = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    assert(error_code == CL_SUCCESS);
    kernel = clCreateKernel(program, "matrixMulSimple", &error_code);
    assert(error_code == CL_SUCCESS);

	setParam<unsigned>(&WA, WAStr, WADefault);
	setParam<unsigned>(&HA, HAStr, HADefault);
	setParam<unsigned>(&WB, WBStr, WBDefault);
	setParam<unsigned>(&HB, HBStr, HBDefault);
	setParam<bool>(&check, checkStr, checkDefault);

    assert(HB == WA);

    gmactime_t s, t;
    unsigned elemsA = WA * HA;
    unsigned elemsB = WB * HB;
             elemsC = WC * HC;
    unsigned sizeA = sizeof(float) * elemsA;
    unsigned sizeB = sizeof(float) * elemsB;
             sizeC = sizeof(float) * elemsC;

    // allocate memory for matrices A and B
	getTime(&s);
    assert(clMalloc(context, (void**) &A, sizeA) == CL_SUCCESS);
    assert(clMalloc(context, (void**) &B, sizeB) == CL_SUCCESS);
    assert(clMalloc(context, (void**) &C, sizeC) == CL_SUCCESS);
	getTime(&t);
	printTime(&s, &t, "Alloc: ", "\n");


	getTime(&s);
    valueInit(A, 100.f, elemsA);
    valueInit(B, 100.f, elemsB);
	getTime(&t);
	printTime(&s, &t, "Init: ", "\n");

	getTime(&s);
    size_t localSize[2] = { BLOCK_SIZE, BLOCK_SIZE };
    size_t globalSize[2];
    globalSize[0] = WC;
    globalSize[1] = HC;

    cl_mem tmp = clBuffer(context, C);
    assert(clSetKernelArg(kernel, 0, sizeof(cl_mem), &tmp) == CL_SUCCESS);
    tmp = clBuffer(context, A);
    assert(clSetKernelArg(kernel, 1, sizeof(cl_mem), &tmp) == CL_SUCCESS);
    tmp = clBuffer(context, B);
    assert(clSetKernelArg(kernel, 2, sizeof(cl_mem), &tmp) == CL_SUCCESS);
    int param = int(WA);
    assert(clSetKernelArg(kernel, 3, sizeof(int), &param) == CL_SUCCESS);
    param     = int(WB);
    assert(clSetKernelArg(kernel, 4, sizeof(int), &param) == CL_SUCCESS);

    assert(clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL) == CL_SUCCESS);
    assert(clFinish(command_queue) == CL_SUCCESS);

    getTime(&t);
    printTime(&s, &t, "Run: ", "\n");

    // compute reference solution
    getTime(&s);
    float err = 0.0;
    float* reference = (float *) malloc(sizeC);
    computeGold(reference, A, B, HA, WA, WB);
    for (unsigned i = 0; i < elemsC; i++) {
        err += fabsf(reference[i] - C[i]);
    }
    getTime(&t);
    printTime(&s, &t, "Check: ", "\n");

    free(reference);

    getTime(&s);
	clFree(context, A);
	clFree(context, B);
	clFree(context, C);
    getTime(&t);
    printTime(&s, &t, "Free: ", "\n");

    fprintf(stderr, "Error: %f\n", err);

    return fabsf(err) != 0.0f;
}
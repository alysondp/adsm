#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gmac/cuda.h>

#include "utils.h"
#include "barrier.h"
#include "debug.h"

#ifdef _MSC_VER
#define VECTORA "inputset\\vectorA"
#define VECTORB "inputset\\vectorB"
#define VECTORC "inputset\\vectorC"
#else
#define VECTORA "inputset/vectorA"
#define VECTORB "inputset/vectorB"
#define VECTORC "inputset/vectorC"
#endif

const size_t vecSize = 1024 * 1024;
const size_t blockSize = 512;

const char *msg = "Done!";

__global__ void vecSet(float *_a, size_t size, float val)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if(i >= size) return;

    _a[i] = val;
}

__global__ void vecAccum(float *_b, const float *_a, size_t size)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if(i >= size) return;

    _b[i] += _a[i];
}

__global__ void vecMove(float *_a, const float *_b, size_t size)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if(i >= size) return;

    _a[i] = _b[i];
}

#define ITERATIONS 250

barrier_t ioAfter;
barrier_t ioBefore;

static void
writeFile(float *v, unsigned nmemb, int it);
static void
readFile(float *v, unsigned nmemb, int it);

float *a, *b, *c;

float error_compute, error_io;

void *doTest(void *)
{
    dim3 Db(blockSize);
    dim3 Dg(vecSize / blockSize);
    gmactime_t s, t;

    // Alloc & init input data
    getTime(&s);
    if(gmacMalloc((void **)&a, vecSize * sizeof(float)) != gmacSuccess)
        CUFATAL();
    if(gmacMalloc((void **)&b, vecSize * sizeof(float)) != gmacSuccess)
        CUFATAL();
    if(gmacMalloc((void **)&c, vecSize * sizeof(float)) != gmacSuccess)
        CUFATAL();
    getTime(&t);
    printTime(&s, &t, "Alloc: ", "\n");

    getTime(&s);
    gmacMemset(a, 0, vecSize * sizeof(float));
    gmacMemset(b, 0, vecSize * sizeof(float));
    getTime(&t);
    printTime(&s, &t, "Memset: ", "\n");

    barrier_wait(&ioBefore);

    if(vecSize % blockSize) Dg.x++;

    for (int i = 0; i < ITERATIONS; i++) {
        getTime(&s);
        vecSet<<<Dg, Db>>>(gmacPtr(a), vecSize, float(i));
        getTime(&t);
        printTime(&s, &t, "Run: ", "\n");
        barrier_wait(&ioAfter);

        getTime(&s);
        vecMove<<<Dg, Db>>>(gmacPtr(c), gmacPtr(a), vecSize);
        gmacThreadSynchronize();
        getTime(&t);
        printTime(&s, &t, "Run: ", "\n");
        barrier_wait(&ioBefore);
    }

    barrier_wait(&ioBefore);

    for (int i = ITERATIONS - 1; i >= 0; i--) {
        barrier_wait(&ioBefore);
        barrier_wait(&ioAfter);
        getTime(&s);
        vecAccum<<<Dg, Db>>>(gmacPtr(b), gmacPtr(a), vecSize);
        gmacThreadSynchronize();
        getTime(&t);
        printTime(&s, &t, "Run: ", "\n");
    }


    error_compute = 0.f;
    getTime(&s);
    for(unsigned i = 0; i < vecSize; i++) {
        error_compute += b[i] - (ITERATIONS - 1)*(ITERATIONS / 2);
    }
    fprintf(stderr,"Error: %f\n", error_compute);
    getTime(&t);
    printTime(&s, &t, "Check: ", "\n");

    getTime(&s);
    gmacFree(a);
    gmacFree(b);
    getTime(&t);
    printTime(&s, &t, "Free: ", "\n");

    return &error_compute;
}

static void
setPath(char *name, size_t len, int it)
{
    static const char path_base[] = "_gmac_file_";
    memset(name, '\0', len);
    sprintf(name, "%s%d", path_base, it);
}

static void
writeFile(float *v, unsigned nmemb, int it)
{
    char path[256];
    setPath(path, 256, it);
    gmactime_t s, t;

    getTime(&s);
    FILE * f = fopen(path, "wb");
    assert(f != NULL);
    assert(fwrite(v, sizeof(float), nmemb, f) == nmemb);
    fclose(f);
    getTime(&t);
    printTime(&s, &t, "Write: ", "\n");
}

static void
readFile(float *v, unsigned nmemb, int it)
{
    char path[256];
    setPath(path, 256, it);
    gmactime_t s, t;

    getTime(&s);
    FILE * f = fopen(path, "rb");
    assert(f != NULL);
    assert(fread(v, sizeof(float), nmemb, f) == nmemb);
    fclose(f);
    getTime(&t);
    printTime(&s, &t, "Read: ", "\n");
}

void *doTestIO(void *)
{
    error_io = 0.0f;
    barrier_wait(&ioBefore);

    for (int i = 0; i < ITERATIONS; i++) {
        barrier_wait(&ioAfter);
        barrier_wait(&ioBefore);
        writeFile(c, vecSize, i);
    }

    barrier_wait(&ioBefore);

    for (int i = ITERATIONS - 1; i >= 0; i--) {
        barrier_wait(&ioBefore);
        readFile(a, vecSize, i);
        barrier_wait(&ioAfter);
    }

    return &error_io;
}

int main(int argc, char *argv[])
{
    thread_t tid, tidIO;

    barrier_init(&ioAfter,2);
    barrier_init(&ioBefore, 2);

    tid = thread_create(doTest, NULL);
    tidIO = thread_create(doTestIO, NULL);

    thread_wait(tid);
    thread_wait(tidIO);

    barrier_destroy(&ioAfter);
    barrier_destroy(&ioBefore);

    return error_io != 0.f;
}

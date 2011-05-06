#ifndef GMAC_STENCIL_COMMON_H_
#define GMAC_STENCIL_COMMON_H_

#include "gmac/opencl.h"

#define STENCIL 4

static const char *stencilCode = "\
\
#define STENCIL 4 \n\
\
__constant \
float devC00; \n\
__constant \
float devZ1; \n\
__constant \
float devZ2; \n\
__constant \
float devZ3; \n\
__constant \
float devZ4; \n\
__constant \
float devX1; \n\
__constant \
float devX2; \n\
__constant \
float devX3; \n\
__constant \
float devX4; \n\
__constant \
float devY1; \n\
__constant \
float devY2; \n\
__constant \
float devY3; \n\
__constant \
float devY4; \n\
\
__kernel \
void \
kernelStencil(__global const float * u2,\
              __global float * u3,\
              __global const float * v,\
              float dt2,\
              unsigned dimZ,\
              unsigned dimRealZ,\
              unsigned dimZX,\
              unsigned dimRealZX,\
              unsigned slices)\
{\n\
    __local float s_data[8 + 2 * STENCIL][32 + 2 * STENCIL]; \n\
\
    unsigned ix = get_global_id(0); \n\
    unsigned iy = get_global_id(1); \n\
\
    unsigned tx = get_local_id(0) + STENCIL; \n\
    unsigned ty = get_local_id(1) + STENCIL; \n\
\
    int index     = (iy * dimZ     + ix) + 3 * dimZX; \n\
    int realIndex = (iy * dimRealZ + ix); \n\
\
    unsigned TILE_OFFSET_LINE = 32 + 2 * STENCIL; \n\
\
    float4 front; \n\
    float4 back; \n\
\
    float current; \n\
\
    back.z  = u2[index - 3 * dimZX]; \n\
    back.y  = u2[index - 2 * dimZX]; \n\
    back.x  = u2[index - 1 * dimZX]; \n\
    current = u2[index            ]; \n\
    front.w = u2[index + 1 * dimZX]; \n\
    front.z = u2[index + 2 * dimZX]; \n\
    front.y = u2[index + 3 * dimZX]; \n\
    front.x = u2[index + 4 * dimZX]; \n\
\
    for (int k = 0; k < (slices - 2 * STENCIL); k++) { \n\
        float tmpU2 = u2[index + ((STENCIL + 1) * dimZX)]; \n\
        index += dimZX; \n\
\
        back.z = back.y; \n\
        back.y = back.x; \n\
        back.x = current; \n\
        current = front.w; \n\
        front.w = front.z; \n\
        front.z = front.y; \n\
        front.y = front.x; \n\
        front.x = tmpU2; \n\
\n\
        barrier(CLK_LOCAL_MEM_FENCE); \n\
\n\
        if (get_local_id(0) < STENCIL) { \n\
            s_data[ty][get_local_id(0)] = u2[index - STENCIL]; \n\
            s_data[ty][tx + 32] = u2[index + 32]; \n\
        }\n\
\
        if (get_local_id(1) < STENCIL) { \n\
            s_data[get_local_id(1)][tx] = u2[index - STENCIL * dimZ]; \n\
            s_data[ty + 8][tx]             = u2[index + 8 * dimZ]; \n\
        }\n\
\n\
        s_data[ty][tx] = current; \n\
        barrier(CLK_LOCAL_MEM_FENCE); \n\
        float tmp  = v[realIndex]; \n\
        float tmp1 = u3[index]; \n\
\n\
        float div  = \
               devX4 * (s_data[ty - 4][tx] + s_data[ty + 4][tx]); \n\
        div += devC00 * current; \n\
        div += devX3 * (s_data[ty - 3][tx] + s_data[ty + 3][tx]); \n\
        div += devX2 * (s_data[ty - 2][tx] + s_data[ty + 2][tx]); \n\
        div += devX1 * (s_data[ty - 1][tx] + s_data[ty + 1][tx]); \n\
        div += devY4 * (front.x + back.w); \n\
        div += devZ4 * (s_data[ty][tx - 4] + s_data[ty][tx + 4]); \n\
        div += devY3 * (front.y + back.z); \n\
        div += devZ3 * (s_data[ty][tx - 3] + s_data[ty][tx + 3]); \n\
        div += devY2 * (front.z + back.y); \n\
        div += devZ2 * (s_data[ty][tx - 2] + s_data[ty][tx + 2]); \n\
        div += devY1 * (front.w + back.x); \n\
        div += devZ1 * (s_data[ty][tx - 1] + s_data[ty][tx + 1]); \n\
\n\
        div = tmp * tmp * div; \n\
        div = dt2 * div + current + current - tmp1; \n\
        u3[index] = div; \n\
\n\
        realIndex += dimRealZX; \n\
    } \n\
}";

#define VELOCITY 2000

barrier_t barrier;

struct JobDescriptor {
    const static int DEFAULT_DIM = 32;
    int gpus;
    int gpuId;

    struct JobDescriptor * prev;
    struct JobDescriptor * next;

    float * u3;
    float * u2;

    unsigned dimRealElems;
    unsigned dimElems;
    unsigned slices;

    unsigned sliceElems()
    {
        return dimElems * dimElems;
    }

    unsigned sliceRealElems()
    {
        return dimRealElems * dimRealElems;
    }

    unsigned elems()
    {
        return dimElems * dimElems * (slices + 2 * STENCIL);
    }

    unsigned realElems()
    {
        return dimRealElems * dimRealElems * slices;
    }

    unsigned size()
    {
        return dimElems * dimElems * (slices + 2 * STENCIL) * sizeof(float);
    }

    unsigned realSize()
    {
        return dimRealElems * dimRealElems * slices * sizeof(float);
    }


};

#define ITERATIONS 50

void *
do_stencil(void * ptr)
{
    JobDescriptor * descr = (JobDescriptor *) ptr;

	float * v = NULL;
	gmactime_t s, t;

	getTime(&s);

	// Alloc 3 volumes for 2-degree time integration
	if(gmacMalloc((void **)&descr->u2, descr->size()) != gmacSuccess)
		CUFATAL();
	gmacMemset(descr->u2, 0, descr->size());
	if(gmacMalloc((void **)&descr->u3, descr->size()) != gmacSuccess)
		CUFATAL();
    gmacMemset(descr->u3, 0, descr->size());

    if(gmacMalloc((void **) &v, descr->realSize()) != gmacSuccess)
		CUFATAL();

    for (size_t k = 0; k < descr->slices; k++) {        
        for (size_t j = 0; j < descr->dimRealElems; j++) {        
            for (size_t i = 0; i < descr->dimRealElems; i++) {        
                size_t iter = k * descr->sliceRealElems() + j * descr->dimRealElems + i;
                v[iter] = VELOCITY;
            }
        }
    }

    if (descr->gpus > 1) {
        barrier_wait(&barrier);
    }

	getTime(&t);
	printTime(&s, &t, "Alloc: ", "\n");

    size_t localSize[2] = {32, 8};
    size_t globalSize[2];
    globalSize[0] = descr->dimElems - 2 * STENCIL;
    globalSize[1] = descr->dimElems - 2 * STENCIL;

	getTime(&s);

    ocl_kernel kernel;
    
    assert(__oclKernelGet("kernelStencil", &kernel) == gmacSuccess);
    assert(__oclKernelConfigure(&kernel, 2, NULL, globalSize, localSize) == gmacSuccess);
    cl_mem tmpMem = cl_mem(oclPtr(v));
    tmpMem = cl_mem(oclPtr(v));
    assert(__oclKernelSetArg(&kernel, &tmpMem, sizeof(cl_mem), 2) == gmacSuccess);
    float dt2 = 0.08f;
    assert(__oclKernelSetArg(&kernel, &dt2, sizeof(dt2), 3) == gmacSuccess);
    assert(__oclKernelSetArg(&kernel, &descr->dimElems,     sizeof(descr->dimElems    ), 4) == gmacSuccess);
    assert(__oclKernelSetArg(&kernel, &descr->dimRealElems, sizeof(descr->dimRealElems), 5) == gmacSuccess);
    unsigned intTmp = descr->sliceElems();
    assert(__oclKernelSetArg(&kernel, &intTmp, sizeof(intTmp), 6) == gmacSuccess);
    intTmp = descr->sliceRealElems();
    assert(__oclKernelSetArg(&kernel, &intTmp, sizeof(intTmp), 7) == gmacSuccess);
    assert(__oclKernelSetArg(&kernel, &descr->slices, sizeof(descr->slices), 8) == gmacSuccess);


    for (uint32_t i = 1; i <= ITERATIONS; i++) {
        float * tmp;
        
        // Call the kernel
        tmpMem = cl_mem(oclPtr(descr->u2));
        assert(__oclKernelSetArg(&kernel, &tmpMem, sizeof(cl_mem), 0) == gmacSuccess);
        tmpMem = cl_mem(oclPtr(descr->u3));
        assert(__oclKernelSetArg(&kernel, &tmpMem, sizeof(cl_mem), 1) == gmacSuccess);
        
        oclError_t ret;
        ret = __oclKernelLaunch(&kernel);
        assert(ret == gmacSuccess);

        if(descr->gpus > 1) {
            barrier_wait(&barrier);

            // Send data
            if (descr->prev != NULL) {
                gmacMemcpy(descr->prev->u3 + descr->elems() - STENCIL * descr->sliceElems(),
                           descr->u3 + STENCIL * descr->sliceElems(),
                           descr->sliceElems() * STENCIL * sizeof(float));
            }
            if (descr->next != NULL) {
                gmacMemcpy(descr->next->u3,
                           descr->u3 + descr->elems() - 2 * STENCIL * descr->sliceElems(),
                           descr->sliceElems() * STENCIL * sizeof(float));                
            }

            barrier_wait(&barrier);
        }

        tmp = descr->u3;
        descr->u3 = descr->u2;
        descr->u2 = tmp;
    }

    assert(oclThreadSynchronize() == gmacSuccess);
    if(descr->gpus > 1) {
        barrier_wait(&barrier);
    }

	getTime(&t);
	printTime(&s, &t, "Run: ", "\n");

    getTime(&s);
	gmacFree(descr->u2);
	gmacFree(descr->u3);
	gmacFree(v);
    getTime(&t);
    printTime(&s, &t, "Free: ", "\n");

    return NULL;
}

const char * dimRealElemsStr = "GMAC_STENCIL_DIM_ELEMS";
const unsigned dimRealElemsDefault = 352;

static unsigned dimElems     = 0;
static unsigned dimRealElems = 0;

#endif

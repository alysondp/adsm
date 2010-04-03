#include "Accelerator.h"
#include "Context.h"

#include "kernel/Process.h"

#include <debug.h>


namespace gmac {
namespace gpu {

Accelerator::Accelerator(int n, CUdevice device) :
	gmac::Accelerator(n), _device(device)
#ifndef USE_MULTI_CONTEXT
    , mutex(paraver::LockCtxLocal)
#endif
{
    unsigned int size = 0;
    CUresult ret = cuDeviceTotalMem(&size, _device);
    CFATAL(ret == CUDA_SUCCESS, "Unable to initialize CUDA %d", ret);
    ret = cuDeviceComputeCapability(&major, &minor, _device);
    CFATAL(ret == CUDA_SUCCESS, "Unable to initialize CUDA %d", ret);
    _memory = size;
    int async = 0;
    ret = cuDeviceGetAttribute(&async, CU_DEVICE_ATTRIBUTE_GPU_OVERLAP, _device);
    CFATAL(ret == CUDA_SUCCESS, "Unable to initialize CUDA %d", ret);
    _async = bool(async);


#ifndef USE_MULTI_CONTEXT
    CUcontext tmp;
    unsigned int flags = 0;
    if(major > 0 && minor > 0) flags |= CU_CTX_MAP_HOST;
    ret = cuCtxCreate(&_ctx, flags, _device);
    CFATAL(ret == CUDA_SUCCESS, "Unable to create CUDA context %d", ret);
    ret = cuCtxPopCurrent(&tmp);
    CFATAL(ret == CUDA_SUCCESS, "Error setting up a new context %d", ret);
#endif
}

Accelerator::~Accelerator()
{}

gmac::Context *Accelerator::create()
{
	TRACE("Attaching context to Accelerator");
	gpu::Context *ctx = new gpu::Context(*this);
	queue.insert(ctx);
	return ctx;
}


void Accelerator::destroy(gmac::Context *context)
{
	TRACE("Destroying Context");
	if(context == NULL) return;
	gpu::Context *ctx = dynamic_cast<gpu::Context *>(context);
	std::set<gpu::Context *>::iterator c = queue.find(ctx);
	ASSERT(c != queue.end());
	//delete ctx;
	queue.erase(c);
}

#ifdef USE_MULTI_CONTEXT
CUcontext
Accelerator::createCUDAContext()
{
    CUcontext ctx, tmp;
    unsigned int flags = 0;
    if(major > 0 && minor > 0) flags |= CU_CTX_MAP_HOST;
    CUresult ret = cuCtxCreate(&ctx, flags, _device);
    if(ret != CUDA_SUCCESS)
        FATAL("Unable to create CUDA context %d", ret);
    ret = cuCtxPopCurrent(&tmp);
    ASSERT(ret == CUDA_SUCCESS);
    return ctx;
}
#endif

}}

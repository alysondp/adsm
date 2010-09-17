#include "Accelerator.h"
#include "Mode.h"

#include <kernel/Process.h>

namespace gmac { namespace cuda {

#ifdef USE_MULTI_CONTEXT
gmac::util::Private<CUcontext> Accelerator::_Ctx;
#endif

void Switch::in()
{
    Mode::current().acc->pushContext();
}

void Switch::out()
{
    Mode::current().acc->popContext();
}

Accelerator::Accelerator(int n, CUdevice device) :
	gmac::Accelerator(n), _device(device)
{
    unsigned int size = 0;
    CUresult ret = cuDeviceTotalMem(&size, _device);
    CFatal(ret == CUDA_SUCCESS, "Unable to initialize CUDA %d", ret);
    ret = cuDeviceComputeCapability(&_major, &_minor, _device);
    CFatal(ret == CUDA_SUCCESS, "Unable to initialize CUDA %d", ret);
    _memory = size;

#ifndef USE_MULTI_CONTEXT
    CUcontext tmp;
    unsigned int flags = 0;
#if CUDART_VERSION >= 2020
    if(_major >= 2 || (_major == 1 && _minor >= 1)) flags |= CU_CTX_MAP_HOST;
#else
    trace("Host mapped memory not supported by the HW");
#endif
    ret = cuCtxCreate(&_ctx, flags, _device);
    CFatal(ret == CUDA_SUCCESS, "Unable to create CUDA context %d", ret);
    ret = cuCtxPopCurrent(&tmp);
    CFatal(ret == CUDA_SUCCESS, "Error setting up a new context %d", ret);
#else
#endif
}

Accelerator::~Accelerator()
{
#ifndef USE_MULTI_CONTEXT
    pushContext();
    ModuleVector::iterator i;
    for(i = _modules.begin(); i != _modules.end(); i++)
        delete *i;
    _modules.clear();
    popContext();
    assertion(cuCtxDestroy(_ctx) == CUDA_SUCCESS);
#endif
}

void Accelerator::init()
{
#ifdef USE_MULTI_CONTEXT
    gmac::util::Private<CUcontext>::init(_Ctx);
#endif
}

gmac::Mode *Accelerator::createMode()
{
    gmac::trace::Function::start("Accelerator","createMode");
	cuda::Mode *mode = new cuda::Mode(this);
	_queue.insert(mode);
	trace("Attaching Execution Mode %p to Accelerator", mode);
    _load++;
    gmac::trace::Function::end("Accelerator");
	return mode;
}

void Accelerator::destroyMode(gmac::Mode *mode)
{
	trace("Destroying Execution Mode %p", mode);
	if(mode == NULL) return;
    gmac::trace::Function::start("Accelerator","destroyMode");
	std::set<Mode *>::iterator c = _queue.find((Mode *)mode);
	assertion(c != _queue.end());
	_queue.erase(c);
    _load--;
    gmac::trace::Function::end("Accelerator");
}


#ifdef USE_MULTI_CONTEXT
CUcontext
Accelerator::createCUcontext()
{
    gmac::trace::Function::start("Accelerator","creaceCUContext");
    CUcontext ctx, tmp;
    unsigned int flags = 0;
#if CUDART_VERSION >= 2020
    if(_major >= 2 || (_major == 1 && _minor >= 1)) flags |= CU_CTX_MAP_HOST;
#else
    trace("Host mapped memory not supported by the HW");
#endif
    CUresult ret = cuCtxCreate(&ctx, flags, _device);
    if(ret != CUDA_SUCCESS)
        Fatal("Unable to create CUDA context %d", ret);
    ret = cuCtxPopCurrent(&tmp);
    assertion(ret == CUDA_SUCCESS);
    gmac::trace::Function::end("Accelerator");
    return ctx;
}

void
Accelerator::destroyCUcontext(CUcontext ctx)
{
    gmac::trace::Function::start("Accelerator","destroyCUContext");
    CFatal(cuCtxDestroy(ctx) == CUDA_SUCCESS, "Error destroying CUDA context");
    gmac::trace::Function::end("Accelerator");
}

#endif

#ifdef USE_MULTI_CONTEXT
ModuleVector Accelerator::createModules()
{
    gmac::trace::Function::start("Accelerator","createModules");
    pushContext();
    ModuleVector modules = ModuleDescriptor::createModules();
    popContext();
    gmac::trace::Function::end("Accelerator");
    return modules;
}

void
Accelerator::destroyModules(ModuleVector & modules)
{
    gmac::trace::Function::start("Accelerator","destroyModules");
    pushContext();
    ModuleVector::iterator i;
    for(i = modules.begin(); i != modules.end(); i++)
        delete *i;
    modules.clear();
    popContext();
    gmac::trace::Function::end("Accelerator");
}

#else
ModuleVector &Accelerator::createModules()
{
    gmac::trace::Function::start("Accelerator","createModules");
    if(_modules.empty()) {
        pushContext();
        _modules = ModuleDescriptor::createModules();
        popContext();
    }
    gmac::trace::Function::end("Accelerator");
    return _modules;
}
#endif

gmacError_t Accelerator::malloc(void **addr, size_t size, unsigned align) 
{
    gmac::trace::Function::start("Accelerator","malloc");
    assertion(addr != NULL);
    *addr = NULL;
    if(align > 1) {
        size += align;
    }
    CUdeviceptr ptr = 0;
    pushContext();
    CUresult ret = cuMemAlloc(&ptr, size);
    popContext();
    if(ret != CUDA_SUCCESS) {
        gmac::trace::Function::end("Accelerator");
        return error(ret);
    }
    CUdeviceptr gpuPtr = ptr;
    if(gpuPtr % align) {
        gpuPtr += align - (gpuPtr % align);
    }
    *addr = (void *)gpuPtr;
    _alignMap.lockWrite();
    _alignMap.insert(AlignmentMap::value_type(gpuPtr, ptr));
    _alignMap.unlock();
    trace("Allocating device memory: %p - %zd bytes (alignment %u)", *addr, size, align);
    gmac::trace::Function::end("Accelerator");
    return error(ret);
}

gmacError_t Accelerator::free(void *addr)
{
    gmac::trace::Function::start("Accelerator","free");
    assertion(addr != NULL);
    AlignmentMap::const_iterator i;
    CUdeviceptr gpuPtr = gpuAddr(addr);
    _alignMap.lockRead();
    i = _alignMap.find(gpuPtr);
    if (i == _alignMap.end()) {
        _alignMap.unlock();
        gmac::trace::Function::end("Accelerator");
        return gmacErrorInvalidValue;
    }
    _alignMap.unlock();
    pushContext();
    CUresult ret = cuMemFree(i->second);
    popContext();
    gmac::trace::Function::end("Accelerator");
    return error(ret);
}

gmacError_t Accelerator::memset(void *addr, int c, size_t size)
{
    gmac::trace::Function::start("Accelerator","memset");
    CUresult ret = CUDA_SUCCESS;
    pushContext();
    if(size % 32 == 0) {
        int seed = c | (c << 8) | (c << 16) | (c << 24);
        ret = cuMemsetD32(gpuAddr(addr), seed, size);
    }
    else if(size % 16) {
        short seed = c | (c << 8);
        ret = cuMemsetD16(gpuAddr(addr), seed, size);
    }
    else ret = cuMemsetD8(gpuAddr(addr), c, size);
    popContext();
    gmac::trace::Function::end("Accelerator");
    return error(ret);
}

gmacError_t Accelerator::sync()
{
    gmac::trace::Function::start("Accelerator","sync");
    pushContext();
    CUresult ret = cuCtxSynchronize();
    popContext();
    gmac::trace::Function::end("Accelerator");
    return error(ret);
}

gmacError_t Accelerator::hostAlloc(void **addr, size_t size)
{
    gmac::trace::Function::start("Accelerator","hostAlloc");
    pushContext();
#if CUDART_VERSION >= 2020
    CUresult ret = cuMemHostAlloc(addr, size, CU_MEMHOSTALLOC_PORTABLE | CU_MEMHOSTALLOC_DEVICEMAP);
#else
    CUresult ret = cuMemAllocHost(addr, size);
#endif
    popContext();
    gmac::trace::Function::end("Accelerator");
    return error(ret);
}

gmacError_t Accelerator::hostFree(void *addr)
{
    gmac::trace::Function::start("Accelerator","hostFree");
    pushContext();
    CUresult r = cuMemFreeHost(addr);
    popContext();
    gmac::trace::Function::end("Accelerator");
    return error(r);
}

void *Accelerator::hostMap(void *addr)
{
    gmac::trace::Function::start("Accelerator","hostMap");
    CUdeviceptr device;
    pushContext();
    CUresult ret = cuMemHostGetDevicePointer(&device, addr, 0);
    popContext();
    if(ret != CUDA_SUCCESS) device = 0;
    gmac::trace::Function::end("Accelerator");
    return (void *)device;
}

}}

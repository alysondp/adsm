#ifndef __API_CUDA_MODE_IPP_H_
#define __API_CUDA_MODE_IPP_H_

namespace gmac { namespace gpu {

inline void Switch::in()
{
    dynamic_cast<Mode *>(Mode::current())->switchIn();
}

inline void Switch::out()
{
    dynamic_cast<Mode *>(Mode::current())->switchOut();
}


inline Buffer::Buffer(paraver::LockName name, Mode *__mode) :
    util::Lock(name),
    __mode(__mode),
    __ready(true)
{
    __size = paramBufferPageLockedSize * paramPageSize;

    gmacError_t ret = __mode->hostAlloc(&__buffer, __size);
    if (ret == gmacSuccess) {
        trace("Using page locked memory: %zd", __size);
    } else {
        trace("Not using page locked memoryError %d");
        __buffer = NULL;
    }
}

inline Buffer::~Buffer()
{
    if(__buffer == NULL) return;
    gmacError_t ret = __mode->hostFree(__buffer);
    if(ret != gmacSuccess) warning("Error release mode buffer. I will continue anyway");
}

inline
void Mode::switchIn()
{
#ifdef USE_MULTI_CONTEXT
    __mutex.lock();
    CUresult ret = cuCtxPushCurrent(__ctx);
    cfatal(ret != CUDA_SUCCESS, "Unable to switch to CUDA mode");
#endif
}

inline
void Mode::switchOut()
{
#ifdef USE_MULTI_CONTEXT
    CUcontext tmp;
    CUresult ret = cuCtxPopCurrent(&tmp);
    __mutex.unlock();
    cfatal(ret != CUDA_SUCCESS, "Unable to switch back from CUDA mode");
#endif
}

inline
void Mode::call(dim3 Dg, dim3 Db, size_t shared, cudaStream_t tokens)
{
    __call = KernelConfig(Dg, Db, shared, tokens);
}

inline
void Mode::argument(const void *arg, size_t size, off_t offset)
{
    __call.pushArgument(arg, size, offset);
}


}}

#endif

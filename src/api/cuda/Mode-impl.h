#ifndef GMAC_API_CUDA_MODE_IMPL_H_
#define GMAC_API_CUDA_MODE_IMPL_H_

#include "core/Process.h"
#include "core/IOBuffer.h"

#include "Context.h"

namespace __impl { namespace cuda {

inline
void Mode::switchIn()
{
#ifdef USE_MULTI_CONTEXT
    getAccelerator().setCUcontext(&cudaCtx_);
#endif
}

inline
void Mode::switchOut()
{
#ifdef USE_MULTI_CONTEXT
    getAccelerator().setCUcontext(NULL);
#endif
}

inline
gmacError_t
Mode::launch(gmac_kernel_id_t id, core::KernelLaunch *&kernel)
{
    KernelMap::iterator i = kernels_.find(id);
    if (i == kernels_.end()) return gmacErrorInvalidValue;

    Kernel * k = dynamic_cast<Kernel *>(i->second);
    switchIn();
    kernel = &(getCUDAContext().launch(*k));
    switchOut();

    return gmacSuccess;
}

inline gmacError_t
Mode::execute(core::KernelLaunch & launch)
{
    switchIn();
    gmacError_t ret = getContext().prepareForCall();
    if(ret == gmacSuccess) {
        trace::SetThreadState(THREAD_T(id_), trace::Running);
        ret = getAccelerator().execute(dynamic_cast<KernelLaunch &>(launch));
    }
    switchOut();
    return ret;
}

inline gmacError_t
Mode::wait(core::KernelLaunch &launch)
{
    switchIn();
    error_ = contextMap_.waitForCall(launch);
    switchOut();

    return error_;
}

inline gmacError_t
Mode::wait()
{
    switchIn();
    error_ = contextMap_.waitForCall();
    switchOut();

    return error_;
}

inline
gmacError_t Mode::bufferToAccelerator(accptr_t dst, core::IOBuffer &buffer, size_t len, size_t off)
{
    TRACE(LOCAL,"Copy %p to device %p ("FMT_SIZE" bytes)", buffer.addr(), (void *) dst, len);
    switchIn();
    Context &ctx = getCUDAContext();
    gmacError_t ret = ctx.bufferToAccelerator(dst, buffer, len, off);
    switchOut();
    return ret;
}

inline
gmacError_t Mode::acceleratorToBuffer(core::IOBuffer &buffer, const accptr_t src, size_t len, size_t off)
{
    TRACE(LOCAL,"Copy %p to host %p ("FMT_SIZE" bytes)", (void *) src, buffer.addr(), len);
    switchIn();
    // Implement a function to remove these casts
    Context &ctx = getCUDAContext();
    gmacError_t ret = ctx.acceleratorToBuffer(buffer, src, len, off);
    switchOut();
    return ret;
}

inline
gmacError_t Mode::call(dim3 Dg, dim3 Db, size_t shared, cudaStream_t tokens)
{
    switchIn();
    Context &ctx = getCUDAContext();
    gmacError_t ret = ctx.call(Dg, Db, shared, tokens);
    switchOut();
    return ret;
}

inline
gmacError_t Mode::argument(const void *arg, size_t size, off_t offset)
{
    switchIn();
    Context &ctx = getCUDAContext();
    gmacError_t ret = ctx.argument(arg, size, offset);
    switchOut();
    return ret;
}

inline Mode &
Mode::getCurrent()
{
    return static_cast<Mode &>(core::Mode::getCurrent());
}

inline Accelerator &
Mode::getAccelerator() const
{
    return *static_cast<Accelerator *>(acc_);
}

inline gmacError_t
Mode::eventTime(uint64_t &t, CUevent start, CUevent end)
{
    /* There is no switch-in because we must already be inside the mode */
    gmacError_t ret = getAccelerator().timeCUevents(t, start, end);
    return ret;
}

}}

#endif
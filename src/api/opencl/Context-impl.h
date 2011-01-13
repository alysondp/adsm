#ifndef GMAC_API_CUDA_CONTEXT_IMPL_H_
#define GMAC_API_CUDA_CONTEXT_IMPL_H_

#include "Accelerator.h"
#include "Kernel.h"

namespace __impl { namespace opencl {

inline gmacError_t
Context::call(cl_uint workDim, size_t *globalWorkOffset, size_t *globalWorkSize, size_t *localWorkSize)
{
    ASSERTION(globalWorkOffset != NULL);
    ASSERTION(globalWorkSize != NULL);
    ASSERTION(localWorkSize != NULL);
    TRACE(LOCAL, "Creating new kernel call");
    call_ = KernelConfig(workDim, globalWorkOffset, globalWorkSize, localWorkSize, streamLaunch_);
    // TODO: perform some checking
    return gmacSuccess;
}

inline gmacError_t
Context::argument(const void *arg, size_t size)
{
    
    call_.pushArgument(arg, size);
    // TODO: perform some checking
    return gmacSuccess;
}

inline const cl_command_queue
Context::eventStream() const
{
    return streamLaunch_;
}

inline Accelerator &
Context::accelerator()
{
    return dynamic_cast<Accelerator &>(acc_);
}

}}

#endif

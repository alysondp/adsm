#ifndef __API_CUDADRV_ACCELERATOR_IPP_
#define __API_CUDADRV_ACCELERATOR_IPP_

#include "Accelerator.h"

namespace gmac { namespace gpu {

inline CUdevice
Accelerator::device() const
{
    return _device;
}

inline size_t
Accelerator::nContexts() const
{
    return queue.size();
}

//inline bool
//Accelerator::async() const
//{
//    return _async;
//}

#ifndef USE_MULTI_CONTEXT
inline void
Accelerator::pushLock()
{
    mutex.lock();
    CUresult ret = cuCtxPushCurrent(_ctx);
    assertion(ret == CUDA_SUCCESS);
}

inline void
Accelerator::popUnlock()
{
    CUcontext tmp;
    CUresult ret = cuCtxPopCurrent(&tmp);
    assertion(ret == CUDA_SUCCESS);
    mutex.unlock();
}

#ifdef USE_VM
#ifndef USE_HOSTMAP_VM
inline 
Context *
Accelerator::lastContext()
{
    return _lastContext;
}

inline
void
Accelerator::setLastContext(Context * ctx)
{
    _lastContext = ctx;
}
#endif
#endif

#endif

inline int
Accelerator::major() const
{
    return _major;
}

inline int
Accelerator::minor() const
{
    return _minor;
}

}}

#endif
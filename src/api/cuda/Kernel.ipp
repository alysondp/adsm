#ifndef __API_CUDADRV_KERNEL_IPP_
#define __API_CUDADRV_KERNEL_IPP_

#include "kernel/Kernel.h"

namespace gmac { namespace gpu {

inline
CUfunction
Kernel::cudaFunction() const
{
    return _f;
}

inline
dim3
KernelConfig::grid() const
{
    return _grid;
}

inline
dim3
KernelConfig::block() const
{
    return _block;
}

inline
size_t
KernelConfig::shared() const
{
    return _shared;
}

inline
cudaStream_t
KernelConfig::tokens() const
{
    return _tokens;
}


}}

#endif

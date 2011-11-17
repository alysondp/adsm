#ifndef GMAC_HAL_CUDA_STREAM_H_
#define GMAC_HAL_CUDA_STREAM_H_

#include <cuda.h>
#include <driver_types.h>
#include <vector_types.h>

#include "hal/types-detail.h"

#include "util/unique.h"

namespace __impl { namespace hal {
    
namespace cuda {

class GMAC_LOCAL stream_t :
    public hal::detail::stream_t<backend_traits, implementation_traits> {
    typedef hal::detail::stream_t<backend_traits, implementation_traits> Parent;

    event_t lastEvent_;

public:
    stream_t(CUstream stream, context_t &context);

    event_t get_last_event();
    void set_last_event(event_t event);

    context_t &get_context();

    Parent::state query();
    gmacError_t sync();
};

}}}

#include "module.h"

#endif /* GMAC_HAL_CUDA_STREAM_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
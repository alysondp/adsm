#ifndef GMAC_HAL_CUDA_DEVICE_H_
#define GMAC_HAL_CUDA_DEVICE_H_

#include "hal/device.h"
#include "util/Unique.h"

#include "types.h"

namespace __impl { namespace hal { namespace cuda {

class coherence_domain;

class device :
    public hal::detail::device<coherence_domain, aspace_t, stream_t, event_t, async_event_t>,
    public util::Unique<device> {
    typedef hal::detail::device<coherence_domain, aspace_t, stream_t, event_t, async_event_t> Parent;
protected:
    CUdevice cudaDevice_;

    int major_;
    int minor_;

    size_t memorySize_;

    bool integrated_;

public:
    device(int cuda_device_id, coherence_domain &coherenceDomain);

    aspace_t create_address_space();
    stream_t create_stream(aspace_t &aspace);

    event_t copy(accptr_t dst, hostptr_t src, size_t count, stream_t &stream);
    event_t copy(hostptr_t dst, accptr_t src, size_t count, stream_t &stream);
    event_t copy(accptr_t dst, accptr_t src, size_t count, stream_t &stream);

    async_event_t copy_async(accptr_t dst, hostptr_t src, size_t count, stream_t &stream);
    async_event_t copy_async(hostptr_t dst, accptr_t src, size_t count, stream_t &stream);
    async_event_t copy_async(accptr_t dst, accptr_t src, size_t count, stream_t &stream);

    gmacError_t sync(async_event_t &event);
    gmacError_t sync(stream_t &stream);
};

}}}

#endif /* GMAC_HAL_DEVICE_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

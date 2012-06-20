#ifndef GMAC_HAL_OPENCL_DEVICE_H_
#define GMAC_HAL_OPENCL_DEVICE_H_

#include "hal/device.h"
#include "util/unique.h"

#include "helper/opencl_helper.h"

#include "types.h"

namespace __impl { namespace hal { namespace opencl {

class coherence_domain;

typedef hal::detail::device<implementation_traits> hal_device;

class device;

class GMAC_LOCAL platform {
    cl_platform_id openclPlatformId_;
    std::list<device *> devices_;

    cl_context ctx_;

public:
    platform(cl_platform_id id, cl_context ctx);

    cl_platform_id get_cl_platform_id() const;

    void add_device(device &d);
    unsigned get_ndevices();

    cl_device_id *get_cl_device_array();
    cl_context get_context();
};

class GMAC_LOCAL device :
    public hal_device,
    public util::unique<device>,
    public gmac::util::mutex<device> {
    typedef hal_device Parent;
    friend class platform;
protected:
    platform &platform_;

    cl_device_id openclDeviceId_;
    cl_context context_;

    helper::opencl_version openclVersion_;

    size_t memorySize_;

    GmacDeviceInfo info_;
    bool isInfoInitialized_;

public:
    device(platform &p,
           cl_device_id openclDeviceId,
           coherence_domain &coherenceDomain);

    aspace *create_context(const SetSiblings &siblings, gmacError_t &err);
    gmacError_t destroy_context(aspace &context);

    stream_t *create_stream(aspace &context);
    gmacError_t destroy_stream(stream_t &stream);

    size_t get_total_memory() const;
    size_t get_free_memory() const;

    bool has_direct_copy(const Parent &dev) const;

    gmacError_t get_info(GmacDeviceInfo &info);

    platform &get_platform();
    const platform &get_platform() const;
};

}}}

#endif /* GMAC_HAL_OPENCL_DEVICE_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
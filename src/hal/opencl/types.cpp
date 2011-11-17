#include "config/common.h"

#include "hal/opencl/helper/opencl_helper.h"

#include "coherence_domain.h"
#include "device.h"
#include "module.h"

#define __GMAC_ERROR(r, err) case r: error = err; break

namespace __impl { namespace hal {

gmacError_t
init_platform()
{
    static bool initialized = false;
    gmacError_t ret = gmacSuccess;

    if (initialized == false) {
        initialized = true;
    } else {
        FATAL("Double HAL platform initialization");
    }

    return ret;
}

opencl::map_platform_repository Modules_("map_platform_repository");

std::list<opencl::device *>
init_devices()
{
    static bool initialized = false;

    if (initialized == false) {
        initialized = true;
    } else {
        FATAL("Double HAL device initialization");
    }

    std::list<opencl::device *> devices;

    TRACE(GLOBAL, "Initializing OpenCL API");
    cl_uint platformSize = 0;
    cl_int ret = CL_SUCCESS;
    ret = clGetPlatformIDs(0, NULL, &platformSize);
    CFATAL(ret == CL_SUCCESS);
    if(platformSize == 0) return devices;   
    cl_platform_id * platforms = new cl_platform_id[platformSize];
    ret = clGetPlatformIDs(platformSize, platforms, NULL);
    CFATAL(ret == CL_SUCCESS);
    MESSAGE("%d OpenCL platforms found", platformSize);

    for (unsigned i = 0; i < platformSize; i++) {
        MESSAGE("Platform [%u/%u]: %s", i + 1, platformSize, opencl::helper::get_platform_name(platforms[i]).c_str());
        cl_uint deviceSize = 0;
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU,
                             0, NULL, &deviceSize);
        ASSERTION(ret == CL_SUCCESS);
	    if(deviceSize == 0) continue;
        cl_device_id *deviceIds = new cl_device_id[deviceSize];
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU,
                             deviceSize, deviceIds, NULL);
        ASSERTION(ret == CL_SUCCESS);
        MESSAGE("... found %u OpenCL devices", deviceSize, i);

#if 0
        opencl::helper::opencl_version clVersion = opencl::helper::get_opencl_version(platforms[i]);
#endif

        cl_context ctx;
        opencl::platform *plat;

        cl_context_properties prop[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[i], 0 };

        ctx = clCreateContext(prop, deviceSize, deviceIds, NULL, NULL, &ret);
        CFATAL(ret == CL_SUCCESS, "Unable to create OpenCL context %d", ret);

        TRACE(GLOBAL, "cl_context %p for platform: %s", ctx, opencl::helper::get_platform_name(platforms[i]).c_str());

        plat = new opencl::platform(platforms[i], ctx);

        for (unsigned j = 0; j < deviceSize; j++) {
            MESSAGE("Device [%u/%u]: %s", j + 1, deviceSize, opencl::helper::get_device_name(deviceIds[j]).c_str());

            // Let's assume that this is not important; TODO: actually deal with this case
            //CFATAL(util::getDeviceVendor(devices[j]) == util::getPlatformVendor(platforms[i]), "Not handled case");
            opencl::device *device = NULL;

            switch (opencl::helper::get_platform(platforms[i])) {
                case opencl::helper::PLATFORM_AMD:
                    if (opencl::helper::is_device_amd_fusion(deviceIds[j])) {
                        device = new opencl::device(*plat, deviceIds[j], *new opencl::coherence_domain());
                    } else {
                        device = new opencl::device(*plat, deviceIds[j], *new opencl::coherence_domain());
                    }
                    break;
                case opencl::helper::PLATFORM_APPLE:
                case opencl::helper::PLATFORM_NVIDIA:
                    device = new opencl::device(*plat, deviceIds[j], *new opencl::coherence_domain());
                    break;
                case opencl::helper::PLATFORM_INTEL:
                case opencl::helper::PLATFORM_UNKNOWN:
                    FATAL("Platform not supported\n");
            }
            devices.push_back(device);
        }
        delete[] deviceIds;
    }
    delete[] platforms;
    initialized = true;

    opencl::compile_embedded_code(devices);

    return devices;
}

}}

namespace __impl { namespace hal { namespace opencl { 

gmacError_t
compile_embedded_code(std::list<opencl::device *> devices)
{
    return gmacErrorFeatureNotSupported;
}

gmacError_t compile_code(platform &plat, const std::string &code, const std::string &flags)
{
    /* module_descriptor *descriptor = */new module_descriptor(code, flags);

    gmacError_t ret;
    code_repository repository = module_descriptor::create_modules(plat, ret);

    Modules_.insert(map_platform_repository::value_type(&plat, repository));

    return ret;
}

gmacError_t compile_binary(platform &plat, const std::string &code, const std::string &flags)
{
    return gmacErrorFeatureNotSupported;
}

gmacError_t error(cl_int err)
{
    gmacError_t error = gmacSuccess;
    switch(err) {
        __GMAC_ERROR(CL_SUCCESS, gmacSuccess);
        __GMAC_ERROR(CL_DEVICE_NOT_FOUND, gmacErrorNoAccelerator);
        __GMAC_ERROR(CL_DEVICE_NOT_AVAILABLE, gmacErrorInvalidAccelerator);
        __GMAC_ERROR(CL_MEM_OBJECT_ALLOCATION_FAILURE, gmacErrorMemoryAllocation);
        __GMAC_ERROR(CL_OUT_OF_HOST_MEMORY, gmacErrorMemoryAllocation);
        __GMAC_ERROR(CL_OUT_OF_RESOURCES, gmacErrorMemoryAllocation);

        default: error = gmacErrorUnknown;
    }
    return error;
}

void
context_t::dispose_event(_event_t &event)
{
    queueEvents_.push(event);
}

context_t::context_t(cl_context ctx, device &dev) :
    Parent(ctx, dev)
{
    TRACE(LOCAL, "Creating context: %p", (*this)());
}

const code_repository &
context_t::get_code_repository()
{
    code_repository *repository;
    platform &plat = get_device().get_platform();
    map_platform_repository::iterator it = Modules_.find(&plat);
    if (it == Modules_.end()) {
        gmacError_t err;
        code_repository tmp = module_descriptor::create_modules(plat, err);
        ASSERTION(err == gmacSuccess);
        repository = &Modules_.insert(map_platform_repository::value_type(&plat, tmp)).first->second;
    } else {
        repository = &it->second;
    }

    return *repository;
}

stream_t::stream_t(cl_command_queue stream, context_t &context) :
    Parent(stream, context)
{
    TRACE(LOCAL, "Creating stream: %p", (*this)());
}

gmacError_t
stream_t::sync()
{
    TRACE(LOCAL, "Waiting for stream: %p", (*this)());
    cl_int ret = clFinish((*this)());

    return error(ret);
}

void
_event_t::reset(bool async, type t)
{
    async_ = async;
    type_ = t;
    err_ = gmacSuccess;
    synced_ = false;
    state_ = None;
    
    remove_triggers();
}

_event_t::state
_event_t::get_state()
{
    if (state_ != End) {
        cl_int status;
        cl_int res = clGetEventInfo(event_,
                                    CL_EVENT_COMMAND_EXECUTION_STATUS,
                                    sizeof(cl_int),
                                    &status, NULL);
        if (res == CL_SUCCESS) {
            if (status == CL_QUEUED) {
                state_ = Queued;
            } else if (status == CL_SUBMITTED) {
                state_ = Submit;
            } else if (status == CL_RUNNING) {
                state_ = Start;
            } else if (status == CL_COMPLETE) {
                state_ = End;
            } else {
                FATAL("Unhandled value");
            }
        }
    }

    return state_;
}

}}}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
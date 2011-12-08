#include "util/Logger.h"

#include "hal/types.h"
#include "device.h"

namespace __impl { namespace hal { namespace cuda {

device::device(CUdevice cudaDevice, coherence_domain &coherenceDomain) :
    Parent(coherenceDomain),
    gmac::util::mutex<device>("device"),
    cudaDevice_(cudaDevice),
    isInfoInitialized_(false)
{
    CUresult ret;
    int val;

    TRACE(GLOBAL, "Creating CUDA accelerator %d", cudaDevice_);
    ret = cuDeviceComputeCapability(&major_, &minor_, cudaDevice_);
    CFATAL(ret == CUDA_SUCCESS, "Unable to initialize CUDA device %d", ret);
    ret = cuDeviceGetAttribute(&val, CU_DEVICE_ATTRIBUTE_INTEGRATED, cudaDevice_);
    CFATAL(ret == CUDA_SUCCESS, "Unable to get attribute %d", ret);
    integrated_ = (val != 0);
}

context_t *
device::create_context(const SetSiblings &siblings, gmacError_t &err)
{
    CUcontext ctx, tmp;
    unsigned int flags = 0;
#if CUDA_VERSION >= 2020
    if(major_ >= 2 || (major_ == 1 && minor_ >= 1)) flags |= CU_CTX_MAP_HOST;
#else
    TRACE(LOCAL,"Host mapped memory not supported by the HW");
#endif
    CUresult res = cuCtxCreate(&ctx, flags, cudaDevice_);
    if(res != CUDA_SUCCESS)
        FATAL("Unable to create CUDA context %d", res);
    res = cuCtxPopCurrent(&tmp);
    ASSERTION(res == CUDA_SUCCESS);

#ifdef USE_PEER_ACCESS
    for (SetSiblings::iterator it = siblings.begin(); it != siblings.end(); it++) {
        
    }
#endif

    err = error(res);

    return new context_t(ctx, *this);
}

gmacError_t
device::destroy_context(context_t &context)
{
    CUresult ret = cuCtxDestroy(context());

    return error(ret);
}

stream_t *
device::create_stream(context_t &context)
{
    context.set(); 

    CUstream stream;
    CUresult ret = cuStreamCreate(&stream, 0);
    CFATAL(ret == CUDA_SUCCESS, "Unable to create CUDA stream");

    return new stream_t(stream, context);
}

gmacError_t
device::destroy_stream(stream_t &stream)
{
    CUresult ret = cuStreamDestroy(stream());

    return error(ret);
}

int
device::get_major() const
{
    return major_;
}

int
device::get_minor() const
{
    return minor_;
}

size_t
device::get_total_memory() const
{
    size_t total, dummy;
    CUresult ret = cuMemGetInfo(&dummy, &total);
    CFATAL(ret == CUDA_SUCCESS, "Error getting device memory size: %d", ret);
    return total;
}

size_t
device::get_free_memory() const
{
    size_t free, dummy;
    CUresult ret = cuMemGetInfo(&free, &dummy);
    CFATAL(ret == CUDA_SUCCESS, "Error getting device memory size: %d", ret);
    return free;
}

bool
device::has_direct_copy(const Parent &_dev) const
{
    const device &dev = reinterpret_cast<const device &>(_dev);
    int canAccess;
    CUresult ret = cuDeviceCanAccessPeer(&canAccess, cudaDevice_, dev.cudaDevice_);
    ASSERTION(ret == CUDA_SUCCESS, "Error querying devices");

    return canAccess == 1;
}

gmacError_t
device::get_info(GmacDeviceInfo &info)
{
    lock();
    if (!isInfoInitialized_) {
        static const size_t MaxAcceleratorNameLength = 128;
        char deviceName[MaxAcceleratorNameLength];
        CUresult res = cuDeviceGetName(deviceName, MaxAcceleratorNameLength, cudaDevice_);
        ASSERTION(res == CUDA_SUCCESS);

        info_.deviceName = deviceName;
        info_.vendorName = "NVIDIA Corporation";
        info_.deviceType = GMAC_DEVICE_TYPE_GPU;
        info_.vendorId = 1;
        info_.isAvailable = 1;
        
        int val;
        res = cuDeviceGetAttribute(&val, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, cudaDevice_);
        ASSERTION(res == CUDA_SUCCESS);
        info_.computeUnits = val;

        CUdevprop deviceProperties;

        res = cuDeviceGetProperties(&deviceProperties, cudaDevice_);
        ASSERTION(res == CUDA_SUCCESS);

        info_.maxDimensions = 3;

        size_t *maxSizes = new size_t[3];
        maxSizes[0] = deviceProperties.maxThreadsDim[0];
        maxSizes[1] = deviceProperties.maxThreadsDim[1];
        maxSizes[2] = deviceProperties.maxThreadsDim[2];

        info_.maxSizes = maxSizes;
        info_.maxWorkGroupSize = deviceProperties.maxThreadsPerBlock;
        info_.globalMemSize = get_total_memory();
        info_.localMemSize  = deviceProperties.sharedMemPerBlock;
        info_.cacheMemSize  = 0;

        int cudaVersion = 0;
        res = cuDriverGetVersion(&cudaVersion);
        if(res == CUDA_SUCCESS) info_.driverMajor = cudaVersion;
        else info_.driverMajor = 0;
        info_.driverMinor = info_.driverRev = 0;

        isInfoInitialized_ = true;
    }
    unlock();

    info = info_;
    return gmacSuccess;
}

}}}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

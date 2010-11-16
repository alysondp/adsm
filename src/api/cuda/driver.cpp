#include <cstring>

#include <vector_types.h>
#include <driver_types.h>

#include <string>
#include <vector>

#include "include/gmac.h"
#include "gmac/init.h"

#include "core/Kernel.h"

#include "Accelerator.h"
#include "Mode.h"
#include "Module.h"


#ifdef __cplusplus
extern "C" {
#endif

using gmac::cuda::Accelerator;
using gmac::cuda::Mode;
using gmac::KernelDescriptor;
using gmac::cuda::ModuleDescriptor;
using gmac::cuda::TextureDescriptor;
using gmac::cuda::VariableDescriptor;

/*!
 * @returns Module **
 */
GMAC_API void ** APICALL __cudaRegisterFatBinary(void *fatCubin)
{
    gmac::Process &proc = gmac::Process::getInstance();
    TRACE(GLOBAL, "CUDA Fat binary: %p", fatCubin);
    ASSERTION(proc.nAccelerators() > 0);
    gmac::enterGmac();
    // Use the first GPU to load the fat binary
    void **ret = (void **) new ModuleDescriptor(fatCubin);
	gmac::exitGmac();
	return ret;
}

GMAC_API void APICALL __cudaUnregisterFatBinary(void **fatCubinHandle)
{
    ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
    delete mod;
}

GMAC_API void APICALL __cudaRegisterFunction(
		void **fatCubinHandle, const char *hostFun, char * /*devFun*/,
		const char *devName, int /*threadLimit*/, uint3 * /*tid*/, uint3 * /*bid*/,
		dim3 * /*bDim*/, dim3 * /*gDim*/)
{
    TRACE(GLOBAL, "CUDA Function");
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERTION(mod != NULL);
	gmac::enterGmac();
    KernelDescriptor k = KernelDescriptor(devName, (gmacKernel_t) hostFun);
    mod->add(k);
	gmac::exitGmac();
}

GMAC_API void APICALL __cudaRegisterVar(void **fatCubinHandle, char *hostVar,
		char * /*deviceAddress*/, const char *deviceName, int /*ext*/, int /*size*/,
		int constant, int /*global*/)
{
    TRACE(GLOBAL, "CUDA Variable %s", deviceName);
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERTION(mod != NULL);
	gmac::enterGmac();
    VariableDescriptor v = VariableDescriptor(deviceName, hostVar, bool(constant != 0));
    mod->add(v);
	gmac::exitGmac();
}

GMAC_API void APICALL __cudaRegisterTexture(void **fatCubinHandle, const struct textureReference *hostVar,
		const void ** /*deviceAddress*/, const char *deviceName, int /*dim*/, int /*norm*/, int /*ext*/)
{
    TRACE(GLOBAL, "CUDA Texture");
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERTION(mod != NULL);
	gmac::enterGmac();
    TextureDescriptor t = TextureDescriptor(deviceName, hostVar);
	mod->add(t);
	gmac::exitGmac();
}

GMAC_API void APICALL __cudaRegisterShared(void ** /*fatCubinHandle*/, void ** /*devicePtr*/)
{
}

GMAC_API void APICALL __cudaRegisterSharedVar(void ** /*fatCubinHandle*/, void ** /*devicePtr*/,
		size_t /*size*/, size_t /*alignment*/, int /*storage*/)
{
}

GMAC_API cudaError_t APICALL cudaConfigureCall(dim3 gridDim, dim3 blockDim,
		size_t sharedMem, cudaStream_t tokens)
{
	gmac::enterGmac();
    Mode &mode = gmac::cuda::Mode::current();
	mode.call(gridDim, blockDim, sharedMem, tokens);
	gmac::exitGmac();
	return cudaSuccess;
}

GMAC_API cudaError_t APICALL cudaSetupArgument(const void *arg, size_t count, size_t offset)
{
	gmac::enterGmac();
    Mode &mode = gmac::cuda::Mode::current();
	mode.argument(arg, count, (off_t)offset);
	gmac::exitGmac();
	return cudaSuccess;
}

extern gmacError_t gmacLaunch(gmacKernel_t k);
GMAC_API cudaError_t APICALL cudaLaunch(gmacKernel_t k)
{
	gmacError_t ret = gmacLaunch(k);
    assert(ret == gmacSuccess);
	return cudaSuccess;
}

#ifdef __cplusplus
}
#endif



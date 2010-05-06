#include <config.h>
#include <debug.h>

#include "gmac/init.h"

#include "kernel/Kernel.h"

#include "Accelerator.h"
#include "Context.h"
#include "Module.h"

#include <cstring>

#include <cuda.h>
#include <vector_types.h>
#include <driver_types.h>

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

using gmac::gpu::Accelerator;
using gmac::gpu::Context;
using gmac::KernelDescriptor;
using gmac::gpu::ModuleDescriptor;
using gmac::gpu::TextureDescriptor;
using gmac::gpu::VariableDescriptor;

/*!
 * @returns Module **
 */
void **__cudaRegisterFatBinary(void *fatCubin)
{
    TRACE("CUDA Fat binary: %p", fatCubin);
    ASSERT(proc->nAccelerators() > 0);
    __enterGmac();
    // Use the first GPU to load the fat binary
    void **ret = (void **) new ModuleDescriptor(fatCubin);
	__exitGmac();
	return ret;
}

void __cudaUnregisterFatBinary(void **fatCubinHandle)
{
	__enterGmac();
    //! \todo Correctly undo everything
    //ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	__exitGmac();
}

void __cudaRegisterFunction(
		void **fatCubinHandle, const char *hostFun, char *devFun,
		const char *devName, int threadLimit, uint3 *tid, uint3 *bid,
		dim3 *bDim, dim3 *gDim)
{
    TRACE("CUDA Function");
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERT(mod != NULL);
	__enterGmac();
    KernelDescriptor k = KernelDescriptor(devName, (gmacKernel_t) hostFun);
    mod->add(k);
	__exitGmac();
}

void __cudaRegisterVar(void **fatCubinHandle, char *hostVar,
		char *deviceAddress, const char *deviceName, int ext, int size,
		int constant, int global)
{
    TRACE("CUDA Variable");
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERT(mod != NULL);
	__enterGmac();
    VariableDescriptor v = VariableDescriptor(deviceName, hostVar, bool(constant));
    mod->add(v);
	__exitGmac();
}

void __cudaRegisterTexture(void **fatCubinHandle, const struct textureReference *hostVar,
		const void **deviceAddress, const char *deviceName, int dim, int norm, int ext)
{
    TRACE("CUDA Texture");
	ModuleDescriptor *mod = (ModuleDescriptor *)fatCubinHandle;
	ASSERT(mod != NULL);
	__enterGmac();
    TextureDescriptor t = TextureDescriptor(deviceName, hostVar);
	mod->add(t);
	__exitGmac();
}

void __cudaRegisterShared(void **fatCubinHandle, void **devicePtr)
{
}

void __cudaRegisterSharedVar(void **fatCubinHandle, void **devicePtr,
		size_t size, size_t alignment, int storage)
{
}

cudaError_t cudaConfigureCall(dim3 gridDim, dim3 blockDim,
		size_t sharedMem, int tokens)
{
	__enterGmac();
    Context * ctx = Context::current();
	ctx->call(gridDim, blockDim, sharedMem, tokens);
	__exitGmac();
	return cudaSuccess;
}

cudaError_t cudaSetupArgument(const void *arg, size_t count, size_t offset)
{
	__enterGmac();
    Context * ctx = Context::current();
	ctx->argument(arg, count, offset);
	__exitGmac();
	return cudaSuccess;
}

extern gmacError_t gmacLaunch(gmacKernel_t k);
cudaError_t cudaLaunch(gmacKernel_t k)
{
	gmacLaunch(k);
	return cudaSuccess;
}

#ifdef __cplusplus
}
#endif



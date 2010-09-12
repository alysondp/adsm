#include <gmac.h>
#include <init.h>

#include <order.h>
#include <config.h>
#include <threads.h>

#include <util/Parameter.h>
#include <util/Private.h>
#include <util/Logger.h>
#include <util/FileLock.h>


#include <kernel/Process.h>
#include <kernel/Context.h>
#include <kernel/IOBuffer.h>

#include <memory/Manager.h>
#include <memory/Allocator.h>

#include <trace/Function.h>

#include <cstdlib>

#ifdef PARAVER
namespace paraver {
extern int init;
}
#endif

gmac::util::Private<const char> __in_gmac;

const char __gmac_code = 1;
const char __user_code = 0;

char __gmac_init = 0;

#ifdef LINUX 
#define GLOBAL_FILE_LOCK "/tmp/gmacSystemLock"
#else
#ifdef DARWIN
#define GLOBAL_FILE_LOCK "/tmp/gmacSystemLock"
#endif
#endif

static void __attribute__((constructor))
gmacInit(void)
{
	gmac::util::Private<const char>::init(__in_gmac);
	__enterGmac();
    __gmac_init = 1;

    gmac::util::Logger::Create("GMAC");
    gmac::util::Logger::TRACE("Initialiazing GMAC");

#ifdef PARAVER
    paraver::init = 1;
#endif
    //gmac::util::FileLock(GLOBAL_FILE_LOCK, trace::LockSystem);

    //FILE * lockSystem;

    paramInit();

    /* Call initialization of interpose libraries */
    osInit();
    threadInit();
    stdcInit();

    gmac::util::Logger::TRACE("Using %s memory manager", paramProtocol);
    gmac::util::Logger::TRACE("Using %s memory allocator", paramAllocator);
    gmac::Process::init(paramProtocol, paramAllocator);
    gmac::util::Logger::ASSERTION(manager != NULL);
    __exitGmac();
}

static void __attribute__((destructor))
gmacFini(void)
{
	__enterGmac();
    gmac::util::Logger::TRACE("Cleaning GMAC");
    delete proc;
    // We do not exitGmac to allow proper stdc function handling
    gmac::util::Logger::Destroy();
}


#if 0
gmacError_t
gmacClear(gmacKernel_t k)
{
    gmacError_t ret = gmacSuccess;
    __enterGmac();
    enterFunction(FuncGmacClear);
    gmac::Kernel *kernel = gmac::Mode::current()->kernel(k);
    if (kernel == NULL) ret = gmacErrorInvalidValue;
    else kernel->clear();
    exitFunction();
    __exitGmac();
    return ret;
}

gmacError_t
gmacBind(void * obj, gmacKernel_t k)
{
    gmacError_t ret = gmacSuccess;
    __enterGmac();
    enterFunction(FuncGmacBind);
    gmac::Kernel *kernel = gmac::Mode::current()->kernel(k);

    if (kernel == NULL) ret = gmacErrorInvalidValue;
    else ret = kernel->bind(obj);
    exitFunction();
    __exitGmac();
    return ret;
}

gmacError_t
gmacUnbind(void * obj, gmacKernel_t k)
{
    gmacError_t ret = gmacSuccess;
    __enterGmac();
    enterFunction(FuncGmacUnbind);
    gmac::Kernel  * kernel = gmac::Mode::current()->kernel(k);
    if (kernel == NULL) ret = gmacErrorInvalidValue;
    else ret = kernel->unbind(obj);
	exitFunction();
	__exitGmac();
    return ret;
}
#endif

size_t
gmacAccs()
{
    size_t ret;
	__enterGmac();
    gmac::trace::Function::start("gmacAccs");
    ret = proc->nAccelerators();
    gmac::trace::Function::end();
	__exitGmac();
	return ret;
}

gmacError_t
gmacMigrate(int acc)
{
	gmacError_t ret;
	__enterGmac();
    gmac::trace::Function::start("gmacMigrate");
    if (gmac::Mode::hasCurrent()) {
        ret = proc->migrate(gmac::Mode::current(), acc);
    } else {
        ret = proc->migrate(NULL, acc);
    }
    gmac::trace::Function::end();
	__exitGmac();
	return ret;
}

gmacError_t
gmacMalloc(void **cpuPtr, size_t count)
{
    gmacError_t ret = gmacSuccess;
    if (count == 0) {
        *cpuPtr = NULL;
        return ret;
    }
	__enterGmac();
    gmac::trace::Function::start("gmacMalloc");
    if(allocator != NULL && count < (paramPageSize / 2)) {
        *cpuPtr = allocator->alloc(count, __builtin_return_address(0));   
    }
    else {
	    count = (int(count) < getpagesize())? getpagesize(): count;
	    ret = manager->alloc(cpuPtr, count);
    }
    gmac::trace::Function::end();
	__exitGmac();
	return ret;
}

gmacError_t
gmacGlobalMalloc(void **cpuPtr, size_t count, int hint)
{
#ifndef USE_MMAP
    gmacError_t ret = gmacSuccess;
    if(count == 0) {
        *cpuPtr = NULL;
        return ret;
    }
    __enterGmac();
    gmac::trace::Function::start("gmacGlobalMalloc");
	count = (count < (size_t)getpagesize()) ? (size_t)getpagesize(): count;
	ret = manager->globalAlloc(cpuPtr, count, hint);
    gmac::trace::Function::end();
    __exitGmac();
    return ret;
#else
    return gmacErrorFeatureNotSupported;
#endif
}

gmacError_t
gmacFree(void *cpuPtr)
{
    gmacError_t ret = gmacSuccess;
	__enterGmac();
    gmac::trace::Function::start("gmacFree");
    if(allocator == NULL || allocator->free(cpuPtr) == false)
        ret = manager->free(cpuPtr);
    gmac::trace::Function::end();
	__exitGmac();
	return ret;
}

void *
gmacPtr(void *ptr)
{
    void *ret = NULL;
    __enterGmac();
    ret = proc->translate(ptr);
    __exitGmac();
    return ret;
}

gmacError_t
gmacLaunch(gmacKernel_t k)
{
    __enterGmac();
    gmac::Mode * mode = gmac::Mode::current();
    gmac::trace::Function::start("gmacLaunch");
    gmac::KernelLaunch * launch = mode->launch(k);

    gmacError_t ret = gmacSuccess;
    gmac::util::Logger::TRACE("Flush the memory used in the kernel");
    gmac::util::Logger::ASSERTION(manager->release() == gmacSuccess);

    // Wait for pending transfers
    mode->sync();
    gmac::util::Logger::TRACE("Kernel Launch");
    ret = mode->execute(launch);

    if(paramAcquireOnWrite) {
        gmac::util::Logger::TRACE("Invalidate the memory used in the kernel");
        //manager->invalidate();
    }

    delete launch;
    gmac::trace::Function::end();
    __exitGmac();

    return ret;
}

gmacError_t
gmacThreadSynchronize()
{
	__enterGmac();
    gmac::trace::Function::start("gmacSync");

	gmacError_t ret = gmac::Mode::current()->sync();
    gmac::util::Logger::TRACE("Memory Sync");
    manager->acquire();

    gmac::trace::Function::end();
	__exitGmac();
	return ret;
}

gmacError_t
gmacGetLastError()
{
	__enterGmac();
	gmacError_t ret = gmac::Mode::current()->error();
	__exitGmac();
	return ret;
}

void *
gmacMemset(void *s, int c, size_t n)
{
    __enterGmac();
    void *ret = s;
    manager->memset(s, c, n);
	__exitGmac();
    return ret;
}

void *
gmacMemcpy(void *dst, const void *src, size_t n)
{
	__enterGmac();
	void *ret = dst;

    gmacError_t err;

	// Locate memory regions (if any)
    gmac::Mode *dstMode = proc->owner(dst);
    gmac::Mode *srcMode = proc->owner(src);
	if (dstMode == NULL && srcMode == NULL) return memcpy(dst, src, n);;
    manager->memcpy(dst, src, n);

	__exitGmac();
	return ret;
}

void
gmacSend(pthread_t id)
{
    __enterGmac();
    proc->send((THREAD_ID)id);
    __exitGmac();
}

void gmacReceive()
{
    __enterGmac();
    proc->receive();
    __exitGmac();
}

void
gmacSendReceive(pthread_t id)
{
	__enterGmac();
	proc->sendReceive((THREAD_ID)id);
	__exitGmac();
}

void gmacCopy(pthread_t id)
{
    __enterGmac();
    proc->copy((THREAD_ID)id);
    __exitGmac();
}

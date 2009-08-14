#include <gmac.h>

#include <stdlib.h>
#include <assert.h>

#include <config/config.h>
#include <config/threads.h>
#include <config/paraver.h>
#include <config/debug.h>

#include <acc/api.h>
#include <memory/MemManager.h>

MUTEX(gmacMutex);
gmac::MemManager *memManager = NULL;
static unsigned memManagerCount = 0;
static size_t pageSize = 0;

static const char *memManagerVar = "GMAC_MANAGER";

#ifdef PARAVER
extern int __init_paraver;
#endif

void gmacRemoveManager(void)
{
	__MUTEX_LOCK(gmacMutex);
	memManagerCount--;
	if(memManagerCount <= 0) {
		delete memManager;
		memManager = NULL;
	}
	__MUTEX_UNLOCK(gmacMutex);
}

void gmacCreateManager(void)
{
	__MUTEX_LOCK(gmacMutex);
	if(memManager == NULL)
		memManager = gmac::getManager(getenv(memManagerVar));
	memManagerCount++;
	__MUTEX_UNLOCK(gmacMutex);
}

static void __attribute__((constructor(199))) gmacInit(void)
{
	__init_paraver = 0;
	pageSize = getpagesize();
	MUTEX_INIT(gmacMutex);
	gmacCreateManager();
}


static void __attribute__((destructor)) gmacFini(void)
{
	gmacRemoveManager();
}

gmacError_t gmacMalloc(void **devPtr, size_t count)
{
	enterFunction(_gmacMalloc_);
	gmacError_t ret = gmacSuccess;
	count = (count < pageSize) ? pageSize : count;
	ret = __gmacMalloc(devPtr, count);
	if(ret != gmacSuccess || !memManager) {
		exitFunction();
		return ret;
	}
	if(!memManager->alloc(*devPtr, count)) {
		__gmacFree(*devPtr);
		exitFunction();
		return gmacErrorMemoryAllocation;
	}
	exitFunction();
	return gmacSuccess;
}

gmacError_t gmacSafeMalloc(void **cpuPtr, size_t count)
{
	enterFunction(_gmacMalloc_);
	gmacError_t ret = gmacSuccess;
	void *devPtr;
	count = (count < pageSize) ? pageSize : count;
	ret = __gmacMalloc(&devPtr, count);
	if(ret != gmacSuccess || !memManager) {
		exitFunction();
		return ret;
	}
	if((*cpuPtr = memManager->safeAlloc(devPtr, count)) == NULL) {
		__gmacFree(devPtr);
		exitFunction();
		return gmacErrorMemoryAllocation;
	}
	exitFunction();
	return gmacSuccess;
}

void *gmacSafePointer(void *devPtr)
{
	if(!memManager) return devPtr;
	return memManager->safe(devPtr);
}

gmacError_t gmacFree(void *devPtr)
{
	enterFunction(_gmacFree_);
	__gmacFree(gmacSafePointer(devPtr));
	if(memManager) {
		memManager->release(devPtr);
	}
	exitFunction();
	return gmacSuccess;
}

gmacError_t gmacMallocPitch(void **devPtr, size_t *pitch,
		size_t widthInBytes, size_t height)
{
	enterFunction(_gmacMalloc_);
	void *cpuAddr = NULL;
	gmacError_t ret = gmacSuccess;
	size_t count = widthInBytes * height;

	if(count < pageSize) {
		height = pageSize / widthInBytes;
		if(pageSize % widthInBytes) height++;
	}

	ret = __gmacMallocPitch(devPtr, pitch, widthInBytes, height);
	if(ret != gmacSuccess && !memManager) {
		exitFunction();
		return ret;
	}

	if(!memManager->alloc(*devPtr, *pitch)) {
		__gmacFree(*devPtr);
		exitFunction();
		return gmacErrorMemoryAllocation;
	}
	exitFunction();
	return gmacSuccess;
}

gmacError_t gmacLaunch(const char *symbol)
{
	enterFunction(_gmacLaunch_);
	gmacError_t ret = gmacSuccess;
	if(memManager) {
		TRACE("Memory Flush");
		memManager->flush();
	}
	TRACE("Kernel Launch");
	ret = __gmacLaunch(symbol);
	exitFunction();
	return ret;
}

gmacError_t gmacThreadSynchronize()
{
	enterFunction(_gmacSync_);
	gmacError_t ret = __gmacThreadSynchronize();
	if(memManager) {
		TRACE("Memory Sync");
		memManager->sync();
	}
	exitFunction();
	return ret;
}

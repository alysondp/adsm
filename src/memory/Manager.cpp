#include "Manager.h"
#include "manager/BatchManager.h"
#include "manager/LazyManager.h"
#include "manager/RollingManager.h"

#include <debug.h>

#include <strings.h>


namespace gmac { namespace memory {

Manager *getManager(const char *managerName)
{
	if(managerName == NULL) return new manager::RollingManager();
	TRACE("Using %s Manager", managerName);
	if(strcasecmp(managerName, "None") == 0)
		return NULL;
	else if(strcasecmp(managerName, "Lazy") == 0)
		return new manager::LazyManager();
	else if(strcasecmp(managerName, "Batch") == 0)
		return new manager::BatchManager();
	return new manager::RollingManager();
}

Manager::Manager()
{
    TRACE("Memory manager starts");
}

Manager::~Manager()
{
    TRACE("Memory manager finishes");
}

Region *Manager::remove(void *addr)
{
    Context * ctx = Context::current();
	Region *ret = ctx->mm().remove(addr);
	if(ret->owner() == ctx) {
		if(ret->relatives().empty() == false) { // Change ownership
			ret->transfer();
			ret->owner()->mm().insert(ret);
		}
	}
	else ret->unrelate(ctx);
	return ret;
}

void Manager::insertVirtual(Context *ctx, void *cpuPtr, void *devPtr, size_t count)
{
#ifndef USE_MMAP
	TRACE("Virtual Request %p -> %p", cpuPtr, devPtr);
	PageTable &pageTable = ctx->mm().pageTable();
	assert(((unsigned long)cpuPtr & (pageTable.getPageSize() -1)) == 0);
	uint8_t *devAddr = (uint8_t *)devPtr;
	uint8_t *cpuAddr = (uint8_t *)cpuPtr;
	TRACE("Page Table Request %p -> %p", cpuAddr, devAddr);
	for(size_t off = 0; off < count; off += pageTable.getPageSize())
		pageTable.insert(cpuAddr + off, devAddr + off);
#endif
}

void Manager::removeVirtual(Context *ctx, void *cpuPtr, size_t count)
{
#ifndef USE_MMAP
	uint8_t *cpuAddr = (uint8_t *)cpuPtr;
	PageTable &pageTable = ctx->mm().pageTable();
	count += ((unsigned long)cpuPtr & (pageTable.getPageSize() -1));
	for(size_t off = 0; off < count; off += pageTable.getPageSize())
		pageTable.remove(cpuAddr + off);
#endif
}

void Manager::map(void *host, void *dev, size_t count)
{
	insert(new Region(host, count));
	insertVirtual(Context::current(), host, dev, count);
}

void Manager::unmap(Context *ctx, void *cpuPtr)
{
	Region *region = Context::current()->mm().find<Region>(cpuPtr);
	assert(region != NULL);
	region->unrelate(ctx);
	removeVirtual(ctx, cpuPtr, region->size());
}

} };

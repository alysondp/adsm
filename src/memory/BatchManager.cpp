#include "BatchManager.h"

#include <debug.h>

#include <kernel/Context.h>

namespace gmac { namespace memory {
void BatchManager::release(void *addr)
{
	Region *reg = remove(ptr(addr));
	unmap(reg->start(), reg->size());
	delete reg;
}
void BatchManager::flush(void)
{
	memory::Map::const_iterator i;
	current()->lock();
	for(i = current()->begin(); i != current()->end(); i++) {
		TRACE("Memory Copy to Device");
		Context::current()->copyToDevice(ptr(i->second->start()),
				i->second->start(), i->second->size());
	}
	current()->unlock();
	gmac::Context::current()->flush();
	gmac::Context::current()->sync();
}

void BatchManager::sync(void)
{
	memory::Map::const_iterator i;
	current()->lock();
	for(i = current()->begin(); i != current()->end(); i++) {
		TRACE("Memory Copy from Device");
		Context::current()->copyToHost(i->second->start(),
				ptr(i->second->start()), i->second->size());
	}
	current()->unlock();
}

} };

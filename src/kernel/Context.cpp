#include "Context.h"

#include "params.h"

#include <memory/Manager.h>

extern gmac::memory::Manager *manager;

namespace gmac {

util::Lock Context::lockCreate(paraver::ctxCreate);

PARAM_REGISTER(paramBufferPageLockedSize,
               size_t,
               4 * 1024 * 1024,
               "GMAC_BUFFER_PAGE_LOCKED_SIZE",
               PARAM_NONZERO);

void contextInit()
{
	PRIVATE_INIT(gmac::Context::key, NULL);
	PRIVATE_INIT(gmac::Context::keyParent, NULL);
	PRIVATE_SET(gmac::Context::key, NULL);
	PRIVATE_SET(gmac::Context::keyParent, NULL);
}


PRIVATE(Context::key);
PRIVATE(Context::keyParent);
unsigned Context::_next = 0;

Context::Context(Accelerator &acc) : acc(acc)
{
	_id = ++_next;
}

void Context::init()
{
	TRACE("Initializing cloned context");
	Process::SharedMap::iterator i;
	Process::SharedMap &sharedMem = proc->sharedMem();
	for(i = sharedMem.begin(); i != sharedMem.end(); i++) {
		TRACE("Mapping Shared Region %p (%d bytes)", i->second.start(), i->second.size());
		void *devPtr;
#ifdef USE_GLOBAL_HOST
		TRACE("Using Host Translation");
		gmacError_t ret = hostMap(i->second.start(), &devPtr, i->second.size());
#else
		gmacError_t ret = malloc(&devPtr, i->second.size());
#endif
		assert(ret == gmacSuccess);
		manager->remap(this, i->second.start(), devPtr, i->second.size());
		i->second.inc();
	}
}
}

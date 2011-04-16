#include <os/loader.h>
#include <gmac.h>

#include <cstring>

#include "init.h"
#include "memory/Manager.h"
#include "memory/Region.h"
#include "core/Context.h"

#include "stdc.h"


SYM(void *, __libc_memset, void *, int, size_t);
SYM(void *, __libc_memcpy, void *, const void *, size_t);

extern gmac::memory::Manager *manager;

void stdcMemInit(void)
{
	logger->trace("Overloading Memory STDC functions");
	LOAD_SYM(__libc_memset, memset);
	LOAD_SYM(__libc_memcpy, memcpy);
}

void *memset(void *s, int c, size_t n)
{
	if(__libc_memset == NULL) stdcMemInit();
	if(__inGmac() == 1) return __libc_memset(s, c, n);
	
	__enterGmac();
	gmac::Context *ctx = manager->owner(s);
	if(ctx == NULL) __libc_memset(s, c, n);
	else {
		logger->trace("GMAC Memset");
		manager->invalidate(s, n);
		ctx->memset(manager->ptr(s), c, n);
	}
	__exitGmac();
}

void *memcpy(void *dst, const void *src, size_t n)
{
	if(__libc_memcpy == NULL) stdcMemInit();
	void *ret = dst;
	size_t ds = 0, ss = 0;

	if(__inGmac() == 1) return __libc_memcpy(dst, src, n);

	__enterGmac();
	// TODO: handle copies involving partial memory regions

	// Locate memory regions (if any)
	gmac::Context *dstCtx = manager->owner(dst);
	gmac::Context *srcCtx = manager->owner(src);

	// Fast path - both regions are in the CPU
	if(dstCtx == NULL && srcCtx == NULL) {
		__exitGmac();
		return __libc_memcpy(dst, src, n);
	}

	logger->trace("GMAC Memcpy");
	if(dstCtx == NULL) { // Copy to Host
		manager->flush(src, n);
		srcCtx->copyToHost(dst, manager->ptr(src), n);
	}
	else if(srcCtx == NULL) { // Copy to Device
		manager->invalidate(dst, n);
		dstCtx->copyToDevice(manager->ptr(dst), src, n);
	}
	else if(dstCtx == srcCtx) {	// Same device copy
		manager->flush(src, n);
		manager->invalidate(dst, n);
		dstCtx->copyDevice(manager->ptr(dst),
		manager->ptr(src), n);
	}
	else {
		void *tmp = malloc(n);
		manager->flush(src, n);
		srcCtx->copyToHost(tmp, manager->ptr(src), n);
		manager->invalidate(dst, n);
		dstCtx->copyToDevice(manager->ptr(dst), tmp, n);
		free(tmp);
	}
	__exitGmac();
	return ret;
}

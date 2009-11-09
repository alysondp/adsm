#include <os/loader.h>
#include <order.h>
#include <gmac.h>

#include <string.h>

#include <memory/Manager.h>
#include <memory/Region.h>
#include <kernel/Context.h>


SYM(void *, __libc_memset, void *, int, size_t);
SYM(void *, __libc_memcpy, void *, const void *, size_t);

extern gmac::memory::Manager *manager;

static void __attribute__((constructor(INTERPOSE))) stdcMemInit(void)
{
	TRACE("Overloading Memory STDC functions");
	LOAD_SYM(__libc_memset, memset);
	LOAD_SYM(__libc_memcpy, memcpy);
}

void *memset(void *s, int c, size_t n)
{
	if(manager == NULL) return __libc_memset(s, c, n);
	
	gmac::Context *ctx = manager->owner(s);
	if(ctx == NULL) __libc_memset(s, c, n);
	else {
		TRACE("GMAC Memset");
		manager->invalidate(s, n);
		ctx->memset(manager->ptr(s), c, n);
	}
}

void *memcpy(void *dst, const void *src, size_t n)
{
	void *ret = dst;
	size_t ds = 0, ss = 0;

	if(manager == NULL) return __libc_memcpy(dst, src, n);

	// TODO: handle copies involving partial memory regions

	// Locate memory regions (if any)
	gmac::Context *dstCtx = manager->owner(dst);
	gmac::Context *srcCtx = manager->owner(src);

	// Fast path - both regions are in the CPU
	if(dstCtx == NULL && srcCtx == NULL) return __libc_memcpy(dst, src, n);

	TRACE("GMAC Memcpy");
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

	return ret;
}


#ifndef GMAC_MEMORY_PROTOCOL_GATHER_IMPL_H_
#define GMAC_MEMORY_PROTOCOL_GATHER_IMPL_H_

#include "memory/BlockGroup.h"

namespace __impl { namespace memory { namespace protocol {

template<typename T>
inline Gather<T>::Gather(size_t limit) :
    GatherBase(limit)
{}

template<typename T>
inline Gather<T>::~Gather()
{}

template<typename T>
inline memory::object *Gather<T>::create_object(size_t size, host_ptr cpuPtr, 
                                               GmacProtection prot, unsigned flags)
{
    // TODO: get mode as parameter
    object *ret = new T(*this, core::Mode::getCurrent(), cpuPtr, 
		size, state(prot));
	if(ret == NULL) return ret;
	if(ret->valid() == false) {
		ret->release();
		return NULL;
	}
	memory_ops::protect(ret->addr(), ret->size(), prot);
	return ret;
}

}}}
#endif
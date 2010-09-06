#ifndef __MEMORY_REPLICATEDOBJECT_IPP
#define __MEMORY_REPLICATEDOBJECT_IPP

namespace gmac { namespace memory {

#ifndef USE_MMAP
template<typename T>
inline ReplicatedObject<T>::ReplicatedObject(size_t size, T init) :
    StateObject<T>(size)
{
    // This line might seem useless, but we first need to make sure that
    // the curren thread has an execution mode attached
    Mode *mode = gmac::Mode::current(); 
    trace("Creating Replicated Object (%zd bytes)", StateObject<T>::__size);
    if(proc->globalMalloc(*this, size) != gmacSuccess) {
        Object::fatal("Unable to create replicated object");
        StateObject<T>::__addr = NULL;
        return;
    }

    StateObject<T>::__addr = StateObject<T>::map(NULL, size);
    if(StateObject<T>::__addr == NULL) {
        proc->globalFree(*this);
        return;
    }

    setupSystem(init);
    trace("Replicated object create @ %p", StateObject<T>::__addr);
}

template<typename T>
inline ReplicatedObject<T>::~ReplicatedObject()
{
    if(StateObject<T>::__addr == NULL) { return; }
    proc->globalFree(*this);
    StateObject<T>::lockWrite();
    accelerator.clear();
    StateObject<T>::unmap(StateObject<T>::__addr, StateObject<T>::__size);
    StateObject<T>::unlock();
}

template<typename T>
inline void *ReplicatedObject<T>::device(void *addr)
{
    StateObject<T>::lockRead();
    off_t offset = (unsigned long)addr - (unsigned long)StateObject<T>::__addr;
    typename AcceleratorMap::const_iterator i = accelerator.find(gmac::Mode::current());
    Object::assertion(i != accelerator.end());
    void *ret = (uint8_t *)i->second->addr() + offset;
    StateObject<T>::unlock();
    return ret;
}

template<typename T>
inline gmacError_t ReplicatedObject<T>::acquire(Block *block)
{
    Object::fatal("Reacquiring ownership of a replicated object is forbiden");
    return gmacErrorInvalidValue;
}

template<typename T>
inline gmacError_t ReplicatedObject<T>::release(Block *block)
{
    gmacError_t ret = gmacSuccess;
    StateObject<T>::lockRead();
    off_t off = (uint8_t *)block->addr() - (uint8_t *)StateObject<T>::__addr;
    typename AcceleratorMap::iterator i;
    for(i = accelerator.begin(); i != accelerator.end(); i++) {
        gmacError_t tmp = i->second->put(off, block);
        if(tmp != gmacSuccess) ret = tmp;
    }
    StateObject<T>::unlock();
    return ret;
}

template<typename T>
inline gmacError_t ReplicatedObject<T>::addOwner(Mode *mode)
{
    void *device = NULL;
    gmacError_t ret;
    ret = mode->malloc(&device, StateObject<T>::__size);
    Object::cfatal(ret == gmacSuccess, "Unable to replicate Object");

    StateObject<T>::lockWrite();
    AcceleratorBlock *dev = new AcceleratorBlock(mode, device, StateObject<T>::__size);
    accelerator.insert(typename AcceleratorMap::value_type(mode, dev));
    typename StateObject<T>::SystemMap::iterator i;
    for(i = StateObject<T>::systemMap.begin(); i != StateObject<T>::systemMap.end(); i++) {
        if(mode->requireUpdate(i->second) == false) continue;
        off_t off = (uint8_t *)i->second->addr() - (uint8_t *)StateObject<T>::__addr;
        dev->put(off, i->second);
    }
    StateObject<T>::unlock();
    trace("Adding replicated object @ %p to mode %p", device, mode);
    return ret;
}

template<typename T>
inline gmacError_t ReplicatedObject<T>::removeOwner(Mode *mode)
{
    StateObject<T>::lockWrite();
    typename AcceleratorMap::iterator i = accelerator.find(mode);
    Object::assertion(i != accelerator.end());
    AcceleratorBlock *acc = i->second;
    accelerator.erase(i);
    gmacError_t ret = mode->free(acc->addr());
    delete acc;
    StateObject<T>::unlock();
    return ret;
}

#endif

}}

#endif

#ifndef __MEMORY_BLOCK_IPP
#define __MEMORY_BLOCK_IPP

#include <kernel/Mode.h>

namespace gmac { namespace memory {
inline Block::Block(void *addr, size_t size) :
    RWLock(paraver::LockBlock),
    __addr(addr),
    __size(size)
{}

inline AcceleratorBlock::AcceleratorBlock(Mode *owner, void *addr, size_t size) :
    Block(addr, size),
    owner(owner)
{ }

inline AcceleratorBlock::~AcceleratorBlock()
{ }

inline gmacError_t AcceleratorBlock::put(off_t off, Block *block)
{
    trace("Mode %p is putting %p into device @ %p", owner, block->addr(), (uint8_t *)__addr + off);
    return owner->copyToDevice((uint8_t *)__addr + off, block->addr(), block->size());
}

inline gmacError_t AcceleratorBlock::get(off_t off, Block *block)
{
    trace("Mode %p is getting %p from device @ %p", owner, block->addr(), (uint8_t *)__addr + off);
    return owner->copyToHost(block->addr(), (uint8_t *)__addr + off, block->size());
}


template<typename T>
inline SystemBlock<T>::SystemBlock(void *addr, size_t size, T state) :
    Block(addr, size),
    __state(state)
{ }

template<typename T>
inline SystemBlock<T>::~SystemBlock()
{ }


template<typename T>
inline T SystemBlock<T>::state()
{ 
    lockRead();
    T ret = __state;
    unlock();
    return ret;
}

template<typename T>
inline void SystemBlock<T>::state(T s)
{
    lockWrite();
    __state = s;
    unlock();
}

}}

#endif

#ifndef __MEMORY_BLOCK_IPP
#define __MEMORY_BLOCK_IPP

#include "core/Mode.h"

namespace gmac { namespace memory {
inline Block::Block(void *addr, size_t size) :
    Lock("memory::Block"),
    __addr(addr),
    __size(size)
{}

inline AcceleratorBlock::AcceleratorBlock(Mode &owner, void *addr, size_t size) :
    Block(addr, size),
    _owner(owner)
{ }

inline AcceleratorBlock::~AcceleratorBlock()
{ }

inline gmacError_t AcceleratorBlock::toDevice(off_t off, Block &block)
{
    trace("Mode %d is putting %p into device @ %p", _owner.id(), block.addr(), (uint8_t *)__addr + off);
    return _owner.copyToDevice((uint8_t *)__addr + off, block.addr(), block.size());
}

inline gmacError_t AcceleratorBlock::toHost(off_t off, Block &block)
{
    trace("Mode %d is getting %p from device @ %p", _owner.id(), block.addr(), (uint8_t *)__addr + off);
    return _owner.copyToHost(block.addr(), (uint8_t *)__addr + off, block.size());
}

inline gmacError_t AcceleratorBlock::toHost(off_t off, void *hostAddr, size_t count)
{
    trace("Mode %d is getting %p from device @ %p", _owner.id(), hostAddr, (uint8_t *)__addr + off);
    return _owner.copyToHost(hostAddr, (uint8_t *)__addr + off, count);
}

template<typename T>
inline SystemBlock<T>::SystemBlock(void *addr, size_t size, T state) :
    Block(addr, size),
    _state(state)
{ }

template<typename T>
inline SystemBlock<T>::~SystemBlock()
{ }


template<typename T>
inline T SystemBlock<T>::state() const
{ 
    T ret = _state;
    return ret;
}

template<typename T>
inline void SystemBlock<T>::state(T s)
{
    _state = s;
}

}}

#endif

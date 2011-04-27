#ifndef GMAC_MEMORY_PROTOCOL_BLOCKLIST_IMPL_H_
#define GMAC_MEMORY_PROTOCOL_BLOCKLIST_IMPL_H_

#include <algorithm>

#include "memory/Block.h"
#include "memory/vm/Model.h"

namespace __impl { namespace memory { namespace protocol { 


inline BlockList::BlockList() :
    Lock("BlockList")
{}

inline BlockList::~BlockList()
{}

inline bool BlockList::empty() const
{
    lock();
    bool ret = Parent::empty();
    unlock();
    return ret;
}

inline size_t BlockList::size() const
{
    lock();
    size_t ret = Parent::size();
    unlock();
    return ret;
}

inline void BlockList::push(Block &block)
{
    lock();
    block.use();
    Parent::push_back(&block);
    unlock();
}

inline Block &BlockList::pop()
{
    ASSERTION(Parent::empty() == false);
    lock();
    Block *ret = Parent::front();
    Parent::pop_front();
    ret->release();
    unlock();
    return *ret;
}

inline void BlockList::remove(Block &block)
{
    lock();
    Parent::remove(&block);
    unlock();
    return;
}

}}}

#endif
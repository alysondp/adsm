#ifndef GMAC_MEMORY_OBJECT_IMPL_H_
#define GMAC_MEMORY_OBJECT_IMPL_H_

#include <fstream>
#include <sstream>

#include "Protocol.h"
#include "Block.h"

#include "util/Logger.h"

namespace __impl { namespace memory {

inline Object::Object(hostptr_t addr, size_t size) :
	gmac::util::RWLock("Object"), 
	addr_(addr),
	size_(size),
	valid_(false)
{
#ifdef DEBUG
    id_ = AtomicInc(Object::Id_);
#endif
}

inline Object::~Object()
{
    BlockMap::iterator i;
    lockWrite();
    gmacError_t ret = coherenceOp(&Protocol::deleteBlock);
    ASSERTION(ret == gmacSuccess);
    for(i = blocks_.begin(); i != blocks_.end(); i++) {
        i->second->release();
    }
    blocks_.clear();
    unlock();
}

#ifdef DEBUG
inline unsigned Object::getId() const
{
    return id_;
}

inline unsigned Object::getDumps(protocol::common::Statistic stat)
{
    if (dumps_.find(stat) == dumps_.end()) dumps_[stat] = 0;
    return dumps_[stat];
}
#endif

inline hostptr_t Object::addr() const
{
    // No need for lock -- addr_ is never modified
    return addr_;   
}

inline hostptr_t Object::end() const
{
    // No need for lock -- addr_ and size_ are never modified
    return addr_ + size_;
}

inline ssize_t Object::blockBase(size_t offset) const
{
    return -1 * (offset % blockSize());
}

inline size_t Object::blockEnd(size_t offset) const
{
    if (offset + blockBase(offset) + blockSize() > size_)
        return size_ - offset;
    else
        return size_t(ssize_t(blockSize()) + blockBase(offset));
}

inline size_t Object::blockSize() const
{
    return BlockSize_;
}

inline size_t Object::size() const
{
    // No need for lock -- size_ is never modified
    return size_;
}

inline bool Object::valid() const
{
    // No need for lock -- valid_ is never modified
	return valid_;
}

inline gmacError_t Object::acquire()
{
    lockRead();
	gmacError_t ret = coherenceOp(&Protocol::acquire);
    unlock();
    return ret;
}

#ifdef USE_VM
inline gmacError_t Object::acquireWithBitmap()
{
    lockRead();
	gmacError_t ret = coherenceOp(&Protocol::acquireWithBitmap);
    unlock();
    return ret;
}
#endif

inline gmacError_t Object::dump(std::ostream &out, protocol::common::Statistic stat)
{
#ifdef DEBUG
    lockWrite();
    std::ostringstream oss;
    oss << (void *) addr();
    out << oss.str() << " ";
	gmacError_t ret = forEachBlock(&Block::dump, out, stat);
    out << std::endl;
    unlock();
    if (dumps_.find(stat) == dumps_.end()) dumps_[stat] = 0;
    dumps_[stat]++;
#else
    gmacError_t ret = gmacSuccess;
#endif
    return ret;
}

template <typename P1, typename P2>
gmacError_t
Object::forEachBlock(gmacError_t (Block::*f)(P1 &, P2), P1 &p1, P2 p2)
{
	gmacError_t ret = gmacSuccess;
	BlockMap::iterator i;
	for(i = blocks_.begin(); i != blocks_.end(); i++) {
		ret = (i->second->*f)(p1, p2);
	}
	return ret;
}

inline gmacError_t Object::toHost()
{
	lockRead();
    gmacError_t ret= coherenceOp(&Protocol::toHost);
    unlock();
    return ret;
}

inline gmacError_t Object::toAccelerator()
{
    lockRead();
	gmacError_t ret = coherenceOp(&Protocol::toAccelerator);
    unlock();
    return ret;
}

inline gmacError_t Object::signalRead(hostptr_t addr)
{
	gmacError_t ret = gmacSuccess;
	lockRead();
	BlockMap::const_iterator i = blocks_.upper_bound(addr);
	if(i == blocks_.end()) ret = gmacErrorInvalidValue;
	else if(i->second->addr() > addr) ret = gmacErrorInvalidValue;
	else ret = i->second->signalRead(addr);
	unlock();
	return ret;
}

inline gmacError_t Object::signalWrite(hostptr_t addr)
{
	gmacError_t ret = gmacSuccess;
	lockRead();
	BlockMap::const_iterator i = blocks_.upper_bound(addr);
	if(i == blocks_.end()) ret = gmacErrorInvalidValue;
	else if(i->second->addr() > addr) ret = gmacErrorInvalidValue;
	else ret = i->second->signalWrite(addr);
	unlock();
	return ret;
}

inline gmacError_t Object::copyToBuffer(core::IOBuffer &buffer, size_t size, 
									  size_t bufferOffset, size_t objectOffset)
{
    lockRead();
	gmacError_t ret = memoryOp(&Protocol::copyToBuffer, buffer, size, 
        bufferOffset, objectOffset);
    unlock();
    return ret;
}

inline gmacError_t Object::copyFromBuffer(core::IOBuffer &buffer, size_t size, 
										size_t bufferOffset, size_t objectOffset)
{
    lockRead();
	gmacError_t ret = memoryOp(&Protocol::copyFromBuffer, buffer, size, 
        bufferOffset, objectOffset);
    unlock();
    return ret;
}

}}

#endif

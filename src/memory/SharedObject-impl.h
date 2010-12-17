#ifndef GMAC_MEMORY_SHAREDOBJECT_IMPL_H_
#define GMAC_MEMORY_SHAREDOBJECT_IMPL_H_

#include "memory/SharedBlock.h"

namespace __impl { namespace memory {

template<typename T>
inline 
accptr_t SharedObject<T>::allocAcceleratorMemory(core::Mode &mode, size_t size)
{
    accptr_t acceleratorAddr;
    // Allocate accelerator memory
    gmacError_t ret = 
		mode.malloc(&acceleratorAddr, size, unsigned(paramPageSize));
	if(ret == gmacSuccess) {
#ifdef USE_VM
        vm::Bitmap &bitmap = mode.dirtyBitmap();
        bitmap.newRange(acceleratorAddr, size);
#endif
        return acceleratorAddr;
    } else {
        return NULL;
    }
}

template<typename T>
inline 
gmacError_t SharedObject<T>::repopulateBlocks(accptr_t accPtr, core::Mode &mode)
{
    // Repopulate the block-set
    hostptr_t mark = addr_;
    size_t offset = 0;
    for(BlockMap::iterator i = blocks_.begin(); i != blocks_.end(); i++) {
        SharedBlock<T> &oldBlock = *dynamic_cast<SharedBlock<T> *>(i->second);
        SharedBlock<T> *newBlock = new SharedBlock<T>(oldBlock.getProtocol(), mode,
                                                      addr_   + offset,
                                                      shadow_ + offset,
                                                      accPtr  + offset,
                                                      oldBlock.size(), oldBlock.state());

        i->second = newBlock;

        offset += oldBlock.size();

        // Decrement reference count
        oldBlock.release();
    }

    return gmacSuccess;
}

template<typename T>
inline SharedObject<T>::SharedObject(Protocol &protocol, core::Mode &owner, hostptr_t hostAddr, size_t size, T init) :
    Object(hostAddr, size),
	owner_(&owner)
{
	// Allocate memory (if necessary)
    if(hostAddr == NULL) {
        addr_ = Memory::map(NULL, size, GMAC_PROT_READWRITE);
        if(addr_ == NULL) return;
    }
    else {
        addr_ = hostAddr;
    }

    // Allocate accelerator memory
    acceleratorAddr_ = allocAcceleratorMemory(owner, size);
    valid_ = (acceleratorAddr_ != NULL);

    if (valid_) {
        // Create a shadow mapping for the host memory
        // TODO: check address
        shadow_ = hostptr_t(Memory::shadow(addr_, size_));
        // Populate the block-set
        hostptr_t mark = addr_;
        size_t offset = 0;
        while(size > 0) {
            size_t blockSize = (size > paramPageSize) ? paramPageSize : size;
            mark += blockSize;
            blocks_.insert(BlockMap::value_type(mark, 
                        new SharedBlock<T>(protocol, owner, addr_ + offset, shadow_ + offset, acceleratorAddr_ + offset, blockSize, init)));
            size -= blockSize;
            offset += blockSize;
        }
        TRACE(LOCAL, "Creating Shared Object @ %p : shadow @ %p : accelerator @ %p) ", addr_, shadow_, (void *) acceleratorAddr_);
    }
}


template<typename T>
inline SharedObject<T>::~SharedObject()
{
	// If the object creation failed, this address will be NULL
    if(acceleratorAddr_ != NULL) owner_->free(acceleratorAddr_);
    Memory::unshadow(shadow_, size_);
#ifdef USE_VM
    vm::Bitmap &bitmap = owner_->dirtyBitmap();
    bitmap.removeRange(acceleratorAddr_, size_);
#endif
    TRACE(LOCAL, "Destroying Shared Object @ %p", addr_);
}

template<typename T>
inline accptr_t SharedObject<T>::acceleratorAddr(const hostptr_t addr) const
{
    accptr_t ret = NULL;
    lockRead();
    if(acceleratorAddr_ != NULL) {
        size_t offset = addr - addr_;
        ret = acceleratorAddr_ + offset;
    }
    unlock();
    return ret;
}

template<typename T>
inline core::Mode &SharedObject<T>::owner(const hostptr_t addr) const
{
    lockRead();
    core::Mode &ret = *owner_;
    unlock();
    return ret;
}

template<typename T>
inline gmacError_t SharedObject<T>::addOwner(core::Mode &owner)
{
	return gmacErrorUnknown; // This kind of objects only accepts one owner
}

template<typename T>
inline
gmacError_t SharedObject<T>::removeOwner(const core::Mode &owner)
{
    lockWrite();
    if(owner_ == &owner) {
        TRACE(LOCAL, "Shared Object @ %p is going orphan", addr_);
        if(acceleratorAddr_ != NULL) {
            gmacError_t ret = coherenceOp(&Protocol::unmapFromAccelerator);
            ASSERTION(ret == gmacSuccess);
            owner_->free(acceleratorAddr_);
        }
        acceleratorAddr_ = NULL;
        owner_ = NULL;
        // Put myself in the orphan map
        Map::insertOrphan(*this);
    }
    unlock();
	return gmacSuccess;
}

template<typename T>
inline
gmacError_t SharedObject<T>::unmapFromAccelerator()
{
    lockWrite();
    // Remove blocks from the coherency domain
    gmacError_t ret = coherenceOp(&Protocol::unmapFromAccelerator);
    // Free accelerator memory
    CFATAL(owner_->free(acceleratorAddr_) == gmacSuccess);
    unlock();
    return ret;
}


template<typename T>
inline gmacError_t SharedObject<T>::mapToAccelerator()
{
    lockWrite();
    gmacError_t ret;

    // Allocate accelerator memory in the new mode
    accptr_t newAcceleratorAddr = allocAcceleratorMemory(*owner_, size_);
    valid_ = (newAcceleratorAddr != NULL);

    if (valid_) {
        acceleratorAddr_ = newAcceleratorAddr;
        // Recreate accelerator blocks
        repopulateBlocks(acceleratorAddr_, *owner_);
        // Add blocks to the coherency domain
        ret = coherenceOp(&Protocol::mapToAccelerator);
    }
    else {
        ret = gmacErrorMemoryAllocation;
    }

    unlock();
	return ret;
}

}}

#endif

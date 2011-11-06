#include "config/config.h"

#include "core/address_space.h"

#include "Memory.h"
#include "object.h"

namespace __impl { namespace memory {

#ifdef DEBUG
Atomic object::Id_ = 0;
#endif

object::~object()
{
    BlockMap::iterator i;
    lockWrite();
    gmacError_t ret = coherenceOp(&Protocol::deleteBlock);
    ASSERTION(ret == gmacSuccess);
    for(i = blocks_.begin(); i != blocks_.end(); i++) {
        i->second->decRef();
    }
    blocks_.clear();
    unlock();
}

object::BlockMap::const_iterator
object::firstBlock(size_t objectOffset, size_t &blockOffset) const
{
    BlockMap::const_iterator i = blocks_.begin();
    if(i == blocks_.end()) return i;
    while(objectOffset >= i->second->size()) {
        objectOffset -= i->second->size();
        i++;
        if(i == blocks_.end()) return i;
    }
    blockOffset = objectOffset;
    return i;
}

gmacError_t
object::coherenceOp(gmacError_t (Protocol::*f)(Block &))
{
    gmacError_t ret = gmacSuccess;
    BlockMap::const_iterator i;
    for(i = blocks_.begin(); i != blocks_.end(); i++) {
        ret = i->second->coherenceOp(f);
        if(ret != gmacSuccess) break;
    }
    return ret;
}

gmacError_t
object::to_io_device(hal::device_output &output, size_t objOff, size_t count)
{
    hal::event_t event;
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    size_t off = 0;
    BlockMap::const_iterator i = firstBlock(objOff, blockOffset);
    for(; i != blocks_.end() && count > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = count < blockSize? count: blockSize;
        event = Block::device_op(&Protocol::to_io_device, output,
                                                          block, blockOffset,
                                                          blockSize, ret);
        //block.memoryOp(op, buffer, blockSize, bufferOffset, blockOffset);
        blockOffset = 0;
        off   += blockSize;
        count -= blockSize;
    }
    return ret;
}

gmacError_t
object::from_io_device(size_t objOff, hal::device_input &input, size_t count)
{
    hal::event_t event;
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    size_t off = 0;
    BlockMap::const_iterator i = firstBlock(objOff, blockOffset);
    for(; i != blocks_.end() && count > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = count < blockSize? count: blockSize;
        event = Block::device_op(&Protocol::from_io_device, block, blockOffset,
                                                            input,
                                                            blockSize, ret);
        //block.memoryOp(op, buffer, blockSize, bufferOffset, blockOffset);
        blockOffset = 0;
        off   += blockSize;
        count -= blockSize;
    }
    return ret;

}

#if 0
gmacError_t object::memoryOp(Protocol::MemoryOp op,
                             core::io_buffer &buffer, size_t size, size_t bufferOffset, size_t objectOffset)
{
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    BlockMap::const_iterator i = firstBlock(objectOffset, blockOffset);
    for(; i != blocks_.end() && size > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = size < blockSize? size: blockSize;
        buffer.wait();
        ret = block.memoryOp(op, buffer, blockSize, bufferOffset, blockOffset);
        blockOffset = 0;
        bufferOffset += blockSize;
        size -= blockSize;
    }
    return ret;
}
#endif

gmacError_t object::memset(size_t offset, int v, size_t size)
{
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    BlockMap::const_iterator i = firstBlock(offset, blockOffset);
    for(; i != blocks_.end() && size > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = size < blockSize? size: blockSize;
        ret = block.memset(v, blockSize, blockOffset);
        blockOffset = 0;
        size -= blockSize;
    }
    return ret;
}

gmacError_t
object::memcpyToObject(size_t objOff, const hostptr_t src, size_t size)
{
    hal::event_t event;
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    size_t off = 0;
    BlockMap::const_iterator i = firstBlock(objOff, blockOffset);
    for(; i != blocks_.end() && size > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = size < blockSize? size: blockSize;
        event = Block::copy_op(&Protocol::copyToBlock, block, blockOffset,
                                                     src + off,
                                                     blockSize, ret);
        //block.memoryOp(op, buffer, blockSize, bufferOffset, blockOffset);
        blockOffset = 0;
        off  += blockSize;
        size -= blockSize;
    }
    return ret;

#if 0
    trace::EnterCurrentFunction();
    gmacError_t ret = gmacSuccess;

#if 0
    // We need to I/O buffers to double-buffer the copy
    core::io_buffer *active;
    core::io_buffer *passive;
#endif

    // Control variables
    size_t left = size;

    // Adjust the first copy to deal with a single block
    size_t copySize = size < blockEnd(objOff)? size: blockEnd(objOff);

#if 0
    size_t bufSize = size < blockSize()? size: blockSize();

    active = owner().create_io_buffer(bufSize, GMAC_PROT_WRITE);
    ASSERTION(bufSize >= copySize);

    if (copySize < size) {
        passive = owner().create_io_buffer(bufSize, GMAC_PROT_WRITE);
    } else {
        passive = NULL;
    }

    // Copy the data to the first block
    ::memcpy(active->addr(), src, copySize);
#endif

    hostptr_t ptr = src;
    while(left > 0) {
        // We do not need for the active buffer to be full because ::memcpy() is
        // a synchronous call
        ret = copyFromBuffer(*active, copySize, 0, objOff);
        ASSERTION(ret == gmacSuccess);
        //if (ret != gmacSuccess) return ret;
        ptr       += copySize;
        left      -= copySize;
        objOff += copySize;
        if(left > 0) {
            // Start copying data from host memory to the passive I/O buffer
            copySize = (left < passive->size()) ? left : passive->size();
            ASSERTION(bufSize >= copySize);
            passive->wait(); // Avoid overwritten a buffer that is already in use
            ::memcpy(passive->addr(), ptr, copySize);
        }
        // Swap buffers
        core::io_buffer *tmp = active;
        active = passive;
        passive = tmp;
    }

#if 0
    // Clean up buffers after they are idle
    if (passive != NULL) {
        passive->wait();
        owner().destroy_io_buffer(*passive);
    }
    if (active  != NULL) {
        active->wait();
        owner().destroy_io_buffer(*active);
    }
#endif

    trace::ExitCurrentFunction();
    return ret;
#endif
}

gmacError_t
object::memcpyObjectToObject(object &dstObj, size_t dstOffset, size_t srcOffset, size_t size)
{
    trace::EnterCurrentFunction();
    gmacError_t ret = gmacSuccess;

    hal::event_t event;

#if 0
    hostptr_t dstPtr = dstObj.addr() + dstOffset;
    hostptr_t srcPtr = addr() + srcOffset;

        accptr_t dstPtr = dstObj.get_device_addr(dstOwner, dstObj.addr() + dstOffset);
        accptr_t srcPtr = get_device_addr(srcOwner, addr() + srcOffset);

        lockWrite();
        dstObj.lockWrite();
#endif
    size_t dummyOffset = 0;
    BlockMap::const_iterator i = firstBlock(srcOffset, dummyOffset);
    TRACE(LOCAL, "FP: %p "FMT_SIZE, dstObj.addr() + dstOffset, size);
    BlockMap::const_iterator j = dstObj.firstBlock(dstOffset, dummyOffset);
    TRACE(LOCAL, "FP: %p vs %p "FMT_SIZE, j->second->addr(), dstObj.addr() + dstOffset, size);
    size_t left = size;
    while (left > 0) {
        size_t copySize = left < dstObj.blockEnd(dstOffset)? left: dstObj.blockEnd(dstOffset);
        // Single copy from the source to fill the buffer
        if (copySize <= blockEnd(srcOffset)) {
            TRACE(LOCAL, "FP: Copying1: "FMT_SIZE" bytes", copySize);
            event = Block::copy_op(&Protocol::copyBlockToBlock,
                                  *j->second, dstOffset % blockSize(),
                                  *i->second, srcOffset % blockSize(), copySize, ret);
            ASSERTION(ret == gmacSuccess);
            i++;
        }
        else { // Two copies from the source to fill the buffer
            TRACE(LOCAL, "FP: Copying2: "FMT_SIZE" bytes", copySize);
            size_t firstCopySize = blockEnd(srcOffset);
            size_t secondCopySize = copySize - firstCopySize;

            event = Block::copy_op(&Protocol::copyBlockToBlock,
                                  *j->second,
                                  dstOffset % blockSize(),
                                  *i->second,
                                  srcOffset % blockSize(),
                                  firstCopySize, ret);
            ASSERTION(ret == gmacSuccess);
            i++;
            event = Block::copy_op(&Protocol::copyBlockToBlock,
                                  *j->second,
                                  (dstOffset + firstCopySize) % blockSize(),
                                  *i->second,
                                  (srcOffset + firstCopySize) % blockSize(),
                                  secondCopySize, ret);
            ASSERTION(ret == gmacSuccess);
        }
        left -= copySize;
        dstOffset += copySize;
        srcOffset += copySize;
        j++;
    }

    if (event.is_valid()) {
        ret = event.sync();
    }

#if 0
        dstObj.unlock();
        unlock();
#endif

    trace::ExitCurrentFunction();
    return ret;

#if 0
    // We need to I/O buffers to double-buffer the copy
    core::io_buffer *active;
    core::io_buffer *passive;

    // Control variables
    size_t left = size;

    // Adjust the first copy to deal with a single block
    size_t copySize = size < dstObj.blockEnd(dstOffset)? size: dstObj.blockEnd(dstOffset);

    size_t bufSize = size < dstObj.blockSize()? size: dstObj.blockSize();
    active = owner().create_io_buffer(bufSize, GMAC_PROT_READWRITE);
    ASSERTION(bufSize >= copySize);

    if (copySize < size) {
        passive = owner().create_io_buffer(bufSize, GMAC_PROT_READWRITE);
    } else {
        passive = NULL;
    }

    // Single copy from the source to fill the buffer
    if (copySize <= blockEnd(srcOffset)) {
        ret = copyToBuffer(*active, copySize, 0, srcOffset);
        ASSERTION(ret == gmacSuccess);
    }
    else { // Two copies from the source to fill the buffer
        size_t firstCopySize = blockEnd(srcOffset);
        size_t secondCopySize = copySize - firstCopySize;
        ASSERTION(bufSize >= firstCopySize + secondCopySize);

        ret = copyToBuffer(*active, firstCopySize, 0, srcOffset);
        ASSERTION(ret == gmacSuccess);
        ret = copyToBuffer(*active, secondCopySize, firstCopySize, srcOffset + firstCopySize);
        ASSERTION(ret == gmacSuccess);
    }

    // Copy first chunk of data
    while(left > 0) {
        active->wait(); // Wait for the active buffer to be full
        ret = dstObj.copyFromBuffer(*active, copySize, 0, dstOffset);
        if(ret != gmacSuccess) {
            trace::ExitCurrentFunction();
            return ret;
        }
        left -= copySize;
        srcOffset += copySize;
        dstOffset += copySize;
        if(left > 0) {
            copySize = (left < dstObj.blockSize()) ? left: dstObj.blockSize();
            ASSERTION(bufSize >= copySize);
            // Avoid overwritting a buffer that is already in use
            passive->wait();

            // Request the next copy
            // Single copy from the source to fill the buffer
            if (copySize <= blockEnd(srcOffset)) {
                ret = copyToBuffer(*passive, copySize, 0, srcOffset);
                ASSERTION(ret == gmacSuccess);
            }
            else { // Two copies from the source to fill the buffer
                size_t firstCopySize = blockEnd(srcOffset);
                size_t secondCopySize = copySize - firstCopySize;
                ASSERTION(bufSize >= firstCopySize + secondCopySize);

                ret = copyToBuffer(*passive, firstCopySize, 0, srcOffset);
                ASSERTION(ret == gmacSuccess);
                ret = copyToBuffer(*passive, secondCopySize, firstCopySize, srcOffset + firstCopySize);
                ASSERTION(ret == gmacSuccess);
            }

            // Swap buffers
            core::io_buffer *tmp = active;
            active = passive;
            passive = tmp;
        }
    }
    // Clean up buffers after they are idle
    if (passive != NULL) {
        passive->wait();
        owner().destroy_io_buffer(*passive);
    }
    if (active  != NULL) {
        active->wait();
        owner().destroy_io_buffer(*active);
    }

    trace::ExitCurrentFunction();
    return ret;
#endif
}

gmacError_t
object::memcpyFromObject(hostptr_t dst, size_t objOff, size_t size)
{
    hal::event_t event;
    gmacError_t ret = gmacSuccess;
    size_t blockOffset = 0;
    size_t off = 0;
    BlockMap::const_iterator i = firstBlock(objOff, blockOffset);
    for(; i != blocks_.end() && size > 0; i++) {
        Block &block = *i->second;
        size_t blockSize = block.size() - blockOffset;
        blockSize = size < blockSize? size: blockSize;
        event = Block::copy_op(&Protocol::copyFromBlock, dst + off,
                                                         block, blockOffset,
                                                         blockSize, ret);
        //block.memoryOp(op, buffer, blockSize, bufferOffset, blockOffset);
        blockOffset = 0;
        off  += blockSize;
        size -= blockSize;
    }
    return ret;

#if 0
    trace::EnterCurrentFunction();
    gmacError_t ret = gmacSuccess;

    // We need to I/O buffers to double-buffer the copy
    core::io_buffer *active;
    core::io_buffer *passive;

    // Control variables
    size_t left = size;

    // Adjust the first copy to deal with a single block
    size_t copySize = size < blockEnd(objOff)? size: blockEnd(objOff);

    size_t bufSize = size < blockSize()? size: blockSize();
    active = owner().create_io_buffer(bufSize, GMAC_PROT_READ);
    ASSERTION(bufSize >= copySize);

    if (copySize < size) {
        passive = owner().create_io_buffer(bufSize, GMAC_PROT_READ);
    } else {
        passive = NULL;
    }

    // Copy the data to the first block
    ret = copyToBuffer(*active, copySize, 0, objOff);
    ASSERTION(ret == gmacSuccess);
    //if(ret != gmacSuccess) return ret;
    while(left > 0) {
        // Save values to use when copying the buffer to host memory
        size_t previousCopySize = copySize;
        left      -= copySize;
        objOff += copySize;
        if(left > 0) {
            // Start copying data from host memory to the passive I/O buffer
            copySize = (left < passive->size()) ? left : passive->size();
            ASSERTION(bufSize >= copySize);
            // No need to wait for the buffer, because ::memcpy is a
            // synchronous call
            ret = copyToBuffer(*passive, copySize, 0, objOff);
            ASSERTION(ret == gmacSuccess);
        }
        // Wait for the active buffer to be full
        active->wait();
        // Copy the active buffer to host
        ::memcpy(dst, active->addr(), previousCopySize);
        dst += previousCopySize;

        // Swap buffers
        core::io_buffer *tmp = active;
        active = passive;
        passive = tmp;
    }
    // No need to wait for the buffers because we waited for them before ::memcpy
    if (passive != NULL) owner().destroy_io_buffer(*passive);
    if (active  != NULL) owner().destroy_io_buffer(*active);

    trace::ExitCurrentFunction();
    return ret;
#endif
}

gmacError_t
object::signalRead(hostptr_t addr)
{
    gmacError_t ret = gmacSuccess;
    lockRead();
    /// \todo is this validate necessary?
    //validate();
    BlockMap::const_iterator i = blocks_.upper_bound(addr);
    if(i == blocks_.end()) ret = gmacErrorInvalidValue;
    else if(i->second->addr() > addr) ret = gmacErrorInvalidValue;
    else ret = i->second->signalRead(addr);
    unlock();
    return ret;
}

gmacError_t
object::signalWrite(hostptr_t addr)
{
    gmacError_t ret = gmacSuccess;
    lockRead();
    modifiedObject();
    BlockMap::const_iterator i = blocks_.upper_bound(addr);
    if(i == blocks_.end()) ret = gmacErrorInvalidValue;
    else if(i->second->addr() > addr) ret = gmacErrorInvalidValue;
    else ret = i->second->signalWrite(addr);
    unlock();
    return ret;
}

gmacError_t
object::dump(std::ostream &out, protocol::common::Statistic stat)
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

}}

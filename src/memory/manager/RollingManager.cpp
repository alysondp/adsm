#include "RollingManager.h"
#include "os/Memory.h"

#include "config/params.h"

#include <kernel/Context.h>

#include <unistd.h>
#include <malloc.h>

#include <typeinfo>

namespace gmac { namespace memory { namespace manager {

RollingBuffer::RollingBuffer() :
    _lock(paraver::rollingBuffer),
    _max(0)
{}

void RollingManager::waitForWrite(void *addr, size_t size)
{
    writeMutex.lock();
    if(writeBuffer) {
        RollingBlock *r = regionRolling[Context::current()]->front();
        r->sync();
        munlock(writeBuffer, writeBufferSize);
    }
    writeBuffer = addr;
    writeBufferSize = size;
    writeMutex.unlock();
}


void RollingManager::writeBack()
{
    RollingBlock *r = regionRolling[Context::current()]->pop();
    waitForWrite(r->start(), r->size());
    mlock(writeBuffer, writeBufferSize);
    assert(r->copyToDevice() == gmacSuccess);
    r->readOnly();
}


PARAM_REGISTER(paramLineSize,
               size_t,
               1024,
               "GMAC_LINESIZE",
               PARAM_NONZERO);

PARAM_REGISTER(paramLruDelta,
               size_t,
               2,
               "GMAC_LRUDELTA",
               PARAM_NONZERO);

RollingManager::RollingManager() :
    Handler(),
    lineSize(0),
    lruDelta(0),
    writeMutex(paraver::writeMutex),
    writeBuffer(NULL),
    writeBufferSize(0)
{
    lineSize = paramLineSize;
    lruDelta = paramLruDelta;
    TRACE("Using %d as Line Size", lineSize);
    TRACE("Using %d as LRU Delta Size", lruDelta);
}

RollingManager::~RollingManager()
{
    std::map<Context *, RollingBuffer *>::iterator r;
    for (r = regionRolling.begin(); r != regionRolling.end(); r++) {
        delete r->second;
    }
}

void *RollingManager::alloc(void *addr, size_t size)
{
    void *cpuAddr = NULL;
    if((cpuAddr = hostMap(addr, size, PROT_NONE)) == NULL)
        return NULL;

    insertVirtual(cpuAddr, addr, size);
    if (!regionRolling[Context::current()]) {
        regionRolling[Context::current()] = new RollingBuffer();
    }
    regionRolling[Context::current()]->inc(lruDelta);
    insert(new RollingRegion(*this, cpuAddr, size, pageTable().getPageSize()));

    return cpuAddr;
}


void RollingManager::release(void *addr)
{
    Region *reg = remove(addr);
    removeVirtual(reg->start(), reg->size());
    if(reg->owner() == Context::current()) {
        hostUnmap(addr, reg->size()); // Global mappings do not have a shadow copy in system memory
        TRACE("Deleting Region %p\n", addr);
        delete reg;
    }
    regionRolling[Context::current()]->dec(lruDelta);
    TRACE("Released %p", addr);
}


void RollingManager::flush()
{
    TRACE("RollingManager Flush Starts");
    waitForWrite();
    while(regionRolling[Context::current()]->empty() == false) {
        RollingBlock *r = regionRolling[Context::current()]->pop();
        assert(r->copyToDevice() == gmacSuccess);
        r->readOnly();
        TRACE("Flush to Device %p", r->start());
    }

    TRACE("RollingManager Flush Ends");
}

void RollingManager::flush(const RegionVector & regions)
{
    // If no dependencies, a global flush is assumed
    if (regions.size() == 0) {
        flush();
    } else {
        TRACE("RollingManager Flush Starts");
        waitForWrite();
        RollingBuffer * buffer = regionRolling[Context::current()];
        size_t blocks = buffer->size();

        for(int i = 0; i < blocks; i++) {
            RollingBlock *r = regionRolling[Context::current()]->pop();
            // Check if we have to flush
            if (std::find(regions.begin(), regions.end(), &r->getParent()) == regions.end()) {
                buffer->push(r);
                continue;
            }

            assert(r->copyToDevice() == gmacSuccess);
            r->readOnly();
            TRACE("Flush to Device %p", r->start());
        }

        TRACE("RollingManager Flush Ends");
    }
}

void RollingManager::invalidate()
{
    TRACE("RollingManager Invalidation Starts");
    memory::Map::iterator i;
    current()->lock();
    for(i = current()->begin(); i != current()->end(); i++) {
        Region *r = i->second;
        assert(typeid(*r) == typeid(RollingRegion));
        dynamic_cast<RollingRegion *>(r)->invalidate();
    }
    current()->unlock();
    //gmac::Context::current()->flush();
    gmac::Context::current()->invalidate();
    TRACE("RollingManager Invalidation Ends");
}

void RollingManager::invalidate(const RegionVector & regions)
{
    // If no dependencies, a global invalidation is assumed
    if (regions.size() == 0) {
        invalidate();
    } else {
        TRACE("RollingManager Invalidation Starts");
        RegionVector::const_iterator i;
        current()->lock();
        for(i = regions.begin(); i != regions.end(); i++) {
            Region *r = *i;
            assert(typeid(*r) == typeid(RollingRegion));
            dynamic_cast<RollingRegion *>(r)->invalidate();
        }
        current()->unlock();
        //gmac::Context::current()->flush();
        gmac::Context::current()->invalidate();
        TRACE("RollingManager Invalidation Ends");
    }
}

Context *RollingManager::owner(const void *addr)
{
    RollingRegion *reg= get(addr);
    if(reg == NULL) return NULL;
    return reg->owner();
}

void RollingManager::invalidate(const void *addr, size_t size)
{
    RollingRegion *reg = get(addr);
    assert(reg != NULL);
    assert(reg->end() >= (void *)((addr_t)addr + size));
    reg->invalidate(addr, size);
}

void RollingManager::flush(const void *addr, size_t size)
{
    RollingRegion *reg = get(addr);
    assert(reg != NULL);
    assert(reg->end() >= (void *)((addr_t)addr + size));
    reg->flush(addr, size);
}

// Handler Interface

bool RollingManager::read(void *addr)
{
    RollingRegion *root = get(addr);
    if(root == NULL) return false;
    ProtRegion *region = root->find(addr);
    assert(region != NULL);
    assert(region->present() == false);
    region->readWrite();
    if(current()->pageTable().dirty(addr)) {
        assert(region->copyToHost() == gmacSuccess);
        current()->pageTable().clear(addr);
    }
    region->readOnly();
    return true;
}


bool RollingManager::write(void *addr)
{
    RollingRegion *root = get(addr);
    if(root == NULL) return false;
    ProtRegion *region = root->find(addr);
    assert(region != NULL);
    assert(region->dirty() == false);
    if (!regionRolling[Context::current()]) {
        regionRolling[Context::current()] = new RollingBuffer();
    }
    while(regionRolling[Context::current()]->overflows()) writeBack();
    region->readWrite();
    if(region->present() == false && current()->pageTable().dirty(addr)) {
        assert(region->copyToHost() == gmacSuccess);
        current()->pageTable().clear(addr);
    }
    regionRolling[Context::current()]->push(
        dynamic_cast<RollingBlock *>(region));
    return true;
}


}}}

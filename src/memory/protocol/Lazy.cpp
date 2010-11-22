#include "Lazy.h"

#include "core/IOBuffer.h"

#include "config/config.h"

#include "memory/Memory.h"
#include "memory/StateBlock.h"

#include "trace/Tracer.h"


#if defined(__GNUC__)
#define MIN std::min
#elif defined(_MSC_VER)
#define MIN min
#endif

namespace __impl { namespace memory { namespace protocol {


LazyBase::LazyBase(unsigned limit) :
    gmac::util::RWLock("Lazy"),
    limit_(limit)
{
}

LazyBase::~LazyBase()
{

}

LazyBase::State LazyBase::state(GmacProtection prot) const
{
	switch(prot) {
		case GMAC_PROT_NONE: 
			return Invalid;
		case GMAC_PROT_READ:
			return ReadOnly;
		case GMAC_PROT_WRITE:
		case GMAC_PROT_READWRITE:
			return Dirty;
	}
	return Dirty;
}


void LazyBase::deleteObject(Object &obj)
{
    obj.release();
}

bool LazyBase::needUpdate(const Block &b) const
{
    const StateBlock<State> &block = dynamic_cast<const StateBlock<State> &>(b);
    switch(block.state()) {        
        case Dirty:
        case HostOnly:
            return false;
        case ReadOnly:
        case Invalid:
            return true;
    }
    return false;
}

gmacError_t LazyBase::signalRead(Block &b)
{
    trace::EnterCurrentFunction();
	StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    gmacError_t ret = gmacSuccess;

    if(block.state() == HostOnly) {
        WARNING("Signal on HostOnly block - Changing protection and continuing");
        Memory::protect(block.addr(), block.size(), GMAC_PROT_READWRITE);
        goto exit_func;
    }


    if (block.state() != Invalid) {
        goto exit_func; // Somebody already fixed it
    }
#ifdef USE_VM
    gmac::core::Mode &mode = gmac::core::Mode::current();
    vm::Bitmap &bitmap = mode.dirtyBitmap();
    if (bitmap.checkAndClear(obj.device(block->addr()))) {
#endif
        ret = block.toHost();
        if(ret != gmacSuccess) goto exit_func;
        Memory::protect(block.addr(), block.size(), GMAC_PROT_READ);
#ifdef USE_VM
    }
#endif
    block.state(ReadOnly);

exit_func:
    trace::ExitCurrentFunction();
    return ret;
}

gmacError_t LazyBase::signalWrite(Block &b)
{
    trace::EnterCurrentFunction();
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    gmacError_t ret = gmacSuccess;
    switch(block.state()) {
        case Dirty:            
            goto exit_func; // Somebody already fixed it
        case Invalid:          
#ifdef USE_VM
            vm::Bitmap &bitmap = mode.dirtyBitmap();
            if (bitmap.checkAndClear(obj.device(block->addr()))) {
#endif
                ret = block.toHost();
                if(ret != gmacSuccess) goto exit_func;
#ifdef USE_VM
            }
#endif
        case HostOnly:
            WARNING("Signal on HostOnly block - Changing protection and continuing");
        case ReadOnly:
			Memory::protect(block.addr(), block.size(), GMAC_PROT_READWRITE);
            break;
    }
    block.state(Dirty);
    addDirty(block);
    TRACE(LOCAL,"Setting block %p to dirty state", block.addr());
    //ret = addDirty(block);
exit_func:
    trace::ExitCurrentFunction();
    return ret;
}

gmacError_t LazyBase::acquire(Block &b)
{
    gmacError_t ret = gmacSuccess;
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    switch(block.state()) {
        case Invalid:
        case ReadOnly:
			if(Memory::protect(block.addr(), block.size(), GMAC_PROT_NONE) < 0)
                FATAL("Unable to set memory permissions");
            block.state(Invalid);
            break;
        case Dirty:
            FATAL("Block in incongruent state in acquire: %p", block.addr());
            break;
        case HostOnly:
            break;
    }
	return ret;
}

#ifdef USE_VM
gmacError_t LazyBase::acquireWithBitmap(const Object &obj)
{
    core::Mode &mode = core::Mode::current();
    vm::Bitmap &bitmap = mode.dirtyBitmap();
    gmacError_t ret = gmacSuccess;
    StateObject<State> &object = dynamic_cast<StateObject<State> &>(obj);
    StateObject<State>::SystemMap &map = object.blocks();
    StateObject<State>::SystemMap::iterator i;
    for(i = map.begin(); i != map.end(); i++) {
        SystemBlock<State> *block = i->second;
        block->lock();
        if (bitmap.check(obj.device(block->addr()))) {
            if(Memory::protect(block->addr(), block->size(), GMAC_PROT_NONE) < 0)
                FATAL("Unable to set memory permissions");
            block->state(Invalid);
        } else {
            if(Memory::protect(block->addr(), block->size(), GMAC_PROT_READ) < 0)
                FATAL("Unable to set memory permissions");
            block->state(ReadOnly);
        }
        block->unlock();
    }
    return ret;
}
#endif



gmacError_t LazyBase::remove(Block &b)
{
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    TRACE(LOCAL,"Releasing block %p", block.addr());
    gmacError_t ret = gmacSuccess;
    switch(block.state()) {
        case HostOnly:
        case Dirty:
        case ReadOnly:
            break;
        case Invalid:
            ret = block.toHost();
            if(ret != gmacSuccess) break;
    }
    if(Memory::protect(block.addr(), block.size(), GMAC_PROT_READWRITE) < 0)
                    FATAL("Unable to set memory permissions");
    block.state(HostOnly);
    dbl_.remove(block);
    return ret;
}

void LazyBase::addDirty(Block &block)
{
    dbl_.push(block);
    if(limit_ == unsigned(-1)) return;
    while(dbl_.size() > limit_) {
        Block *b = dbl_.pop();
        b->coherenceOp(&Protocol::release);
    }
    return;
}

gmacError_t LazyBase::release()
{
    while(dbl_.empty() == false) {
        Block *b = dbl_.pop();
        b->coherenceOp(&Protocol::release);
    }
    return gmacSuccess;
}

gmacError_t LazyBase::release(Block &b)
{
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    TRACE(LOCAL,"Releasing block %p", block.addr());
    gmacError_t ret = gmacSuccess;
    switch(block.state()) {
        case Dirty:
            ret = block.toDevice();
            if(ret != gmacSuccess) break;
			if(Memory::protect(block.addr(), block.size(), GMAC_PROT_READ) < 0)
                    FATAL("Unable to set memory permissions");
            block.state(ReadOnly);
            break;
        case Invalid:
        case ReadOnly:
        case HostOnly:
            break;
    }
    return ret;
}

gmacError_t LazyBase::toHost(Block &b)
{
    gmacError_t ret = gmacSuccess;
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    switch(block.state()) {
        case Invalid:
			if(Memory::protect(block.addr(), block.size(), GMAC_PROT_READWRITE) < 0)
                FATAL("Unable to set memory permissions");
            ret = block.toHost();
            if(ret != gmacSuccess) break;
            block.state(ReadOnly);
            break;
        case Dirty:
        case ReadOnly:
        case HostOnly:
            break;
    }
    return ret;
}

gmacError_t LazyBase::toDevice(Block &b)
{
    gmacError_t ret = gmacSuccess;
    StateBlock<State> &block = dynamic_cast<StateBlock<State> &>(b);
    switch(block.state()) {
        case Dirty:
            ret = block.toDevice();
            if(ret != gmacSuccess) break;
			if(Memory::protect(block.addr(), block.size(), GMAC_PROT_READ) < 0)
                FATAL("Unable to set memory permissions");
            block.state(ReadOnly);
        break;
    case Invalid:
    case ReadOnly:
    case HostOnly:
        break;
    }
    return ret;
}

gmacError_t LazyBase::copyToBuffer(const Block &b, core::IOBuffer &buffer, size_t size,
							   unsigned bufferOffset, unsigned objectOffset) const
{
	gmacError_t ret = gmacSuccess;
	const StateBlock<State> &block = dynamic_cast<const StateBlock<State> &>(b);
	switch(block.state()) {
		case Invalid:
			ret = block.copyFromDevice(buffer, size, bufferOffset, objectOffset);
			break;
		case ReadOnly:
		case Dirty:
        case HostOnly:
			ret = block.copyFromHost(buffer, size, bufferOffset, objectOffset);
	}
	return ret;
}

gmacError_t LazyBase::copyFromBuffer(const Block &b, core::IOBuffer &buffer, size_t size, 
							   unsigned bufferOffset, unsigned objectOffset) const
{
	gmacError_t ret = gmacSuccess;
	const StateBlock<State> &block = dynamic_cast<const StateBlock<State> &>(b);
	switch(block.state()) {
		case Invalid:
			ret = block.copyToDevice(buffer, size, bufferOffset, objectOffset);
			break;
		case ReadOnly:
			ret = block.copyToDevice(buffer, size, bufferOffset, objectOffset);
			if(ret != gmacSuccess) break;
			ret = block.copyToHost(buffer, size, bufferOffset, objectOffset);
			break;
		case Dirty:			
        case HostOnly:
			ret = block.copyToHost(buffer, size, bufferOffset, objectOffset);
			break;
	}
	return ret;
}



}}}

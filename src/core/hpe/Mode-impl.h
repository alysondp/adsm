#ifndef GMAC_CORE_HPE_MODE_IMPL_H_
#define GMAC_CORE_HPE_MODE_IMPL_H_

#include "memory/Object.h"

#include "core/hpe/Accelerator.h"
#include "core/hpe/Process.h"
#include "core/hpe/Context.h"

namespace __impl { namespace core { namespace hpe {

inline void ContextMap::add(THREAD_T id, Context *ctx)
{
    lockWrite();
    Parent::insert(Parent::value_type(id, ctx));
    unlock();
}

inline Context *ContextMap::find(THREAD_T id)
{
    lockRead();
    Parent::iterator i = Parent::find(id);
    Context *ret = NULL;
    if(i != end()) ret = i->second;
    unlock();
    return ret;
}

inline void ContextMap::remove(THREAD_T id)
{
    lockWrite();
    Parent::erase(id);
    unlock();
}

inline void ContextMap::clean()
{
    Parent::iterator i;
    lockWrite();
    for(i = begin(); i != end(); i++) delete i->second;
    Parent::clear();
    unlock();
}

inline gmacError_t ContextMap::prepareForCall()
{
    Parent::iterator i;
    gmacError_t ret = gmacSuccess;
    lockRead();
    for(i = begin(); i != end(); i++) {
        ret = i->second->prepareForCall();
        if(ret != gmacSuccess) break;
    }
    unlock();
    return ret;
}

inline gmacError_t ContextMap::waitForCall()
{
    Parent::iterator i;
    gmacError_t ret = gmacSuccess;
    lockRead();
    for(i = begin(); i != end(); i++) {
        ret = i->second->waitForCall();
        if(ret != gmacSuccess) break;
    }
    unlock();
    return ret;
}

inline gmacError_t ContextMap::waitForCall(KernelLaunch &launch)
{
    Parent::iterator i;
    gmacError_t ret = gmacSuccess;
    lockRead();
    for(i = begin(); i != end(); i++) {
        ret = i->second->waitForCall(launch);
        if(ret != gmacSuccess) break;
    }
    unlock();
    return ret;
}

inline
memory::ObjectMap &Mode::getObjectMap()
{
    return map_;
}

inline
const memory::ObjectMap &Mode::getObjectMap() const
{
    return map_;
}


inline void Mode::cleanUpContexts()
{
    contextMap_.clean();
}

inline void Mode::init()
{
    util::Private<Mode>::init(key);
}

inline void Mode::initThread()
{
    key.set(NULL);
}

inline bool
Mode::hasCurrent()
{
    return key.get() != NULL;
}

inline
Accelerator &
Mode::getAccelerator() const
{
    return *acc_;
}

#ifdef USE_VM
inline memory::vm::BitmapShared &
Mode::acceleratorDirtyBitmap()
{
    return acceleratorBitmap_;
}

inline const memory::vm::BitmapShared &
Mode::acceleratorDirtyBitmap() const
{
    return acceleratorBitmap_;
}
#endif

inline gmacError_t 
Mode::releaseObjects()
{
    switchIn();
    releasedObjects_ = true;
    switchOut();
    return error_;
}

inline gmacError_t
Mode::dump(std::string name, memory::protocol::common::Statistic stat, hostptr_t ptr)
{
#ifdef DEBUG
    if (ptr == NULL) {
        map_.dumpObjects(name, stat);
    } else {
        map_.dumpObject(name, stat, ptr);
#if 0
        std::ofstream out(name.str().c_str(), std::ios_base::trunc);
        ASSERTION(out.good());
        memory::Object *obj = getObject(addr);
        ASSERTION(obj != NULL);
        obj->dump(out);
        out.close();
#endif
    }

#endif
    return gmacSuccess;
}

inline gmacError_t 
Mode::acquireObjects()
{
    switchIn();
    releasedObjects_ = false;
    error_ = contextMap_.waitForCall();
    switchOut();
    return error_;
}

inline Process &Mode::process()
{
    return proc_;
}

inline const Process &Mode::process() const
{
    return proc_;
}

inline void
Mode::memInfo(size_t &free, size_t &total)
{
    switchIn();
    acc_->memInfo(free, total);
    switchOut();
}

}}}

#endif

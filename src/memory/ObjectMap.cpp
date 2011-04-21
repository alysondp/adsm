#include "core/Mode.h"

#include "ObjectMap.h"
#include "Object.h"
#include "Protocol.h"

namespace __impl { namespace memory {

Object *ObjectMap::mapFind(const hostptr_t addr, size_t size) const
{
    ObjectMap::const_iterator i;
    Object *ret = NULL;
    lockRead();
    const uint8_t *limit = (const uint8_t *)addr + size;
    i = upper_bound(addr);
    if(i != end() && i->second->addr() <= limit) ret = i->second;
    unlock();
    return ret;
}

ObjectMap::ObjectMap(const char *name) :
    gmac::util::RWLock(name)
{
}

ObjectMap::~ObjectMap()
{
}

size_t ObjectMap::size() const
{
    lockRead();
    size_t ret = Parent::size();
    unlock();
    return ret;
}

bool ObjectMap::insert(Object &obj)
{
    lockWrite();
    std::pair<iterator, bool> ret = Parent::insert(value_type(obj.end(), &obj));
    if(ret.second == true) obj.use();
    unlock();
    return ret.second;
}

bool ObjectMap::remove(Object &obj)
{
    lockWrite();
    iterator i = find(obj.end());
    bool ret = (i != end());
    if(ret == true) {
        obj.release();
        Parent::erase(i);
    }
    unlock();
    return ret;
}

Object *ObjectMap::get(const hostptr_t addr, size_t size) const
{
    Object *ret = NULL;
    ret = mapFind(addr, size);
    if(ret != NULL) ret->use();
    return ret;
}

size_t ObjectMap::memorySize() const
{
    size_t total = 0;
    const_iterator i;
    lockRead();
    for(i = begin(); i != end(); i++) total += i->second->size();
    unlock();
    return total;
}

gmacError_t ObjectMap::forEach(ObjectOp op) const
{
    const_iterator i;
    lockRead();
    for(i = begin(); i != end(); i++) {
        gmacError_t ret = (i->second->*op)();
        if(ret != gmacSuccess) {
            unlock();
            return ret;
        }
    }
    unlock();
    return gmacSuccess;
}

gmacError_t ObjectMap::forEach(ConstObjectOp op) const
{
    const_iterator i;
    lockRead();
    for(i = begin(); i != end(); i++) {
        gmacError_t ret = (i->second->*op)();
        if(ret != gmacSuccess) {
            unlock();
            return ret;
        }
    }
    unlock();
    return gmacSuccess;
}

gmacError_t ObjectMap::forEach(core::Mode &mode, ModeOp op) const
{
    const_iterator i;
    lockRead();
    for(i = begin(); i != end(); i++) {
        gmacError_t ret = (i->second->*op)(mode);
        if(ret != gmacSuccess) {
            unlock();
            return ret;
        }
    }
    unlock();
    return gmacSuccess;
}


}}

#include "Cache.h"

#include "core/Context.h"
#include "core/Mode.h"
#include "memory/Manager.h"

#include "util/Parameter.h"
#include "util/Private.h"

namespace gmac { namespace memory { namespace allocator {

Arena::Arena(size_t objSize) :
    ptr(NULL),
    size(0)
{
    gmacError_t ret = Manager::get()->alloc(&ptr, paramPageSize);
    if(ret != gmacSuccess) return;
    for(size_t s = 0; s < paramPageSize; s += objSize, size++) {
        trace("Arena %p pushes %p (%zd bytes)", this, (void *)((uint8_t *)ptr + s), objSize);
        _objects.push_back((void *)((uint8_t *)ptr + s));
    }
}

Arena::~Arena()
{
    util::Logger::CFatal(_objects.size() == size, "Destroying non-full Arena");
    _objects.clear();
    gmacError_t ret = Manager::get()->free(ptr);
}


Cache::~Cache()
{
    ArenaMap::iterator i;
    for(i = arenas.begin(); i != arenas.end(); i++) {
        delete i->second;
    }
  
}

void *Cache::get()
{
    ArenaMap::iterator i;
    lock();
    for(i = arenas.begin(); i != arenas.end(); i++) {
        if(i->second->empty()) continue;
        trace("Cache %p gets memory from arena %p", this, i->second);
        unlock();
        return i->second->get();
    }
    // There are no free objects in any arena
    Arena *arena = new Arena(objectSize);
    trace("Cache %p creates new arena %p with key %p", this, arena, arena->key());
    arenas.insert(ArenaMap::value_type(arena->key() , arena));
    void *ptr = arena->get();
    unlock();
    return ptr;
}

}}}

#ifndef __MEMORY_ALLOCATOR_CACHE_IPP__
#define __MEMORY_ALLOCATOR_CACHE_IPP__

namespace gmac { namespace memory { namespace allocator {

inline
void *Arena::address() const
{
    return ptr;
}

inline
const ObjectList &Arena::objects() const
{
    return _objects;
}

inline
bool Arena::full() const
{
    return _objects.size() == size;
}

inline
bool Arena::empty() const
{
    return _objects.empty();
}

inline
void *Arena::get()
{
    assertion(_objects.empty() == false);
    void *ret = _objects.front();
    _objects.pop_front();
    trace("Arena %p has %zd available objects", this, _objects.size());
    return ret;
}

inline
void Arena::put(void *obj)
{
    _objects.push_back(obj);
}

inline
Cache::Cache(size_t size) :
    util::Lock("Cache"),
    objectSize(size),
    arenaSize(paramPageSize)
{ }


inline
void Cache::put(void *obj)
{
    lock();
    void *key = (void *)((unsigned long)obj & ~(paramPageSize - 1));
    ArenaMap::const_iterator i;
    i = arenas.find(key);
    CFatal(i != arenas.end(), "Address for invalid arena: %p, %p", obj, key);
    i->second->put(obj);
    unlock();
}

}}}

#endif

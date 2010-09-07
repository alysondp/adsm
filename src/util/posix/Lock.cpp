#include "Lock.h"

#include <string>

namespace gmac { namespace util {

#ifdef PARAVER
const char *ParaverLock::eventName = "Lock";
const char *ParaverLock::exclusiveName = "Exclusive";
unsigned ParaverLock::count = 0;
ParaverLock::LockMap *ParaverLock::map = NULL;
paraver::EventName *ParaverLock::event = NULL;
paraver::StateName *ParaverLock::exclusive = NULL;
#endif

ParaverLock::ParaverLock(const char *name)
{
#ifdef PARAVER
    if(event == NULL)
        event = paraver::Factory<paraver::EventName>::create(eventName);

    if(map == NULL) map = new LockMap();
    LockMap::const_iterator i = map->find(std::string(name));
    if(i == map->end()) {
        id = ++count;
        event->registerType(id, std::string(name));
        map->insert(LockMap::value_type(std::string(name), id));
    }
    else id = i->second;

    if(exclusive == NULL)
        exclusive = paraver::Factory<paraver::StateName>::create(exclusiveName);
#endif
}

Lock::Lock(const char *name) :
    ParaverLock(name)
{
    pthread_mutex_init(&__mutex, NULL);
}

Lock::~Lock()
{
    pthread_mutex_destroy(&__mutex);
}

RWLock::RWLock(const char *name) :
    ParaverLock(name)
{
    pthread_rwlock_init(&__lock, NULL);
}

RWLock::~RWLock()
{
    pthread_rwlock_destroy(&__lock);
}

}}

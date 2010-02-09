#include "Process.h"
#include "Context.h"
#include "Accelerator.h"

#include <debug.h>
#include <gmac/init.h>
#include <memory/Manager.h>

gmac::Process *proc = NULL;

namespace gmac {

SharedMemory::SharedMemory(void *_addr, size_t _size, size_t _count) :
    _addr(_addr),
    _size(_size),
    _count(_count)
{}

ThreadQueue::ThreadQueue() :
    hasContext(paraver::queue)
{}

size_t Process::_totalMemory = 0;


Process::Process() :
    mutex(paraver::process), current(0)
{}

Process::~Process()
{
	TRACE("Cleaning process");
	std::vector<Accelerator *>::iterator a;
	std::list<Context *>::iterator c;
	QueueMap::iterator q;
	mutex.lock();
	for(c = _contexts.begin(); c != _contexts.end(); c++) {
		(*c)->destroy();
	}
	for(a = _accs.begin(); a != _accs.end(); a++)
		delete *a;
    for(q = _queues.begin(); q != _queues.end(); q++)
        delete q->second.queue;
	_accs.clear();
	mutex.unlock();
	memoryFini();
}

void
Process::init(const char *name)
{
    // Process is a singleton class. The only allowed instance is proc
    if(proc != NULL) return;
    contextInit();
    proc = new Process();
    apiInit();
    memoryInit(name);
}

void
Process::initThread()
{
    ThreadQueue q;
    q.hasContext.lock();
    q.queue = NULL;
	mutex.lock();
	_queues.insert(QueueMap::value_type(SELF(), q));
    mutex.unlock();
}

void Process::create()
{
	TRACE("Creating new context");
	mutex.lock();
	Context *ctx = _accs[0]->create();
	ctx->init();
	_contexts.push_back(ctx);
    ThreadQueue q;
    q.queue = new Queue();
	_queues.insert(QueueMap::value_type(SELF(), q));
	mutex.unlock();
}

void Process::clone(gmac::Context *ctx, int acc)
{
    QueueMap::iterator q = _queues.find(SELF());
	assert(q != _queues.end());

	TRACE("Cloning context");
	mutex.lock();
    Context * clon;
    int usedAcc;

    if (acc != ACC_AUTO_BIND) {
        assert(acc < _accs.size());
        usedAcc = acc;
        clon = _accs[acc]->clone(*ctx);
    } else {
        // Bind the new Context to the accelerator with less contexts
        // attached to it
        usedAcc = 0;
        for (int i = 1; i < _accs.size(); i++) {
            if (_accs[i]->nContexts() < _accs[usedAcc]->nContexts()) {
                usedAcc = i;
            }
        }

        clon = _accs[usedAcc]->clone(*ctx);
        clon->init();
        _contexts.push_back(clon);
    }
    q->second.queue = new Queue();
    q->second.hasContext.unlock();
	mutex.unlock();
	TRACE("Cloned context on Acc#%d", usedAcc);
}

gmacError_t Process::migrate(int acc)
{
	mutex.lock();
    if (acc >= _accs.size()) return gmacErrorInvalidValue;
    gmacError_t ret = gmacSuccess;
	TRACE("Migrating context");
    if (Context::hasCurrent()) {
        // Really migrate data
        abort();
    } else {
        // Create the context in the requested accelerator
        Context::create(acc);
    }
	TRACE("Migrated context");
	mutex.unlock();
    return ret;
}


void Process::remove(Context *ctx)
{
	mutex.lock();
	_contexts.remove(ctx);
    ThreadQueue q = _queues[SELF()];
    if (q.queue != NULL) {
        delete q.queue;
    }
	_queues.erase(SELF());
	mutex.unlock();
	ctx->destroy();
}

void Process::accelerator(Accelerator *acc) 
{
	_accs.push_back(acc);
	_totalMemory += acc->memory();
}

void *Process::translate(void *addr) 
{
	void *ret = NULL;
	std::list<Context *>::const_iterator i;
	for(i = _contexts.begin(); i != _contexts.end(); i++) {
		ret = (*i)->mm().pageTable().translate(addr);
		if(ret != NULL) return ret;
	}
	return ret;
}

void Process::sendReceive(THREAD_ID id)
{
    Context * ctx = Context::current();
	QueueMap::iterator q = _queues.find(id);
	assert(q != _queues.end());
    q->second.hasContext.lock();
    q->second.hasContext.unlock();
	q->second.queue->push(ctx);
	PRIVATE_SET(Context::key, NULL);
	q = _queues.find(SELF());
	assert(q != _queues.end());
	PRIVATE_SET(Context::key, q->second.queue->pop());
}

}

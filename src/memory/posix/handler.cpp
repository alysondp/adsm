#include <csignal>
#include <cerrno>

#include "memory/handler.h"
#include "memory/manager.h"
#include "trace/Tracer.h"

namespace __impl { namespace memory {

struct sigaction defaultAction;
unsigned handler::Count_ = 0;

#if defined(LINUX)
int handler::Signum_ = SIGSEGV;
#elif defined(DARWIN)
int handler::Signum_ = SIGBUS;
#endif

static manager *Manager_ = NULL;

static void segvHandler(int s, siginfo_t *info, void *ctx)
{
    if (Manager_ == NULL) return defaultAction.sa_sigaction(s, info, ctx);
       
    handler::Entry();
    trace::EnterCurrentFunction();
	mcontext_t *mCtx = &((ucontext_t *)ctx)->uc_mcontext;

#if defined(LINUX)
	unsigned long writeAccess = mCtx->gregs[REG_ERR] & 0x2;
#elif defined(DARWIN)
	unsigned long writeAccess = (*mCtx)->__es.__err & 0x2;
#endif
    host_ptr addr = host_ptr(info->si_addr);

	if(!writeAccess) TRACE(GLOBAL, "Read SIGSEGV for %p", addr);
	else TRACE(GLOBAL, "Write SIGSEGV for %p", addr);

	bool resolved = false;
    address_space_ptr aspace = Manager_->get_owner(addr);
    if (aspace) {
	    if(!writeAccess) resolved = Manager_->signal_read(aspace, addr);
    	else             resolved = Manager_->signal_write(aspace, addr);
    }

	if (resolved == false) {
		fprintf(stderr, "Uoops! I could not find a mapping for %p. I will abort the execution\n", addr);
		abort();
		// TODO: set the signal mask and other stuff
		if(defaultAction.sa_flags & SA_SIGINFO) 
			return defaultAction.sa_sigaction(s, info, ctx);
		return defaultAction.sa_handler(s);
	}

    trace::ExitCurrentFunction();
    handler::Exit();
}


void handler::setHandler() 
{
	struct sigaction segvAction;
	memset(&segvAction, 0, sizeof(segvAction));
	segvAction.sa_sigaction = segvHandler;
	segvAction.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&segvAction.sa_mask);

	if(sigaction(Signum_, &segvAction, &defaultAction) < 0)
		FATAL("sigaction: %s", strerror(errno));

	Handler_ = this;
	TRACE(GLOBAL, "New signal handler programmed");
}

void handler::restoreHandler()
{
	if(sigaction(Signum_, &defaultAction, NULL) < 0)
		FATAL("sigaction: %s", strerror(errno));

	Handler_ = NULL;
	TRACE(GLOBAL, "Old signal handler restored");
}

void handler::setManager(manager &manager)
{
    Manager_ = &manager;
}

}}
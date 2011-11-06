#ifndef GMAC_HAL_OPENCL_CONTEXT_IMPL_H_
#define GMAC_HAL_OPENCL_CONTEXT_IMPL_H_

#include "util/Logger.h"

namespace __impl { namespace hal { namespace opencl {

inline
gmacError_t
list_event::sync()
{
    cl_event *evs = get_event_array();

    cl_int res = clWaitForEvents(Parent::size(), evs);

    return error(res);
}

inline
void
list_event::set_synced()
{
    for (Parent::iterator it  = Parent::begin();
            it != Parent::end();
            it++) {
        (*it).set_synced();
    }
}

inline
cl_event *
list_event::get_event_array()
{
    cl_event *ret = NULL;
    if (Parent::size() > 0) {
        ret = new cl_event[Parent::size()];
        int i = 0;
        for (Parent::iterator it = Parent::begin();
             it != Parent::end();
             it++) {
            ret[i++] = (*it)();
        }
    }

    return ret;
}

inline
size_t
list_event::size() const
{
    return Parent::size(); 
}

inline
buffer_t::buffer_t(hostptr_t addr, context_t &context) :
    Parent(context),
    addr_(addr)
{
}

inline
hostptr_t
buffer_t::get_addr()
{
    return addr_;
}

inline
accptr_t
buffer_t::get_device_addr()
{
    return get_context().get_device_addr_from_pinned(addr_);
}

inline
accptr_t
context_t::get_device_addr_from_pinned(hostptr_t addr)
{
    FATAL("NOT SUPPORTED IN OPENCL");

    return accptr_t(0);
}

}}} /* GMAC_HAL_OPENCL_CONTEXT_IMPL_H_ */

#endif

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

#ifndef GMAC_HAL_OPENCL_STREAM_IMPL_H_
#define GMAC_HAL_OPENCL_STREAM_IMPL_H_

namespace __impl { namespace hal { namespace opencl {

inline
stream_t::Parent::state
stream_t::query()
{
    Parent::state ret = Running;

    if (lastEvent_.is_valid()) {
        cl_int val;
        cl_int res = clGetEventInfo(lastEvent_(), 
                                    CL_EVENT_COMMAND_EXECUTION_STATUS,
                                    sizeof(cl_int),
                                    &val,
                                    NULL);
        ASSERTION(res == CL_SUCCESS);

        if (res == CL_QUEUED ||
            res == CL_SUBMITTED ||
            res == CL_RUNNING) {
            ret = Running;
        } else if (res == CL_COMPLETE) {
            ret = Empty;
        } else {
            FATAL("Unhandled case");
        }
    }

    return ret;
}

inline
void
stream_t::set_last_event(event_t event)
{
    lastEvent_ = event;
}

inline
event_t
stream_t::get_last_event()
{
    return lastEvent_;
}

inline
context_t &
stream_t::get_context()
{
    return reinterpret_cast<context_t &>(Parent::get_context());
}

}}}

#endif

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
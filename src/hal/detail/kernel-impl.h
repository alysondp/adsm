#ifndef GMAC_HAL_KERNEL_IMPL_H_
#define GMAC_HAL_KERNEL_IMPL_H_

namespace __impl { namespace hal { namespace detail { namespace code {

inline
kernel::kernel(const std::string &name) :
    name_(name)
{
}

inline
const std::string &
kernel::get_name() const
{
    return name_;
}

inline
kernel::config::config(unsigned ndims) :
    ndims_(ndims)
{
}

inline
unsigned
kernel::config::get_ndims() const
{
    return ndims_;
}

inline
kernel::launch::launch(const kernel &kernel) :
    kernel_(kernel)
{
}

inline
const kernel &
kernel::launch::get_kernel() const
{
    return kernel_;
}

inline
event_ptr
kernel::launch::get_event()
{
    return event_;
}

} // namespace detail

}}}

#endif /* GMAC_HAL_KERNEL_IMPL_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
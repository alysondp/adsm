#ifndef GMAC_HAL_KERNEL_IMPL_H_
#define GMAC_HAL_KERNEL_IMPL_H_

namespace __impl { namespace hal { namespace detail {

template <typename D, typename B, typename I>
inline
kernel_t<D, B, I>::kernel_t(typename B::kernel kernel, const std::string &name) :
    kernel_(kernel),
    name_(name)
{
}

template <typename D, typename B, typename I>
inline
typename B::kernel &
kernel_t<D, B, I>::operator()()
{
    return kernel_;
}

template <typename D, typename B, typename I>
inline
const typename B::kernel &
kernel_t<D, B, I>::operator()() const
{
    return kernel_;
}

template <typename D, typename B, typename I>
inline
const std::string &
kernel_t<D, B, I>::get_name() const
{
    return name_;
}

template <typename D, typename B, typename I>
inline
kernel_t<D, B, I>::config::config(unsigned ndims) :
    ndims_(ndims)
{
}

template <typename D, typename B, typename I>
inline
unsigned
kernel_t<D, B, I>::config::get_ndims() const
{
    return ndims_;
}

template <typename D, typename B, typename I>
inline
kernel_t<D, B, I>::launch::launch(typename I::kernel &kernel,
                                  typename I::kernel::config &conf,
                                  typename I::stream &stream) :
    kernel_(kernel),
    config_(conf),
    stream_(stream)
{
}

template <typename D, typename B, typename I>
inline
typename I::stream &
kernel_t<D, B, I>::launch::get_stream()
{
    return stream_;
}

template <typename D, typename B, typename I>
inline
const typename I::stream &
kernel_t<D, B, I>::launch::get_stream() const
{
    return stream_;
}

template <typename D, typename B, typename I>
inline
const typename I::kernel &
kernel_t<D, B, I>::launch::get_kernel() const
{
    return dynamic_cast<typename I::kernel &>(kernel_);
}

template <typename D, typename B, typename I>
inline
const typename I::kernel::config &
kernel_t<D, B, I>::launch::get_config() const
{
    return config_;
}

} // namespace detail

}}

#endif /* GMAC_HAL_KERNEL_IMPL_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

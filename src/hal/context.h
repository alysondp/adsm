#ifndef GMAC_HAL_TYPES_ASPACE_H_
#define GMAC_HAL_TYPES_ASPACE_H_

#include <list>

namespace __impl { namespace hal {

namespace detail {

template <typename D, typename B, typename I>
class list_event :
    public std::list<typename I::async_event *> {

    typedef std::list<typename I::async_event *> Parent;
public:
    static list_event empty;
};

template <typename D, typename B, typename I>
list_event<D, B, I> list_event<D, B, I>::empty;

template <typename D, typename B, typename I>
class GMAC_LOCAL buffer_t {
    typename I::context &context_;

protected:
    buffer_t(typename I::context &context);

public:
    virtual hostptr_t get_addr() = 0;
    virtual accptr_t get_device_addr() = 0;
    typename I::context &get_context();
    const typename I::context &get_context() const;
};

template <typename D, typename B, typename I>
class GMAC_LOCAL code_repository
{
public:
    virtual const typename I::kernel *kernel(gmac_kernel_id_t key) const = 0;
    virtual const typename I::kernel *kernelByName(const std::string &name) const = 0;
};

template <typename D, typename B, typename I>
class GMAC_LOCAL context_t {
protected:
    typename B::context context_;
    D &device_;

    context_t(typename B::context context, D &device);

public:
    D &get_device();
    const D &get_device() const;

    typename B::context &operator()();
    const typename B::context &operator()() const;

    virtual accptr_t alloc(size_t count, gmacError_t &err) = 0;
    //virtual gmacError_t alloc_host_pinned(hostptr_t &ptr, size_t count, GmacProtection hint, gmacError_t &err) = 0;
    virtual typename I::buffer *alloc_buffer(size_t count, GmacProtection hint, gmacError_t &err) = 0;
    virtual gmacError_t free(accptr_t acc) = 0;
    //virtual gmacError_t free_host_pinned(hostptr_t ptr) = 0;
    virtual gmacError_t free_buffer(typename I::buffer &buffer) = 0;

    virtual typename I::event *copy(accptr_t dst, hostptr_t src, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::event *copy(accptr_t dst, hostptr_t src, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::event *copy(accptr_t dst, hostptr_t src, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::event *copy(hostptr_t dst, accptr_t src, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::event *copy(hostptr_t dst, accptr_t src, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::event *copy(hostptr_t dst, accptr_t src, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::event *copy(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::event *copy(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::event *copy(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::async_event *copy_async(accptr_t dst, typename I::buffer src, size_t off, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(accptr_t dst, typename I::buffer src, size_t off, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(accptr_t dst, typename I::buffer src, size_t off, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::async_event *copy_async(typename I::buffer dst, size_t off, accptr_t src, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(typename I::buffer dst, size_t off, accptr_t src, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(typename I::buffer dst, size_t off, accptr_t src, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::async_event *copy_async(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::async_event *copy_async(accptr_t dst, accptr_t src, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::event *memset(accptr_t dst, int c, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::event *memset(accptr_t dst, int c, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::event *memset(accptr_t dst, int c, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual typename I::async_event *memset_async(accptr_t dst, int c, size_t count, typename I::stream &stream, list_event<D, B, I> &dependencies, gmacError_t &err) = 0;
    virtual typename I::async_event *memset_async(accptr_t dst, int c, size_t count, typename I::stream &stream, typename I::async_event &event, gmacError_t &err) = 0;
    virtual typename I::async_event *memset_async(accptr_t dst, int c, size_t count, typename I::stream &stream, gmacError_t &err) = 0;

    virtual const typename I::code_repository &get_code_repository() const = 0;
};

}

}}

#endif /* ASPACE_H */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

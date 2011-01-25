#ifndef GMAC_CONFIG_CUDA_COMMON_H_
#define GMAC_CONFIG_CUDA_COMMON_H_

#include <cuda.h>

struct accptr_t {
    CUdeviceptr ptr_;

    inline accptr_t() :
        ptr_(NULL)
    {}
    inline accptr_t(void * ptr) :
        ptr_(CUdeviceptr(long_t(ptr)))
    {}
    inline accptr_t(CUdeviceptr ptr) :
        ptr_(ptr)
    {}
    inline accptr_t(long int ptr) :
        ptr_(ptr)
    {}
    inline accptr_t(long_t ptr) :
#if CUDA_VERSION < 3020
        ptr_(CUdeviceptr(ptr))
#else
        ptr_(ptr)
#endif
    {}
    inline accptr_t(int ptr) :
        ptr_(ptr)
    {}

#if CUDA_VERSION < 3020
    inline operator uint32_t() const { return uint32_t(ptr_); }
#else
    inline operator CUdeviceptr() const { return ptr_; }
#endif
    inline operator void *() const { return (void *)(ptr_); }

    inline void *get() const { return (void *)(ptr_); }

    template <typename T>
    inline
    bool operator==(T ptr)
    {
        return (((T)this->ptr_) == ptr);
    }

    template <typename T>
    inline
    bool operator!=(T ptr)
    {
        return (((T)this->ptr_) != ptr);
    }

};

template <typename T>
static inline
accptr_t operator+(const accptr_t &a, T b)
{
    return accptr_t(a.ptr_ + CUdeviceptr(b));
}

template <typename T>
static inline
accptr_t operator-(const accptr_t &a, T b)
{
    return accptr_t(a.ptr_ - CUdeviceptr(b));
}

#endif

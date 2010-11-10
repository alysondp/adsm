#ifndef GMAC_MEMORY_TEST_SYSTEMBLOCK_IPP_
#define GMAC_MEMORY_TEST_SYSTEMBLOCK_IPP_

namespace gmac { namespace memory { namespace __dbc {

template <typename T>
inline
SystemBlock<T>::SystemBlock(void *addr, size_t size, T state) :
    __impl::SystemBlock<T>(addr, size, state)
{
    REQUIRES(size > 0);
    REQUIRES(addr != NULL);
}

template <typename T>
inline
SystemBlock<T>::~SystemBlock()
{
}

}}}

#endif

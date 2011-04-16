#ifndef GMAC_CORE_ACCELERATOR_IMPL_H_
#define GMAC_CORE_ACCELERATOR_IMPL_H_

#include "util/Logger.h"

namespace __impl { namespace core {

inline unsigned
Accelerator::load() const
{
    return load_;
}

inline unsigned
Accelerator::id() const
{
    return id_;
}

inline unsigned
Accelerator::busId() const
{
    return busId_;
}

inline unsigned
Accelerator::busAccId() const
{
    return busAccId_;
}

inline bool
Accelerator::integrated() const
{
    return integrated_;
}

inline bool
Accelerator::getMapping(accptr_t &acc, hostptr_t addr, size_t size)
{
    size_t s;
    bool ret = allocations_.find(addr, acc, s);
    // For now, we require the mappings to match in size
	if (ret == true) { ASSERTION(s == size); }
    return ret;
}

}}

#endif
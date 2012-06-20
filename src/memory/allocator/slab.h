/* Copyright (c) 2009-2011 University of Illinois
                           Universitat Politecnica de Catalunya
                   All rights reserved.

Developed by: IMPACT Research Group / Grup de Sistemes Operatius
              University of Illinois / Universitat Politecnica de Catalunya
              http://impact.crhc.illinois.edu/
              http://gso.ac.upc.edu/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal with the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimers.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimers in the
     documentation and/or other materials provided with the distribution.
  3. Neither the names of IMPACT Research Group, Grup de Sistemes Operatius,
     University of Illinois, Universitat Politecnica de Catalunya, nor the
     names of its contributors may be used to endorse or promote products
     derived from this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
WITH THE SOFTWARE.  */

#ifndef GMAC_MEMORY_ALLOCATOR_SLAB_H_
#define GMAC_MEMORY_ALLOCATOR_SLAB_H_

#include "config/common.h"
#include "memory/allocator.h"

#include "cache.h"

namespace __impl { namespace memory { namespace allocator {

/**
 * Simple slab allocator
 */
class GMAC_LOCAL slab : public memory::allocator_interface {
protected:
    class GMAC_LOCAL map_address :
    	public std::map<host_const_ptr, cache *>,
    	gmac::util::lock_rw<map_address> {
    	typedef gmac::util::lock_rw<map_address> lock;
    protected:
        friend class slab;
    public:
        map_address() : lock("memory::slab::map_address") {}
    };

    typedef std::map<long_t, cache *> map_cache;

    class GMAC_LOCAL map_aspace :
    	public std::map<address_space_ptr, map_cache>,
    	gmac::util::lock_rw<map_aspace> {
        friend class slab;

        typedef gmac::util::lock_rw<map_aspace> lock;
    public:
        map_aspace() : lock("memory::slab::map_aspace") {}
    };

    map_address addresses_;
    map_aspace aspaces_; // Per-context cache map

    cache &create_cache(address_space_ptr aspace, map_cache &map, long_t key, size_t size);
    cache &get(address_space_ptr current, long_t key, size_t size);
    void cleanup(address_space_ptr current);

    manager &manager_;

    virtual ~slab();
public:
    slab(manager &manager);

    host_ptr alloc(address_space_ptr current, const size_t size, host_const_ptr addr);
    bool free(address_space_ptr current, host_ptr addr);
};

}}}

#include "slab-impl.h"

#if defined(USE_DBC)
namespace __dbc { namespace memory { namespace allocator {
typedef __impl::memory::allocator::slab slab;
}}}
#endif

#endif
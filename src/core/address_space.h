/* Copyright (c) 2009, 2010, 2011 University of Illinois
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
 WITH THE SOFTWARE.
 */

#ifndef GMAC_CORE_ADDRESS_SPACE_H_
#define GMAC_CORE_ADDRESS_SPACE_H_

#include <map>

#include "config/common.h"

#include "hal/types.h"

#include "memory/map_object.h"

#include "util/lock.h"
#include "util/Reference.h"
#include "util/unique.h"

namespace __impl {
    
namespace memory {
class map_object;
}
    
namespace core {

class io_buffer;

class GMAC_LOCAL address_space :
    public util::unique<address_space, GmacAddressSpaceId> {

    memory::map_object map_;

protected:
    address_space();
    virtual ~address_space();

public:
    virtual io_buffer *create_io_buffer(size_t count, GmacProtection prot) = 0;
    virtual gmacError_t destroy_io_buffer(io_buffer &buffer) = 0;

    virtual gmacError_t map(accptr_t &dst, hostptr_t src, size_t count, unsigned align = 1) = 0;
    virtual gmacError_t unmap(hostptr_t addr, size_t count) = 0;

    virtual hostptr_t alloc_host_pinned(size_t count, gmacError_t &err) = 0;
    virtual gmacError_t free_host_pinned(hostptr_t ptr) = 0;

    virtual accptr_t get_host_pinned_mapping(hostptr_t ptr, gmacError_t &err) = 0;

    virtual gmacError_t copy(accptr_t acc, const hostptr_t host, size_t count) = 0;
    virtual gmacError_t copy(hostptr_t host, const accptr_t acc, size_t count) = 0;
    virtual gmacError_t copy(accptr_t dst, const accptr_t src, size_t count) = 0;

    virtual hal::async_event_t *copy_async(accptr_t acc, const hostptr_t host, size_t count, gmacError_t &err) = 0;
    virtual hal::async_event_t *copy_async(hostptr_t host, const accptr_t acc, size_t count, gmacError_t &err) = 0;
    virtual hal::async_event_t *copy_async(accptr_t dst, const accptr_t src, size_t count, gmacError_t &err) = 0;

    virtual gmacError_t copy(accptr_t dst, io_buffer &buffer, size_t off, size_t count) = 0;
    virtual gmacError_t copy(io_buffer &buffer, size_t off, const accptr_t dst, size_t count) = 0;

    virtual gmacError_t memset(accptr_t addr, int c, size_t size) = 0;
    virtual hal::async_event_t *memset_async(accptr_t addr, int c, size_t size, gmacError_t &err) = 0;

    memory::map_object &get_object_map();
    const memory::map_object &get_object_map() const;

    virtual bool is_integrated() const = 0;

    virtual bool has_direct_copy(const address_space &aspace) const = 0;

#if 0
    virtual void notify_pending_changes() = 0;
#endif
};

}}

#include "address_space-impl.h"

#endif

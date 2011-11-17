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
WITH THE SOFTWARE.  */

#ifndef GMAC_MEMORY_PROTOCOL_DBC_LAZYBASE_H_
#define GMAC_MEMORY_PROTOCOL_DBC_LAZYBASE_H_

#include "config/dbc/types.h"

namespace __dbc { namespace memory { namespace protocol {

class GMAC_LOCAL LazyBase :
    public __impl::memory::protocol::LazyBase,
    public virtual Contract {
    DBC_TESTED(__impl::memory::protocol::LazyBase)

protected:
    LazyBase(bool eager);
    virtual ~LazyBase();

    typedef __impl::memory::protocol::LazyBase Parent;
    typedef __impl::memory::block_ptr BlockPtrImpl;
    typedef __impl::memory::object ObjectImpl;
    typedef __impl::memory::protocol::lazy::State StateImpl;
    typedef __impl::memory::protocol::lazy::Block LazyBlockImpl;
    typedef __impl::util::smart_ptr<LazyBlockImpl>::shared LazyBlockPtrImpl;
    typedef __impl::core::io_buffer io_buffer_impl;

public:
    gmacError_t signal_read(BlockPtrImpl block, hostptr_t addr);
    gmacError_t signal_write(BlockPtrImpl block, hostptr_t addr);

    gmacError_t acquire(BlockPtrImpl obj, GmacProtection &prot);
    gmacError_t release(BlockPtrImpl block);

    gmacError_t releaseAll();

    gmacError_t toHost(BlockPtrImpl block);

    __impl::hal::event_t memset(BlockPtrImpl block, size_t blockOffset, int v, size_t size,
                                gmacError_t &err);

    gmacError_t flushDirty();

    __impl::hal::event_t copyBlockToBlock(BlockPtrImpl d, size_t dstOffset, BlockPtrImpl s, size_t srcOffset, size_t count, gmacError_t &err);
};

}}}

#endif

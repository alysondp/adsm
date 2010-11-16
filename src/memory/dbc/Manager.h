/* Copyright (c) 2009, 2010 University of Illinois
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

#ifndef GMAC_MEMORY_DBC_MANAGER_H_
#define GMAC_MEMORY_DBC_MANAGER_H_

#include "dbc/types.h"
#include "dbc/Contract.h"
#include "memory/Manager.h"

namespace gmac { namespace memory { namespace __dbc {

//! Memory Manager Interface

//! Memory Managers implement a policy to move data from/to
//! the CPU memory to/from the accelerator memory.
class GMAC_LOCAL Manager :
    public __impl::Manager,
    public virtual gmac::dbc::Contract {
public:
    Manager();
    ~Manager();

    gmacError_t map(void *addr, size_t size, GmacProtection prot);
    gmacError_t unmap(void *addr, size_t size);
    gmacError_t alloc(void **addr, size_t size);

    gmacError_t globalAlloc(void **addr, size_t size, GmacGlobalMallocType hint);
    gmacError_t free(void *addr);

    bool read(void *addr);
    bool write(void *addr);

    gmacError_t toIOBuffer(gmac::core::IOBuffer &buffer, const void *addr, size_t size);
    gmacError_t fromIOBuffer(void *addr, gmac::core::IOBuffer &buffer, size_t size);

    gmacError_t memcpy(void * dst, const void * src, size_t n);
    gmacError_t memset(void * dst, int c, size_t n);
};

}}}

#endif

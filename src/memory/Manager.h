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

#ifndef __MEMORY_MEMMANAGER_H_
#define __MEMORY_MEMMANAGER_H_

#include <memory/Protocol.h>

#include <gmac/gmac.h>
#include <util/Logger.h>

#include <stdint.h>

#include <iostream>
#include <iterator>
#include <map>


namespace gmac { namespace memory {


//! Memory Manager Interface

//! Memory Managers implement a policy to move data from/to
//! the CPU memory to/from the accelerator memory.
class Manager : public util::Logger {
protected:
    static int __count;
    static Manager *__manager;

    Protocol *protocol;

    Manager();
    ~Manager();
public:
    // Manager management
    static Manager *create();
    static void destroy();
    static Manager *get();

    // Memory management functions
    gmacError_t alloc(void **addr, size_t size);
#ifndef USE_MMAP
    gmacError_t globalAlloc(void **addr, size_t size, int hint);
    bool requireUpdate(Block *block);
#endif
    gmacError_t free (void *addr);

    // Coherence protocol interface
    gmacError_t acquire();
    gmacError_t release();
    gmacError_t invalidate();

    bool read(void *addr);
    bool write(void *addr);

    gmacError_t adquire(void *addr, size_t size);
    gmacError_t release(void *addr, size_t size);
    gmacError_t invalidate(void *addr, size_t size);
};

} }

#endif

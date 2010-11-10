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

#ifndef GMAC_MEMORY_PROTOCOL_TEST_LAZY_H_
#define GMAC_MEMORY_PROTOCOL_TEST_LAZY_H_

#include "memory/protocol/Lazy.h"
#include "dbc/types.h"

namespace gmac { namespace memory { namespace protocol { namespace __dbc {

class GMAC_LOCAL Lazy :
    public __impl::Lazy,
    public virtual gmac::dbc::Contract {

protected:
    gmacError_t copyHostToDirty(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);
    gmacError_t copyHostToReadOnly(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                   const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);
    gmacError_t copyHostToInvalid(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                  const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);

    gmacError_t copyAcceleratorToDirty(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                       const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);
    gmacError_t copyAcceleratorToReadOnly(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                          const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);
    gmacError_t copyAcceleratorToInvalid(const StateObject<State> &objectDst, Block &blockDst, unsigned blockOffDst,
                                         const StateObject<State> &objectSrc, Block &blockSrc, unsigned blockOffSrc, size_t count);
public:
    Lazy(unsigned limit);
    virtual ~Lazy();

    gmacError_t signalRead(const Object &obj, void *addr);
    gmacError_t signalWrite(const Object &obj, void *addr);

    gmacError_t toIOBuffer(IOBuffer &buffer, unsigned bufferOff, const Object &obj, unsigned objectOff, size_t n);
    gmacError_t fromIOBuffer(const Object &obj, unsigned objectOff, IOBuffer &buffer, unsigned bufferOff, size_t n);

    gmacError_t toPointer(void *dst, const Object &objSrc, unsigned objectOff, size_t n);
    gmacError_t fromPointer(const Object &dstObj, unsigned objectOff, const void *src, size_t n);

    gmacError_t copy(const Object &objDst, unsigned offDst, const Object &objSrc, unsigned offSrc, size_t count);
    gmacError_t memset(const Object &obj, unsigned objectOff, int c, size_t count);
};

}}}}

#endif

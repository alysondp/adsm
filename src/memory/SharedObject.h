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

#ifndef GMAC_MEMORY_SHAREDOBJECT_H_
#define GMAC_MEMORY_SHAREDOBJECT_H_

#include "memory/Object.h"

namespace __impl { 

namespace core {
	class Mode;
}

namespace memory {

template<typename State>
class GMAC_LOCAL SharedObject : public gmac::memory::Object {
    DBC_FORCE_TEST(SharedObject<State>)
protected:
    hostptr_t shadow_;
	accptr_t  acceleratorAddr_;
	core::Mode *owner_;

    static accptr_t allocAcceleratorMemory(core::Mode &mode, hostptr_t addr, size_t size);
    gmacError_t repopulateBlocks(accptr_t accPtr, core::Mode &mode);

    void modifiedObject();
public:
	SharedObject(Protocol &protocol, core::Mode &owner, hostptr_t addr, size_t size, typename State::ProtocolState init);
    virtual ~SharedObject();

    accptr_t acceleratorAddr(core::Mode &current, const hostptr_t addr) const;
	core::Mode &owner(core::Mode &current, const hostptr_t addr) const;

	gmacError_t addOwner(core::Mode &owner);
	gmacError_t removeOwner(core::Mode &owner);

    gmacError_t unmapFromAccelerator();
    gmacError_t mapToAccelerator();
};

}}

#include "SharedObject-impl.h"

#ifdef USE_DBC
#include "memory/dbc/SharedObject.h"
#endif

#endif

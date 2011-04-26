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

#ifndef GMAC_MEMORY_SHAREDBLOCK_H_
#define GMAC_MEMORY_SHAREDBLOCK_H_

#include "config/common.h"
#include "config/config.h"

#include "include/gmac/types.h"

#include "Block.h"
#include "StateBlock.h"

namespace __impl {

namespace core {
    class Mode;
}

namespace memory {

template<typename State>
class GMAC_LOCAL SharedBlock : public StateBlock<State> {
protected:
    //! Owner of the block
    core::Mode &owner_;

    //! Accelerator memory address of the block
    accptr_t acceleratorAddr_;

public:
    //! Default construcutor
    /*!
        \param protocol Memory coherence protocol used by the block
        \param owner Owner of the memory block
        \param hostAddr Host memory address for applications to accesss the block
        \param shadowAddr Shadow host memory mapping that is always read/write
        \param acceleratorAddr Accelerator memory address for applications to accesss the block
        \param size Size (in bytes) of the memory block
        \param init Initial block state
    */
    SharedBlock(Protocol &protocol, core::Mode &owner, hostptr_t hostAddr,
                hostptr_t shadowAddr, accptr_t acceleratorAddr, size_t size, typename State::ProtocolState init);

    //! Default destructor
    virtual ~SharedBlock();

    //! Get memory block owner
    /*!
        \return A reference to the owner mode of the memory block
    */
    core::Mode &owner(core::Mode &current) const;

    //! Get memory block address at the accelerator
    /*!
     * \param addr Address within the block
        \return Accelerator memory address of the block
    */
    accptr_t acceleratorAddr(core::Mode &current, const hostptr_t addr) const;

    //! Get memory block address at the accelerator
    /*!
        \return Accelerator memory address of the block
    */
    accptr_t acceleratorAddr(core::Mode &current) const;

    gmacError_t toAccelerator(unsigned blockOffset, size_t count);

    gmacError_t toHost(unsigned blockOffset, size_t count);

    gmacError_t copyToHost(const hostptr_t src, size_t size,
                           size_t blockOffset = 0) const;

    gmacError_t copyToHost(core::IOBuffer &buffer, size_t size,
                           size_t bufferOffset = 0, size_t blockOffset = 0) const;

    gmacError_t copyToAccelerator(const hostptr_t src, size_t size,
                                  size_t blockOffset = 0) const;

    gmacError_t copyToAccelerator(core::IOBuffer &buffer, size_t size,
                                  size_t bufferOffset = 0, size_t blockOffset = 0) const;

    gmacError_t copyFromHost(hostptr_t dst, size_t size,
                             size_t blockOffset = 0) const;

    gmacError_t copyFromHost(core::IOBuffer &buffer, size_t size,
                             size_t bufferOffset = 0, size_t blockOffset = 0) const;

    gmacError_t copyFromAccelerator(hostptr_t dst, size_t size,
        size_t blockOffset = 0) const;

    gmacError_t copyFromAccelerator(core::IOBuffer &buffer, size_t size,
                                    size_t bufferOffset = 0, size_t blockOffset = 0) const;

    gmacError_t hostMemset(int v, size_t size, size_t blockOffset = 0) const;

    gmacError_t acceleratorMemset(int v, size_t size, size_t blockOffset = 0) const;
};


}}

#include "SharedBlock-impl.h"

#endif

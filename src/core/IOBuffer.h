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

#ifndef GMAC_CORE_IOBUFFER_H_
#define GMAC_CORE_IOBUFFER_H_

#include "config/common.h"
#include "include/gmac/types.h"
#include "util/Lock.h"

#include "Mode.h"

namespace gmac { namespace core {

class GMAC_LOCAL IOBuffer : public util::Lock {
public:
    typedef enum { Idle, ToHost, ToAccelerator } State;
protected:
    void *addr_;
    size_t size_;

    State state_;
    Mode *mode_;
public:
    IOBuffer(void *addr, size_t size);
    virtual ~IOBuffer();
	IOBuffer &operator =(const IOBuffer &) {
        FATAL("Assigment of I/O buffers is not supported");
        return *this;
    }

    uint8_t *addr() const;
    uint8_t *end() const;
    size_t size() const;

    void lock();
    void unlock();

    State state() const;
    void toHost(Mode &mode);
    void toAccelerator(Mode &mode);

    gmacError_t wait();
};

}}

#include "IOBuffer.ipp"

#endif

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

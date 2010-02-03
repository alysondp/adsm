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

#ifndef __KERNEL_KERNEL_H
#define __KERNEL_KERNEL_H

#include "Descriptor.h"

#include "memory/Region.h"

#include <vector>

namespace gmac {

class Argument {
public:
    void * _ptr;
    size_t _size;
    Argument(void * ptr, size_t size);
private:
    friend class Kernel;
};

typedef std::vector<Argument> ArgVector;

/// \todo create a pool of objects to avoid mallocs/frees
class KernelConfig : public ArgVector{
protected:
	static const unsigned StackSize = 4096;

    char _stack[StackSize];
    off_t _argsSize;

    KernelConfig(const KernelConfig & c);
public:
    /// \todo create a pool of objects to avoid mallocs/frees
    KernelConfig();
    virtual ~KernelConfig();

    void pushArgument(const void * arg, size_t size, off_t offset);
    off_t argsSize() const;

    char * argsArray();
};

typedef std::vector<memory::Region *> RegionVector;
typedef Descriptor<gmacKernel_t> KernelDescriptor;

class KernelLaunch;

class Kernel : public RegionVector, public KernelDescriptor
{
public:
    Kernel(const KernelDescriptor & k);

    virtual KernelLaunch * launch(KernelConfig & c) = 0;
    gmacError_t bind(void * addr);
    gmacError_t unbind(void * addr);
};

class KernelLaunch {
public:
    virtual gmacError_t execute() = 0;
};

}

#include "Kernel.ipp"

#endif /* KERNEL_H */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

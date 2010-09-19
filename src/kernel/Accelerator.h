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

#ifndef __KERNEL_ACCELERATOR_H_
#define __KERNEL_ACCELERATOR_H_

#include <gmac/gmac.h>
#include <util/Logger.h>
#include <stddef.h>

namespace gmac {

class Mode;
class KernelLaunch;

/*!
	\brief Generic Accelerator Class
	Defines the standard interface all accelerators MUST
	implement
*/
class Accelerator : public util::Logger {
protected:
    size_t _memory;
    unsigned _id;

    unsigned _load;
public:
	Accelerator(int n);

    unsigned id() const;
	virtual ~Accelerator();

    virtual Mode *createMode() = 0;
    virtual void destroyMode(Mode *) = 0;
    inline virtual unsigned load() const { return _load; }

    /*!  \brief Allocates memory on the accelerator memory */
	virtual gmacError_t malloc(void **addr, size_t size, unsigned align = 1) = 0;

	/*!  \brief Releases memory previously allocated by Malloc */
	virtual gmacError_t free(void *addr) = 0;

    virtual gmacError_t sync() = 0;

	/*!  \brief Copies data from system memory to accelerator memory */
	virtual gmacError_t copyToDevice(void *dev, const void *host,
			size_t size) = 0;

	/*!  \brief Copies data from accelerator memory to system memory */
	virtual gmacError_t copyToHost(void *host, const void *dev,
			size_t size) = 0;

	/*!  \brief Copies data from accelerator memory to accelerator memory */
	virtual gmacError_t copyDevice(void *dst, const void *src,
			size_t size) = 0;
};

}

#include "Accelerator.ipp"

#endif

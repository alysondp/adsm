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

#ifndef __MEMORY_MEMREGION_H_
#define __MEMORY_MEMREGION_H_

#include <config.h>

#include <gmac/gmac.h>

#include <util/Lock.h>
#include <util/Logger.h>
#include <memory/os/Memory.h>

#include <stdint.h>
#include <csignal>

#include <iostream>
#include <list>
#include <set>

namespace gmac {

class Context;

namespace memory {

typedef unsigned long addr_t;

//! Generic Memory Region Descriptor
class Region : public util::RWLock {
protected:
	Context *_context;
	std::list<Context *> _relatives;
	//! Starting memory address for the region
	addr_t _addr;
	//! Size in bytes of the region
	size_t _size;
	//! Is the region shared among contexts?
	bool _shared;

	addr_t __addr(void *addr) const;
	addr_t __addr(const void *addr) const;
	void * __void(addr_t addr) const;

public:
	//! Constructor
	//! \param addr   Start memory address
	//! \param size   Size in bytes
	//! \param shared Is the region shared among contexts?
	Region(void *addr, size_t size, bool shared);

	virtual ~Region();

	Context *owner();
#ifdef USE_VM
    virtual gmacError_t copyToDevice();
    virtual gmacError_t copyToHost();
#else
    gmacError_t copyToDevice();
    gmacError_t copyToHost();
#endif

    bool shared();

	void sync();

	virtual void syncToHost();

	//! Returns the size (in bytes) of the Region
	size_t size() const;
	//! Returns the address of the Region
	void *start() const;
	//! Returns the end address of the region
	void *end() const;

	virtual void relate(Context *ctx);
	virtual void unrelate(Context *ctx);
	virtual void transfer();
	virtual std::list<Context *> &relatives();
};

typedef std::set<Region *> RegionSet;

#include "Region.ipp"

} };

#endif
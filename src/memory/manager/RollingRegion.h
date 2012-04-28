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

#ifndef __MEMORY_CACHEREGION_H_
#define __MEMORY_CACHEREGION_H_

#include "Region.h"
#include "ProtRegion.h"

#include <config.h>

#include <cstdlib>

#include <map>
#include <set>

namespace gmac { namespace memory { namespace manager {

class RollingBlock;
class RollingManager;

class BlockList : public std::set<RollingBlock *>, public util::RWLock
{
public:
    BlockList();
};

class RollingRegion : public Region {
public:
protected:
   RollingManager &manager;

   // Set of all sub-regions forming the region
   std::vector<RollingBlock *> _subRegions;
   typedef std::map<const void *, RollingBlock *> Map;
   Map _map;

   // List of sub-regions that are present in memory
   BlockList _memory;

   size_t cacheLine;
   size_t offset;

   friend class RollingBlock;
   void push(RollingBlock *region);

public:
   RollingRegion(RollingManager &manager, void *addr, size_t size, bool shared, size_t chacheLine);
   ~RollingRegion();
   void syncToHost();
   void relate(Context *ctx);
   void unrelate(Context *ctx);
   void transfer();

   RollingBlock *find(const void *);
   void invalidate();
   void invalidate(const void *, size_t);
   void invalidateWithBitmap(int prot);
   void invalidateWithBitmap(const void *, size_t, int prot);
   void flush();
   void flush(const void *, size_t);

   void transferNonDirty();
   void transferDirty();

   const std::vector<RollingBlock *> & subRegions();
};

class RollingBlock : public ProtRegion {
protected:
    RollingRegion &_parent;
    friend class RollingRegion;
    void preInvalidate();

    void algorithm();

#ifdef USE_VM
    unsigned transfers;
#endif

#ifdef USE_VM
    gmacError_t toDevice(Context * ctx, void * addr, size_t size);
    gmacError_t toHost(Context * ctx, void * addr, size_t size);
#endif
public:
    RollingBlock(RollingRegion &parent, void *addr, size_t size, bool shared);
    ~RollingBlock();

    // Override this methods to insert the regions in the list
    // of sub-regions present in _memory
#ifdef USE_VM
    void invalidate();
    void * startChunk(unsigned chunk) const;
    size_t sizeChunk() const;
    size_t chunks() const;
    unsigned chunk(const void * addr) const;

    void readOnlyChunk(unsigned chunk);
    void readWriteChunk(unsigned chunk);

    unsigned lastChunk;
    unsigned seqChunks;
    bool isSeq;
#endif
    void readOnly();
    void readWrite();

    void flush();

#ifdef USE_VM
    gmacError_t copyToDevice();
    gmacError_t copyToHost();
#endif

    RollingRegion & getParent();
};

}}}

#include "RollingRegion.ipp"

#endif
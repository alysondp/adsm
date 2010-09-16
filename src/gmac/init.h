/* Copyright (c) 2009 University of Illinois
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

#ifndef __INIT_H_
#define __INIT_H_

#include <util/Private.h>
#include <util/Logger.h>

#include "kernel/Process.h"

namespace gmac {

class Process;
class Context;

namespace memory {
class Manager;
class Allocator;
}

}

#include <cstdio>
namespace gmac {

extern Process *proc;
extern memory::Manager *manager;
extern memory::Allocator *allocator;

extern util::Private<const char> _inGmac;
extern const char _gmacCode;
extern const char _userCode;
extern char _gmac_init;

inline void enterGmac() { _inGmac.set(&_gmacCode); }
inline void exitGmac()  { _inGmac.set(&_userCode); }

inline char inGmac() { 
    if(_gmac_init == 0) return 1;
    char *ret = (char  *)_inGmac.get();
    if(ret == NULL) return 0;
    else if(*ret == _gmacCode) return 1;
    return 0;
}

}
#endif

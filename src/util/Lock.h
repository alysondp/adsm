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

#ifndef __UTIL_LOCK_H_
#define __UTIL_LOCK_H_

#include <config.h>
#include <threads.h>
#include <paraver.h>

#include <iostream>


namespace gmac { namespace util {

class Lock {
protected:
	MUTEX(__mutex);
	LockName __name;
public:
	Lock(LockName __name) : __name(__name) {
		MUTEX_INIT(__mutex);
	}

	~Lock() {
		MUTEX_DESTROY(__mutex);
	}

	inline void lock() {
		enterLock(__name);
		MUTEX_LOCK(__mutex);
		exitLock();
	}

	inline void unlock() {
		MUTEX_UNLOCK(__mutex);
	}
};

class RWLock {
protected:
	LOCK(__lock);
	LockName __name;
public:
	RWLock(LockName __name) : __name(__name) {
		LOCK_INIT(__lock);
	}

	~RWLock {
		LOCK_DESTROY(__lock);
	}

	inline void read() {
		enterLock(__name);
		LOCK_READ(__lock);
		exitLock();
	}

	inline void write() {
		enterLock(__name);
		LOCK_WRITE(__lock);
		exitLock();
	}

	inline void unlock() {
		LOCK_UNLOCK(__lock);
	}
};

} };
#endif

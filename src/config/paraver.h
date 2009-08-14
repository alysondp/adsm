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

#ifndef __CONFIG_PARAVER_H
#define __CONFIG_PARAVER_H

#ifdef PARAVER_GMAC
#include <paraver/Trace.h>
#include <paraver/Types.h>
extern paraver::Trace *trace;

typedef enum {
	_None_ = 0,
	_accMalloc_, _accFree_,
	_accHostDeviceCopy_, _accDeviceHostCopy_, _accDeviceDeviceCopy_,
	_accLaunch_, _accSync_,
	_gmacMalloc_, _gmacFree_, _gmacLaunch_, _gmacSync_, _gmacSignal_,
} FunctionName;


/* Macros to issue traces in paraver mode */
#define addThread()	trace->__addThread()
#define pushState(s)	trace->__pushState(*paraver::s)
#define popState()	trace->__popState()
#define pushEvent(e, ...)	trace->__pushEvent(*paraver::e, ##__VA_ARGS__)
#define enterFunction(s) trace->__pushEvent(*paraver::_Function_, s)
#define exitFunction() trace->__pushEvent(*paraver::_Function_, 0)

#else

#define addThread()
#define pushState(s)
#define popState()
#define pushEvent(e)
#define enterFunction(s)
#define exitFunction()

#endif


#endif

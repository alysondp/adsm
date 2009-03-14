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

#ifndef __PARAVER_TRACE_H
#define __PARAVER_TRACE_H

#include "Time.h"
#include "Element.h"
#include "Record.h"
#include "Names.h"

#include <common/config.h>
#include <common/threads.h>

#include <sys/time.h>

#include <assert.h>

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <fstream>

namespace paraver {

class Trace {
protected:
	std::list<Application *> apps;

	Time_t startTime;
	Time_t endTime;
	Time_t pendingTime;

	inline Time_t getTimeStamp() {
		Time_t tm = getTime() - startTime;
		endTime = (endTime > tm) ? endTime : tm;
		return tm;
	}

	void setPending(Time_t t) {
		pendingTime = (pendingTime > t) ? pendingTime : t;
	}

	void buildApp(std::ifstream &in);

	std::ofstream of;
	MUTEX(ofMutex);
	std::list<Record *> records;

public:
	Trace() : startTime(getTime()), endTime(0), pendingTime(0) {};
	Trace(const char *fileName);

	inline void addThread(void) {
		Task *task = apps.back()->getTask(getpid());
		task->addThread(gettid());
	}
	inline void addTask(void) {
		apps.back()->addTask(getpid());
	}

	void pushState(const StateName &state);
	void popState();
	void event(unsigned type, unsigned value);

	void read(const char *filename);
	void write();
	friend std::ostream &operator<<(std::ostream &os, const Trace &trace);
};

};
#endif

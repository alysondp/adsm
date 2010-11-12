#include "unit/init.h"
#include "trace/Function.h"
#include "core/Process.h"
#include "core/Accelerator.h"

#include "gtest/gtest.h"

using gmac::Process;
using gmac::trace::Function;
using gmac::Accelerator;

Accelerator *Accelerator_ = NULL;
static bool Trace_ = false;

void InitTrace(void)
{
	if(Trace_ == true) return;
	Trace_ = true;
	Function::init();
}

gmac::Accelerator &GetAccelerator()
{
	return *Accelerator_;
}

void FiniAccelerator()
{
	if(Accelerator_ == NULL) return;
	delete Accelerator_;
	Accelerator_ = NULL;
}

void InitProcess()
{
    InitTrace();
    InitAccelerator();

	Process::create<Process>();
    Process &proc = Process::getInstance();
    proc.addAccelerator(Accelerator_);
}


void FiniProcess()
{
	ASSERT_TRUE(Accelerator_ != NULL);
	Accelerator_ = NULL;
    Process::destroy();
}


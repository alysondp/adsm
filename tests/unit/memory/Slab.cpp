#include "gtest/gtest.h"
#include "core/hpe/Mode.h"
#include "core/hpe/Process.h"
#include "core/hpe/Thread.h"
#include "memory/Manager.h"
#include "memory/allocator/Slab.h"

#include <set>

using namespace gmac::core::hpe;
using namespace gmac::memory;

class SlabTest : public testing::Test {
public:
    static Process *Process_;
    static manager *Manager_;
    
    static void SetUpTestCase();
    static void TearDownTestCase();
};

Process *SlabTest::Process_ = NULL;
manager *SlabTest::Manager_ = NULL;

extern void OpenCL(Process &);
extern void CUDA(Process &);

void SlabTest::SetUpTestCase()
{
    Process_ = new Process();
    ASSERT_TRUE(Process_ != NULL);
#if defined(USE_CUDA)
    CUDA(*Process_);
#endif
#if defined(USE_OPENCL)
    OpenCL(*Process_);
#endif
    Manager_ = new manager(*Process_);
    ASSERT_TRUE(Manager_ != NULL);
}

void SlabTest::TearDownTestCase()
{
    ASSERT_TRUE(Manager_ != NULL);
    Manager_->destroy(); Manager_ = NULL;
    ASSERT_TRUE(Process_ != NULL);
    Process_->destroy(); Process_ = NULL;
}

TEST_F(SlabTest, Creation)
{
    ASSERT_TRUE(Manager_ != NULL);
    for(int i = 0; i < 16; i++) {
        allocator::slab *slab = new allocator::slab(*Manager_);
        ASSERT_TRUE(slab != NULL);
        slab->destroy();
    }
}

TEST_F(SlabTest, SmallObject)
{
    ASSERT_TRUE(Manager_ != NULL);
    allocator::slab *slab = new allocator::slab(*Manager_);

    const size_t objectSize = 1024;
    const unsigned numObjects = 128 * 1024;
    std::set<host_ptr> allocations;
    for(unsigned n = 0; n < numObjects; n++) {
        host_ptr ptr = slab->alloc(Thread::getCurrentVirtualDevice(), objectSize, host_ptr(0x1000));
        if(ptr == NULL) break;
        ASSERT_EQ(true, allocations.insert(ptr).second);
    }

    std::set<host_ptr>::const_iterator i;
    for(i = allocations.begin(); i != allocations.end(); i++) {
        ASSERT_EQ(true, slab->free(Thread::getCurrentVirtualDevice(), *i));
    }

    allocations.clear();
    slab->destroy();
}

TEST_F(SlabTest, MultiSizeObject)
{
    ASSERT_TRUE(Manager_ != NULL);
    allocator::slab *slab = new allocator::slab(*Manager_);
    ASSERT_TRUE(slab != NULL);

    const size_t minObjectSize = 64;
    const size_t maxObjectSize = 64 * 1024;
    const unsigned numObjects = 4 * 1024;
    std::set<host_ptr> allocations;
    for(unsigned n = 0; n < numObjects; n++) {
        for(size_t size = minObjectSize; size < maxObjectSize; size *= 2) {
            host_ptr ptr = slab->alloc(Thread::getCurrentVirtualDevice(), size, host_ptr(0x1000));
            if(ptr == NULL) break;
            ASSERT_EQ(true, allocations.insert(ptr).second);
        }
    }
    
    std::set<host_ptr>::const_iterator i;
    for(i = allocations.begin(); i != allocations.end(); i++) {
        ASSERT_EQ(true, slab->free(Thread::getCurrentVirtualDevice(), *i));
    }

    allocations.clear();
    slab->destroy();

}

TEST_F(SlabTest, MultiKeyObject)
{
    ASSERT_TRUE(Manager_ != NULL);
    allocator::slab *slab = new allocator::slab(*Manager_);
    ASSERT_TRUE(slab != NULL);

    const size_t objectSize = 1024;
    const unsigned numObjects = 4 * 1020;
    std::set<host_ptr> allocations;
    for(unsigned n = 0; n < numObjects; n++) {
        host_ptr ptr = slab->alloc(Thread::getCurrentVirtualDevice(), objectSize, host_ptr((n + 1) * 0x1000L));
        if(ptr == NULL) break;
        ASSERT_EQ(true, allocations.insert(ptr).second);
    }


    std::set<host_ptr>::const_iterator i;
    for(i = allocations.begin(); i != allocations.end(); i++) {
        ASSERT_EQ(true, slab->free(Thread::getCurrentVirtualDevice(), *i));
    }

    allocations.clear();
    slab->destroy();
}

TEST_F(SlabTest, MultiSizeKeyObject)
{
    ASSERT_TRUE(Manager_ != NULL);
    allocator::slab *slab = new allocator::slab(*Manager_);
    ASSERT_TRUE(slab != NULL);

    const size_t minObjectSize = 64;
    const size_t maxObjectSize = 16 * 1024;
    const unsigned numObjects = 4 * 1024;
    const unsigned keys = 16;
    std::set<host_ptr> allocations;
    for(unsigned n = 0; n < numObjects; n++) {
        for(size_t size = minObjectSize; size < maxObjectSize; size *= 2) {
            host_ptr key = host_ptr(((n & (keys - 1)) + 1) * 0x1000L);
            host_ptr ptr = slab->alloc(Thread::getCurrentVirtualDevice(), size, key);
            if(ptr == NULL) break;
            ASSERT_EQ(true, allocations.insert(ptr).second);
        }
    }
    
    std::set<host_ptr>::const_iterator i;
    for(i = allocations.begin(); i != allocations.end(); i++) {
        ASSERT_EQ(true, slab->free(Thread::getCurrentVirtualDevice(), *i));
    }

    allocations.clear();
    slab->destroy();

}

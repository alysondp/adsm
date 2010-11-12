#include "unit/core/Accelerator.h"
#include "api/cuda/Accelerator.h"

using gmac::cuda::Accelerator;

TEST_F(AcceleratorTest, AcceleratorHost) {
    int *host = NULL;
	Accelerator &accelerator = dynamic_cast<Accelerator &>(GetAccelerator());
    ASSERT_EQ(gmacSuccess, accelerator.hostAlloc((void **)&host, Size_));
    ASSERT_TRUE(host != NULL);
    ASSERT_TRUE(accelerator.hostMap(host) != NULL);
    ASSERT_EQ(gmacSuccess, accelerator.hostFree(host));
}

TEST_F(AcceleratorTest, AcceleratorMemset) {
    Accelerator &accelerator = dynamic_cast<Accelerator &>(GetAccelerator());
	unsigned *host = NULL;
    host = new unsigned[Size_];
    ASSERT_TRUE(host != NULL);
    unsigned *device = NULL;
    memset(host, 0x5a, Size_ * sizeof(unsigned));
    ASSERT_EQ(gmacSuccess, accelerator.malloc((void **)&device, Size_ * sizeof(unsigned)));
    ASSERT_TRUE(device != NULL);
    ASSERT_EQ(gmacSuccess, accelerator.memset(device, 0xa5, Size_ * sizeof(unsigned)));
    ASSERT_EQ(gmacSuccess, accelerator.copyToHost(host, device, Size_ * sizeof(unsigned)));
    for(int j = 0; j < Size_; j++) ASSERT_EQ(0xa5a5a5a5, host[j]);
    ASSERT_EQ(gmacSuccess, accelerator.free(device));

    delete[] host;
}
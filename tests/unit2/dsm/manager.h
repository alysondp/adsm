#include "gtest/gtest.h"

#include "dsm/manager.h"
#include "hal/types.h"
#include "util/misc.h"

#include "unit2/dsm/mapping.h"

using __impl::util::range;
using __impl::hal::ptr;

using __impl::dsm::coherence::block;
using __impl::dsm::coherence::block_ptr;

class manager_mapping_test : public testing::Test {
public:

protected:
	static void SetUpTestCase();
	static void TearDownTestCase();
};

class manager;
typedef manager *manager_ptr;

class GMAC_LOCAL manager :
    public __impl::dsm::manager {

    typedef __impl::dsm::manager parent;

public:
    // Forward types
    typedef parent::range_mapping range_mapping;
    typedef parent::map_mapping_group map_mapping_group;

    // Forward protected functions
    static void aspace_created(manager *m, __impl::hal::aspace &aspace);
    static void aspace_destroyed(manager *m, __impl::hal::aspace &aspace);

    template <bool IsOpen>
    range_mapping
    get_mappings_in_range(map_mapping_group &mappings, __impl::hal::ptr begin, size_t count)
    {
        return parent::get_mappings_in_range<IsOpen>(mappings, begin, count);
    }

    map_mapping_group &
    get_aspace_mappings(__impl::hal::aspace &aspace)
    {
        return parent::get_aspace_mappings(aspace);
    }

    __impl::dsm::mapping_ptr
    merge_mappings(range_mapping &range)
    {
        return parent::merge_mappings(range);
    }

    virtual ~manager()
    {
    }

    /**
     * Default constructor
     */
    manager() :
        parent()
    {
    }

    /**
     * Map the given host memory pointer to the accelerator memory. If the given
     * pointer is NULL, host memory is alllocated too.
     * \param mode Execution mode where to allocate memory
     * \param addr Memory address to be mapped or NULL if host memory is requested
     * too
     * \param size Size (in bytes) of shared memory to be mapped 
     * \param flags 
     * \param err Reference to store the error code for the operation
     * \return Address that identifies the allocated memory
     */
    gmacError_t link(__impl::hal::ptr ptr1,
                     __impl::hal::ptr ptr2, size_t count, int flags);

    gmacError_t unlink(__impl::hal::ptr mapping, size_t count);

    gmacError_t acquire(__impl::hal::ptr mapping, size_t count, int flags);
    gmacError_t release(__impl::hal::ptr mapping, size_t count);

    gmacError_t sync(__impl::hal::ptr mapping, size_t count);

    gmacError_t memcpy(__impl::hal::ptr dst, __impl::hal::ptr src, size_t count);
    gmacError_t memset(__impl::hal::ptr ptr, int c, size_t count);

    gmacError_t from_io_device(__impl::hal::ptr addr, __impl::hal::device_input &input, size_t count);
    gmacError_t to_io_device(__impl::hal::device_output &output, __impl::hal::const_ptr addr, size_t count);

    gmacError_t flush_dirty(__impl::dsm::address_space_ptr aspace);

    //////////////////////
    // Helper functions //
    //////////////////////
    bool helper_insert(__impl::hal::aspace &ctx, mapping_ptr m)
    {
        gmacError_t ret;

        parent::map_mapping_group &group = parent::get_aspace_mappings(ctx);
        ret = parent::insert_mapping(group, m);

        return ret == gmacSuccess;
    }

    bool helper_clear_mappings(__impl::hal::aspace &ctx)
    {
        parent::map_mapping_group &group = parent::get_aspace_mappings(ctx);

        group.clear();

        return true;
    }
};

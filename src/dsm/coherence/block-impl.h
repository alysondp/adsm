#ifndef GMAC_DSM_COHERENCE_BLOCK_IMPL_H_
#define GMAC_DSM_COHERENCE_BLOCK_IMPL_H_

namespace __impl { namespace dsm { namespace coherence {

inline
block::block(size_t size) :
    lock("block"),
    size_(size),
    owner_(nullptr)
{
    TRACE(LOCAL, FMT_ID2" Creating " FMT_SIZE" bytes", get_print_id2(), size);
}

inline
block::~block()
{
    TRACE(LOCAL, FMT_ID2" Deleting", get_print_id2());
}

inline
block_ptr
block::split(size_t off)
{
    TRACE(LOCAL, FMT_ID2" Splitting " FMT_SIZE, get_print_id2(), off);

    // Create new block
    block_ptr nBlock = block_ptr(new block(size_ - off));
    // Set new size
    size_ = off;

    for (auto m : mappings_) {
        auto ret = nBlock->mappings_.insert(mappings::value_type(m.first, m.second));
        ASSERTION(ret.second == true);
        // Update the offset within mapping for the new block
        ret.first->second.off_ += off;
        m.first->block_splitted<false>(shared_from_this(), nBlock);
    }

    return block_ptr(nBlock);
}

inline
void
block::shift(mapping_ptr m, size_t off)
{
    TRACE(LOCAL, FMT_ID2" Shift " FMT_SIZE" bytes", get_print_id2(), off);

    mappings::iterator it = mappings_.find(m);
    ASSERTION(it != mappings_.end());

    it->second.off_ += off;
}

inline
error
block::register_mapping(mapping_ptr m, size_t off)
{
    TRACE(LOCAL, FMT_ID2" Register " FMT_ID2, get_print_id2(), m->get_print_id2());
    ASSERTION(mappings_.find(m) == mappings_.end(), "Mapping already registered");

    state s = state::STATE_INVALID;

    if (mappings_.size() == 0) {
         s = state::STATE_SHARED;
    }

    mapping_descriptor descr = {
                                   off,
                                   s
                               };
    mappings_.insert(mappings::value_type(m, descr));
    return error::DSM_SUCCESS;
}

inline
error
block::unregister_mapping(mapping &m)
{
    TRACE(LOCAL, FMT_ID2" Unregister " FMT_ID2, get_print_id2(), m.get_print_id2());
    CHECK(&m != owner_, error::DSM_ERROR_OWNERSHIP);

    mappings::iterator it;
    it = mappings_.find(&m);

    ASSERTION(it != mappings_.end(), "Mapping not registered");
    mappings_.erase(it);

    return error::DSM_SUCCESS;
}

inline
error
block::transfer_mappings(block &&b)
{
    TRACE(LOCAL, FMT_ID2" Transferring mappings from " FMT_ID2, get_print_id2(), b.get_print_id2());

    CHECK(this != &b, error::DSM_ERROR_INVALID_VALUE);
    CHECK(size_ == b.size_, error::DSM_ERROR_INVALID_VALUE);

    // TODO: check what happens the same mapping was already registered
    mappings_.insert(b.mappings_.begin(), b.mappings_.end());

    b.mappings_.clear();

    return error::DSM_SUCCESS;
}

inline
size_t
block::get_size() const
{
    return size_;
}

}}}

#endif /* GMAC_DSM_COHERENCE_BLOCK_IMPL_H_ */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */

/* Copyright (c) 2009-2012 University of Illinois
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

#ifndef GMAC_DSM_COHERENCE_BLOCK_H_
#define GMAC_DSM_COHERENCE_BLOCK_H_

#include <map>
#include <memory>

#include "util/factory.h"
#include "util/lock.h"

#include "dsm/types.h"

#include "types.h"

namespace __impl { namespace dsm { namespace coherence {

class GMAC_LOCAL block :
    public util::unique<block>,
    gmac::util::lock_rw<block>,
    public std::enable_shared_from_this<block> {

    friend class util::factory<block, block_ptr>;

    struct mapping_descriptor {
        size_t off_;
        state state_;

        mapping_descriptor(size_t off, state s) :
            off_(off),
            state_(s)
        {
        }
    };

    size_t size_;
    mapping_ptr owner_;
    GmacProtection prot_;

    typedef gmac::util::lock_rw<block> lock;

    typedef std::map<mapping_ptr, mapping_descriptor> mappings;
    mappings mappings_;

    error update(mapping_ptr m, size_t off);

    block(size_t size);

public:
    virtual ~block();

    block_ptr split(size_t off);
    void shift(mapping_ptr m, size_t off);

    error acquire(mapping_ptr aspace, GmacProtection prot);
    error release(mapping_ptr aspace);

    error register_mapping(mapping_ptr m, size_t off);
    error unregister_mapping(mapping &m);

    template <bool Write>
    error handle_fault(mapping_ptr m, size_t offLocal)
    {
        ASSERTION(mappings_.find(m) != mappings_.end());
        ASSERTION(!Write || prot_is_writable(m->get_protection()));
        ASSERTION( Write || prot_is_readable(m->get_protection()));
        ASSERTION(has_mapping_flag(m->get_flags(), mapping_flags::MAP_USE_PROTECT));

        error ret = error::DSM_SUCCESS;

        mapping_descriptor &desc = mappings_.find(m)->second;

        switch (desc.state_) {
        case state::STATE_DIRTY:
        case state::STATE_SHARED:
            ret = error::DSM_ERROR_PROTOCOL;
            break;
        case state::STATE_INVALID:
            // TODO: Check for asynchronous updates
            ret = update(m, desc.off_);
            if (ret == error::DSM_SUCCESS && Write) {
                GmacProtection prot = prot_is_readable(m->get_protection())? GMAC_PROT_READWRITE: GMAC_PROT_WRITE;

                // Unprotect memory to allow write accesses
                hal::error errHal;
                errHal = m->get_ptr().get_view().get_vaspace().protect(m->get_ptr() + desc.off_, size_, prot);
                if (errHal != hal::error::HAL_SUCCESS) {
                    ret = error::DSM_ERROR_HAL;
                }

                desc.state_ = state::STATE_DIRTY;
            } else if (ret == error::DSM_SUCCESS) {
                // Unprotect memory to allow read accesses
                hal::error errHal;
                errHal = m->get_ptr().get_view().get_vaspace().protect(m->get_ptr() + desc.off_, size_, GMAC_PROT_READ);
                if (errHal != hal::error::HAL_SUCCESS) {
                    ret = error::DSM_ERROR_HAL;
                }

                desc.state_ = state::STATE_SHARED;
            }
            break;
        }

        return ret;
    }

    error transfer_mappings(block &&b);

    size_t get_size() const;
};

}}}

#include "block-impl.h"

#endif // GMAC_DSM_COHERENCE_BLOCK_H_

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
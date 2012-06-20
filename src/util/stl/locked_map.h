/* Copyright (c) 2011 University of Illinois
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
 WITH THE SOFTWARE.
 */

#ifndef GMAC_UTIL_STL_LOCKED_MAP_H_
#define GMAC_UTIL_STL_LOCKED_MAP_H_

#include "util/lock.h"

namespace __impl { namespace util { namespace stl {

template <typename K, typename V>
class GMAC_LOCAL locked_map :
    private std::map<K, V>,
    protected gmac::util::lock_rw<locked_map<K, V> > {
    typedef std::map<K, V> Parent;
    typedef gmac::util::lock_rw<locked_map<K, V> > Lock;

public:
    typedef typename std::map<K, V>::iterator iterator;
    typedef typename std::map<K, V>::const_iterator const_iterator;
    typedef typename std::map<K, V>::value_type value_type;
    typedef typename std::map<K, V>::size_type size_type;
    typedef typename std::map<K, V>::key_type key_type;

    locked_map(const std::string &name);

    std::pair<iterator, bool> insert(const value_type &x);
    iterator insert(iterator position, const value_type &x);
    void insert(iterator first, iterator last);

    iterator find(const key_type &key);
    //iterator begin();
    iterator end();

    const_iterator find(const key_type &key) const;
    //const_iterator begin() const;
    const_iterator end() const;

    void erase(iterator position);
    size_type erase(const key_type &x);
    void erase(iterator first, iterator last);

    iterator upper_bound(const key_type& x);
    const_iterator upper_bound(const key_type& x) const;

    size_type size() const;
};

}}}

#include "locked_map-impl.h"

#endif // GMAC_UTIL_STL_LOCKED_MAP_H_

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
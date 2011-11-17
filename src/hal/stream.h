#ifndef GMAC_HAL_TYPES_STREAM_H_
#define GMAC_HAL_TYPES_STREAM_H_

#include <algorithm>
#include <queue>

#include "util/gmac_base.h"
#include "util/lock.h"
#include "util/Logger.h"

namespace __impl { namespace hal {

namespace detail {

template <typename T>
class GMAC_LOCAL map_pool :
    std::map<size_t, std::list<T *> >,
    gmac::util::mutex {

    typedef std::list<T *> queue_subset;
    typedef std::map<size_t, queue_subset> Parent;

public:
    map_pool() :
        gmac::util::mutex("map_pool")
    {
    }

    T *pop(size_t size)
    {
        T *ret = NULL;

        lock();
        typename Parent::iterator it;
        it = Parent::lower_bound(size);
        if (it != Parent::end()) {
            queue_subset &queue = it->second;
            if (queue.size() > 0) {
                ret = queue.front();
                queue.pop_front();
            }
        }
        unlock();

        return ret;
    }

    bool remove(T *val, size_t size)
    {
        bool ret = false;

        lock();
        typename Parent::iterator it;
        it = Parent::lower_bound(size);
        if (it != Parent::end()) {
            queue_subset &queue = it->second;

            typename queue_subset::iterator it2;
            it2 = std::find(queue.begin(), queue.end(), val);
            if (it2 != queue.end()) {
                queue.erase(it2);
                ret = true;
            }
        }
        unlock();

        return ret;
    }

    void push(T *v, size_t size)
    {
        lock();
        typename Parent::iterator it;
        it = Parent::find(size);
        if (it != Parent::end()) {
            queue_subset &queue = it->second;
            queue.push_back(v);
        } else {
            queue_subset queue;
            queue.push_back(v);
            Parent::insert(typename Parent::value_type(size, queue));
        }
        unlock();
    }
};

class GMAC_LOCAL local_mutex :
    public gmac::util::mutex {
public:
    local_mutex(const std::string &name) : gmac::util::mutex(name.c_str())
    {}
    void lock() { gmac::util::mutex::lock(); }
    void unlock() { gmac::util::mutex::unlock(); }
};

template <typename B, typename I>
class GMAC_LOCAL stream_t :
    public util::gmac_base<stream_t<B, I> > {
    typedef typename I::context context_parent_t;
    friend class I::context;

private:
#if 0
    typedef map_pool<typename I::buffer> map_buffer;

    local_mutex lockBuffer_;
    typename I::buffer *buffer_;

    map_buffer mapBuffersIn_;
    map_buffer mapBuffersOut_;
#endif
    
protected:
    typename B::stream stream_;
    context_parent_t &context_;

    stream_t(typename B::stream stream, context_parent_t &context);

public:
    enum state {
        Empty,
        Running
    };

    context_parent_t &get_context();
    typename B::stream &operator()();
    const typename B::stream &operator()() const;

    virtual state query() = 0;
    virtual gmacError_t sync() = 0;

};

}

}}

#endif /* STREAM_H */

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
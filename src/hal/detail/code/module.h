#ifndef GMAC_HAL_DETAIL_CODE_REPOSITORY_H_
#define GMAC_HAL_DETAIL_CODE_REPOSITORY_H_

#include <string>

#include "hal/error.h"

#include "util/tags.h"

namespace __impl { namespace hal { namespace detail { namespace code {

class kernel;

class GMAC_LOCAL repository
{
public:
    enum module_type {
        MODULE_FILE,
        MODULE_BUFFER,
        MODULE_HANDLE
    };

    class GMAC_LOCAL module_descriptor_base :
        public util::taggeable<> {
        const module_type type_;
        const std::string flags_;

    protected:
        module_descriptor_base(module_type type, const std::string &flags) :
            type_(type),
            flags_(flags)
        {
        }

    public:
        module_type get_type() const
        {
            return type_;
        }

        const std::string &get_flags() const
        {
            return flags_;
        }
    };

    template <module_type MType>
    class module_descriptor;
    
    typedef module_descriptor<MODULE_FILE>   descriptor_file;
    typedef module_descriptor<MODULE_BUFFER> descriptor_buffer;
    typedef module_descriptor<MODULE_HANDLE> descriptor_handle;

    typedef std::list<descriptor_file>   list_descriptor_file;
    typedef std::list<descriptor_buffer> list_descriptor_buffer;
    typedef std::list<descriptor_handle> list_descriptor_handle;

protected:
    list_descriptor_file   descriptorsFile_;
    list_descriptor_buffer descriptorsBuffer_;
    list_descriptor_handle descriptorsHandle_;

public:
    hal::error load_from_file(const std::string &path,
                              const std::string &flags,
                              const util::taggeable<>::set_tag &tags = util::taggeable<>::empty);

    hal::error load_from_mem(const char *ptr,
                             size_t size,
                             const std::string &flags,
                             const util::taggeable<>::set_tag &tags = util::taggeable<>::empty);

    hal::error load_from_handle(const void *handle,
                                const std::string &flags,
                                const util::taggeable<>::set_tag &tags = util::taggeable<>::empty);

    const list_descriptor_file &get_files() const
    {
        return descriptorsFile_;
    }

    const list_descriptor_buffer &get_buffers() const
    {
        return descriptorsBuffer_;
    }

    const list_descriptor_handle &get_handles() const
    {
        return descriptorsHandle_;
    }
};

template <>
class GMAC_LOCAL repository::module_descriptor<repository::MODULE_FILE> :
    public module_descriptor_base
{
    const std::string path_;
public:
    module_descriptor(const std::string &path,
                      const std::string &flags) :
        repository::module_descriptor_base(MODULE_FILE, flags),
        path_(path)
    {
    }

    const std::string &get_path() const
    {
        return path_;
    }
};

template <>
class GMAC_LOCAL repository::module_descriptor<repository::MODULE_BUFFER> :
    public module_descriptor_base
{
    const char *const ptr_;
    const size_t size_;
public:
    module_descriptor(const char * const ptr, size_t size,
                      const std::string &flags) :
        module_descriptor_base(MODULE_BUFFER, flags),
        ptr_(ptr),
        size_(size)
    {
    }

    const char *get_ptr() const
    {
        return ptr_;
    }
};

template <>
class GMAC_LOCAL repository::module_descriptor<repository::MODULE_HANDLE> :
    public module_descriptor_base
{
    const void * const handle_;
public:
    module_descriptor(const void * const handle,
                      const std::string &flags) :
        module_descriptor_base(MODULE_HANDLE, flags),
        handle_(handle)
    {
    }

    const void *get_handle() const
    {
        return handle_;
    }
};

class GMAC_LOCAL repository_view
{
public:
    virtual ~repository_view()
    {}
#if 0
    virtual kernel *get_kernel(gmac_kernel_id_t key) = 0;
#endif
    virtual const kernel *get_kernel(const std::string &name,
                                     const util::taggeable<>::set_tag &filter = util::taggeable<>::empty) = 0;
};
    
}}}}

#endif

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
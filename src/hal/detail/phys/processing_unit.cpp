#include "processing_unit.h"
#include "hal/detail/types.h"

namespace __impl { namespace hal { namespace detail { namespace phys {

processing_unit::processing_unit(type t, platform &platform, aspace &as) :
    platform_(platform),
    type_(t),
    as_(as)
{
    as.add_processing_unit(*this);
}

void
processing_unit::add_memory_connection(const memory_connection &connection)
{
    connections_.insert(connection);
    connection.mem->add_attached_unit(*this);
}

}}}}
/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
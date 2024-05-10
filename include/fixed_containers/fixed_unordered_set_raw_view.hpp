#pragma once

#include "fixed_containers/fixed_doubly_linked_list_raw_view.hpp"

namespace fixed_containers
{

class FixedUnorderedSetRawView
  : public fixed_doubly_linked_list_detail::FixedDoublyLinkedListRawView<uint32_t>
{
    using Base = fixed_doubly_linked_list_detail::FixedDoublyLinkedListRawView<uint32_t>;

public:
    using Base::const_iterator;
    using Base::Iterator;
    using Base::iterator;

private:
    static constexpr const void* get_linked_list_ptr(const void* map_ptr)
    {
        // `value_storage_` is the first member of `FixedRobinhoodHashtable`
        return map_ptr;
    }

public:
    FixedUnorderedSetRawView(const void* set_ptr,
                             std::size_t elem_size,
                             std::size_t elem_align,
                             std::size_t elem_count)
      : Base(get_linked_list_ptr(set_ptr), elem_size, elem_align, elem_count)
    {
    }
};

}  // namespace fixed_containers

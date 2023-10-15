#pragma once

#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"
#include "fixed_containers/stack_adapter.hpp"

#include <cstddef>

namespace fixed_containers
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedStack : public StackAdapter<FixedVector<T, MAXIMUM_SIZE, CheckingType>>
{
private:
    using Base = StackAdapter<FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

public:
    constexpr FixedStack()
      : Base{}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedStack(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base{first, last, loc}
    {
    }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr typename FixedStack<T, MAXIMUM_SIZE, CheckingType>::size_type is_full(
    const FixedStack<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= MAXIMUM_SIZE;
}

}  // namespace fixed_containers

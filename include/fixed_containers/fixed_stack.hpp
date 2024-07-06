#pragma once

#include "fixed_containers/concepts.hpp"
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
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

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
[[nodiscard]] constexpr bool is_full(const FixedStack<T, MAXIMUM_SIZE, CheckingType>& container)
{
    return container.size() >= MAXIMUM_SIZE;
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedStack<T, MAXIMUM_SIZE, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std

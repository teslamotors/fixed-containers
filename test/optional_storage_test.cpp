#include "fixed_containers/optional_storage.hpp"

#include <cstdint>
#include <tuple>

namespace fixed_containers::optional_storage_detail
{

static_assert(sizeof(OptionalStorage<int>) == sizeof(int));
static_assert(sizeof(OptionalStorage<double>) == sizeof(double));
static_assert(sizeof(OptionalStorage<std::uint8_t>) == sizeof(uint8_t));
static_assert(sizeof(OptionalStorage<std::uint16_t>) == sizeof(uint16_t));
static_assert(sizeof(OptionalStorage<std::uint32_t>) == sizeof(uint32_t));
static_assert(sizeof(OptionalStorage<std::uint64_t>) == sizeof(uint64_t));
static_assert(sizeof(OptionalStorage<std::tuple<uint8_t, uint64_t>>) ==
              sizeof(std::tuple<uint8_t, uint64_t>));

}  // namespace fixed_containers::optional_storage_detail

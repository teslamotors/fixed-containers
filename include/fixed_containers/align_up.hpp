#pragma once

#include "fixed_containers/assert_or_abort.hpp"

#include <cstdlib>
#include <type_traits>

namespace fixed_containers
{

template <class M, class N>
constexpr std::common_type_t<M, N> align_up(M m_val, N n_val)
{
    assert_or_abort(n_val > 0);
    return m_val + n_val - 1 - ((m_val + n_val - 1) % n_val);
}

}  // namespace fixed_containers

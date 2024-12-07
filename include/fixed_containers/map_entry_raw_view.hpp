#pragma once

#include <cstddef>
#include <iterator>

class MapEntryRawView
{
private:
    const std::byte* base_ptr_;
    const std::ptrdiff_t value_offs_;

public:
    static constexpr std::ptrdiff_t get_value_offs(std::size_t key_size,
                                                   std::size_t /*key_alignment*/,
                                                   std::size_t /*value_size*/,
                                                   std::size_t value_alignment)
    {
        std::size_t value_offs = key_size;
        // align the value start addr to the correct alignment
        if (value_offs % value_alignment != 0)
        {
            value_offs += value_alignment - value_offs % value_alignment;
        }
        return static_cast<std::ptrdiff_t>(value_offs);
    }

public:
    MapEntryRawView(const void* ptr,
                    std::size_t key_size,
                    std::size_t key_alignment,
                    std::size_t value_size,
                    std::size_t value_alignment)
      : base_ptr_{reinterpret_cast<const std::byte*>(ptr)}
      , value_offs_{get_value_offs(key_size, key_alignment, value_size, value_alignment)}
    {
    }

    [[nodiscard]] const std::byte* key() const { return base_ptr_; }

    [[nodiscard]] const std::byte* value() const { return std::next(base_ptr_, value_offs_); }

    [[nodiscard]] std::ptrdiff_t value_offset() const { return value_offs_; }
};

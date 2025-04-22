#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/forward_iterator.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <memory>

namespace fixed_containers
{

template <typename K, typename TableImpl, typename CheckingType>
class FixedSetAdapter
{
public:
    using key_type = K;
    using value_type = K;
    using reference = const K&;
    using const_reference = const K&;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

private:
    using TableIndex = typename TableImpl::OpaqueIndexType;
    using TableIteratedIndex = typename TableImpl::OpaqueIteratedType;

    class ReferenceProvider
    {
        friend class FixedSetAdapter;

    private:
        const TableImpl* table_;
        TableIteratedIndex current_index_;

        constexpr ReferenceProvider(const TableImpl* const table,
                                    const TableIteratedIndex& value_table_index)
          : table_(table)
          , current_index_(value_table_index)
        {
        }

    public:
        constexpr ReferenceProvider() noexcept
          : table_(nullptr)
          , current_index_(TableImpl::invalid_index())
        {
        }

        constexpr void advance() noexcept { current_index_ = table_->next_of(current_index_); }

        [[nodiscard]] constexpr const_reference get() const noexcept
        {
            return table_->key_at(current_index_);
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept = default;
    };

    template <IteratorConstness CONSTNESS>
    using Iterator = ForwardIterator<ReferenceProvider, ReferenceProvider, CONSTNESS>;

public:
    using const_iterator = Iterator<IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = const_iterator;

    using size_type = std::size_t;
    using difference_type = ptrdiff_t;

public:
    static constexpr size_type static_max_size() noexcept { return TableImpl::CAPACITY; }

public:
    TableImpl IMPLEMENTATION_DETAIL_DO_NOT_USE_table_;

private:
    [[nodiscard]] constexpr TableImpl& table() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_table_; }

    [[nodiscard]] constexpr const TableImpl& table() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_table_;
    }

public:
    template <typename... Args>
    explicit constexpr FixedSetAdapter(Args&&... args)
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_table_(std::forward<Args>(args)...)
    {
    }

    constexpr FixedSetAdapter() = default;

public:
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator{ReferenceProvider{std::addressof(table()), table().begin_index()}};
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return const_iterator{ReferenceProvider{std::addressof(table()), table().end_index()}};
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }

    [[nodiscard]] constexpr size_type max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return table().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return table().size() == 0; }

    constexpr void clear() noexcept { table().clear(); }

    constexpr std::pair<iterator, bool> insert(
        const K& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(value);
        if (table().exists(idx))
        {
            return {create_const_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, value);
        return {create_const_iterator(idx), true};
    }

    constexpr std::pair<iterator, bool> insert(
        K&& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(value);
        if (table().exists(idx))
        {
            return {create_const_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, std::move(value));
        return {create_const_iterator(idx), true};
    }

    constexpr const_iterator insert(const_iterator /*hint*/,
                                    const K& key,
                                    const std_transition::source_location& loc =
                                        std_transition::source_location::current()) noexcept
    {
        return insert(key, loc).first;
    }

    constexpr const_iterator insert(const_iterator /*hint*/,
                                    K&& key,
                                    const std_transition::source_location& loc =
                                        std_transition::source_location::current()) noexcept
    {
        return insert(std::move(key), loc).first;
    }

    template <InputIterator InputIt>
    constexpr void insert(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
    {
        for (; first != last; std::advance(first, 1))
        {
            this->insert(*first, loc);
        }
    }

    constexpr void insert(std::initializer_list<value_type> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
    {
        this->insert(list.begin(), list.end(), loc);
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            return {create_const_iterator(idx), false};
        }

        check_not_full(std_transition::source_location::current());
        idx = table().emplace(idx, key, std::forward<Args>(args)...);
        return {create_const_iterator(idx), true};
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            return {create_const_iterator(idx), false};
        }

        check_not_full(std_transition::source_location::current());
        idx = table().emplace(idx, std::move(key), std::forward<Args>(args)...);
        return {create_const_iterator(idx), true};
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const_iterator /*hint*/,
                                                    const K& key,
                                                    Args&&... args) noexcept
    {
        return try_emplace(key, std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const_iterator /*hint*/,
                                                    K&& key,
                                                    Args&&... args) noexcept
    {
        return try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        return insert(K{std::forward<Args>(args)...});
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> emplace_hint(const_iterator /*hint*/,
                                                     Args&&... args) noexcept
    {
        return emplace(std::forward<Args>(args)...);
    }

    constexpr iterator erase(const_iterator pos) noexcept
    {
        // TODO: shouldn't these be CheckingType:: checks?
        assert_or_abort(pos != cend());
        const TableIndex idx = table().opaque_index_of(*pos);
        assert_or_abort(table().exists(idx));
        const TableIteratedIndex next_idx = table().erase(idx);
        return iterator{ReferenceProvider{std::addressof(table()), next_idx}};
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        const ReferenceProvider& start =
            first.template private_reference_provider<const ReferenceProvider&>();
        const ReferenceProvider& end =
            last.template private_reference_provider<const ReferenceProvider&>();
        const TableIteratedIndex next_idx =
            table().erase_range(start.current_index_, end.current_index_);
        return iterator{ReferenceProvider{std::addressof(table()), next_idx}};
    }

    constexpr size_type erase(const key_type& key) noexcept
    {
        const TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            return 0;
        }
        table().erase(idx);
        return 1;
    }

    [[nodiscard]] constexpr iterator find(const K& key) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        return create_checked_iterator(idx);
    }

    [[nodiscard]] constexpr const_iterator find(const K& key) const noexcept
    {
        const TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            return cend();
        }
        return create_const_iterator(idx);
    }

    // TODO: handle transparent versions of functions (need to check tranparency carefully)

    [[nodiscard]] constexpr bool contains(const K& key) const noexcept
    {
        const TableIndex idx = table().opaque_index_of(key);
        return table().exists(idx);
    }

    [[nodiscard]] constexpr std::size_t count(const K& key) const noexcept
    {
        return static_cast<std::size_t>(contains(key));
    }

    template <typename TableImpl2, typename CheckingType2>
    [[nodiscard]] constexpr bool operator==(
        const FixedSetAdapter<K, TableImpl2, CheckingType2>& other) const
    {
        if (size() != other.size())
        {
            return false;
        }
        return std::ranges::all_of(*this,
                                   [&other](const auto& key) { return other.contains(key); });
    }

private:
    constexpr iterator create_checked_iterator(const TableIndex& index) noexcept
    {
        // check for nonexistent indices and replace them with end() so the iterator compares
        // correctly
        if (!table().exists(index))
        {
            return end();
        }
        return create_const_iterator(index);
    }

    [[nodiscard]] constexpr iterator create_const_iterator(
        const TableIndex& start_index) const noexcept
    {
        return iterator{
            ReferenceProvider{std::addressof(table()), table().iterated_index_from(start_index)}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(table().size() < TableImpl::CAPACITY))
        {
            CheckingType::length_error(TableImpl::CAPACITY + 1, loc);
        }
    }
};

template <typename K, typename TableImpl, typename CheckingType>
[[nodiscard]] constexpr bool is_full(const FixedSetAdapter<K, TableImpl, CheckingType>& container)
{
    return container.size() >= container.max_size();
}

template <typename K, typename TableImpl, typename CheckingType, typename Predicate>
constexpr typename FixedSetAdapter<K, TableImpl, CheckingType>::size_type erase_if(
    FixedSetAdapter<K, TableImpl, CheckingType>& container, Predicate predicate)
{
    return erase_if_detail::erase_if_impl(container, predicate);
}

}  // namespace fixed_containers

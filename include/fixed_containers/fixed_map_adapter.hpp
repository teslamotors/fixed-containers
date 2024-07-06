#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/emplace.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/forward_iterator.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <memory>

namespace fixed_containers
{

template <typename K, typename V, typename TableImpl, typename CheckingType>
class FixedMapAdapter
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using reference = std::pair<const K&, V&>;
    using const_reference = std::pair<const K&, const V&>;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

private:
    using TableIndex = typename TableImpl::OpaqueIndexType;
    using TableIteratedIndex = typename TableImpl::OpaqueIteratedType;

    template <bool IS_CONST>
    class PairProvider
    {
        friend class PairProvider<!IS_CONST>;
        friend class FixedMapAdapter;
        using ConstOrMutableTable = std::conditional_t<IS_CONST, const TableImpl, TableImpl>;

    private:
        ConstOrMutableTable* table_;
        TableIteratedIndex current_index_;

        constexpr PairProvider(ConstOrMutableTable* const table,
                               const TableIteratedIndex& value_table_index)
          : table_(table)
          , current_index_(value_table_index)
        {
        }

    public:
        constexpr PairProvider() noexcept
          : table_(nullptr)
          , current_index_(TableImpl::invalid_index())
        {
        }

        constexpr PairProvider(const PairProvider&) = default;
        constexpr PairProvider(PairProvider&&) noexcept = default;
        constexpr PairProvider& operator=(const PairProvider&) = default;
        constexpr PairProvider& operator=(PairProvider&&) noexcept = default;

        // https://github.com/llvm/llvm-project/issues/62555
        template <bool IS_CONST_2>
        constexpr PairProvider(const PairProvider<IS_CONST_2>& mutable_other) noexcept
            requires(IS_CONST and !IS_CONST_2)
          : PairProvider{mutable_other.table_, mutable_other.current_index_}
        {
        }

        constexpr void advance() noexcept { current_index_ = table_->next_of(current_index_); }

        [[nodiscard]] constexpr std::conditional_t<IS_CONST, const_reference, reference> get()
            const noexcept
        {
            // auto for auto const/mut
            return {table_->key_at(current_index_), table_->value_at(current_index_)};
        }

        template <bool IS_CONST2>
        constexpr bool operator==(const PairProvider<IS_CONST2>& other) const noexcept
        {
            return table_ == other.table_ && current_index_ == other.current_index_;
        }
    };

    template <IteratorConstness CONSTNESS>
    using Iterator = ForwardIterator<PairProvider<true>, PairProvider<false>, CONSTNESS>;

public:
    using const_iterator = Iterator<IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator<IteratorConstness::MUTABLE_ITERATOR>;

    using size_type = std::size_t;
    using difference_type = ptrdiff_t;

public:
    static constexpr size_type static_max_size() noexcept { return TableImpl::CAPACITY; }

public:
    TableImpl IMPLEMENTATION_DETAIL_DO_NOT_USE_table_;

private:
    constexpr TableImpl& table() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_table_; }

    [[nodiscard]] constexpr const TableImpl& table() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_table_;
    }

public:
    template <typename... Args>
    explicit constexpr FixedMapAdapter(Args&&... args)
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_table_(std::forward<Args>(args)...)
    {
    }

    constexpr FixedMapAdapter() = default;

public:
    [[nodiscard]] constexpr V& at(const K& key,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
    {
        const TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return table().value(idx);
    }

    [[nodiscard]] constexpr const V& at(
        const K& key,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        const TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return table().value(idx);
    }

    constexpr V& operator[](const K& key) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            check_not_full(std_transition::source_location::current());
            idx = table().emplace(idx, key);
        }
        return table().value(idx);
    }

    constexpr V& operator[](K&& key) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (!table().exists(idx))
        {
            check_not_full(std_transition::source_location::current());
            idx = table().emplace(idx, std::move(key));
        }
        return table().value(idx);
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator{PairProvider<true>{std::addressof(table()), table().begin_index()}};
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return const_iterator{PairProvider<true>{std::addressof(table()), table().end_index()}};
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator begin() noexcept
    {
        return iterator{PairProvider<false>{std::addressof(table()), table().begin_index()}};
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator end() noexcept
    {
        return iterator{PairProvider<false>{std::addressof(table()), table().end_index()}};
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return table().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return table().size() == 0; }

    constexpr void clear() noexcept { table().clear(); }

    constexpr std::pair<iterator, bool> insert(
        const value_type& pair,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(pair.first);
        if (table().exists(idx))
        {
            return {create_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, pair.first, pair.second);
        return {create_iterator(idx), true};
    }

    constexpr std::pair<iterator, bool> insert(
        value_type&& pair,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(pair.first);
        if (table().exists(idx))
        {
            return {create_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, std::move(pair.first), std::move(pair.second));
        return {create_iterator(idx), true};
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

    template <class M>
        requires std::is_assignable_v<mapped_type&, M&&>
    constexpr std::pair<iterator, bool> insert_or_assign(
        const key_type& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            table().value(idx) = std::forward<M>(obj);
            return {create_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, key, std::forward<M>(obj));
        return {create_iterator(idx), true};
    }

    template <class M>
        requires std::is_assignable_v<mapped_type&, M&&>
    constexpr std::pair<iterator, bool> insert_or_assign(
        key_type&& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            table().value(idx) = std::forward<M>(obj);
            return {create_iterator(idx), false};
        }

        check_not_full(loc);
        idx = table().emplace(idx, std::move(key), std::forward<M>(obj));
        return {create_iterator(idx), true};
    }

    template <class M>
        requires std::is_assignable_v<mapped_type&, M&&>
    constexpr iterator insert_or_assign(const_iterator /*hint*/,
                                        const key_type& key,
                                        M&& obj,
                                        const std_transition::source_location& loc =
                                            std_transition::source_location::current()) noexcept
    {
        return insert_or_assign(key, std::forward<M>(obj), loc).first;
    }

    template <class M>
        requires std::is_assignable_v<mapped_type&, M&&>
    constexpr iterator insert_or_assign(const_iterator /*hint*/,
                                        key_type&& key,
                                        M&& obj,
                                        const std_transition::source_location& loc =
                                            std_transition::source_location::current()) noexcept
    {
        return insert_or_assign(std::move(key), std::forward<M>(obj), loc).first;
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            return {create_iterator(idx), false};
        }

        check_not_full(std_transition::source_location::current());
        idx = table().emplace(idx, key, std::forward<Args>(args)...);
        return {create_iterator(idx), true};
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) noexcept
    {
        TableIndex idx = table().opaque_index_of(key);
        if (table().exists(idx))
        {
            return {create_iterator(idx), false};
        }

        check_not_full(std_transition::source_location::current());
        idx = table().emplace(idx, std::move(key), std::forward<Args>(args)...);
        return {create_iterator(idx), true};
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
        return emplace_detail::emplace_in_terms_of_try_emplace_impl(*this,
                                                                    std::forward<Args>(args)...);
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
        const TableIndex idx = table().opaque_index_of(pos->first);
        assert_or_abort(table().exists(idx));
        const TableIteratedIndex next_idx = table().erase(idx);
        return iterator{PairProvider<false>{std::addressof(table()), next_idx}};
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        const PairProvider<true>& start =
            first.template private_reference_provider<const PairProvider<true>&>();
        const PairProvider<true>& end =
            last.template private_reference_provider<const PairProvider<true>&>();
        const TableIteratedIndex next_idx =
            table().erase_range(start.current_index_, end.current_index_);
        return iterator{PairProvider<false>{std::addressof(table()), next_idx}};
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
        const TableIndex idx = table().opaque_index_of(key);
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

    // TODO: make a subclass of this for ordered maps with all the fun functions there

    template <typename MapImpl2, typename CheckingType2>
    [[nodiscard]] constexpr bool operator==(
        const FixedMapAdapter<K, V, MapImpl2, CheckingType2>& other) const
    {
        using Other = FixedMapAdapter<K, V, MapImpl2, CheckingType2>;
        if (size() != other.size())
        {
            return false;
        }
        return std::ranges::all_of(
            *this,
            [&other](const auto& pair)
            {
                const typename Other::const_iterator other_it = other.find(pair.first);
                return other_it != other.end() && other_it->second == pair.second;
            });
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
        return create_iterator(index);
    }

    constexpr iterator create_iterator(const TableIndex& start_index) noexcept
    {
        return iterator{
            PairProvider<false>{std::addressof(table()), table().iterated_index_from(start_index)}};
    }

    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const TableIndex& start_index) const noexcept
    {
        return const_iterator{
            PairProvider<true>{std::addressof(table()), table().iterated_index_from(start_index)}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(table().size() < TableImpl::CAPACITY))
        {
            CheckingType::length_error(TableImpl::CAPACITY + 1, loc);
        }
    }
};

template <typename K, typename V, typename TableImpl, typename CheckingType>
[[nodiscard]] constexpr bool is_full(
    const FixedMapAdapter<K, V, TableImpl, CheckingType>& container)
{
    return container.size() >= container.max_size();
}

template <typename K, typename V, typename TableImpl, typename CheckingType, typename Predicate>
constexpr typename FixedMapAdapter<K, V, TableImpl, CheckingType>::size_type erase_if(
    FixedMapAdapter<K, V, TableImpl, CheckingType>& container, Predicate predicate)
{
    return erase_if_detail::erase_if_impl(container, predicate);
}

}  // namespace fixed_containers

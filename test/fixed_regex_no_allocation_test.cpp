#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/fixed_regex.hpp"
#include "fixed_containers/fixed_string.hpp"

#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <new>
#include <string>
#include <string_view>
#include <utility>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

namespace
{
bool allocations_forbidden = false;

void check_allocation() { fixed_containers::assert_or_abort(!allocations_forbidden); }

void* allocate(std::size_t size)
{
    check_allocation();
    void* result = std::malloc(size == 0 ? 1 : size);
    if (result == nullptr) std::abort();
    return result;
}

void* allocate_aligned(std::size_t size, std::align_val_t alignment)
{
    check_allocation();
    const auto align = static_cast<std::size_t>(alignment);
    size = size == 0 ? align : ((size + align - 1) / align) * align;
#if defined(_MSC_VER)
    void* result = _aligned_malloc(size, align);
#else
    void* result = std::aligned_alloc(align, size);
#endif
    if (result == nullptr) std::abort();
    return result;
}

void deallocate_aligned(void* ptr)
{
#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}
}  // namespace

// This executable deliberately has no test-framework dependency. Any new during
// the measured scope fails immediately, including allocation by hidden temporaries.
void operator delete(void* ptr, std::size_t size) noexcept;
void operator delete[](void* ptr, std::size_t size) noexcept;
void operator delete(void* ptr, std::size_t size, std::align_val_t alignment) noexcept;
void operator delete[](void* ptr, std::size_t size, std::align_val_t alignment) noexcept;
void* operator new(std::size_t size) { return allocate(size); }
void* operator new[](std::size_t size) { return allocate(size); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept { return allocate(size); }
void* operator new(std::size_t size, std::align_val_t alignment)
{
    return allocate_aligned(size, alignment);
}
void* operator new[](std::size_t size, std::align_val_t alignment)
{
    return allocate_aligned(size, alignment);
}
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    return allocate_aligned(size, alignment);
}
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    return allocate_aligned(size, alignment);
}
void operator delete(void* ptr) noexcept { std::free(ptr); }
void operator delete[](void* ptr) noexcept { std::free(ptr); }
void operator delete(void* ptr, std::size_t) noexcept { std::free(ptr); }
void operator delete[](void* ptr, std::size_t) noexcept { std::free(ptr); }
void operator delete(void* ptr, const std::nothrow_t&) noexcept { std::free(ptr); }
void operator delete[](void* ptr, const std::nothrow_t&) noexcept { std::free(ptr); }
void operator delete(void* ptr, std::align_val_t) noexcept { deallocate_aligned(ptr); }
void operator delete[](void* ptr, std::align_val_t) noexcept { deallocate_aligned(ptr); }
void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept { deallocate_aligned(ptr); }
void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{
    deallocate_aligned(ptr);
}
void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    deallocate_aligned(ptr);
}
void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    deallocate_aligned(ptr);
}

#if defined(FIXED_CONTAINERS_WRAP_ALLOCATIONS)
extern "C"
{
    void* regex_real_malloc(std::size_t size) asm("__real_malloc");
    void* regex_real_calloc(std::size_t count, std::size_t size) asm("__real_calloc");
    void* regex_real_realloc(void* ptr, std::size_t size) asm("__real_realloc");
    void* regex_real_aligned_alloc(std::size_t alignment,
                                   std::size_t size) asm("__real_aligned_alloc");
    void* regex_wrap_malloc(std::size_t size) asm("__wrap_malloc");
    void* regex_wrap_calloc(std::size_t count, std::size_t size) asm("__wrap_calloc");
    void* regex_wrap_realloc(void* ptr, std::size_t size) asm("__wrap_realloc");
    void* regex_wrap_aligned_alloc(std::size_t alignment,
                                   std::size_t size) asm("__wrap_aligned_alloc");
    void* regex_wrap_malloc(std::size_t size)
    {
        check_allocation();
        return regex_real_malloc(size);
    }
    void* regex_wrap_calloc(std::size_t count, std::size_t size)
    {
        check_allocation();
        return regex_real_calloc(count, size);
    }
    void* regex_wrap_realloc(void* ptr, std::size_t size)
    {
        check_allocation();
        return regex_real_realloc(ptr, size);
    }
    void* regex_wrap_aligned_alloc(std::size_t alignment, std::size_t size)
    {
        check_allocation();
        return regex_real_aligned_alloc(alignment, size);
    }
}
#endif

int main()
{
    using namespace fixed_containers;
    const std::string external_input{"prefix: abcabc 123 suffix"};
    const std::string external_pattern{"([[:alpha:]]+)\\1"};
    const std::string long_input(100000, 'a');
    allocations_forbidden = true;
    {
        using Regex = FixedRegex<128>;
        Regex regex{external_pattern};
        FixedMatchResults<std::string::const_iterator, 8> match{};
        assert_or_abort(regex_search(external_input, match, regex));
        assert_or_abort(match[0] == "abcabc");
        assert_or_abort(match.str(1) == "abc");
        assert_or_abort(match.format("$1/$&") == "abc/abcabc");
        auto copy = regex;
        auto moved = std::move(copy);
        regex.assign("(?!b)(?=(a+))a+", Regex::icase);
        assert_or_abort(regex_match("AAA", regex));
        regex = moved;
        swap(regex, moved);
        regex.imbue(regex.getloc());
        regex.assign("[[.a.]][[=b=]][c-e]", Regex::collate);
        assert_or_abort(regex_match("abc", regex));
        regex.assign("[[:digit:]]+", Regex::extended);
        assert_or_abort(regex_search(external_input, regex));
        regex.assign("(,)|(;+)");
        constexpr std::string_view INPUT = "a,b;;c";
        auto iterator = FixedRegexIterator{INPUT.begin(), INPUT.end(), regex};
        assert_or_abort(iterator != std::default_sentinel);
        assert_or_abort(iterator->str() == ",");
        ++iterator;
        assert_or_abort(iterator->str() == ";;");
        ++iterator;
        assert_or_abort(iterator == std::default_sentinel);
        auto tokens = FixedRegexTokenIterator{INPUT.begin(), INPUT.end(), regex, -1};
        assert_or_abort(tokens->str() == "a");
        ++tokens;
        auto tokens_copy = tokens;
        ++tokens;
        assert_or_abort(tokens_copy->str() == "b");
        assert_or_abort(tokens->str() == "c");
        assert_or_abort(regex_replace<32>(INPUT, regex, "-") == "a-b-c");
        FixedString<32> output{};
        regex_replace(std::back_inserter(output), INPUT.begin(), INPUT.end(), regex, "-");
        assert_or_abort(output == "a-b-c");
        regex.assign("a*");
        assert_or_abort(regex_replace<32>("aba", regex, "_") == "__b__");
        assert_or_abort(regex_match(long_input, FixedRegex<2, 1>{"a*"}));
        assert_or_abort(regex_search(long_input, FixedRegex<2, 1>{"a+", Regex::extended}));
        FixedSubMatch<const char*> matched{"a", "a", true};
        FixedSubMatch<const char*> unmatched{};
        matched.swap(unmatched);
        assert_or_abort(!matched.matched && unmatched.matched);
    }  // Destruction is included in the measured scope.
    allocations_forbidden = false;
}

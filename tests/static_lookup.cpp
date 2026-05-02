#include <cppwindow/utils.hpp>

#include <cassert>

namespace {

enum class Wrapper : unsigned int
{
    Unknown = 0,
    First = 1,
    A = First,
    B,
    Last = B
};

struct Traits
{
    using WrapperType = Wrapper;
    using BackendType = int;

    static constexpr Wrapper WrapperNone = Wrapper::Unknown;
    static constexpr Wrapper WrapperFirst = Wrapper::First;
    static constexpr Wrapper WrapperLast = Wrapper::Last;
    static constexpr int BackendNone = -1;
    static constexpr int BackendFirst = 10;
    static constexpr int BackendLast = 12;
};

constexpr cwin::StaticLookup<Traits> Lookup(
    {
        { Wrapper::A, 10 },
        { Wrapper::B, 12 },
    });

static_assert(Lookup.toBackend(Wrapper::A) == 10);
static_assert(Lookup.toBackend(Wrapper::B) == 12);
static_assert(Lookup.toBackend(Wrapper::Unknown) == Traits::BackendNone);
static_assert(Lookup.toBackend(static_cast<Wrapper>(99)) == Traits::BackendNone);

static_assert(Lookup.toWrapper(10) == Wrapper::A);
static_assert(Lookup.toWrapper(11) == Wrapper::Unknown);
static_assert(Lookup.toWrapper(12) == Wrapper::B);
static_assert(Lookup.toWrapper(-1) == Wrapper::Unknown);
static_assert(Lookup.toWrapper(99) == Wrapper::Unknown);

}  // namespace

int main()
{
    assert(Lookup.toBackend(Wrapper::A) == 10);
    assert(Lookup.toWrapper(12) == Wrapper::B);
}

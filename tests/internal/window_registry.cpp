#include "window_registry.hpp"

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace {

struct RegistryStorage
{
    int resetCount = 0;

    void reset() noexcept
    {
        ++resetCount;
    }
};

struct WrongResetReturn
{
    bool reset() noexcept
    {
        return true;
    }
};

struct ThrowingReset
{
    void reset() {}
};

struct MissingReset
{};

static_assert(cwin::Resettable<RegistryStorage>);
static_assert(!cwin::Resettable<WrongResetReturn>);
static_assert(!cwin::Resettable<ThrowingReset>);
static_assert(!cwin::Resettable<MissingReset>);

static_assert(!std::is_copy_constructible_v<cwin::WindowStorageRegistry<RegistryStorage>>);
static_assert(!std::is_copy_assignable_v<cwin::WindowStorageRegistry<RegistryStorage>>);
static_assert(!std::is_move_constructible_v<cwin::WindowStorageRegistry<RegistryStorage>>);
static_assert(!std::is_move_assignable_v<cwin::WindowStorageRegistry<RegistryStorage>>);
static_assert(noexcept(std::declval<cwin::WindowStorageRegistry<RegistryStorage>&>().resetAll()));

}  // namespace

int main()
{
    cwin::WindowStorageRegistry<RegistryStorage> registry;

    auto first = std::make_shared<RegistryStorage>();
    auto expired = std::make_shared<RegistryStorage>();
    auto second = std::make_shared<RegistryStorage>();

    registry.registerStorage(first);
    registry.registerStorage(expired);
    registry.registerStorage(second);
    expired.reset();

    assert(first->resetCount == 0);
    assert(second->resetCount == 0);

    registry.resetAll();
    assert(first->resetCount == 1);
    assert(second->resetCount == 1);

    auto third = std::make_shared<RegistryStorage>();
    registry.registerStorage(third);
    registry.resetAll();

    assert(first->resetCount == 2);
    assert(second->resetCount == 2);
    assert(third->resetCount == 1);
}

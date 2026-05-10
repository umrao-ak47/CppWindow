#include "window_registry.hpp"

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct RegistryStorage
{
    int resetCount = 0;
    int visitCount = 0;

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

struct GoodHandler
{
    void operator()(RegistryStorage&) const {}
};

struct BadHandler
{
    void operator()(int) const {}
};

template <typename Fn>
concept CanForEach = requires(cwin::WindowStorageRegistry<RegistryStorage>& registry, Fn fn) {
    registry.forEach(fn);
};

static_assert(cwin::Resettable<RegistryStorage>);
static_assert(!cwin::Resettable<WrongResetReturn>);
static_assert(!cwin::Resettable<ThrowingReset>);
static_assert(!cwin::Resettable<MissingReset>);

static_assert(CanForEach<GoodHandler>);
static_assert(!CanForEach<BadHandler>);

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

    std::vector<RegistryStorage*> visited;
    registry.forEach([&](RegistryStorage& storage) {
        ++storage.visitCount;
        visited.push_back(&storage);
    });

    assert(visited.size() == 2);
    assert(visited[0] == first.get());
    assert(visited[1] == second.get());
    assert(first->visitCount == 1);
    assert(second->visitCount == 1);
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

#pragma once

#include "mod/sort/SortPlanner.h"

#include "mc/world/item/ItemStack.h"

#include <map>
#include <utility>
#include <vector>

class CreativeItemRegistry;

namespace lamina_sort::game {

/// Turns real item stacks into planner input.
///
/// Mergeability is never decided here: two stacks share a planner group only
/// if vanilla's own `ItemStackBase::isStackable(other)` says the player could
/// stack them through the inventory UI. Ordering uses the client's Creative
/// inventory registry when available and stable item identity otherwise.
class StackClassifier {
public:
    /// `creativeRegistry` may be null; keys then fall back to identifier order.
    explicit StackClassifier(CreativeItemRegistry const* creativeRegistry) : mCreativeRegistry(creativeRegistry) {}

    /// Classifies `slots` (null stacks become empty slots). Every returned
    /// group id indexes `representatives()`.
    [[nodiscard]] std::vector<sort::SlotStack> classify(std::vector<ItemStack> const& slots);

    /// One stack per group, in group id order. Used to verify that a slot
    /// still holds the item kind the plan expects.
    [[nodiscard]] std::vector<ItemStack> const& representatives() const { return mRepresentatives; }

    /// Whether the last classify() could consult the Creative registry.
    [[nodiscard]] bool usedCreativeOrder() const { return mUsedCreativeOrder; }

private:
    int           groupOf(ItemStack const& stack);
    sort::SortKey keyOf(int group);
    int           creativeIndexOf(ItemStack const& stack);

    CreativeItemRegistry const*        mCreativeRegistry;
    std::vector<ItemStack>             mRepresentatives;
    std::map<int, sort::SortKey>       mKeyCache;           ///< group -> key
    std::map<std::pair<int, int>, int> mCreativeIndexCache; ///< (id, aux) -> index
    bool                               mUsedCreativeOrder{false};
};

} // namespace lamina_sort::game

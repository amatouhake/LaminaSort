#include "mod/game/StackClassifier.h"

#include "mc/deps/nbt/CompoundTag.h"
#include "mc/deps/shared_types/item/CreativeItemCategory.h"
#include "mc/world/item/ItemInstance.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/registry/CreativeGroupInfo.h"
#include "mc/world/item/registry/CreativeItemEntry.h"
#include "mc/world/item/registry/CreativeItemRegistry.h"

#include <climits>
#include <string>

namespace lamina_sort::game {

namespace {

// The Creative screen shows its categories as tabs in this fixed order, which
// differs from the enum order. Anything else (command-only, undefined) goes
// after the visible tabs.
int tabRank(SharedTypes::CreativeItemCategory category) {
    switch (category) {
    case SharedTypes::CreativeItemCategory::Construction:
        return 0;
    case SharedTypes::CreativeItemCategory::Equipment:
        return 1;
    case SharedTypes::CreativeItemCategory::Items:
        return 2;
    case SharedTypes::CreativeItemCategory::Nature:
        return 3;
    default:
        return 4;
    }
}

// Composes one comparable number out of (tab, group, entry) so that the
// Creative screen's visual order (tab by tab, group by group, item by item)
// is preserved. The limits are far above the registry's real sizes.
int creativeOrdinal(CreativeItemRegistry const& registry, CreativeItemEntry const& entry) {
    constexpr int kGroupSpan = 10000;   // entries per group
    constexpr int kTabSpan   = 1000000; // entries per tab
    auto const&   groups     = registry.mCreativeGroups.get();
    int           rank       = 4;
    if (entry.mGroupIndex < groups.size()) {
        rank = tabRank(groups[entry.mGroupIndex].mCategory);
    }
    return rank * kTabSpan + static_cast<int>(entry.mGroupIndex) * kGroupSpan + static_cast<int>(entry.mIndex);
}

} // namespace

std::vector<sort::SlotStack> StackClassifier::classify(std::vector<ItemStack> const& slots) {
    mRepresentatives.clear();
    mKeyCache.clear();
    mUsedCreativeOrder = mCreativeRegistry != nullptr;

    std::vector<sort::SlotStack> result;
    result.reserve(slots.size());
    for (auto const& stack : slots) {
        if (stack.isNull() || stack.mCount <= 0) {
            result.push_back(sort::SlotStack::emptySlot());
            continue;
        }
        sort::SlotStack s;
        s.count        = stack.mCount;
        s.maxStackSize = stack.getMaxStackSize();
        s.group        = groupOf(stack);
        // Every member of a group is, by vanilla's verdict, the same kind of
        // item; keying off the group's representative guarantees they share
        // one key even if their user data is encoded slightly differently
        // (e.g. an empty compound versus none).
        s.key = keyOf(s.group);
        result.push_back(std::move(s));
    }
    return result;
}

int StackClassifier::groupOf(ItemStack const& stack) {
    // isStackable(other) is the game's own verdict on whether two stacks may
    // be combined: same item, aux value, user data (enchantments, names,
    // damage, container contents, ...) and block, and both stackable at all.
    // Unstackable items therefore never join a group, not even with an
    // identical copy of themselves.
    for (size_t i = 0; i < mRepresentatives.size(); ++i) {
        if (stack.isStackable(mRepresentatives[i])) {
            return static_cast<int>(i);
        }
    }
    mRepresentatives.push_back(stack);
    return static_cast<int>(mRepresentatives.size() - 1);
}

sort::SortKey StackClassifier::keyOf(int group) {
    if (auto it = mKeyCache.find(group); it != mKeyCache.end()) return it->second;
    auto const&   stack = mRepresentatives[static_cast<size_t>(group)];
    sort::SortKey key;
    key.creativeIndex = creativeIndexOf(stack);
    key.typeName      = stack.getTypeName();
    key.aux           = stack.getAuxValue();
    key.damage        = stack.getDamageValue();
    // Stacks that vanilla keeps apart because of their user data still need a
    // deterministic relative order; the custom name reads well in logs and
    // the NBT hash separates everything else (enchantments, lore, contents).
    if (stack.mUserData && !stack.mUserData->mTags.empty()) {
        key.detail  = stack.getCustomName();
        key.detail += '|';
        key.detail += std::to_string(stack.mUserData->hash());
    }
    // Adventure-mode restrictions live outside the NBT compound but still
    // keep stacks apart in vanilla.
    if (stack.mCanPlaceOnHash != 0 || stack.mCanDestroyHash != 0) {
        key.detail += "|p" + std::to_string(stack.mCanPlaceOnHash) + "|d" + std::to_string(stack.mCanDestroyHash);
    }
    mKeyCache.emplace(group, key);
    return key;
}

int StackClassifier::creativeIndexOf(ItemStack const& stack) {
    if (!mCreativeRegistry) return INT_MAX;

    auto const id       = static_cast<int>(stack.getId());
    auto const aux      = static_cast<int>(stack.getAuxValue());
    auto const cacheKey = std::make_pair(id, aux);
    if (auto it = mCreativeIndexCache.find(cacheKey); it != mCreativeIndexCache.end()) {
        return it->second;
    }

    // Prefer the entry with the same item and aux value (potion variants and
    // the like), otherwise the first entry of the item; stacks that differ
    // only in user data then share a primary key and are ordered by `detail`.
    int idOnly = INT_MAX;
    int found  = INT_MAX;
    for (auto const& entry : mCreativeRegistry->mCreativeItems.get()) {
        auto const& item = entry.mItemInstance.get();
        if (item.getId() != id) continue;
        auto const ordinal = creativeOrdinal(*mCreativeRegistry, entry);
        if (item.getAuxValue() == aux) {
            found = ordinal;
            break;
        }
        if (idOnly == INT_MAX) idOnly = ordinal;
    }
    if (found == INT_MAX) found = idOnly;
    mCreativeIndexCache.emplace(cacheKey, found);
    return found;
}

} // namespace lamina_sort::game

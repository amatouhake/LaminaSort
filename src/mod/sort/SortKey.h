#pragma once

#include <climits>
#include <string>
#include <vector>

// Pure ordering vocabulary. No Minecraft or LeviLamina dependency: the game
// integration fills these structures in, the planner only compares them.
namespace lamina_sort::sort {

/// Top-level regions of a sorted inventory, in the order they appear. This
/// is the only classification LaminaSort maintains itself; everything inside
/// a section follows the game's own Creative registry order.
enum class Section : int {
    ShulkerBox   = 0, ///< Every Shulker Box, filled ones first.
    Equipment    = 1, ///< Tools, weapons, armour, bows, shields, elytra, ...
    Items        = 2, ///< Materials, food, potions, utility and misc items.
    Construction = 3, ///< Building blocks.
    Nature       = 4, ///< Natural blocks.
    Unknown      = 5, ///< Not in the Creative registry (add-on / custom).
};

/// One enchantment as stored on an item: registry id and level.
struct Enchantment {
    int id{0};
    int level{0};

    friend bool operator==(Enchantment const& a, Enchantment const& b) { return a.id == b.id && a.level == b.level; }
};

/// Ordering key for one kind of item. Compared field by field, in
/// declaration order, by `compareKeys`.
struct SortKey {
    Section section{Section::Unknown};
    /// Position inside the section: the Creative registry ordinal (group,
    /// then entry) for ordinary items; for Shulker Boxes 0 = filled, 1 =
    /// empty. INT_MAX when unknown.
    int creativeIndex{INT_MAX};
    /// Namespaced item identifier ("minecraft:cobblestone"). Shulker Boxes
    /// use the generic "shulker_box" here and keep their colour in `tail`, so
    /// that name and contents are compared before colour.
    std::string typeName;
    int         aux{0};
    /// 0 when the item carries a custom name (named items lead), else 1.
    int nameRank{1};
    /// Normalised custom name (see normalizeName), empty when unnamed.
    std::string name;
    /// 0 when enchanted (enchanted variants lead), else 1.
    int variantRank{1};
    /// Enchantments in canonical order (see canonicalEnchantments).
    std::vector<Enchantment> enchantments;
    /// Shulker Box content signature (see ContentSignature); empty otherwise.
    std::string contents;
    /// Damage value: lower (better condition) first.
    int damage{0};
    /// Late tie-breaker with a human meaning, e.g. a Shulker Box's colour id.
    std::string tail;
    /// Stable last-resort text (user data hash, restriction hashes) so that
    /// stacks LaminaSort cannot tell apart semantically still order
    /// deterministically. Never the primary rule for common cases.
    std::string detail;
};

/// Lexicographic comparison in field order; enchantment lists compare
/// entry by entry (id ascending, level descending) and a list that extends
/// another sorts first. Returns <0, 0 or >0.
[[nodiscard]] int compareKeys(SortKey const& a, SortKey const& b);

inline bool operator<(SortKey const& a, SortKey const& b) { return compareKeys(a, b) < 0; }
inline bool operator==(SortKey const& a, SortKey const& b) { return compareKeys(a, b) == 0; }

/// Stable human-readable form of a custom name: formatting codes ("§x")
/// removed, ASCII letters lower-cased, surrounding whitespace trimmed.
[[nodiscard]] std::string normalizeName(std::string const& name);

/// Canonical enchantment order: by id ascending, then level descending, with
/// exact duplicates removed. The id order is the game's enchantment registry
/// order, which is also how it lists them.
[[nodiscard]] std::vector<Enchantment> canonicalEnchantments(std::vector<Enchantment> list);

/// One kind of item found inside a container item, for content signatures.
struct ContentEntry {
    Section     section{Section::Unknown};
    int         creativeIndex{INT_MAX};
    std::string typeName;
    int         aux{0};
    int         count{0};
};

/// Builds a deterministic signature of a container's contents that does not
/// depend on which internal slots the items occupy: entries of the same kind
/// are merged (counts summed), the kinds are sorted the same way the
/// inventory itself would be, and the result is serialised so that plain
/// string comparison orders signatures sensibly (first kind, then its total,
/// then the next kind, ...). Empty when there are no items.
[[nodiscard]] std::string contentSignature(std::vector<ContentEntry> entries);

/// Short label for logs, e.g. "S", "E", "I", "C", "N", "?".
[[nodiscard]] char sectionLabel(Section section);

} // namespace lamina_sort::sort

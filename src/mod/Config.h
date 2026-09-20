#pragma once

namespace lamina_sort {

/// Persistent settings, stored as JSON in the mod's config directory.
/// Bump `version` when the layout changes incompatibly.
struct Config {
    int version = 1;

    /// Default key code for the Sort action (Windows virtual-key code; 0x52 is
    /// 'R'). The binding is registered with the game's keyboard settings and
    /// can be remapped there; this only seeds the default.
    int sortKeyCode = 0x52;

    /// Allow sorting an ordinary storage container (chest, barrel, shulker
    /// box, ...) when the pointer hovers one of its slots. The player's main
    /// inventory is always sortable.
    bool sortContainers = true;
};

} // namespace lamina_sort

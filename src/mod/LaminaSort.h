#pragma once

#include "ll/api/mod/NativeMod.h"

namespace lamina_sort {

class LaminaSort {

public:
    static LaminaSort& getInstance();

    LaminaSort() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace lamina_sort

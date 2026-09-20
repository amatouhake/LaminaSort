#pragma once

#include "ll/api/mod/NativeMod.h"

#include "mod/Config.h"

class IClientInstance;

namespace lamina_sort {

class LaminaSort {

public:
    static LaminaSort& getInstance();

    LaminaSort() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] Config const& getConfig() const { return mConfig; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

private:
    /// Explicit Sort trigger (key press). Sorts the region selected for the
    /// currently open container screen, if any.
    void onSortRequested(IClientInstance& client);

    ll::mod::NativeMod& mSelf;
    Config              mConfig;
};

} // namespace lamina_sort

#pragma once

#include "ll/api/event/ListenerBase.h"

#include <memory>

class ContainerScreenController;
class ScreenController;

namespace ll::event::inline render {
class AfterUIRenderEvent;
}

namespace lamina_sort::game {

/// Remembers which container screen (survival inventory, chest, ...) is
/// currently shown, so an explicit Sort trigger can act on it.
///
/// The controller is observed from the game's own UI render callback and held
/// through a weak pointer to the ScreenView's shared controller, so a screen
/// that has been torn down can never be dereferenced. Leaving the screen also
/// drops it eagerly via the `onLeave` hook.
///
/// All members run on the client's main (render/input) thread.
class ScreenTracker {
public:
    static ScreenTracker& getInstance();

    void install();
    void uninstall();

    /// The active container screen controller, or null when no container
    /// screen is open (or it has already been destroyed).
    [[nodiscard]] std::shared_ptr<ContainerScreenController> current() const;

    /// Called from the onLeave hook.
    void onControllerLeft(ContainerScreenController& controller);

private:
    void onAfterUIRender(ll::event::AfterUIRenderEvent& event);

    std::weak_ptr<ScreenController> mCurrent;
    ll::event::ListenerPtr          mRenderListener;
    bool                            mInstalled{false};
};

} // namespace lamina_sort::game

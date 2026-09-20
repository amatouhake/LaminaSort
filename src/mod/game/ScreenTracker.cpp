#include "mod/game/ScreenTracker.h"

#include "mod/game/TextInputTracker.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/render/UIRenderEvent.h"
#include "ll/api/memory/Hook.h"

#include "mc/client/gui/screens/ScreenController.h"
#include "mc/client/gui/screens/ScreenView.h"
#include "mc/client/gui/screens/controllers/ContainerScreenController.h"

namespace lamina_sort::game {

namespace {

// Closing a screen does not always render a final frame we could observe, so
// forget the controller as soon as the game tells it to leave. The base
// implementation is reached by every container screen subclass.
LL_TYPE_INSTANCE_HOOK(
    ContainerScreenLeaveHook,
    ll::memory::HookPriority::Normal,
    ContainerScreenController,
    &ContainerScreenController::$onLeave,
    void
) {
    ScreenTracker::getInstance().onControllerLeft(*this);
    origin();
}

using Hooks = ll::memory::HookRegistrar<ContainerScreenLeaveHook>;

} // namespace

ScreenTracker& ScreenTracker::getInstance() {
    static ScreenTracker instance;
    return instance;
}

void ScreenTracker::install() {
    if (mInstalled) return;
    Hooks::hook();
    mRenderListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::AfterUIRenderEvent>(
        [this](ll::event::AfterUIRenderEvent& event) { onAfterUIRender(event); }
    );
    mInstalled = true;
}

void ScreenTracker::uninstall() {
    if (!mInstalled) return;
    if (mRenderListener) {
        ll::event::EventBus::getInstance().removeListener(mRenderListener);
        mRenderListener.reset();
    }
    Hooks::unhook();
    mCurrent.reset();
    mCurrentView = nullptr;
    mInstalled   = false;
}

std::shared_ptr<ContainerScreenController> ScreenTracker::current() const {
    auto controller = mCurrent.lock();
    if (!controller) return nullptr;
    // Only controllers that reported _isContainerScreen() are ever stored.
    return std::static_pointer_cast<ContainerScreenController>(controller);
}

void ScreenTracker::onControllerLeft(ContainerScreenController& controller) {
    auto current = mCurrent.lock();
    if (current && current.get() == static_cast<ScreenController*>(&controller)) {
        mCurrent.reset();
        TextInputTracker::getInstance().forget(mCurrentView);
        mCurrentView = nullptr;
    }
}

void ScreenTracker::onAfterUIRender(ll::event::AfterUIRenderEvent& event) {
    // Every ScreenView on the stack renders each frame; keep the most recent
    // container screen. The HUD and other overlays are not container screens.
    auto const& controller = event.screenView().mController;
    if (!controller || !controller->_isContainerScreen()) {
        return;
    }
    if (mCurrent.lock() != controller) {
        mCurrent     = controller;
        mCurrentView = &event.screenView();
        // A freshly shown screen starts with no text box selected; drop any
        // stale knowledge a previous screen at the same address left behind.
        TextInputTracker::getInstance().forget(mCurrentView);
    }
}

} // namespace lamina_sort::game

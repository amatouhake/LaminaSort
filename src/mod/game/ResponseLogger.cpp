#include "mod/game/ResponseLogger.h"

#include "mod/LaminaSort.h"

#include "ll/api/memory/Hook.h"

#include "mc/client/network/ClientNetworkHandler.h"
#include "mc/network/packet/ItemStackResponseContainerInfo.h"
#include "mc/network/packet/ItemStackResponseInfo.h"
#include "mc/network/packet/ItemStackResponsePacket.h"
#include "mc/network/packet/ItemStackResponseSlotInfo.h"
#include "mc/world/inventory/network/ItemStackNetResult.h"

namespace lamina_sort::game {

namespace {

// Observes the server's verdict on each item stack request before the game
// applies it. Every sort step is such a request; a rejection means vanilla
// discards the client's prediction for that step and restores the server's
// slot contents.
LL_TYPE_INSTANCE_HOOK(
    ItemStackResponseHook,
    ll::memory::HookPriority::Normal,
    ClientNetworkHandler,
    &ClientNetworkHandler::$handle,
    void,
    ::NetworkIdentifier const&       source,
    ::ItemStackResponsePacket const& packet
) {
    auto&  logger   = LaminaSort::getInstance().getSelf().getLogger();
    size_t accepted = 0;
    for (auto const& response : packet.mResponses.get()) {
        if (response.mResult == ItemStackNetResult::Success) {
            ++accepted;
            continue;
        }
        logger.warn(
            "Server rejected item stack request #{} (result {}); the predicted slots are reverted by vanilla",
            response.mClientRequestId->mRawId,
            static_cast<int>(response.mResult)
        );
    }
    if (accepted > 0) {
        logger.debug("Server accepted {} item stack request(s)", accepted);
    }
    origin(source, packet);
}

using Hooks = ll::memory::HookRegistrar<ItemStackResponseHook>;

bool gInstalled = false;

} // namespace

void ResponseLogger::install() {
    if (gInstalled) return;
    Hooks::hook();
    gInstalled = true;
}

void ResponseLogger::uninstall() {
    if (!gInstalled) return;
    Hooks::unhook();
    gInstalled = false;
}

} // namespace lamina_sort::game

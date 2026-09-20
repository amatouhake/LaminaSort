#include "mod/LaminaSort.h"

#include "mod/game/ResponseLogger.h"
#include "mod/game/ScreenTracker.h"
#include "mod/game/SortSession.h"

#include "ll/api/Config.h"
#include "ll/api/input/KeyHandle.h"
#include "ll/api/input/KeyRegistry.h"
#include "ll/api/io/FileSink.h"
#include "ll/api/io/LogLevel.h"
#include "ll/api/io/PatternFormatter.h"
#include "ll/api/mod/RegisterHelper.h"

#include "mc/client/game/IClientInstance.h"
#include "mc/client/multiplayer/ClientLevel.h"
#include "mc/world/item/registry/CreativeItemRegistry.h"
#include "mc/world/item/registry/ItemRegistryRef.h"

namespace lamina_sort {

namespace {
constexpr std::string_view kSortKeyName = "sort";
} // namespace

LaminaSort& LaminaSort::getInstance() {
    static LaminaSort instance;
    return instance;
}

bool LaminaSort::load() {
#ifdef LAMINASORT_TRACE
    // Trace builds mirror every line, flushed immediately, into the mod
    // directory so the diagnostics can be followed while the game runs
    // (LeviLamina's shared log file is only flushed at exit).
    getSelf().getLogger().setLevel(ll::io::LogLevel::Debug);
    auto sink = std::make_shared<ll::io::FileSink>(
        getSelf().getModDir() / "trace.log",
        ll::makePolymorphic<ll::io::PatternFormatter>("[{3:.3%F %T.} {2}][{1}] {0}", false),
        std::ios::app
    );
    sink->setFlushLevel(ll::io::LogLevel::Debug);
    getSelf().getLogger().addSink(std::move(sink));
    getSelf().getLogger().info("Trace build: debug logging enabled, mirrored to trace.log");
#endif
    getSelf().getLogger().debug("Loading...");

    // Missing file -> written with defaults; unknown/old version -> merged
    // with defaults and rewritten, so the file always reflects the schema.
    auto const configPath = getSelf().getConfigDir() / "config.json";
    try {
        if (!ll::config::loadConfig(mConfig, configPath)) {
            ll::config::saveConfig(mConfig, configPath);
        }
    } catch (std::exception const& e) {
        getSelf().getLogger().error("Failed to load {}: {}; using defaults", configPath.string(), e.what());
        mConfig = Config{};
    }
    getSelf().getLogger().debug(
        "Config: sortKeyCode=0x{:X} sortContainers={}",
        mConfig.sortKeyCode,
        mConfig.sortContainers
    );

    // The key is registered with the game's own keyboard settings (remappable
    // there as "LaminaSort.sort"). Registration must happen at load time so
    // the binding exists before the input mappings are built.
    auto& key = ll::input::KeyRegistry::getInstance().getOrCreateKey(kSortKeyName, {mConfig.sortKeyCode});
    key.registerButtonDownHandler([this](::FocusImpact, ::IClientInstance& client) { onSortRequested(client); });
    return true;
}

bool LaminaSort::enable() {
    getSelf().getLogger().debug("Enabling...");
    game::ScreenTracker::getInstance().install();
    game::ResponseLogger::install();
    return true;
}

bool LaminaSort::disable() {
    getSelf().getLogger().debug("Disabling...");
    game::ResponseLogger::uninstall();
    game::ScreenTracker::getInstance().uninstall();
    return true;
}

void LaminaSort::onSortRequested(IClientInstance& client) {
    auto& logger = getSelf().getLogger();

    auto controller = game::ScreenTracker::getInstance().current();
    if (!controller) {
        logger.debug("Sort key pressed with no container screen open; ignored");
        return;
    }
    auto const region = game::SortSession::selectRegion(*controller, mConfig);
    if (!region) {
        logger.info(
            "Sort key pressed but this screen has no sortable region; ignored ({})",
            game::SortSession::describeScreen(*controller)
        );
        return;
    }

    // Creative order comes from the client's own registry, filled from the
    // server's creative content. It may be absent (e.g. not yet received).
    CreativeItemRegistry const* creativeRegistry = nullptr;
    if (auto* level = client.getLevel()) {
        creativeRegistry = level->getItemRegistry().getCreativeItemRegistry().get();
        if (creativeRegistry && creativeRegistry->mCreativeItems->empty()) {
            creativeRegistry = nullptr;
        }
    }

    logger.debug("Screen: {}", game::SortSession::describeScreen(*controller));
    game::SortSession::run(*controller, *region, creativeRegistry, logger);
}

} // namespace lamina_sort

LL_REGISTER_MOD(lamina_sort::LaminaSort, lamina_sort::LaminaSort::getInstance());

#include "mod/LaminaSort.h"

#include "ll/api/mod/RegisterHelper.h"

namespace lamina_sort {

LaminaSort& LaminaSort::getInstance() {
    static LaminaSort instance;
    return instance;
}

bool LaminaSort::load() {
    getSelf().getLogger().debug("Loading...");
    return true;
}

bool LaminaSort::enable() {
    getSelf().getLogger().debug("Enabling...");
    return true;
}

bool LaminaSort::disable() {
    getSelf().getLogger().debug("Disabling...");
    return true;
}

} // namespace lamina_sort

LL_REGISTER_MOD(lamina_sort::LaminaSort, lamina_sort::LaminaSort::getInstance());

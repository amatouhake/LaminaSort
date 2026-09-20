# LaminaSort

A client-side inventory sorting mod for Minecraft Bedrock Edition, built on
[LeviLamina Client](https://github.com/LiteLDev/LeviLamina).

LaminaSort automates the ordinary inventory organisation a player performs by
hand: it merges compatible partial stacks and arranges the result into
sensible sections (Shulker Boxes, gear, items, blocks) that follow the game's
own Creative order, using the same client inventory machinery that the mouse
does. It is not an inventory editor, needs no server component and never
touches server-authoritative state directly.

## Target

* LeviLamina **v26.51.3** (client)
* Minecraft Bedrock Edition **1.26.51.x** (Windows x64)

Requires only the Minecraft client with LeviLamina: no server mod, operator
permissions, cheats, commands or experiments.

## Current scope (MVP)

* Press the **Sort** key (default `R`, remappable under *Settings → Keyboard &
  Mouse* as `LaminaSort.sort`) while an inventory or container screen is open.
* The **main 27 player inventory slots** are sorted. The hotbar, armour,
  offhand, crafting and every other special slot are never touched.
* When an ordinary storage container (chest, trapped chest, ender chest,
  barrel, shulker box, chest minecart/boat) is open and the pointer hovers one
  of **its** slots, the container is sorted instead. Furnaces, brewing stands,
  crafting, smithing, enchanting and other interfaces with role-specific slots
  are never sorted.
* Compatible partial stacks are consolidated first (`42 + 10 → 52`,
  `42 + 30 → 64 + 8`, `42 + 10 + 25 → 64 + 13`), respecting each item's own
  max stack size (16 for eggs, 1 for tools). Existing stacks are topped up
  before a new partial stack is left over.
* Sorting an already sorted region is a no-op; repeated presses are stable.
* Vanilla item locks are respected: a stack locked **in its slot**
  (`minecraft:item_lock` = `lock_in_slot`) stays exactly where it is and the
  rest of the region is sorted around it; nothing is ever merged into or out
  of it. A stack locked **in the inventory** (`lock_in_inventory`) may still
  change slots inside the player inventory, as vanilla allows; inside a
  container it is left alone as well.
* The Sort key is ignored while a text box is being edited (Creative
  search, anvil name, ...), so typing an `r` never triggers a sort. The key
  also does nothing while an item is held on the cursor.

Sorting is explicit only. Nothing happens in the background, and there is
nothing to configure: the default order is the product.

## What the result looks like

One press turns a mixed chest into these sections, in this order:

1. **Shulker Boxes** – every box, whatever its colour, comes first. Boxes with
   something inside come before empty ones. Named boxes lead, then boxes with
   the same contents sit next to each other regardless of how the items are
   arranged inside them, then colour. "Same contents" is judged the way the
   inventory itself is ordered: an inner item's custom name, enchantments and
   remaining durability count, so a box holding an *Efficiency V* pickaxe is
   not the same as one holding a plain or a worn pickaxe. Only the box's own
   items are inspected (vanilla never nests container items).
2. **Gear** – tools, weapons, armour, bows, shields, elytra and the like, in
   the game's own Creative order.
3. **Items** – food, materials, potions, utility and miscellaneous items, in
   the game's own Creative order.
4. **Blocks** – building blocks, then natural blocks, in Creative order.
5. Anything the game does not list in its Creative inventory (add-on or
   custom items) goes last, in a stable identifier order.
6. Empty slots.

Variants of one item are kept together and ordered the same way every time:
custom-named ones first (by name), then enchanted ones (grouped by
enchantment, higher level first), then plain ones; among damageable items the
one in better condition comes first. Stacks of one kind are placed biggest
first, so a merged run reads `64, 64, 12`.

## How it works

Three concerns are kept apart:

* `src/mod/sort/` is the pure planner and ordering vocabulary. It works on
  synthetic slot states (count, max stack size, an opaque *mergeability
  group* and an ordering key) and produces a list of *Move* / *Swap*
  operations plus the expected final layout. It has no Minecraft dependency
  and is covered by unit tests.
* `src/mod/game/StackClassifier` turns real item stacks into planner input.
  **Mergeability is decided by vanilla**: two stacks share a group only if
  `ItemStackBase::isStackable(other)` — the check the game itself uses when
  stacks are combined in the UI — says so. Damage, enchantments, names, lore,
  components, container contents, `can_place_on` restrictions and every other
  vanilla-relevant difference therefore keep stacks apart, exactly as the
  inventory screen would. LaminaSort never defines its own equality.
  The ordering key is a thin semantic layer over the game's own data: the
  section comes from the Creative category of the item's registry entry
  (Shulker Boxes and food are the only overrides), the position inside a
  section from the Creative group and entry, and variant order from the
  item's custom name, enchantment list (registry id order, level descending),
  damage value and, for Shulker Boxes, a signature of the contents. That
  signature is built from the inner stacks' own ordering keys (name,
  enchantments, damage, ... but not nested contents), merged per kind and
  sorted, so it does not depend on the internal slot layout. A Shulker Box is
  recognised by the item's class (`ShulkerBoxBlockItem`), not by its name.
  Item locks are read with the game's `ItemLockHelper`. A hash of the
  remaining item data is used only as a last-resort tie-breaker.
* `src/mod/game/SortSession` executes the plan through the screen's own
  `ContainerManagerController`: a Move is `handlePlaceAmount` (the transfer
  behind shift-click / drag placement) and a Swap is `handleSwap` (the
  transfer behind the hotbar hotkey swap). Both go through the vanilla
  `ItemStackRequest` path: the client predicts the change immediately and
  the server validates the request afterwards. Before and after every step
  the touched slots are compared with the planner's simulation of the
  client-side prediction; any disagreement, a refused transfer or an item
  held on the cursor aborts the remaining steps.

  The server's verdict arrives asynchronously, after the whole sequence of
  steps has already been issued, and is **not** fed back into the sort: if
  the server rejects a request, vanilla itself discards the client's
  prediction for that step and restores the server's slot contents, exactly
  as it does for a rejected manual drag. LaminaSort only observes the
  `ItemStackResponse` and logs a warning for each rejection (visible in the
  log; a `--trace=y` build shows the accepted count too). It never retries,
  re-plans or "fixes up" a rejected step. Since every step is an ordinary
  validated transfer, the inventory is consistent whichever steps the server
  accepted.

## Configuration

On first start LaminaSort writes `mods/LaminaSort/config/config.json`:

```json
{
    "version": 1,
    "sortKeyCode": 82,
    "sortContainers": true
}
```

* `sortKeyCode` – default Windows virtual-key code for the Sort key (`82` is
  `R`). It only seeds the game's keyboard settings; remap it in game.
* `sortContainers` – allow sorting an opened storage container when its slots
  are hovered. When `false`, only the player inventory is ever sorted.

There are no sorting profiles or ordering options; the default order described
above is the only one. The file is read once at mod load; restart the game
after editing it.

## Installation

Download `LaminaSort-client-windows-x64.zip` from the
[GitHub Releases](https://github.com/amatouhake/LaminaSort/releases) page and
copy the `LaminaSort/` directory it contains (`LaminaSort.dll` +
`manifest.json`) into the `mods/` directory of a LeviLamina client
installation (for a LeviLauncher instance:
`%APPDATA%\levilauncher.exe\versions\<version>\mods\LaminaSort\`).

The repository also ships a `tooth.json`, so the release can be installed as
the LIP package `github.com/amatouhake/LaminaSort` where LIP / LeviLauncher
package installation is available.

LaminaSort is early (`0.x`) software: it is usable, but its behaviour and
configuration may still change between releases.

## Building

Requirements: [xmake](https://xmake.io), Visual Studio 2022 build tools, and a
clang-cl toolchain (LLVM).

```shell
xmake f -y -p windows -a x64 -m release --target_type=client
xmake
```

The packaged mod (`LaminaSort.dll` + `manifest.json`) is written to
`bin/LaminaSort/`; install that folder as described under
[Installation](#installation). The `version` in the generated `manifest.json`
is derived from the nearest `vMAJOR.MINOR.PATCH` Git tag (`0.0.0` when there
is none).

Unit tests for the game-independent planner:

```shell
xmake build LaminaSortTests
xmake run LaminaSortTests
```

For runtime diagnostics, configure with `--trace=y`; the mod then logs the
selected region, the planned layout, every operation, the server's responses
and text-box focus changes at debug level and mirrors them, flushed
immediately, to `mods/LaminaSort/trace.log` (appended across runs).

Building against the LeviLamina SDK needs an LLVM that the SDK package can be
compiled with; the 26.51.x SDK was verified with LLVM 22.1.8 (`clang-cl`).
If the `levilamina` package fails to compile with the Visual Studio-bundled
LLVM, put a matching LLVM `bin` directory first on `PATH` for `xmake f`.

## Not in scope

Move matching / move all, LaminaSort's own slot locking (vanilla item locks
are honoured, see above), auto-restock, background sorting, inventory search
or highlighting, custom sort profiles and any server-side component are
deliberately left out.

## License

[MIT](LICENSE) © amatouhake

Bootstrapped from the CC0-1.0 licensed
[levilamina-mod-template](https://github.com/LiteLDev/levilamina-mod-template)

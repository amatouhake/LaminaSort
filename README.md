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

* LeviLamina **v26.51.1** (client)
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

Sorting is explicit only. Nothing happens in the background, and there is
nothing to configure: the default order is the product.

## What the result looks like

One press turns a mixed chest into these sections, in this order:

1. **Shulker Boxes** – every box, whatever its colour, comes first. Boxes with
   something inside come before empty ones. Named boxes lead, then boxes with
   the same kind of contents sit next to each other regardless of how the
   items are arranged inside them, then colour.
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
  damage value and, for Shulker Boxes, a signature of the contents. A hash of
  the remaining item data is used only as a last-resort tie-breaker.
* `src/mod/game/SortSession` executes the plan through the screen's own
  `ContainerManagerController`: a Move is `handlePlaceAmount` (the transfer
  behind shift-click / drag placement) and a Swap is `handleSwap` (the
  transfer behind the hotbar hotkey swap). Both go through the vanilla
  `ItemStackRequest` path: the client predicts the change, the server
  validates it and rejects anything invalid, and vanilla then restores the
  server's slot contents. Before and after every step the touched slots are
  compared with the planner's simulation; any disagreement, refusal or held
  cursor item aborts the remaining steps. Steps already issued are ordinary
  validated transfers, so a partial sort is always a valid inventory.

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

## Building

Requirements: [xmake](https://xmake.io), Visual Studio 2022 build tools, and a
clang-cl toolchain (LLVM).

```shell
xmake f -y -p windows -a x64 -m release --target_type=client
xmake
```

The packaged mod (`LaminaSort.dll` + `manifest.json`) is written to
`bin/LaminaSort/`. Copy that folder into the `mods/` directory of a LeviLamina
client installation (for a LeviLauncher instance:
`%APPDATA%\levilauncher.exe\versions\<version>\mods\LaminaSort\`).

Unit tests for the game-independent planner:

```shell
xmake build LaminaSortTests
xmake run LaminaSortTests
```

For runtime diagnostics, configure with `--trace=y`; the mod then logs the
selected region, the planned layout, every operation and the server's
responses at debug level and mirrors them, flushed immediately, to
`mods/LaminaSort/trace.log`.

## Not in scope

Move matching / move all, slot locking, auto-restock, background sorting,
inventory search or highlighting, custom sort profiles and any server-side
component are deliberately left out.

## License

[MIT](LICENSE) © amatouhake

Bootstrapped from the CC0-1.0 licensed
[levilamina-mod-template](https://github.com/LiteLDev/levilamina-mod-template)

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Sort key (default `R`, remappable in the game's keyboard settings) that
  sorts the main 27 player inventory slots from any inventory or container
  screen. Hotbar, armour, offhand and crafting slots are never touched.
- Sorting an opened ordinary storage container (chest, barrel, shulker box,
  ...) when one of its slots is hovered; interfaces with role-specific slots
  are never sorted.
- Consolidation of partial stacks that vanilla allows to stack
  (`42 + 10 -> 52`, `42 + 30 -> 64 + 8`), decided by the game's own
  `ItemStackBase::isStackable` check.
- Zero-config default order: Shulker Boxes (filled before empty, named
  first, then by contents, then colour), then gear, items, building blocks,
  natural blocks, unknown items, empty slots. Inside a section the client's
  Creative registry order is kept; food always counts as an item.
- Shulker Box content signature built from the inner stacks' full ordering
  keys (custom name, enchantments, damage, ...) merged per kind and sorted,
  independent of the internal slot layout and bounded to one level.
  Shulker Boxes are recognised by item class (`ShulkerBoxBlockItem`).
- Vanilla item locks: `lock_in_slot` stacks stay in place and are never
  merged into or out of; the rest of the region is sorted around them.
  `lock_in_inventory` stacks may still move inside the player inventory and
  are left alone inside containers.
- The Sort key is ignored while a text box (Creative search, anvil name,
  ...) is being edited, so typing the letter never triggers a sort.
- Variants of one item are ordered by custom name, then enchantments
  (registry order, higher level first), then damage (better condition first);
  the NBT hash is only a last-resort tie-breaker.
- Execution through the vanilla container transfer path
  (`ContainerManagerController::handlePlaceAmount` / `handleSwap`), verified
  step by step against the planned client-side state; any disagreement
  aborts the sort. Server rejections arrive asynchronously, are reverted by
  vanilla and are only logged, never fed back into the sort.
- Configuration file (`sortKeyCode`, `sortContainers`) and a `--trace=y`
  diagnostic build.
- Unit tests for the pure sort planner (`LaminaSortTests`).

### Changed

- Target LeviLamina Client 26.51.3 (built against its SDK; `--trace=y` uses
  the `RotatePolicy`-based `ll::io::FileSink` introduced in 26.51.2).

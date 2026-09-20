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
- Creative inventory ordering taken from the client's creative item registry,
  with identifier order as fallback.
- Execution through the vanilla container transfer path
  (`ContainerManagerController::handlePlaceAmount` / `handleSwap`), verified
  step by step against the planned state; any disagreement aborts the sort.
- Configuration file (`sortKeyCode`, `sortContainers`) and a `--trace=y`
  diagnostic build.
- Unit tests for the pure sort planner (`LaminaSortTests`).

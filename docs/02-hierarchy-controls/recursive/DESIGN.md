# Recursive Expansion — Design

**Latch v2.0.3 · User guide**

## Shift-click expands or collapses a branch

On an **unlatched** parent triangle, hold **Shift** and click to recursively expand or collapse unlocked descendants. This works through the normal Outliner hierarchy rather than moving any Actors.

A **red** descendant retains its own latched-open or latched-closed instruction even when a higher branch is recursively changed.

## Example

Create one root folder with several child folders. Latch one child open and another closed. Shift-click the unlocked root expander. Latch should recursively adjust the unlocked descendants while leaving those two red child latches intact.

## Mixed View interaction

If a selected or pinned descendant remains under a newly collapsed branch, a **yellow** minimum exposure path may immediately return. That is expected: logical collapse and required presentation are different decisions.

## Common mistakes

Normal click is not recursive. Alt-click toggles a latch rather than expanding every child. Shift-clicking directly on a latched parent does not erase its saved latch. Use a Latch menu operation or Alt-click to remove protection intentionally.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.

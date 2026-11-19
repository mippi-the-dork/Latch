# Viewport Context Menu — Design

**Latch v2.0.3 · User guide**

## Use Latch without returning to the Outliner

When Actors are selected in the Level Editor viewport, right-click to access their Latch actions from the viewport context menu. This is useful when the object you need to pin or protect was just selected in a crowded environment.

## Example

Select an important gameplay Actor in the viewport. Open the right-click menu and use the appropriate **Latch → Keep Visible** operation. Its Outliner row can now stay exposed through collapsed ancestor folders. A parent whose expansion is locked can also be managed through the applicable Actor-oriented commands.

## What is different?

Viewport commands operate on **selected Actors**, not arbitrary folders. Folders exist as Outliner organizational items and do not have an independent viewport representation. For a folder's latch, use the World Outliner.

## Avoid confusing selection and visibility

Selecting an Actor in the viewport can itself temporarily cause a **yellow Mixed View** path. That is not equivalent to pinning the Actor. To keep it reachable after deselection, explicitly enable Keep Visible.

The viewport menu changes the Outliner presentation only; it does not reparent or move the selected Actor.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.

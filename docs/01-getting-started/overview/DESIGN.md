# What Latch Does — Design

**Latch v2.0.3 · User guide**

## The problem Latch solves

In a large Unreal level, opening one Actor from the viewport or filtering the Outliner can disturb the carefully arranged tree you were using to work. Latch records your intended expansion state separately from temporary exposure and selection.

**Red triangles** mean a parent is latched open or closed. **Yellow triangles** mean a parent is logically collapsed while Latch exposes the narrow path to a required descendant. **White pins** mark Actors or Folders that should remain reachable through collapsed ancestors.

## Example: two parts of a level

Collapse a finished area, then Alt-click its triangle to hold it closed. Expand an active folder and Alt-click to hold it open. Pin the gameplay Actor you constantly reference. Latch can expose the route to that pinned item without fully expanding every sibling.

## Limits

Latch changes World Outliner presentation, not Actor visibility, transforms, attachments, folder membership, ordering or gameplay. There is no Latch Actor to place and no level serialization of Latch state. Its settings are editor workspace state rather than shared level assets.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.

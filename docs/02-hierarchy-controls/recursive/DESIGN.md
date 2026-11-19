# Recursive Expansion — Design

**Latch v2.0.3 · User guide**

## Shift-click expands or collapses a branch

<!-- Media target: GIF | recursive-protected | docs/assets/media/recursive-protected.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="recursive-protected">
  <img src="../../assets/media-placeholder.png" alt="Animation of recursive Shift-click respecting red latched child folders" loading="lazy">
  <figcaption>Recursive expansion changes unlocked descendants but preserves the intended expansion of red latched children.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a parent folder with at least three nested child folders, one red-open and one red-closed. Shift-click the unlocked parent to collapse and expand; show the red children retaining their protected logical states.</span></figcaption>
</figure>


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
